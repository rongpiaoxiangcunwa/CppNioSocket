#ifndef _AbstractUnpooledSlicedCharBuf_hpp_
#define _AbstractUnpooledSlicedCharBuf_hpp_

#include "AbstractDerivedCharBuf.hpp"

namespace comm { 
namespace buffer {
	class AbstractUnpooledSlicedCharBuf : public AbstractDerivedCharBuf {
	public:
		AbstractUnpooledSlicedCharBuf(const shared_ptr<CharBuf>& buffer, int index, int length) 
		: AbstractDerivedCharBuf(length), buffer_(buffer), adjustment_(index) {
			writerIndex(length);
		}
		
		virtual int capacity() const { 
			return maxCapacity(); 
		}
		
		virtual void capacity(int newCapacity) {
			throw std::runtime_error("Unsupported capacity(int newCapacity) in sliced buffer");
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
			checkRangeBounds(index, length, capacity());
			return unwrap()->slice(idx(index), length);
		}

		virtual CharBuf* unwrap() const {
			return buffer_.get();
		}

		virtual char* array() {
			return unwrap()->array();
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
		shared_ptr<CharBuf> buffer_;
		int adjustment_;
	};
}//namespace net
}//namespace comm

#endif