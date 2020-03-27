#ifndef COMM_BUFFER_POOLED_CHARBUF_HPP
#define COMM_BUFFER_POOLED_CHARBUF_HPP

#include <boost/function.hpp>
#include "AbstractReferenceCountedCharBuf.hpp"

namespace comm {
namespace buffer {
class PoolChunk;
class PoolThreadCache;
class CharBufAllocator;

class PooledCharBuf : public AbstractReferenceCountedCharBuf
{
public:
	typedef boost::function< void(const boost::shared_ptr<CharBuf>&) > RECYCLER;

	PooledCharBuf(int _maxCapacity) : AbstractReferenceCountedCharBuf(_maxCapacity) {
		init();
	}
	
	PooledCharBuf(int initialCapacity, int _maxCapacity, const RECYCLER& recycler = RECYCLER()) 
	: recycler_(recycler), AbstractReferenceCountedCharBuf(_maxCapacity) {
		init();
	}

	void reuse(int maxCapacity) {
		maxCapacity_ = maxCapacity;
		resetRefCnt();
		readerIndex_ = writerIndex_ = markedWriterIndex_ = markedReaderIndex_ = 0;
	}
	
	virtual int capacity() const { return length_; }
	virtual void capacity(int newCapacity);
	virtual CharBuf* unwrap() const { return 0; }
	virtual shared_ptr<CharBuf> retainedSlice(int index, int length);

	PoolThreadCache* cache() const { return cache_; }
	void init(PoolChunk* chunk, long handle, int offset, int length, int maxLength, PoolThreadCache* cache);
	void initUnpooled(PoolChunk* chunk, int length);
	void trimIndicesToCapacity(int newCapacity);
	
protected:
	virtual void deallocate();
	void init0(PoolChunk* chunk, long handle, int offset, int length, int maxLength, PoolThreadCache* cache);
	void init();
	void checkNewCapacity(int newCapacity);
	
public:
	PoolChunk* chunk_;
    long handle_;
	int offset_;
    int length_;
    int maxLength_;
    char* memory_;
	RECYCLER recycler_;
    CharBufAllocator* allocator_;
	PoolThreadCache* cache_;
};

}//namespace buffer
}//namespace comm

#endif