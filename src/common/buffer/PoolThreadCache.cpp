#include "PoolThreadCache.hpp"
#include <ObjectQueue.hpp>
#include <boost/limits.hpp>
#include "PoolArena.hpp"

namespace comm{ namespace buffer{

namespace detail {
	
	class MemoryRegionCache {
	public:
		class Entry {
		public:
			Entry(PoolChunk* chunk, long handle) : chunk_(chunk), handle_(handle) {
			}

			Entry() : chunk_(0), handle_(0) { }

		public:
			PoolChunk* chunk_;
			long handle_;
		};

	public:
		MemoryRegionCache(int size, PoolArena::SizeClass sizeClass): size_(size), sizeClass_(sizeClass), allocations_(0) {
		}

		bool add(PoolChunk* chunk, long handle) {
			if (queue_.size() >= size_) {
				std::cout << "[TID#" << gettid() << "] MemoryRegionCache::add() exceeds " << this << " " << size_ << std::endl;
				return false;
			}
			
			queue_.add(Entry(chunk, handle));
			return true;
		}

		bool allocate(PooledCharBuf* buf, int reqCapacity) {
            Entry _entry = queue_.take(0);
			Entry *entry = &_entry;
            if (!entry->chunk_) {
                return false;
            }
            initBuf(entry->chunk_, entry->handle_, buf, reqCapacity);
			#ifdef DEBUG_LOG
            // allocations is not thread-safe which is fine as this is only called from the same thread all time.
            std::cout << "[TID#" << gettid() << "] MemoryRegionCache::allocate() " << this << " " << allocations_ << std::endl;
			#endif
            ++allocations_;
			//delete entry;
            return true;
        }

		int free(bool finalizer) {
			 return free(std::numeric_limits<int>::max(), finalizer);
		}

		void trim() {
            int _free = size_ - allocations_;
            allocations_ = 0;
			
            // We not even allocated all the number that are
            if (_free > 0) {
                free(_free, false);
            }
        }
		
	protected:
		virtual void initBuf(PoolChunk* chunk,  long handle, PooledCharBuf* buf, int reqCapacity) = 0;
		
		int free(int max, bool finalizer) {
			int numFreed = 0;
            for (; numFreed < max; numFreed++) {
                Entry entry = queue_.take(0);
                if (entry.chunk_) {
                   freeEntry(&entry, finalizer);
                } else {
                    break;
                }
            }
			if (numFreed > 0) std::cout << "[TID#" << gettid() << "] MemoryRegionCache::free() " << this << " " << numFreed << std::endl;
            return numFreed;
		}

		void freeEntry(Entry *entry, bool finalizer) {
			entry->chunk_->arena->freeChunk(entry->chunk_, entry->handle_, sizeClass_);
			//delete entry;
		}
		
	protected:
		int size_;
		ObjectQueue< Entry > queue_;
		PoolArena::SizeClass sizeClass_;
		int allocations_;
	};
	
	class SubPageMemoryRegionCache : public MemoryRegionCache {
	public:
        SubPageMemoryRegionCache(int size, PoolArena::SizeClass sizeClass) : MemoryRegionCache(size, sizeClass) {
        }

     protected:
		virtual void initBuf(PoolChunk* chunk, long handle, PooledCharBuf* buf, int reqCapacity) {
			chunk->initBufWithSubpage(buf, handle, reqCapacity);
		}
    };

	class NormalMemoryRegionCache : public MemoryRegionCache {
	public:
        NormalMemoryRegionCache(int size) : MemoryRegionCache(size, PoolArena::Normal) {
        }

      protected:
		virtual void initBuf(PoolChunk* chunk, long handle, PooledCharBuf* buf, int reqCapacity) {
             chunk->initBuf(buf, handle, reqCapacity);
        }
    };
	

	static int log2(int val) {
        int res = 0;
        while (val > 1) {
            val >>= 1;
            res++;
        }
        return res;
    }
	
	static MemoryRegionCache** createSubPageCaches(int cacheSize, int numCaches, PoolArena::SizeClass sizeClass) {
		if (cacheSize > 0 && numCaches > 0) {
            MemoryRegionCache** cache = new MemoryRegionCache*[numCaches];
            for (int i = 0; i < numCaches; i++) {
                cache[i] = new SubPageMemoryRegionCache(cacheSize, sizeClass);
            }
            return cache;
        } 
		return 0;
	}

	static MemoryRegionCache** createNormalCaches(int cacheSize, int maxCachedBufferCapacity, PoolArena *area, int &normalHeapCachesSize) {
		if (cacheSize > 0 && maxCachedBufferCapacity > 0) {
			int max = std::min(area->chunkSize_, maxCachedBufferCapacity);
			normalHeapCachesSize = std::max(1, log2(max / area->pageSize_) + 1);
			MemoryRegionCache** cache = new MemoryRegionCache*[normalHeapCachesSize];
			for (int i = 0; i < normalHeapCachesSize; i++) {
				cache[i] = new NormalMemoryRegionCache(cacheSize);
			}
			return cache;
		}
		return 0;
	}
	
}// namespace detail


const int PoolThreadCache::DEFAULT_TINY_CACHE_SIZE;
const int PoolThreadCache::DEFAULT_SMALL_CACHE_SIZE;
const int PoolThreadCache::DEFAULT_NORMAL_CACHE_SIZE ;
const int PoolThreadCache::DEFAULT_MAX_CACHED_BUFFER_CAPACITY;
const int PoolThreadCache::DEFAULT_CACHE_TRIM_INTERVAL;

PoolThreadCache::PoolThreadCache(PoolArena* heapArena, int tinyCacheSize, int smallCacheSize, int normalCacheSize,
	int maxCachedBufferCapacity, int freeSweepAllocationThreshold) : heapArena_(heapArena), 
	tinySubPageHeapCachesSize_(0), smallSubPageHeapCachesSize_(0), normalHeapCachesSize_(0),
	tinySubPageHeapCaches_(0), smallSubPageHeapCaches_(0), normalHeapCaches_(0), 
	freeSweepAllocationThreshold_(freeSweepAllocationThreshold), allocations_(0) {
	if (heapArena_ != 0) {
		// Create the caches for the heap allocations
		numShiftsNormalHeap_ = detail::log2(heapArena->pageSize_);
		tinySubPageHeapCachesSize_ = PoolArena::numTinySubpagePools;
		smallSubPageHeapCachesSize_ = heapArena->numSmallSubpagePools_;
	    tinySubPageHeapCaches_ = detail::createSubPageCaches(tinyCacheSize, PoolArena::numTinySubpagePools, PoolArena::Tiny);
	    smallSubPageHeapCaches_ = detail::createSubPageCaches(smallCacheSize, heapArena->numSmallSubpagePools_, PoolArena::Small);
	    //numShiftsNormalHeap = log2(heapArena.pageSize);
	    normalHeapCaches_ = detail::createNormalCaches(normalCacheSize, maxCachedBufferCapacity, heapArena, normalHeapCachesSize_);
	    //heapArena.numThreadCaches.getAndIncrement();
	}
}

bool PoolThreadCache::allocateTiny(PoolArena* area, PooledCharBuf* buf, int reqCapacity, int normCapacity) {
	return allocate(cacheForTiny(area, normCapacity), buf, reqCapacity);
}

bool PoolThreadCache::allocateSmall(PoolArena* area, PooledCharBuf* buf, int reqCapacity, int normCapacity) {
	return allocate(cacheForSmall(area, normCapacity), buf, reqCapacity);
}

bool PoolThreadCache::allocateNormal(PoolArena* area, PooledCharBuf* buf, int reqCapacity, int normCapacity) {
	return allocate(cacheForNormal(area, normCapacity), buf, reqCapacity);
}

bool PoolThreadCache::add(PoolArena* area, PoolChunk* chunk, long handle, int normCapacity, int sizeClass) {
	detail::MemoryRegionCache* _cache = cache(area, normCapacity, sizeClass);
	if (!_cache) {
		return false;
	}
	return _cache->add(chunk, handle);
}

/////
detail::MemoryRegionCache* PoolThreadCache::cache(PoolArena* area, int normCapacity, int sizeClass) {
	switch (static_cast<PoolArena::SizeClass>(sizeClass)) {
		case PoolArena::Normal:
			return cacheForNormal(area, normCapacity);
		case PoolArena::Small:
			return cacheForSmall(area, normCapacity);
		case PoolArena::Tiny:
			return cacheForTiny(area, normCapacity);
		default:
			throw std::runtime_error("unknow PoolArena::SizeClass");
	}
}

detail::MemoryRegionCache* PoolThreadCache::cacheForNormal(PoolArena* area, int normCapacity) {
	 int idx = detail::log2(normCapacity >> numShiftsNormalHeap_);
     return cache(normalHeapCaches_, normalHeapCachesSize_, idx);
}
 
detail::MemoryRegionCache* PoolThreadCache::cacheForTiny(PoolArena* area, int normCapacity) {
	int idx = PoolArena::tinyIdx(normCapacity);
	return cache(tinySubPageHeapCaches_, tinySubPageHeapCachesSize_, idx);
}

detail::MemoryRegionCache* PoolThreadCache::cacheForSmall(PoolArena* area, int normCapacity) {
	int idx = PoolArena::smallIdx(normCapacity);
    return cache(smallSubPageHeapCaches_, smallSubPageHeapCachesSize_, idx);
}

detail::MemoryRegionCache* PoolThreadCache::cache(detail::MemoryRegionCache** cache, int len, int idx) {
	if (!cache || idx >len - 1) {
		return 0;
	}
	return cache[idx];
}

bool PoolThreadCache::allocate(detail::MemoryRegionCache* cache, PooledCharBuf* buf, int reqCapacity) {
	if (!cache) {
        // no cache found so just return false here
        return false;
    }
    bool allocated = cache->allocate(buf, reqCapacity);
    if (++allocations_ >= freeSweepAllocationThreshold_) {
        allocations_ = 0;
        trim();
    }
    return allocated;
}

 void PoolThreadCache::trim() {
    trim(tinySubPageHeapCaches_, tinySubPageHeapCachesSize_);
    trim(smallSubPageHeapCaches_, smallSubPageHeapCachesSize_);
    trim(normalHeapCaches_, normalHeapCachesSize_);
}

void PoolThreadCache::trim(detail::MemoryRegionCache** caches, int len){
    if (!caches) {
        return;
    }
	
    for (int i = 0; i < len; i++) {
        trim(caches[i]);
    }
}

void PoolThreadCache::trim(detail::MemoryRegionCache* cache) {
	 if (!cache) {
        return;
    }
    cache->trim();
}

 	
}}//namespace