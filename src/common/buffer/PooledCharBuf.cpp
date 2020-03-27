#include <algorithm>
#include "PooledCharBuf.hpp"
#include "PoolChunk.hpp"
#include "PoolArena.hpp"
#include "PoolThreadCache.hpp"
#include "CharBufAllocator.hpp"
#include "PooledSlicedCharBuf.hpp"

namespace comm {
namespace buffer {

void PooledCharBuf::capacity(int newCapacity) {
	if (newCapacity == length_) return ;
    checkNewCapacity(newCapacity);
    if (!chunk_->unpooled) {
        // If the request capacity does not require reallocation, just update the length of the memory.
        if (newCapacity > length_) {
            if (newCapacity <= maxLength_) {
                length_ = newCapacity;
                return ;
            }
        } else if (newCapacity > maxLength_ >> 1 && (maxLength_ > 512 || newCapacity > maxLength_ - 16)) {
            // here newCapacity < length
            length_ = newCapacity;
            trimIndicesToCapacity(newCapacity);
            return ;
        }
    }

    // Reallocation required.
    shared_ptr<PooledCharBuf> buf = boost::dynamic_pointer_cast<PooledCharBuf>(get_shared_ptr());
    chunk_->arena->reallocate(buf, newCapacity, true);
}

void PooledCharBuf::deallocate() {
	if (handle_ >= 0) {
        chunk_->arena->free(chunk_, handle_, maxLength_, cache_);
		memory_ = 0;
		handle_ = -1;
        chunk_ = 0;
		// return this obj to pool
       if (!recycler_.empty()) recycler_(get_shared_ptr());
    }
}

shared_ptr<CharBuf> PooledCharBuf::retainedSlice(int _index, int _length) {
	shared_ptr<AbstractCharBuf> unwraped = boost::dynamic_pointer_cast<AbstractCharBuf>(get_shared_ptr());
	return PooledSlicedCharBuf::newInstance(unwraped, get_shared_ptr(), _index, _length);
}
	
void PooledCharBuf::initUnpooled(PoolChunk* chunk, int length) {
    init0(chunk, 0, chunk->offset, length, length, 0);
}

void PooledCharBuf::init(PoolChunk* chunk, long handle, int offset, int length, int maxLength, PoolThreadCache* cache) {
	#ifdef DEBUG_LOG
	std::cout <<"[TID#" << gettid() << "] PooledCharBuf::init " << this  << " chunk:" << chunk << " handle:" << handle << " offset:" << offset << " length:" << length << " maxLength:" << maxLength  << std::endl;
	#endif
	init0(chunk, handle, offset, length, maxLength, cache);
}

void PooledCharBuf::init0(PoolChunk* chunk, long handle, int offset, int length, int maxLength, PoolThreadCache* cache) {
    BOOST_ASSERT( handle >= 0 );
    BOOST_ASSERT( chunk != 0 );
    chunk_ = chunk;
	handle_ = handle;
    offset_ = offset;
    length_ = length;
    maxLength_ = maxLength;
    memory_ = chunk->memory;
    allocator_ = (CharBufAllocator*) (chunk->arena->parent());
    cache_ = cache;
}

void PooledCharBuf::init() {
	chunk_ = 0;
    handle_ = 0;
	offset_ = 0;
    length_ = 0;
    maxLength_ = 0;
    memory_ = 0;
    allocator_ = 0;
	cache_ = 0;
}

void PooledCharBuf::checkNewCapacity(int newCapacity) {
	if (newCapacity < 0 || newCapacity > maxCapacity()) {
		throw std::runtime_error((boost::format("newCapacity %1% expected:(0 - %2%)") % newCapacity % maxCapacity()).str());
	}
}

void PooledCharBuf::trimIndicesToCapacity(int newCapacity) {
	if (writerIndex() > newCapacity) {
        setIndex0(std::min(readerIndex(), newCapacity), newCapacity);
    }
}
	
}//namespace buffer
}//namespace comm