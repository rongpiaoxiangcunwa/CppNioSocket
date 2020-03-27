#ifndef _UnpooledHeapCharBuf_hpp_
#define _UnpooledHeapCharBuf_hpp_

#include "AbstractReferenceCountedCharBuf.hpp"

namespace comm {
namespace buffer {
	class PooledHeapCharBuf;
	
	class UnpooledHeapCharBuf : 
		public AbstractReferenceCountedCharBuf, 
		public boost::enable_shared_from_this<UnpooledHeapCharBuf> 
	{
	public:		
		friend class PooledHeapCharBuf;
		UnpooledHeapCharBuf() : AbstractReferenceCountedCharBuf(CHAR_BUF_MAX_CAPACITY), capacity_(8192), array_(0) {
			array_ = allocateArray(capacity_);
		}
		
		UnpooledHeapCharBuf(int initialCapacity) : AbstractReferenceCountedCharBuf(CHAR_BUF_MAX_CAPACITY), capacity_(initialCapacity), array_(0) {
			array_ = allocateArray(capacity_);
		}

		UnpooledHeapCharBuf(int initialCapacity, int _maxCapacity) : AbstractReferenceCountedCharBuf(_maxCapacity), capacity_(initialCapacity), array_(0) {
			array_ = allocateArray(capacity_);
		}

		UnpooledHeapCharBuf(const UnpooledHeapCharBuf& rhs) : AbstractReferenceCountedCharBuf(rhs.maxCapacity_) , array_(0) {
			deepCopy(rhs);
		}

		UnpooledHeapCharBuf& operator=(const UnpooledHeapCharBuf& rhs) {
			if (this == &rhs) return *this;
			deepCopy(rhs);
			return *this;
		}

		~UnpooledHeapCharBuf() {
			delete []array_;
			array_ = 0;
		}

		virtual int capacity() const { return capacity_; }
		virtual void capacity(int newCapacity);
		virtual void setChars(int index, const char* src, int srcIndex, int length) ;
		virtual void setChars(int index, const CharBuf& src, int srcIndex, int length);
		virtual void getChars(int index, char* dst , int dstIndex, int length) const;
		
		virtual shared_ptr<CharBuf> get_shared_ptr() {
			return shared_from_this();
		}

		virtual CharBuf* unwrap() const { return 0; }
		virtual char* array() { return array_; }
		
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
		
		char* allocateArray(int initialCapacity);
		
	private:
		void deepCopy(const UnpooledHeapCharBuf& rhs) ;
		
	private:
		int capacity_;
		char* array_;
	};
	
}//namespace buffer
}//namespace comm

#endif
