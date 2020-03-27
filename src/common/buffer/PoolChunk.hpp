#ifndef COMM_BUFFER_POOL_CHUNK_HPP
#define COMM_BUFFER_POOL_CHUNK_HPP

#include "PoolSubpage.hpp"

namespace comm { 
namespace buffer {
	class PoolArena;
	class PoolChunkList;
	class PooledCharBuf;
	/*
		为了能够简单的操作内存，必须保证每次分配到的内存是连续的。
		底层的内存分配和回收管理主要由PoolChunk实现，其内部维护一棵平衡二叉树memoryMap，
		所有子节点管理的内存也属于其父节点
	*/
	class PoolChunk 
	{
	public:
		static int numberOfLeadingZeros(unsigned int i);
		
		PoolChunk(PoolArena* _arena,  int _chunkSize,  int _pageSize, int _pageShifts, int _maxOrder, int _offset) 
		{
			init();
			arena = _arena;
			chunkSize_ = _chunkSize; //16Mb
			pageSize = _pageSize; //8192
			pageShifts = _pageShifts; //13 因为只有当申请内存大小大于2^13（8192）时才会使用方法allocateRun分配内存。
			maxOrder = _maxOrder; //11
			offset = _offset;
			unusable = maxOrder + 1;
			freeBytes_ = _chunkSize;
			memory = new char[chunkSize_]; //16Mb
			memset(memory, 0x00, chunkSize_);
			log2ChunkSize = log2(chunkSize_);
	       	maxSubpageAllocs = 1 << maxOrder; // 2048
	       	
	       	std::cout << "[TID#" << gettid() << "] " << __FUNCTION__ << " chunkSize_:" << chunkSize_ 
				<< " pageSize:"  << pageSize << " pageShifts:" << pageShifts  << " maxOrder:" << maxOrder  << " offset:" << offset << std::endl;
			
			memoryMap = new uint8_t[maxSubpageAllocs << 1];
        	depthMap = new uint8_t[maxSubpageAllocs << 1];
			int memoryMapIndex = 1;
			for (int d = 0; d <= maxOrder; ++d) { // move down the tree one level at a time
				int depth = 1 << d;
				for (int p = 0; p < depth; ++p) {
					// in each level traverse left to right and set value to the depth of subtree
					memoryMap[memoryMapIndex] = (uint8_t)d;
					depthMap[memoryMapIndex] = (uint8_t)d;
					memoryMapIndex++;
				}
			}
			
			subpages = newSubpageArray(maxSubpageAllocs);
			for (int i = 0; i < maxSubpageAllocs; i++) subpages[i] = 0;
		}

		PoolChunk(PoolArena* _arena, int _size, int _offset) 
		{
			init();
			unpooled = true;
			arena = _arena;
			chunkSize_ = _size;
			offset = _offset;
			memory = new char[_size];
			unusable = (uint8_t) (maxOrder + 1);
			log2ChunkSize = log2(chunkSize_);
		}
		
		~PoolChunk() {
			delete []memory;
			delete []memoryMap;
			delete []depthMap;
			for (int i = 0; i < maxSubpageAllocs; i++ ){
				delete subpages[i];
				subpages[i] = 0;
			}
			delete []subpages;
		}
		
		int usage();
		int chunkSize() const { return chunkSize_; }
		int freeBytes() const { return freeBytes_; };
		bool allocate(PooledCharBuf* buf, int reqCapacity, int normCapacity);
		void free(long handle);
		void destroy();
		void initBuf(PooledCharBuf* buf , long handle, int reqCapacity) ;
		void initBufWithSubpage(PooledCharBuf* buf, long handle, int reqCapacity);
		
		
	private:
		static int log2(int val);
		PoolSubpage** newSubpageArray(int size);
		long allocateRun(int normCapacity);
		long allocateSubpage(int normCapacity);
		int allocateNode(int d);
		void updateParentsAlloc(int id);
		void updateParentsFree(int id);
		int runLength(int id);
		int runOffset(int id);
		void initBufWithSubpage(PooledCharBuf* buf, long handle, int bitmapIdx, int reqCapacity);
		uint8_t value(int id) { return memoryMap[id]; }
		void setValue(int id, uint8_t val) { memoryMap[id] = val; }
		uint8_t depth(int id) { return depthMap[id];}
		
		int subpageIdx(int memoryMapIdx) {
			/**
				得到第11层叶子节点的偏移索引，= id - 2048
				获取 memoryMapIdx 在subPage中对应的下标- memoryMapIdx 从2048开始， 
				与maxSubpageAllocs亦或之后得到的就是其下标
			*/
        	return memoryMapIdx ^ maxSubpageAllocs;
		}

		//二叉树中的分配的节点下标
		int memoryMapIdx(long handle) {
    		return (int) handle;
		}

		//subpage 分配的具体位置(bitmap对应的内存段)
		int bitmapIdx(long handle) {
    		return (int) (handle >> 32);
		}

		void init() {
			unpooled = false;
			arena = 0;
			memory = 0;
			chunkSize_ = 0;
			pageSize = 0;
			pageShifts = 0;
			maxOrder = 0;
			offset = 0;
			unusable = 0;
			freeBytes_ = 0;
			memoryMap = 0;
			depthMap = 0;
			subpages = 0;
			log2ChunkSize = 0;
			maxSubpageAllocs = 0;
			parent = 0;
			prev = next = 0;
		}
		
	public:
		bool unpooled;
		PoolArena* arena;
		char *memory;
		int chunkSize_;//default chunksize 16Mb
		int pageSize;
		int pageShifts;
		int maxOrder;
		int offset;
		uint8_t unusable;
		int freeBytes_;
		uint8_t *memoryMap;
		uint8_t *depthMap;
		PoolSubpage **subpages;
		int log2ChunkSize;
		int maxSubpageAllocs;
		PoolChunkList *parent;
		PoolChunk *prev;
		PoolChunk *next;
	};
}//namespace net
}//namespace comm
#endif
