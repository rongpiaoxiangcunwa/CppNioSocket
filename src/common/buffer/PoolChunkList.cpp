#include "PoolChunkList.hpp"
#include "PoolArena.hpp"
#include "PooledCharBuf.hpp"
#include <algorithm>

namespace comm { 
namespace buffer {

PoolChunkList::PoolChunkList(PoolArena* arena, PoolChunkList* nextList, int minUsage, int maxUsage, int chunkSize)
:nextList_(nextList), prevList_(0), arena_(arena), minUsage_(minUsage), maxUsage_(maxUsage), head_(0)
{
	maxCapacity_ = calculateMaxCapacity(minUsage, chunkSize);
}

bool PoolChunkList::allocate(shared_ptr<PooledCharBuf>& buf, int reqCapacity, int normCapacity) {
	if (normCapacity > maxCapacity_) {
		// Either this PoolChunkList is empty or the requested capacity is larger then the capacity which can
		// be handled by the PoolChunks that are contained in this PoolChunkList.
		return false;
	}

	for (PoolChunk* cur = head_; cur; cur = cur->next) {
		if (cur->allocate(buf.get(), reqCapacity, normCapacity)) {
			std::cout << "[TID#" << gettid() << "] PoolChunkList::allocate() " << minUsage_ << std::endl;
			if (cur->usage() >= maxUsage_) {
				remove(cur);
				nextList_->add(cur);
			}
			return true;
		}
	}
	return false;
}

bool PoolChunkList::free(PoolChunk* chunk, long handle) {
	chunk->free(handle);
    if (chunk->usage() < minUsage_) {
        remove(chunk);
        // Move the PoolChunk down the PoolChunkList linked-list.
        return move0(chunk);
    }
    return true;
}

void PoolChunkList::add(PoolChunk* chunk) {
	std::cout <<"[TID#" << gettid() << "] PoolChunkList::add " << chunk << " " << minUsage_ << "-" << maxUsage_ << std::endl;
	if (chunk->usage() >= maxUsage_) {
	    nextList_->add(chunk);
	    return;
	}
	add0(chunk);
}

bool PoolChunkList::move(PoolChunk* chunk) {
    BOOST_ASSERT(chunk->usage() < maxUsage_);
    if (chunk->usage() < minUsage_) {
        // Move the PoolChunk down the PoolChunkList linked-list.
        return move0(chunk);
    }
    // PoolChunk fits into this PoolChunkList, adding it here.
    add0(chunk);
    return true;
}

void PoolChunkList::remove(PoolChunk* cur) {
    if (cur == head_) {
        head_ = cur->next;
        if (head_) {
            head_->prev = 0;
        }
    } else {
        PoolChunk* next = cur->next;
        cur->prev->next = next;
        if (next) {
            next->prev = cur->prev;
        }
    }
}

void PoolChunkList::add0(PoolChunk* chunk) {
	std::cout <<"[TID#" << gettid() << "] PoolChunkList::add0 " << chunk << " " << minUsage_ << "-" << maxUsage_ << std::endl;
    chunk->parent = this;
    if (head_ == NULL) {
        head_ = chunk;
        chunk->prev = 0;
        chunk->next = 0;
    } else {
        chunk->prev = 0;
        chunk->next = head_;
        head_->prev = chunk;
        head_ = chunk;
    }
}

bool PoolChunkList::move0(PoolChunk* chunk) {
    if (!prevList_) {
        // There is no previous PoolChunkList so return false which result in having the PoolChunk destroyed and
        // all memory associated with the PoolChunk will be released.
        BOOST_ASSERT(chunk->usage() == 0);
        return false;
    }
    return prevList_->move(chunk);
}
	
int PoolChunkList::calculateMaxCapacity(int minUsage, int chunkSize) {
	minUsage = std::max(1, minUsage);
	if (minUsage == 100) {
	    // If the minUsage is 100 we can not allocate anything out of this list.
	    return 0;
	}

	// Calculate the maximum amount of bytes that can be allocated from a PoolChunk in this PoolChunkList.
	//
	// As an example:
	// - If a PoolChunkList has minUsage == 25 we are allowed to allocate at most 75% of the chunkSize because
	//   this is the maximum amount available in any PoolChunk in this PoolChunkList.
	return  (int) (chunkSize * (100L - minUsage) / 100L);
}

}//namespace net
}//namespace comm
