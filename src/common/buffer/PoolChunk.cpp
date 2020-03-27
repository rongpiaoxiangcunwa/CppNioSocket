#include "PoolChunk.hpp"
#include "PoolArena.hpp"
#include "PoolChunkList.hpp"
#include "PooledCharBuf.hpp"
#include "PooledCharBufAllocator.hpp"
namespace comm { 
namespace buffer {

int PoolChunk::usage() {
	if (freeBytes_ == 0) {
		return 100;
	}

	int freePercentage = (int)(freeBytes_ * 100L / chunkSize());
	if (freePercentage == 0) {
		return 99;
	}
	
	return 100 - freePercentage;
}

bool PoolChunk::allocate(PooledCharBuf* buf, int reqCapacity, int normCapacity) {
	long handle = 0;
	if (normCapacity >= pageSize) {
		handle = allocateRun(normCapacity);
	} else {
		handle = allocateSubpage(normCapacity);
	}
	
	if (handle < 0) {
		return false;
	}

	//ByteBuffer nioBuffer = cachedNioBuffers != null ? cachedNioBuffers.pollLast() : null;
	initBuf(buf, handle, reqCapacity);
	return true;
}

void PoolChunk::free(long handle) {
	int memoryMapIdx_ = memoryMapIdx(handle);
    int bitmapIdx_ = bitmapIdx(handle);
    if (bitmapIdx_ != 0) { // free a subpage
        PoolSubpage* subpage = subpages[subpageIdx(memoryMapIdx_)];
		std::cout << "[TID#" << gettid() << "] PoolChunk::free() "  << this << " handle " << handle << std::endl;
        BOOST_ASSERT(subpage && subpage->doNotDestroy());
        // Obtain the head of the PoolSubPage pool that is owned by the PoolArena and synchronize on it.
        // This is need as we may add it back and so alter the linked-list structure.
        PoolSubpage* head = arena->findSubpagePoolHead(subpage->elementSize());
        if (subpage->free(head, bitmapIdx_ & 0x3FFFFFFF)) {
            return;
        }
    }
    freeBytes_ += runLength(memoryMapIdx_);
    setValue(memoryMapIdx_, depth(memoryMapIdx_));
    updateParentsFree(memoryMapIdx_);
}

void PoolChunk::destroy() {
	arena->destroyChunk(this);
}

void PoolChunk::initBuf(PooledCharBuf* buf , long handle, int reqCapacity) {
	int _memoryMapIdx = memoryMapIdx(handle);
	int _bitmapIdx = bitmapIdx(handle);
	if (_bitmapIdx == 0) {
		uint8_t val = value(_memoryMapIdx);
		BOOST_ASSERT( val == unusable );
		buf->init(this, handle, runOffset(_memoryMapIdx) + offset,
	      		  reqCapacity, runLength(_memoryMapIdx), arena->parent()->threadCache());
	} else {
		initBufWithSubpage(buf, handle, _bitmapIdx, reqCapacity);
	}
}

void PoolChunk::initBufWithSubpage(PooledCharBuf* buf, long handle, int reqCapacity) {
	initBufWithSubpage(buf, handle, bitmapIdx(handle), reqCapacity);
}

void PoolChunk::initBufWithSubpage(PooledCharBuf* buf, long handle, int bitmapIdx, int reqCapacity) {
	BOOST_ASSERT( bitmapIdx != 0 );

	int _memoryMapIdx = memoryMapIdx(handle);

	PoolSubpage* subpage = subpages[subpageIdx(_memoryMapIdx)];
	BOOST_ASSERT( subpage->doNotDestroy() );
	BOOST_ASSERT( reqCapacity <= subpage->elementSize());

	buf->init(this, handle, runOffset(_memoryMapIdx) + (bitmapIdx & 0x3FFFFFFF) * subpage->elementSize() + offset,
			  reqCapacity, subpage->elementSize(), arena->parent()->threadCache());
}

long PoolChunk::allocateRun(int normCapacity) {
	//从第d层开始节点匹配
	int d = maxOrder - (log2(normCapacity) - pageShifts);
	int id = allocateNode(d);
	if (id < 0) {
		return id;
	}
	freeBytes_ -= runLength(id);
	return id;
}

long PoolChunk::allocateSubpage(int normCapacity) {
    // Obtain the head of the PoolSubPage pool that is owned by the PoolArena and synchronize on it.
    // This is need as we may add it back and so alter the linked-list structure.
    PoolSubpage *head = arena->findSubpagePoolHead(normCapacity);
    int d = maxOrder; //subpages are only be allocated from pages i.e., leaves
    int id = allocateNode(d); //在树中找到匹配的叶子节点
    if (id < 0) {
    	return id;// 叶子节点全部分配完毕
    }
	
    freeBytes_ -= pageSize;
    int _subpageIdx = subpageIdx(id); // 得到叶子节点的偏移索引，从0开始，即2048-0,2049-1,...
    PoolSubpage* subpage = subpages[_subpageIdx];
    if (!subpage) {
		/*
		新建或初始化subpage并加入到chunk的subpages数组，
		同时将subpage加入到arena的subpage双向链表中，最后完成分配请求的内存
		*/
        subpage = new PoolSubpage(head, this, id, runOffset(id), pageSize, normCapacity);
        subpages[_subpageIdx] = subpage;
    } else {
	    /*
	    subpage初始化后分配了内存，但一段时间后该subpage分配的内存释放并从arena的双向链表中删除，
	    此时subpage不为null，当再次请求分配时，只需要调用init()将其加入到areana的双向链表中即可
	    */
        subpage->init(head, normCapacity);
    }
    return subpage->allocate();
}


int PoolChunk::allocateNode(int d) {
	int id = 1;
	/* has last d bits = 0 and rest all = 1. if d= 9 , 
	   initial = -512 ->  11111111111111111111111000000000 
	*/
	int initial = -(1 << d); 
	uint8_t val = value(id);
	if (val > d) { // unusable
		return -1;
	}

	/* id & initial == 1 << d for all ids at depth d, for < d it is 0 */
	while (val < d || (id & initial) == 0) { 
		id <<= 1; //匹配子节点
		val = value(id);
		if (val > d) { //子节点被分配，而且剩余节点的内存大小不够，此时需要在兄弟节点上继续查找
			id ^= 1;
			val = value(id);
		}
	}
	
   	val = value(id);
	if (val != d || ((id & initial) != (1 << d))) {
		throw std::runtime_error((boost::format("val = %d, id & initial = %d, d = %d") % val % (id & initial) % d).str().c_str());
	}
	
	setValue(id, unusable); //分配成功的节点需要标记为不可用，防止被再次分配，在memoryMap对应位置更新为12
	updateParentsAlloc(id); //分配节点完成后，其父节点的状态也需要更新，并可能引起更上一层父节点的更新，
	return id;
}

void PoolChunk::updateParentsAlloc(int id) {
	while (id > 1) {
		int parentId = id >> 1;
		uint8_t val1 = value(id);
		uint8_t val2 = value(id ^ 1);
		uint8_t val = val1 < val2 ? val1 : val2;
		setValue(parentId, val);
		id = parentId;
	}
}

void PoolChunk::updateParentsFree(int id) {
    int logChild = depth(id) + 1;
    while (id > 1) {
        int parentId = id >> 1;
        uint8_t val1 = value(id);
        uint8_t val2 = value(id ^ 1);
        logChild -= 1; // in first iteration equals log, subsequently reduce 1 from logChild as we traverse up
        if (val1 == logChild && val2 == logChild) {
            setValue(parentId, (uint8_t) (logChild - 1));
        } else {
            uint8_t val = val1 < val2 ? val1 : val2;
            setValue(parentId, val);
        }
        id = parentId;
    }
}

int PoolChunk::runLength(int id) {
	// represents the size in #bytes supported by node 'id' in the tree
	// 得到节点对应可分配的字节，1号节点为16MB-ChunkSize，2048节点为8KB-PageSize
	return 1 << log2ChunkSize - depth(id);
}

int PoolChunk::runOffset(int id) {
	// represents the 0-based offset in #bytes from start of the byte-array chunk
	//得到节点在chunk底层的字节数组中的偏移量
	int shift = id ^ ( 1 << depth(id) );//2048 ^ 1 << 11
	return shift * runLength(id);
}

int PoolChunk::numberOfLeadingZeros(unsigned int i) {
	// HD, Figure 5-6
	if (i == 0) return 32;
	unsigned int n = 1;
	if (i >> 16 == 0) { n += 16; i <<= 16; }
	if (i >> 24 == 0) { n +=  8; i <<=  8; }
	if (i >> 28 == 0) { n +=  4; i <<=  4; }
	if (i >> 30 == 0) { n +=  2; i <<=  2; }
	n -= i >> 31;
	return n;
}

int PoolChunk::log2(int val) {
	return 31 - numberOfLeadingZeros(val);
}

PoolSubpage** PoolChunk::newSubpageArray(int size) {
	return new PoolSubpage*[size];
}

}//namespace net
}//namespace comm
