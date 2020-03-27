#ifndef COMM_BUFFER_POOLARENA_HPP
#define COMM_BUFFER_POOLARENA_HPP

#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>
#include "PoolThreadCache.hpp"
#include "PoolSubpage.hpp"
#include "PoolChunkList.hpp"

using boost::shared_ptr;

namespace comm { namespace buffer {
	
class PooledCharBufAllocator;

class PoolArena {
public:
	enum SizeClass {
		Tiny,
		Small,
		Normal
    };

	PoolArena(PooledCharBufAllocator* parent, int pageSize, int maxOrder,  int pageShifts,  int chunkSize);
	~PoolArena();
	
	shared_ptr<PooledCharBuf> allocate(int reqCapacity, int maxCapacity);

	void free(PoolChunk *chunk, long handle, int normCapacity, PoolThreadCache* cache);
	
	PoolSubpage* findSubpagePoolHead(int elemSize);
	
	PooledCharBufAllocator* parent() { return parent_; }

	void destroyChunk(PoolChunk* chunk);

	void reallocate(shared_ptr<PooledCharBuf>& buf, int newCapacity, bool freeOldMemory);

	static int tinyIdx(int normCapacity) { return normCapacity >> 4;}
	
	static int smallIdx(int normCapacity) {
        int tableIdx = 0;
        int i = normCapacity >> 10; // / 1024
        while (i != 0) {
            i >>= 1;
            tableIdx++;
        }
        return tableIdx;
    }

	static bool isTiny(int normCapacity) { return (normCapacity & 0xFFFFFE00) == 0;}

	bool isTinyOrSmall(int normCapacity) { return (normCapacity & ~(pageSize_ - 1)) == 0;}
	
private:
	shared_ptr<PooledCharBuf> newCharBuf(int maxCapacity);
	void allocate(shared_ptr<PooledCharBuf>& buf, int reqCapacity) ;
	int normalizeCapacity(int reqCapacity);
	void allocateNormal(shared_ptr<PooledCharBuf>& buf, int reqCapacity, int normCapacity);
	PoolChunk* newChunk(int pageSize, int maxOrder, int pageShifts, int chunkSize);
	void allocateHuge(shared_ptr<PooledCharBuf>& buf, int reqCapacity); 	
	PoolChunk* newUnpooledChunk(int capacity);
	SizeClass sizeClass(int normCapacity);

public:
	void freeChunk(PoolChunk* chunk, long handle, SizeClass sizeClass);	

	static const int numTinySubpagePools = 32; 
private:
	boost::recursive_mutex mutex_;
	PooledCharBufAllocator *parent_;
public:
	int maxOrder_;
	int pageSize_;
	int pageShifts_;
	int chunkSize_;
	int numSmallSubpagePools_;
	/*
		分配长度小于512字节的内存，默认为32. 
   		因为内存分配最小为16，每次增加16，直到512，区间[16，512)一共有32个不同值
   	*/
   	PoolSubpage** tinySubpagePools_;
	/*
		用于分配大于等于512字节且小于8192字节的内存，每次增加2倍,默认长度为4；
	*/
    PoolSubpage** smallSubpagePools_;
private:
	PoolChunkList* qInit_;//存储内存利用率0-25%的chunk
	PoolChunkList* q000_;//存储内存利用率1-50%的chunk
	PoolChunkList* q025_;//存储内存利用率25-75%的chunk
   	PoolChunkList* q050_;//存储内存利用率50-100%的chunk 
	PoolChunkList* q075_;//存储内存利用率75-100%的chunk
	PoolChunkList* q100_;//存储内存利用率100%的chunk
	PoolThreadCache* cache_;
};

}}//namespace comm
#endif
