#include "UnpooledHeapCharBuf.hpp"

#include <string.h>
#include <stdlib.h>
#include "HeapCharBufUtil.hpp"

namespace comm {
namespace buffer {
	
	void UnpooledHeapCharBuf::capacity(int newCapacity) {
        int oldCapacity = capacity_;
		if (oldCapacity == newCapacity) return ;
		capacity_ = newCapacity;
		if( newCapacity < oldCapacity) {
			int _readerIndex = readerIndex();
			if (_readerIndex < newCapacity) {
				int _writerIndex = writerIndex();
				if (_writerIndex > newCapacity) {
				    _writerIndex = newCapacity;
				    writerIndex(newCapacity);
				}
			} else {
				readerIndex_ = writerIndex_ = newCapacity;
			}
		}
		array_ = (char*)realloc(array_, newCapacity);
	}

	void UnpooledHeapCharBuf::setChars(int index, const char* src, int srcIndex, int length)  {
		checkIndex(index, length);
		HeapCharBufUtil::getChars(src, srcIndex, array_, index, length);
	}
	
	void UnpooledHeapCharBuf::setChars(int index, const CharBuf& src, int srcIndex, int length) {
		src.getChars(srcIndex, array_, index, length);
	}

	void UnpooledHeapCharBuf::getChars(int index, char* dst, int dstIndex, int length) const  {
		checkIndex(index, length);
		HeapCharBufUtil::getChars(array_, index, dst, dstIndex, length);
	}
		
	char* UnpooledHeapCharBuf::allocateArray(int initialCapacity) { 
		return new char[initialCapacity]; 
	}

	void UnpooledHeapCharBuf::deepCopy(const UnpooledHeapCharBuf& rhs) {
		readerIndex_ = rhs.readerIndex_;
		writerIndex_ = rhs.writerIndex_;
		markedReaderIndex_ = rhs.markedReaderIndex_;
		markedWriterIndex_ = rhs.markedWriterIndex_;
		maxCapacity_ = rhs.maxCapacity_;
		capacity_ = rhs.capacity_;
		delete []array_;
		array_ = 0;
		if (capacity_ > 0 ) {
			array_ = new char[capacity_];
			memcpy(array_, rhs.array_, capacity_);
		}
	}
	
	void UnpooledHeapCharBuf::_setChar(int index, char v) {
		HeapCharBufUtil::setChar(array_, index, v);
	}

	void UnpooledHeapCharBuf::_setChars(int index, const char* src, size_t len) {
		HeapCharBufUtil::setChars(array_, index, src, len);
	}
	
	void UnpooledHeapCharBuf::_setShort(int index, int16_t value)  {
		HeapCharBufUtil::setShort(array_, index, value);
	}

	void UnpooledHeapCharBuf::_setUShort(int index, uint16_t value)  {
		HeapCharBufUtil::setUShort(array_, index, value);
	}

	void UnpooledHeapCharBuf::_setInt(int index, int32_t value) {
		HeapCharBufUtil::setInt(array_, index, value);
	}

	void UnpooledHeapCharBuf::_setUInt(int index, uint32_t value) {
		HeapCharBufUtil::setUInt(array_, index, value);
	}
	
	void UnpooledHeapCharBuf::_setLong(int index, int64_t value)  {
		HeapCharBufUtil::setLong(array_, index, value);
	}

	void UnpooledHeapCharBuf::_setULong(int index, uint64_t value) {
		HeapCharBufUtil::setULong(array_, index, value);
	}

	char UnpooledHeapCharBuf::_getChar(int index) {
		return HeapCharBufUtil::getChar(array_, index);
	}

	int16_t UnpooledHeapCharBuf::_getShort(int index) {
		return HeapCharBufUtil::getShort(array_, index);
	}

	uint16_t UnpooledHeapCharBuf::_getUShort(int index) {
		return HeapCharBufUtil::getUShort(array_, index);
	}

	int32_t UnpooledHeapCharBuf::_getInt(int index) {
		return HeapCharBufUtil::getInt(array_, index);
	}
	
	uint32_t UnpooledHeapCharBuf::_getUInt(int index) {
		return HeapCharBufUtil::getUInt(array_, index);
	}
	
	int64_t UnpooledHeapCharBuf::_getLong(int index) {
		return HeapCharBufUtil::getLong(array_, index);
	}
	
	uint64_t UnpooledHeapCharBuf::_getULong(int index) {
		return HeapCharBufUtil::getULong(array_, index);
	}
	
	void UnpooledHeapCharBuf::_getString(int index, string &dst, size_t len) {
		HeapCharBufUtil::getString(array_, index, dst, len);
	}
	
	
}//namespace buffer
}//namespace comm