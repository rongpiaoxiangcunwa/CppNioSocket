#include "PoolArena.hpp"
#include "PooledCharBufAllocator.hpp"
#include "PooledHeapCharBuf.hpp"
#include <limits.h>

namespace comm { 
namespace buffer {
const int PoolArena::numTinySubpagePools;

PoolArena::PoolArena(PooledCharBufAllocator* parent, int pageSize, int maxOrder,  int pageShifts, int chunkSize) 
: parent_(parent), maxOrder_(maxOrder), pageSize_(pageSize), pageShifts_(pageShifts), chunkSize_(chunkSize)
{
	tinySubpagePools_ = new PoolSubpage*[numTinySubpagePools];//32
	for (int i = 0; i < numTinySubpagePools; i ++) {
	    tinySubpagePools_[i] = new PoolSubpage(pageSize);
		tinySubpagePools_[i]->prev_ = tinySubpagePools_[i];
		tinySubpagePools_[i]->next_ = tinySubpagePools_[i];
	}

	numSmallSubpagePools_ = pageShifts - 9; 
	smallSubpagePools_ = new PoolSubpage*[numSmallSubpagePools_];//4
	for (int i = 0; i < numSmallSubpagePools_; i++) {
	    smallSubpagePools_[i] = new PoolSubpage(pageSize);
		smallSubpagePools_[i]->prev_ =  smallSubpagePools_[i];
		smallSubpagePools_[i]->next_ =  smallSubpagePools_[i];
	}

	//qInit_->q000_->q025_->q050_->q075_->q100_->NULL
	//NULL<-     <-	  <-     <-     <-							  
	q100_ = new PoolChunkList(this, 0, 100, INT_MAX, chunkSize);
	q075_ = new PoolChunkList(this, q100_, 75, 100, chunkSize);
	q050_ = new PoolChunkList(this, q075_, 50, 100, chunkSize);
	q025_ = new PoolChunkList(this, q050_, 25, 75, chunkSize);
	q000_ = new PoolChunkList(this, q025_, 1, 50, chunkSize);
	qInit_ = new PoolChunkList(this, q000_, INT_MIN, 25, chunkSize);

	q100_->prevList(q075_);
	q075_->prevList(q050_);
	q050_->prevList(q025_);
	q025_->prevList(q000_);
	q000_->prevList(0);
	qInit_->prevList(qInit_);
	cache_ = new PoolThreadCache(this,PoolThreadCache::DEFAULT_TINY_CACHE_SIZE, PoolThreadCache::DEFAULT_SMALL_CACHE_SIZE, 
		PoolThreadCache::DEFAULT_NORMAL_CACHE_SIZE, PoolThreadCache::DEFAULT_MAX_CACHED_BUFFER_CAPACITY, PoolThreadCache::DEFAULT_CACHE_TRIM_INTERVAL);
}

PoolArena::~PoolArena() {
	for (int i = 0; i < numTinySubpagePools; i++) {
		delete tinySubpagePools_[i];
	}

	for (int i = 0; i < numSmallSubpagePools_; i++) {
		delete smallSubpagePools_[i];
	}
	
	delete []tinySubpagePools_;
	delete []smallSubpagePools_;
	delete qInit_;
	delete q000_;
	delete q025_;
	delete q050_;
	delete q075_;
	delete q100_;
	delete cache_;
}

void PoolArena::destroyChunk(PoolChunk* chunk) {
	delete chunk;
}

void PoolArena::reallocate(shared_ptr<PooledCharBuf>& buf, int newCapacity, bool freeOldMemory) {
	BOOST_ASSERT( newCapacity >= 0 && newCapacity <= buf->maxCapacity());
	std::cout <<"[TID#" << gettid() << "] PoolArena::reallocate()  newCapacity: " << newCapacity << std::endl;
    int oldCapacity = buf->length_;
    if (oldCapacity == newCapacity) {
        return;
    }

    PoolChunk* oldChunk = buf->chunk_;
    long oldHandle = buf->handle_;
    char* oldMemory = buf->memory_;
    int oldMaxLength = buf->maxLength_;
	int oldOffset = buf->offset_;
	boost::lock_guard<boost::recursive_mutex> lock(mutex_);
    // This does not touch buf's reader/writer indices
    allocate(buf, newCapacity);
    int bytesToCopy;
    if (newCapacity > oldCapacity) {
        bytesToCopy = oldCapacity;
    } else {
        buf->trimIndicesToCapacity(newCapacity);
        bytesToCopy = newCapacity;
    }
	
	memcpy(buf->memory_ + buf->offset_, oldMemory + oldOffset, bytesToCopy);
	
    if (freeOldMemory) {
        free(oldChunk, oldHandle, oldMaxLength, buf->cache_);
    }
}
	
void PoolArena::free(PoolChunk *chunk, long handle, int normCapacity, PoolThreadCache* cache) {
	if (chunk->unpooled) {
		destroyChunk(chunk);
	} else {
		SizeClass _sizeClass = sizeClass(normCapacity);
		#if 0
		boost::lock_guard<boost::recursive_mutex> lock(mutex_);
		
		if (cache_ && cache_->add(this, chunk, handle, normCapacity, _sizeClass)) {
		    // cached so not free it.
		    return;
		}
		#endif
		
		freeChunk(chunk, handle, _sizeClass);
	}
}

shared_ptr<PooledCharBuf> PoolArena::allocate(int reqCapacity, int maxCapacity) {
	boost::lock_guard<boost::recursive_mutex> lock(mutex_);
	shared_ptr<PooledCharBuf> buf = newCharBuf(maxCapacity);
	allocate(buf, reqCapacity);
	return buf;
}

void PoolArena::allocate(shared_ptr<PooledCharBuf>& buf, int reqCapacity) {
	int normCapacity = normalizeCapacity(reqCapacity);
	#ifdef DEBUG_LOG
	std::cout <<"[TID#" << gettid() << "] PoolArena::allocate reqCapacity: " << reqCapacity << "-" << normCapacity<< std::endl;
	#endif
	if (isTinyOrSmall(normCapacity)) { //capacity < pageSize
		int tableIdx = 0;
		PoolSubpage** table = NULL;
		bool tiny = isTiny(normCapacity);
		if (tiny) { //< 512
		#if 0
			if (cache_->allocateTiny(this, buf.get(), reqCapacity, normCapacity)) {
				// was able to allocate out of the cache so move on
				return;
			}
		#endif
			tableIdx = tinyIdx(normCapacity);
			table = tinySubpagePools_;
		} else {
		#if 0
			if (cache_->allocateSmall(this, buf.get(), reqCapacity, normCapacity)) {
				// was able to allocate out of the cache so move on
				//std::cout <<"[TID#" << gettid() << "] PoolArena::allocate from allocateSmall cache " << std::endl;
				return;
			}
		#endif
			tableIdx = smallIdx(normCapacity);
			table = smallSubpagePools_;
		}

		PoolSubpage *head = table[tableIdx];
		/**
		* Synchronize on the head. This is needed as {@link PoolChunk#allocateSubpage(int)} and
		* {@link PoolChunk#free(long)} may modify the doubly linked list as well.
		*/
		
		PoolSubpage* s = head->next_;
		if (s != head) {
			BOOST_ASSERT(s->doNotDestroy() && s->elementSize() == normCapacity);
			long handle = s->allocate();
			BOOST_ASSERT(handle >= 0);
			s->chunk_->initBufWithSubpage(buf.get(), handle, reqCapacity);
			//incTinySmallAllocation(tiny); // statistic for the tiny/small allocations
			return;
		}
		
		allocateNormal(buf, reqCapacity, normCapacity);
		//incTinySmallAllocation(tiny);
		return;
	}
	
	if (normCapacity <= chunkSize_) {
	#if 0
		if (cache_->allocateNormal(this, buf.get(), reqCapacity, normCapacity)) {
			// was able to allocate out of the cache so move on
			return;
		}
	#endif
		allocateNormal(buf, reqCapacity, normCapacity);
		//++allocationsNormal_;
	} else {
		// Huge allocations are never served via the cache so just call allocateHuge
		allocateHuge(buf, reqCapacity);
	}
}

void PoolArena::allocateHuge(shared_ptr<PooledCharBuf>& buf, int reqCapacity) {
	PoolChunk* chunk = newUnpooledChunk(reqCapacity);
	buf->initUnpooled(chunk, reqCapacity);
}

PoolChunk* PoolArena::newUnpooledChunk(int capacity) {
	return new PoolChunk(this, capacity, 0);
}

void PoolArena::allocateNormal(shared_ptr<PooledCharBuf>& buf, int reqCapacity, int normCapacity) {
    if (q050_->allocate(buf, reqCapacity, normCapacity) || q025_->allocate(buf, reqCapacity, normCapacity) ||
        q000_->allocate(buf, reqCapacity, normCapacity) || qInit_->allocate(buf, reqCapacity, normCapacity) ||
        q075_->allocate(buf, reqCapacity, normCapacity)) {
        //std::cout << "[TID#" << gettid()) << "] " << __FUNCTION__ << " alloc from PoolChunkList"  << std::endl;
        return;
    }
    // Add a new chunk.
    std::cout << "[TID#" << gettid() << "] " << __FUNCTION__ << " alloc from new Chunk"  << std::endl;
    PoolChunk* c = newChunk(pageSize_, maxOrder_, pageShifts_, chunkSize_);
    bool success = c->allocate(buf.get(), reqCapacity, normCapacity);
	BOOST_ASSERT(success);
    qInit_->add(c);
}

PoolChunk* PoolArena::newChunk(int pageSize, int maxOrder, int pageShifts, int chunkSize) {
	return new PoolChunk(this, chunkSize, pageSize, pageShifts, maxOrder, 0);
}

/*
	当size>=512时，size成倍增长512->1024->2048->4096->8192，而size<512则是从16开始，每次加16字节
*/
int PoolArena::normalizeCapacity(int reqCapacity) {
	if (reqCapacity >= chunkSize_) return reqCapacity;
	
	if (!isTiny(reqCapacity)) { // >= 512
		int normalizedCapacity = reqCapacity;
		normalizedCapacity--;
		normalizedCapacity |= normalizedCapacity >>  1;
		normalizedCapacity |= normalizedCapacity >>  2;
		normalizedCapacity |= normalizedCapacity >>  4;
		normalizedCapacity |= normalizedCapacity >>  8;
		normalizedCapacity |= normalizedCapacity >> 16;
		normalizedCapacity++;
		if (normalizedCapacity < 0) {
			normalizedCapacity >>= 1;
		}
		return normalizedCapacity;
	}

	// Quantum-spaced
	if ((reqCapacity & 15) == 0) {
		return reqCapacity;
	}

	return (reqCapacity & ~15) + 16;
}

shared_ptr<PooledCharBuf> PoolArena::newCharBuf(int maxCapacity) {
	return PooledHeapCharBuf::newInstance(maxCapacity);
}

PoolSubpage* PoolArena::findSubpagePoolHead(int elemSize) {
	int tableIdx;
    PoolSubpage** table = 0;
    if (isTiny(elemSize)) { // < 512
        tableIdx = elemSize >> 4;
        table = tinySubpagePools_;
    } else {
        tableIdx = 0;
        elemSize >>= 10;
        while (elemSize != 0) {
            elemSize >>= 1;
            tableIdx++;
        }
        table = smallSubpagePools_;
    }

    return table[tableIdx];
}

PoolArena::SizeClass PoolArena::sizeClass(int normCapacity) {
	if (!isTinyOrSmall(normCapacity)) {
        return PoolArena::Normal;
    }
    return isTiny(normCapacity) ? PoolArena::Tiny : PoolArena::Small;
}

void PoolArena::freeChunk(PoolChunk* chunk, long handle, SizeClass sizeClass) {
	boost::lock_guard<boost::recursive_mutex> lock(mutex_);
	if (!chunk->parent->free(chunk, handle)) {
		destroyChunk(chunk);
	}
}

}//namespace buffer
}//namespace comm