#ifndef COMM_BUFFER_POOLED_CHARBUF_ALLOCATOR_HPP
#define COMM_BUFFER_POOLED_CHARBUF_ALLOCATOR_HPP

#include <set>
#include <stdexcept>
#include <boost/limits.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include "AbstractCharBufAllocator.hpp"
#include "PooledHeapCharBuf.hpp"
#include "UnpooledHeapCharBuf.hpp"
#include <iostream>
#include "PoolThreadCache.hpp"
#include "PoolArena.hpp"

namespace comm {
namespace buffer {
	
class PooledCharBufAllocator : public AbstractCharBufAllocator 
{
public:
	PoolThreadCache* threadCache() { return 0;}
	
protected:
	virtual shared_ptr<CharBuf> newHeapBuffer(int initialCapacity, int maxCapacity) {
		PoolArena* poolArena = getOrCreateCache();
		if (poolArena) {
			return poolArena->allocate(initialCapacity, maxCapacity);
		}
		std::cout <<"[TID#" << gettid() << "] create new UnpooledHeapCharBuf "  << std::endl;
		return shared_ptr<UnpooledHeapCharBuf>(new UnpooledHeapCharBuf(initialCapacity, maxCapacity));
	}

protected:
	PoolArena* getOrCreateCache() {
		PoolArena* poolArena = poolArena_.get();
		if (!poolArena) {
			std::cout << "[TID#" << gettid() << "] create new PoolArena " << std::endl;
			int chunkSize = validateAndCalculateChunkSize(DEFAULT_PAGE_SIZE, DEFAULT_MAX_ORDER);
			int pageShifts = validateAndCalculatePageShifts(DEFAULT_PAGE_SIZE);
			poolArena = new PoolArena(this, DEFAULT_PAGE_SIZE, DEFAULT_MAX_ORDER, pageShifts, chunkSize);
			poolArena_.reset(poolArena);
		}
		return poolArena;
	}

	int validateAndCalculateChunkSize(int pageSize, int maxOrder) {
		if (maxOrder > 14) {
			throw std::runtime_error((boost::format("maxOrder:%1% expected: 0-14") % maxOrder).str());
		}

        // Ensure the resulting chunkSize does not overflow.
        int chunkSize = pageSize;
        for (int i = maxOrder; i > 0; i--) {
            if (chunkSize > MAX_CHUNK_SIZE / 2) {
				throw std::runtime_error((boost::format("pageSize(%1%) << maxOrder(%2%) must not exceed %3%") % pageSize % maxOrder % MAX_CHUNK_SIZE).str());
            }
            chunkSize <<= 1;
        }
        return chunkSize;
	}

	int validateAndCalculatePageShifts(int pageSize) {
		if (pageSize < MIN_PAGE_SIZE) {
			throw std::runtime_error((boost::format("pageSize:%1% (expected:%2%)") % pageSize % MIN_PAGE_SIZE).str());
        }

        if ((pageSize & pageSize - 1) != 0) {
			throw std::runtime_error((boost::format("pageSize:%1% (expected:power of 2)") % pageSize).str());
        }

        // Logarithm base 2. At this point we know that pageSize is a power of two.
        return 32 - 1 - PoolChunk::numberOfLeadingZeros(pageSize);
	}

private:
	boost::thread_specific_ptr< PoolArena > poolArena_;
	const static int DEFAULT_PAGE_SIZE = 8192;
	const static int DEFAULT_MAX_ORDER = 11;
	const static int MIN_PAGE_SIZE = 4096;
    const static int MAX_CHUNK_SIZE;
};
	
}//namespace buffer
}//namespace comm

#endif
