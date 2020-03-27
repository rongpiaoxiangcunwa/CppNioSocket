#include <algorithm>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include "CommUtil.hpp"
#include "AbstractCharBuf.hpp"
#include "UnpooledSlicedCharBuf.hpp"

namespace comm {
namespace buffer {
	void AbstractCharBuf::readerIndex(int index) {
		checkIndexBounds(index, writerIndex_, maxCapacity());
		readerIndex_ = index;
	}

	void AbstractCharBuf::writerIndex(int index) {
		checkIndexBounds(readerIndex_, index, maxCapacity());
		writerIndex_ = index;
	}

	void AbstractCharBuf::writeBool(bool v) { 
		writeChar(v ? 1 : 0); 
	}
	
	void AbstractCharBuf::writeChar(char v) {
		ensureWritable( sizeof(char) );
		_setChar(writerIndex_++, v);
	}

	void AbstractCharBuf::writeChars(const char *src, int length) {
		ensureWritable( length );
		_setChars( writerIndex_, src, length );
		writerIndex_ += length;
	}

	void AbstractCharBuf::writeChars(CharBuf &src) {
		writeChars(src, src.readableBytes());
	}

	void AbstractCharBuf::writeChars(CharBuf &src, int length) {
		checkReadableBounds(src, length);
		writeChars(src, src.readerIndex(), length);
		src.readerIndex(src.readerIndex() + length);
	}

	void AbstractCharBuf::writeChars(CharBuf &src, int srcIndex, int length) {
		ensureWritable(length);
		setChars(writerIndex_, src, srcIndex, length);
		writerIndex_ += length;
	}
		
	void AbstractCharBuf::writeShort(int16_t value)  {
		ensureWritable( sizeof(int16_t) );
		_setShort(writerIndex_, value);
        writerIndex_ += sizeof(int16_t);
	}
	
	void AbstractCharBuf::writeUShort(uint16_t value)  {
		ensureWritable( sizeof(uint16_t) );
		_setUShort(writerIndex_, value);
        writerIndex_ += sizeof(uint16_t);
	}

	void AbstractCharBuf::writeInt(int32_t value) {
		ensureWritable( sizeof(int32_t) );
		_setInt(writerIndex_, value);
		writerIndex_ += sizeof(int32_t);
	}
	
	void AbstractCharBuf::writeUInt(uint32_t value) {
		ensureWritable( sizeof(uint32_t) );
		_setUInt(writerIndex_, value);
		writerIndex_ += sizeof(uint32_t);
	}

	void AbstractCharBuf::writeLong(int64_t value) {
		ensureWritable( sizeof(int64_t) );
		_setLong(writerIndex_, value);
		writerIndex_ += sizeof(int64_t);
	}
	
	void AbstractCharBuf::writeULong(uint64_t value) {
		ensureWritable( sizeof(uint64_t) );
		_setULong(writerIndex_, value);
		writerIndex_ += sizeof(uint64_t);
	}

	void AbstractCharBuf::writeFloat(float value) {
		int32_t t = 0;
		memcpy(&t, &value, sizeof(t));
		writeInt(t);
	}
	
	void AbstractCharBuf::writeDouble(double value)  {
		int64_t t = 0;
		memcpy(&t, &value, sizeof(t));
		writeLong(t);
	}
	
	void AbstractCharBuf::writeString(const string& value) {
		writeChars( value.data(), value.length() );
	}

	void AbstractCharBuf::setBool(int index, bool value) {
		 setChar(index, value? 1 : 0);
	}
	
	void AbstractCharBuf::setChar(int index, char value) {
		 checkIndex(index, sizeof(char));
       	 _setChar(index, value);
	}
	
	void AbstractCharBuf::setShort(int index, int16_t value)  {
		checkIndex(index, sizeof(int16_t));
       	 _setShort(index, value);
	}
	
	void  AbstractCharBuf::setUShort(int index, uint16_t value)  {
		checkIndex(index, sizeof(uint16_t));
       	 _setUShort(index, value);
	}
	
	void  AbstractCharBuf::setInt(int index, int32_t value)  {
		checkIndex(index, sizeof(int32_t));
       	 _setInt(index, value);
	}
	
	void  AbstractCharBuf::setUInt(int index, uint32_t value)  {
		checkIndex(index, sizeof(uint32_t));
       	 _setUInt(index, value);
	}
	
	void  AbstractCharBuf::setLong(int index, int64_t value)  {
		checkIndex(index, sizeof(int64_t));
       	 _setLong(index, value);
	}
	
	void  AbstractCharBuf::setULong(int index, uint64_t value) {
		checkIndex(index, sizeof(uint64_t));
       	 _setULong(index, value);
	}
	
	void  AbstractCharBuf::setFloat(int index, float value)  {
		int32_t t = 0;
		memcpy(&t, &value, sizeof(t));
		setInt(index, t);
	}
	
	void  AbstractCharBuf::setDouble(int index, double value) {
		int64_t t = 0;
		memcpy(&t, &value, sizeof(t));
		setLong(index, t);
	}
	
	void  AbstractCharBuf::setString(int index, const string& value) {
		 setChars(index, value.data(), 0, value.length() );
	}
	
	bool AbstractCharBuf::readBool() {
		return readChar() != 0;
	}
	
	char AbstractCharBuf::readChar() {
		checkReadable( sizeof(char) );
		char c = _getChar(readerIndex_);
		readerIndex_ += sizeof(char);
		return c;
	}

	void AbstractCharBuf::readChars(char* dst , int dstIndex, int length) {
		checkReadable( length );
		getChars(readerIndex_, dst, dstIndex, length);
		readerIndex_ += length;
	}
	
	int16_t AbstractCharBuf::readShort() {
		checkReadable(sizeof(int16_t));
		int16_t v = _getShort(readerIndex_) ;
		readerIndex_ += sizeof(int16_t);
		return v;
	}
	
	uint16_t AbstractCharBuf::readUShort() {
		checkReadable(sizeof(uint16_t));
		uint16_t v = _getUShort(readerIndex_);
		readerIndex_ += sizeof(uint16_t);
		return v;
	}
	
	int32_t AbstractCharBuf::readInt() {
		checkReadable(sizeof(int32_t));
		int32_t v = _getInt(readerIndex_);
		readerIndex_ += sizeof(int32_t);
		return v;
	}
	
	uint32_t AbstractCharBuf::readUInt() {
		checkReadable(sizeof(uint32_t));
		uint32_t v = _getUInt(readerIndex_);
		readerIndex_ += sizeof(uint32_t);
		return v;
	}
	
	int64_t AbstractCharBuf::readLong()  {
		checkReadable(sizeof(int64_t));
		int64_t v = _getLong(readerIndex_);
		readerIndex_ += sizeof(int64_t);
		return v;
	}
	
	uint64_t AbstractCharBuf::readULong() {
		checkReadable(sizeof(uint64_t));
		uint64_t v = _getULong(readerIndex_);
		readerIndex_ += sizeof(uint64_t);
		return v;
	}
	
	float AbstractCharBuf::readFloat() {
		int32_t i = readInt();
		float f = 0;
		memcpy(&f, &i, sizeof(float));
		return f;
	}
	
	double AbstractCharBuf::readDouble() {
		int64_t l = readLong();
		double d = 0;
		memcpy(&d, &l, sizeof(double));
		return d;
	}
	
	void AbstractCharBuf::readString(string& dst, int len) {
		checkReadable(len);
		_getString(readerIndex_, dst, len);
		readerIndex_ += len;
	}

	bool AbstractCharBuf::getBool(int index) {
		return getChar(index) != 0;
	}
	
	char AbstractCharBuf::getChar(int index) {
		checkIndex(index, sizeof(char));
		return _getChar(index);
	}
	
	int16_t AbstractCharBuf::getShort(int index) {
		checkIndex(index, sizeof(int16_t));
		return _getShort(index);
	}
	
	uint16_t AbstractCharBuf::getUShort(int index) {
		checkIndex(index, sizeof(uint16_t));
		return _getUShort(index);
	}
	
	int32_t AbstractCharBuf::getInt(int index) {
		checkIndex(index, sizeof(int32_t ));
		return _getInt(index);
	}
	
	uint32_t AbstractCharBuf::getUInt(int index) {
		checkIndex(index, sizeof(uint32_t ));
		return _getUInt(index);
	}
	
	int64_t AbstractCharBuf::getLong(int index) {
		checkIndex(index, sizeof(int64_t ));
		return _getLong(index);
	}
	
	uint64_t AbstractCharBuf::getULong(int index) {
		checkIndex(index, sizeof(uint64_t));
		return _getULong(index);
	}
	
	float AbstractCharBuf::getFloat(int index) {
		int32_t i = _getInt(index);
		float f = 0.0;
		memcpy(&f, &i, sizeof(f));
		return f;
	}
	
	double AbstractCharBuf::getDouble(int index) {
		int64_t l = _getLong(index);
		double d = 0.0;
		memcpy(&d, &l, sizeof(d));
		return d;
	}
	
	void AbstractCharBuf::getString(int index, string& dst, int len) {
		checkIndex(index, len);
		_getString(index, dst, len);
	}

	void AbstractCharBuf::skipBytes(int n) {
		checkReadable(n);
		readerIndex_ += n;
	}

	void AbstractCharBuf::discardReadBytes() {
		if (0 == readerIndex_ ) return;
		
		if (readerIndex_ != writerIndex_) {
			setChars(0, *this, readerIndex_, writerIndex_ - readerIndex_);
            writerIndex_ -= readerIndex_;
            adjustMarkers(readerIndex_);
            readerIndex_ = 0;
		} else {
			adjustMarkers(readerIndex_);
			writerIndex_ = readerIndex_ = 0;
		}
	}

	void AbstractCharBuf::discardSomeReadBytes() {
		if (readerIndex_ == 0) {
            return ;
        }

        if (readerIndex_ == writerIndex_) {
            adjustMarkers(readerIndex_);
            writerIndex_ = readerIndex_ = 0;
            return ;
        }

        if (readerIndex_ >= capacity() >> 1) {
			std::cout << "[TID#" << gettid() << "] AbstractCharBuf::discardSomeReadBytes() readerIndex_:" << readerIndex_ << " writerIndex_:"  << writerIndex_ << std::endl; 
            setChars(0, *this, readerIndex_, writerIndex_ - readerIndex_);
            writerIndex_ -= readerIndex_;
            adjustMarkers(readerIndex_);
            readerIndex_ = 0;
        }
	}

	shared_ptr<CharBuf> AbstractCharBuf::slice(int index, int length) {
		return shared_ptr<CharBuf>(new UnpooledSlicedCharBuf(get_shared_ptr(), index, length));
	}
	
	shared_ptr<CharBuf> AbstractCharBuf::retainedSlice(int index, int length) {
		 shared_ptr<CharBuf> sliceBuffer = slice(index, length); 
		 sliceBuffer->retain();
		 return sliceBuffer;
	}

	void AbstractCharBuf::adjustMarkers(int decrement) {
		if (markedReaderIndex_ <= decrement) {
			markedReaderIndex_ = 0;
			if (markedWriterIndex_ <= decrement) {
				markedWriterIndex_ = 0;
			} else {
				markedWriterIndex_ -= decrement;
			}
		} else {
			markedReaderIndex_ -= decrement;
			markedWriterIndex_ -= decrement;
		}
	}
	
	void AbstractCharBuf::checkIndex(int index, int fieldLength) const {
		checkRangeBounds(index, fieldLength, capacity());
	}
	
	void AbstractCharBuf::checkRangeBounds(int index, int fieldLength, int capacity) const {
		bool r = (index | fieldLength | index + fieldLength | capacity - (index + fieldLength)) < 0;
		if (r) {
			throw std::runtime_error("checkRangeBounds failed");
		}
	}
		
	void AbstractCharBuf::checkIndexBounds(int readerIndex, int writerIndex, int capacity) {
		if (readerIndex < 0 || readerIndex > writerIndex || writerIndex > capacity) {
			throw std::runtime_error("checkIndexBounds failed");
		}
	}

	void AbstractCharBuf::ensureWritable(int minWritableBytes) {
		if (minWritableBytes > writableBytes()) {
			if (minWritableBytes > maxCapacity() - writerIndex()) {
				throw std::runtime_error("writerIndex exceeds maxCapacity");
			} else {
				int newCapacity = calculateNewCapacity( writerIndex() + minWritableBytes, maxCapacity() );
				capacity(newCapacity);
			}
		}
	}

	void AbstractCharBuf::checkReadable(int minimumReadableBytes) {
		if ( readerIndex_ + minimumReadableBytes > writerIndex_) {
			throw std::runtime_error("readerIndex + length exceeds writerIndex");
		}
	}

	int AbstractCharBuf::calculateNewCapacity(int minNewCapacity, int maxCapacity) {
		if (minNewCapacity > maxCapacity) {
			throw std::runtime_error("minNewCapacity expected: not greater than maxCapacity");
		}
		int newCapacity = CommUtil::roundUpPower2( capacity() );
		for(; newCapacity < minNewCapacity; newCapacity <<= 1) ;
		return std::min(newCapacity, maxCapacity);
	}

	void AbstractCharBuf::checkReadableBounds(CharBuf& src, int length) {
		if (length > src.readableBytes()) {
			throw std::runtime_error((boost::format("length(%d) exceeds src.readableBytes(%d) where src is: %p") % length % src.readableBytes() % (&src)).str().c_str());
		}
	}

}//namespace buffer
}//namespace comm