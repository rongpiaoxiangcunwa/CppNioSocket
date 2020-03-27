#ifndef COMM_BUFFER_POOL_CHUNK_LIST_HPP
#define COMM_BUFFER_POOL_CHUNK_LIST_HPP

#include "PoolChunk.hpp"
#include <algorithm>

namespace comm { 
namespace buffer {
	
class PoolArena;
class PooledCharBuf;

class PoolChunkList {
public:
	PoolChunkList(PoolArena* arena, PoolChunkList* nextList, int minUsage, int maxUsage, int chunkSize);
	/**
	 * Return the minimum usage of the chunk list before which chunks are promoted to the previous list.
	 */
	int minUsage() const { return std::max(1, minUsage_); }

	/**
	 * Return the maximum usage of the chunk list after which chunks are promoted to the next list.
	 */
	int maxUsage() const { return std::min(maxUsage_, 100);}

	void prevList(PoolChunkList* prevList) { prevList_ = prevList; }

	bool allocate(shared_ptr<PooledCharBuf>& buf, int reqCapacity, int normCapacity);

	bool free(PoolChunk* chunk, long handle);

	void add(PoolChunk* chunk);
	
	bool move(PoolChunk* chunk);
	
private:
	int calculateMaxCapacity(int minUsage, int chunkSize);
	void remove(PoolChunk* cur);
	void add0(PoolChunk* chunk);
	bool move0(PoolChunk* chunk);
	
public:
	PoolChunkList* nextList_;
	 // This is only update once when create the linked like list of PoolChunkList in PoolArena constructor.
    PoolChunkList *prevList_;
private:
	PoolArena* arena_;
    int minUsage_;
    int maxUsage_;
   	int maxCapacity_;
    PoolChunk *head_;
};

}//namespace net
}//namespace comm

#endif
