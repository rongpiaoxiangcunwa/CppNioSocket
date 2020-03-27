#ifndef COMM_BUFFER_POOL_THREAD_CACHE_HPP
#define COMM_BUFFER_POOL_THREAD_CACHE_HPP
#include <iostream>

namespace comm{ namespace buffer{
namespace detail {
	class MemoryRegionCache;
}

class PoolArena;
class PooledCharBuf;
class PoolChunk;

class PoolThreadCache {
public:
	PoolThreadCache(PoolArena* heapArena, int tinyCacheSize, int smallCacheSize, int normalCacheSize,
		int maxCachedBufferCapacity, int freeSweepAllocationThreshold);
	
	virtual ~PoolThreadCache() {
		for (int i = 0; i < tinySubPageHeapCachesSize_; i++) {
			delete tinySubPageHeapCaches_[i];
			tinySubPageHeapCaches_[i] = 0;
		}

		delete[] tinySubPageHeapCaches_;
		tinySubPageHeapCaches_ = 0;

		for (int i = 0; i < smallSubPageHeapCachesSize_; i++) {
			delete smallSubPageHeapCaches_[i];
			smallSubPageHeapCaches_[i] = 0;
		}
		delete[] smallSubPageHeapCaches_;
		smallSubPageHeapCaches_ = 0;
		

		for (int i = 0; i < normalHeapCachesSize_; i++) {
			delete normalHeapCaches_[i];
			normalHeapCaches_[i] = 0;
		}
		delete[] normalHeapCaches_;
		normalHeapCaches_ = 0;
	}
	
    bool allocateTiny(PoolArena* area, PooledCharBuf* buf, int reqCapacity, int normCapacity);
	
	bool allocateSmall(PoolArena* area, PooledCharBuf* buf, int reqCapacity, int normCapacity);

	bool allocateNormal(PoolArena* area, PooledCharBuf* buf, int reqCapacity, int normCapacity);

	bool add(PoolArena* area, PoolChunk* chunk, long handle, int normCapacity, int sizeClass);

private:
	 detail::MemoryRegionCache* cacheForTiny(PoolArena* area, int normCapacity);
	 detail::MemoryRegionCache* cacheForSmall(PoolArena* area, int normCapacity);
	 detail::MemoryRegionCache* cacheForNormal(PoolArena* area, int normCapacity);
	 
	 detail::MemoryRegionCache* cache(detail::MemoryRegionCache** cache, int len, int idx);
	 bool allocate(detail::MemoryRegionCache* cache, PooledCharBuf* buf, int reqCapacity);
	 void trim();
	 void trim(detail::MemoryRegionCache** caches, int len) ;
	 void trim(detail::MemoryRegionCache* cache);

	 detail::MemoryRegionCache* cache(PoolArena* area, int normCapacity, int sizeClass);
	 
private:
	PoolArena* heapArena_;
	int numShiftsNormalHeap_;
	int tinySubPageHeapCachesSize_;
	int smallSubPageHeapCachesSize_;
	int normalHeapCachesSize_;
	detail::MemoryRegionCache** tinySubPageHeapCaches_;
	detail::MemoryRegionCache** smallSubPageHeapCaches_;
	detail::MemoryRegionCache** normalHeapCaches_;
	int freeSweepAllocationThreshold_;
	int allocations_;
public:
	static const int DEFAULT_TINY_CACHE_SIZE = 512;
	static const int DEFAULT_SMALL_CACHE_SIZE = 256;
	static const int DEFAULT_NORMAL_CACHE_SIZE = 64;
	static const int DEFAULT_MAX_CACHED_BUFFER_CAPACITY = 32 * 1024;
	static const int DEFAULT_CACHE_TRIM_INTERVAL = 100000;
};
	
} } //namespace comm

#endif
