#include "PoolSubpage.hpp"
#include "PoolChunk.hpp"
#include <boost/assert.hpp>

namespace comm { 
namespace buffer {

PoolSubpage::PoolSubpage(int pageSize)  {
	init();
	pageSize_ = pageSize;
}

PoolSubpage::PoolSubpage(PoolSubpage *head, PoolChunk *chunk, int memoryMapIdx, int runOffset, int pageSize, int elemSize) {
	init();
	chunk_ = chunk;
    memoryMapIdx_ = memoryMapIdx;
  	runOffset_ = runOffset;
    pageSize_ = pageSize;
    bitmap_ = new long[pageSize >> 10]; // pageSize(8192) / 16(每个段的最小值) / 64(一个long可以标记64位) (deflat is 8 )
    init(head, elemSize);
}

PoolSubpage::~PoolSubpage() {
	delete []bitmap_;
}

/*
	最后结果是一个long整数，其中低32位表示二叉树中分配的节点，高32位表示subPage中分配的具体位置
	Returns the bitmap index of the subpage allocation.
*/
long PoolSubpage::allocate() {
	if (elemSize_ == 0) {
        return toHandle(0);
    }

    if (numAvail_ == 0 || !doNotDestroy_) {
        return -1;
    }

    int bitmapIdx = getNextAvail();
    int q = bitmapIdx >> 6;//在bitmap_中的下标
    int r = bitmapIdx & 63;//bitmap_第q个元素的未被使用的bit位
    BOOST_ASSERT((bitmap_[q] >> r & 1) == 0);
    bitmap_[q] |= 1L << r;//将内存段标记为被占用
    
    if (--numAvail_ == 0) {
        removeFromPool();
    }
	std::cout << "[TID#" << gettid() << "] PoolSubpage::allocate() bitmapIdx:" << bitmapIdx  << " chunk_:" << chunk_ << " memoryMapIdx_:" << memoryMapIdx_ << std::endl;
    return toHandle(bitmapIdx);
}

bool PoolSubpage::free(PoolSubpage* head, int bitmapIdx) {
	if (elemSize_ == 0) {
	    return true;
	}
	
	int q = bitmapIdx >> 6;
	int r = bitmapIdx & 63;
	BOOST_ASSERT( (bitmap_[q] >> r & 1) != 0 );
	bitmap_[q] ^= 1L << r;

	setNextAvail(bitmapIdx);

	if (numAvail_++ == 0) {
	    addToPool(head);
	    return true;
	}

	if (numAvail_ != maxNumElems_) {
	    return true;
	} else {
	    // Subpage not in use (numAvail == maxNumElems)
	    if (prev_ == next_) {
	        // Do not remove if this subpage is the only one left in the pool.
	        return true;
	    }
	    // Remove this subpage from the pool if there are other subpages left in the pool.
	    //这样这个page又可以被作为大于8192的大内存块被使用
	    doNotDestroy_ = false;
	    removeFromPool();
		std::cout << "[TID#" << gettid() << "] PoolSubpage::free " << " remove " << this << " from pool" << std::endl;
	    return false;
	}
}

void PoolSubpage::destroy() {
    if (chunk_) chunk_->destroy();
}

void PoolSubpage::init() {
	chunk_ = 0;
	prev_ = next_ = 0;
	memoryMapIdx_ = runOffset_ = -1;
	pageSize_ = 0;
	bitmap_ = 0;
	doNotDestroy_ = true;
	elemSize_ = -1;
	maxNumElems_ = bitmapLength_ = nextAvail_ = numAvail_ = 0;
}

void PoolSubpage::removeFromPool() {
	BOOST_ASSERT(prev_ != 0 && next_ != 0);
	prev_->next_ = next_;
	next_->prev_ = prev_;
	next_ = 0;
	prev_ = 0;
}

int PoolSubpage::getNextAvail() {
    int nextAvail = nextAvail_;
    if (nextAvail >= 0) {
        nextAvail_ = -1;
        return nextAvail;
    }
    return findNextAvail();
}

int PoolSubpage::findNextAvail() {
    long* bitmap = bitmap_;
    int bitmapLength = bitmapLength_;
    for (int i = 0; i < bitmapLength; i++) {
        long bits = bitmap[i];
        if (~bits != 0) {
            return findNextAvail0(i, bits);
        }
    }
    return -1;
}

int PoolSubpage::findNextAvail0(int i, long bits) {
    int baseVal = i << 6;
    for (int j = 0; j < 64; j++) {
    	if ((bits & 1) == 0) {
            int val = baseVal | j;
            if (val < maxNumElems_) {
                return val;
            } else {
                break;
            }
        }
        bits >>= 1;
    }
    return -1;
}

void PoolSubpage::init(PoolSubpage* head, int elemSize) {
	doNotDestroy_ = true;
    elemSize_ = elemSize;
    if (elemSize != 0) {
        maxNumElems_ = numAvail_ = pageSize_ / elemSize;
        nextAvail_ = 0;
        bitmapLength_ = maxNumElems_ >> 6;
        if ((maxNumElems_ & 63) != 0) {
            bitmapLength_++;
        }

        for (int i = 0; i < bitmapLength_; i++) {
            bitmap_[i] = 0;
        }
    }
    addToPool(head);
}

// add to PoolArena::tinySubpagePools_ or PoolArena::smallSubpagePools_
void PoolSubpage::addToPool(PoolSubpage* head) {
    BOOST_ASSERT(prev_ == 0 && next_ == 0);
    prev_ = head;
    next_ = head->next_;
    next_->prev_ = this;
    prev_->next_ = this;
}

}//namespace buffer
}//namespace comm