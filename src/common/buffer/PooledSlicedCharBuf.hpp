#ifndef COMM_BUFFER_POOLED_SLICED_CHARBUF_HPP
#define COMM_BUFFER_POOLED_SLICED_CHARBUF_HPP

#include "AbstractPooledDerivedCharBuf.hpp"
#include <boost/format.hpp>

namespace comm { namespace buffer {
	
	class PooledSlicedCharBuf : public AbstractPooledDerivedCharBuf, 
					            public boost::enable_shared_from_this<PooledSlicedCharBuf> {
	public:
		static void checkSliceOutOfBounds(int index, int length, CharBuf* buffer) {
	        if (isOutOfBounds(index, length, buffer->capacity())) {
	            throw std::runtime_error((boost::format("buffer .slice(%1%, %2%)") % index % length).str()); 
	        }
	    }
		
		static bool isOutOfBounds(int index, int length, int capacity) {
	        return (index | length | (index + length) | (capacity - (index + length))) < 0;
	    }

		static shared_ptr<PooledSlicedCharBuf> newInstance(const shared_ptr<AbstractCharBuf>& unwrapped, const shared_ptr<CharBuf> &wrapped, int index, int length) {
	        checkSliceOutOfBounds(index, length, unwrapped.get());
	        return shared_ptr<PooledSlicedCharBuf>( new PooledSlicedCharBuf(unwrapped, wrapped, index, length));
	    }

		PooledSlicedCharBuf(const shared_ptr<AbstractCharBuf>& unwrapped, const shared_ptr<CharBuf> &wrapped, int adjustment, int length) {
			init(unwrapped, wrapped, 0, length, length);
			markedReaderIndex_ = markedWriterIndex_ = 0;
			adjustment_ = adjustment;
		}
		
		virtual shared_ptr<CharBuf> get_shared_ptr() {
			return shared_from_this();
		}

		virtual int capacity() const { return maxCapacity(); }

		virtual void capacity(int newCapacity) {
			throw std::runtime_error("Unsupported capacity(int newCapacity) in PooledSlicedCharBuf");
		}

		virtual void setChars(int index, const char* src, int srcIndex, int length)  {
			checkRangeBounds(index, length, capacity());
       		unwrap()->setChars(idx(index), src, srcIndex, length);
		}
		
		virtual void setChars(int index, const CharBuf& src, int srcIndex, int length) {
			checkRangeBounds(index, length, capacity());
        	unwrap()->setChars(idx(index), src, srcIndex, length);
		}
		
		virtual void getChars(int index, char* dst, int dstIndex, int length) const {
			checkRangeBounds(index, length, capacity());
        	unwrap()->getChars(idx(index), dst, dstIndex, length);
		}
		
		virtual shared_ptr<CharBuf> slice(int index, int length) {
			//todo
			throw std::runtime_error("Unsupported slice(int newCapacity, int length) in PooledSlicedCharBuf");
		}
		
		virtual shared_ptr<CharBuf> retainedSlice(int index, int length) {
			//todo
			throw std::runtime_error("Unsupported retainedSlice(int index, int length) in PooledSlicedCharBuf");
		}
		
	protected:
		virtual void _setChar(int index, char value)  {
			unwrap()->setChar(idx(index), value);
		}
		
		virtual void _setShort(int index, int16_t value)  {
			unwrap()->setShort(idx(index), value);
		}
		
		virtual void _setUShort(int index, uint16_t value)  {
			unwrap()->setUShort(idx(index), value);
		}
		
		virtual void _setInt(int index, int32_t value)  {
			unwrap()->setInt(idx(index), value);
		}
		
		virtual void _setUInt(int index, uint32_t value)  {
			unwrap()->setUInt(idx(index), value);
		}
		
		virtual void _setLong(int index, int64_t value)  {
			unwrap()->setLong(idx(index), value);
		}
		
		virtual void _setULong(int index, uint64_t value)  {
			unwrap()->setULong(idx(index), value);
		}
		
		virtual void _setChars(int index, const char* src, size_t len)  {
			unwrap()->setChars(idx(index), src, 0, len);
		}

		virtual char _getChar(int index) {
			return unwrap()->getChar(idx(index));
		}
		
		virtual int16_t _getShort(int index) {
			return unwrap()->getShort(idx(index));
		}
		
		virtual uint16_t _getUShort(int index){
			return unwrap()->getUShort(idx(index));
		}
		
		virtual int32_t _getInt(int index) {
			return unwrap()->getInt(idx(index));
		}
		
		virtual uint32_t _getUInt(int index) {
			return unwrap()->getUInt(idx(index));
		}
		
		virtual int64_t _getLong(int index) {
			return unwrap()->getLong(idx(index));
		}
		
		virtual uint64_t _getULong(int index) {
			return unwrap()->getULong(idx(index));
		}
		
		virtual void _getString(int index, string &dst, size_t len) {
			unwrap()->getString(idx(index), dst, len);
		}
		
		int idx(int index) const { 
			return index + adjustment_; 
		}
		
	private:
		int adjustment_;
	};
}}//namespace comm
#endif
