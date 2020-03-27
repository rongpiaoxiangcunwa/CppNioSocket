#ifndef COMM_BUFFER_POOL_SUB_PAGE_HPP
#define COMM_BUFFER_POOL_SUB_PAGE_HPP
#include <iostream>
#include "CharBuf.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
using boost::shared_ptr; 

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif 
namespace comm { 
namespace buffer {

class PoolChunk;

class PoolSubpage {
public:
	PoolSubpage(int pageSize);
	PoolSubpage(PoolSubpage *head, PoolChunk *chunk, int memoryMapIdx, int runOffset, int pageSize, int elemSize); 
	~PoolSubpage();
	
	void init(PoolSubpage* head, int elemSize);
	bool doNotDestroy() const { return doNotDestroy_; }
	/**
	* Return the number of maximal elements that can be allocated out of the sub-page.
	*/
	int maxNumElements() const { 
		if (!chunk_) return 0;
		return maxNumElems_; 
	}

	/**
	* Return the number of available elements to be allocated.
	*/
	int numAvailable() const { 
		if (!chunk_) return 0;
		return numAvail_; 
	}

	/**
	* Return the size (in bytes) of the elements that will be allocated.
	*/
	int elementSize() const {
		if (!chunk_) return 0;
		return elemSize_; 
	}

	/**
	* Return the size (in bytes) of this page.
	*/
	int pageSize() const  { return pageSize_; }

	/**
     * Returns the bitmap index of the subpage allocation.
     */
	long allocate();

	/**
     * @return {@code true} if this subpage is in use.
     *         {@code false} if this subpage is not used by its chunk and thus it's OK to be released.
     */
	bool free(PoolSubpage* head, int bitmapIdx);

	void destroy();
	
private:
	void init();
	void addToPool(PoolSubpage* head);
	int getNextAvail();
	int findNextAvail();
	int findNextAvail0(int i, long bits);
	void removeFromPool();
	
	long toHandle(int bitmapIdx) {
    	return 0x4000000000000000L | (long) bitmapIdx << 32 | memoryMapIdx_;
    }
	
	void setNextAvail(int bitmapIdx) {
        nextAvail_ = bitmapIdx;
    }

public:
	PoolChunk* chunk_;
	PoolSubpage* prev_;
    PoolSubpage* next_;
	
private:
    int memoryMapIdx_;// ��ǰpage��chunk.memoryMap �е� id
    int runOffset_;// ��ǰpage��chunk.memory��ƫ����
    int pageSize_;
    long *bitmap_;//ͨ����ÿһ��������λ�ı�����޸�һ���ڴ��ռ��״̬
	int bitmapLength_;
    bool doNotDestroy_;
    int elemSize_;// ��page�зֺ�ÿһ�εĴ�С
   	int maxNumElems_;// ��page�����Ķ�����
   	int nextAvail_;// ��һ�����õ�λ��
    int numAvail_;// ���õĶ�����
};

}//namespace net
}//namespace comm

#endif
