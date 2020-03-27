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
	//�ӵ�d�㿪ʼ�ڵ�ƥ��
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
    int id = allocateNode(d); //�������ҵ�ƥ���Ҷ�ӽڵ�
    if (id < 0) {
    	return id;// Ҷ�ӽڵ�ȫ���������
    }
	
    freeBytes_ -= pageSize;
    int _subpageIdx = subpageIdx(id); // �õ�Ҷ�ӽڵ��ƫ����������0��ʼ����2048-0,2049-1,...
    PoolSubpage* subpage = subpages[_subpageIdx];
    if (!subpage) {
		/*
		�½����ʼ��subpage�����뵽chunk��subpages���飬
		ͬʱ��subpage���뵽arena��subpage˫�������У������ɷ���������ڴ�
		*/
        subpage = new PoolSubpage(head, this, id, runOffset(id), pageSize, normCapacity);
        subpages[_subpageIdx] = subpage;
    } else {
	    /*
	    subpage��ʼ����������ڴ棬��һ��ʱ����subpage������ڴ��ͷŲ���arena��˫��������ɾ����
	    ��ʱsubpage��Ϊnull�����ٴ��������ʱ��ֻ��Ҫ����init()������뵽areana��˫�������м���
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
		id <<= 1; //ƥ���ӽڵ�
		val = value(id);
		if (val > d) { //�ӽڵ㱻���䣬����ʣ��ڵ���ڴ��С��������ʱ��Ҫ���ֵܽڵ��ϼ�������
			id ^= 1;
			val = value(id);
		}
	}
	
   	val = value(id);
	if (val != d || ((id & initial) != (1 << d))) {
		throw std::runtime_error((boost::format("val = %d, id & initial = %d, d = %d") % val % (id & initial) % d).str().c_str());
	}
	
	setValue(id, unusable); //����ɹ��Ľڵ���Ҫ���Ϊ�����ã���ֹ���ٴη��䣬��memoryMap��Ӧλ�ø���Ϊ12
	updateParentsAlloc(id); //����ڵ���ɺ��丸�ڵ��״̬Ҳ��Ҫ���£��������������һ�㸸�ڵ�ĸ��£�
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
	// �õ��ڵ��Ӧ�ɷ�����ֽڣ�1�Žڵ�Ϊ16MB-ChunkSize��2048�ڵ�Ϊ8KB-PageSize
	return 1 << log2ChunkSize - depth(id);
}

int PoolChunk::runOffset(int id) {
	// represents the 0-based offset in #bytes from start of the byte-array chunk
	//�õ��ڵ���chunk�ײ���ֽ������е�ƫ����
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
