#ifndef COMM_BUFFER_POOLED_HEAP_CHARBUF_HPP
#define COMM_BUFFER_POOLED_HEAP_CHARBUF_HPP

#include <boost/thread.hpp>
#include <ObjectQueue.hpp>
#include "PooledCharBuf.hpp"

namespace comm {
namespace buffer {

class PooledHeapCharBuf : public PooledCharBuf, 
						  public boost::enable_shared_from_this<PooledHeapCharBuf>
{
public:
	static ObjectQueue< shared_ptr<PooledHeapCharBuf> > cachedCharBufs;
	
	static void recycle(const boost::shared_ptr<CharBuf>& obj) {
		shared_ptr<PooledHeapCharBuf> buf = boost::dynamic_pointer_cast<PooledHeapCharBuf>(obj);
		cachedCharBufs.add(buf);
		int n = cachedCharBufs.size();
		if (n > 10000) std::cout <<"[TID#" << gettid() << "] PooledHeapCharBuf::recycle cached CharBuf is " << n  << std::endl; 
	}
	
	static shared_ptr<PooledHeapCharBuf> newInstance(int _maxCapacity) {
		#if 0
		shared_ptr<PooledHeapCharBuf> buf = cachedCharBufs.take(0);
		if (!buf) {
			buf = shared_ptr<PooledHeapCharBuf>(new PooledHeapCharBuf(_maxCapacity, _maxCapacity, boost::bind(&recycle, _1)));
		}
		#endif
		shared_ptr<PooledHeapCharBuf> buf = shared_ptr<PooledHeapCharBuf>(new PooledHeapCharBuf(_maxCapacity));
		//buf->reuse(_maxCapacity);
		return buf;
	}

	//添加 RECYCLER 的支持
	PooledHeapCharBuf(int _maxCapacity) : PooledCharBuf(_maxCapacity) {
	}
	
	PooledHeapCharBuf(int initialCapacity, int _maxCapacity, const RECYCLER &recycler) 
	:  PooledCharBuf(initialCapacity, _maxCapacity, recycler) {
	}

	~PooledHeapCharBuf() { 
		#ifdef DEBUG_LOG
		std::cout <<"[TID#" << gettid() << "] PooledHeapCharBuf::~PooledHeapCharBuf " << this  << std::endl; 
		#endif
	}
	
	
	virtual void setChars(int index, const char* src, int srcIndex, int length) ;
	virtual void setChars(int index, const CharBuf& src, int srcIndex, int length);
	virtual void getChars(int index, char* dst , int dstIndex, int length) const;
	
	virtual shared_ptr<CharBuf> get_shared_ptr() { return shared_from_this();}
	
	virtual char* array() { return memory_ + offset_; }
	
protected:
	virtual void _setChar(int index, char v);
	virtual void _setChars(int index, const char* src, size_t len);
	virtual void _setShort(int index, int16_t value) ;
	virtual void _setUShort(int index, uint16_t value);
	virtual void _setInt(int index, int32_t value);
	virtual void _setUInt(int index, uint32_t value);
	virtual void _setLong(int index, int64_t value) ;
	virtual void _setULong(int index, uint64_t value) ;
	

	virtual char _getChar(int index);
	virtual int16_t _getShort(int index);
	virtual uint16_t _getUShort(int index);
	virtual int32_t _getInt(int index);
	virtual uint32_t _getUInt(int index);
	virtual int64_t _getLong(int index);
	virtual uint64_t _getULong(int index);
	virtual void _getString(int index, string &dst, size_t len);
		
protected:
	int idx(int index) const { return index + offset_; }
};

}//namespace buffer
}//namespace comm

#endif
