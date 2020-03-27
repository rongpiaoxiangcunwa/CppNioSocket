#ifndef _AbstractCharBuf_hpp_
#define _AbstractCharBuf_hpp_

#include "CharBuf.hpp"

namespace comm {
namespace buffer {
	
	class AbstractCharBuf : public CharBuf {
	public:
		AbstractCharBuf(int maxCapacity) : readerIndex_(0), writerIndex_(0), markedReaderIndex_(0), markedWriterIndex_(0), 
		maxCapacity_(maxCapacity) {
		}

		virtual int maxCapacity()  const { return maxCapacity_; }
		virtual void maxCapacity(int _maxCapacity) { maxCapacity_ = _maxCapacity; }

		virtual bool isReadable() { return writerIndex_ > readerIndex_; }
		virtual bool isWritable() { return capacity() > writerIndex_;}
		virtual int  readerIndex() { return readerIndex_; }
		virtual void readerIndex(int index);
		virtual void markReaderIndex() { markedReaderIndex_ = readerIndex_ ;}
		virtual void resetReaderIndex() { readerIndex(markedReaderIndex_);  }
		
		virtual int  writerIndex() { return writerIndex_; }
		virtual void writerIndex(int index);
		virtual void markWriterIndex() { markedWriterIndex_ = writerIndex_; }
		virtual void resetWriterIndex() { writerIndex(markedWriterIndex_); } 

		virtual int readableBytes() { return writerIndex() - readerIndex();}
		virtual int writableBytes()  { return capacity() - writerIndex(); }

		virtual void writeBool(bool v) ;
		virtual void writeChar(char v) ;
		virtual void writeChars(const char *src, int length);
		virtual void writeChars(CharBuf &src);
		virtual void writeChars(CharBuf &src, int length);
		virtual void writeChars(CharBuf &src, int srcIndex, int length);
		virtual void writeShort(int16_t v) ;
		virtual void writeUShort(uint16_t v) ;
		virtual void writeInt(int32_t v) ;
		virtual void writeUInt(uint32_t v) ;
		virtual void writeLong(int64_t v) ;
		virtual void writeULong(uint64_t v) ;
		virtual void writeFloat(float v) ;
		virtual void writeDouble(double v);
		virtual void writeString(const string& v);

		virtual bool readBool();
		virtual char readChar();
		virtual void readChars(char* dst , int dstIndex, int length);
		virtual int16_t readShort();
		virtual uint16_t readUShort();
		virtual int32_t readInt();
		virtual uint32_t readUInt();
		virtual int64_t readLong() ;
		virtual uint64_t readULong();
		virtual float readFloat();
		virtual double readDouble();
		virtual void readString(string& dst, int len);

		virtual bool getBool(int index) ;
		virtual char getChar(int index);
		virtual int16_t getShort(int index) ;
		virtual uint16_t getUShort(int index) ;
		virtual int32_t getInt(int index) ;
		virtual uint32_t getUInt(int index) ;
		virtual int64_t getLong(int index);
		virtual uint64_t getULong(int index) ;
		virtual float getFloat(int index) ;
		virtual double getDouble(int index) ;
		virtual void getString(int index, string& dst, int len);

		virtual void setBool(int index, bool value) ;
		virtual void setChar(int index, char value) ;
		virtual void setShort(int index, int16_t value) ;
		virtual void setUShort(int index, uint16_t value) ;
		virtual void setInt(int index, int32_t value) ;
		virtual void setUInt(int index, uint32_t value) ;
		virtual void setLong(int index, int64_t value) ;
		virtual void setULong(int index, uint64_t value);
		virtual void setFloat(int index, float value) ;
		virtual void setDouble(int index, double value);
		virtual void setString(int index, const string& value) ;

		virtual void skipBytes(int n);
		virtual void discardReadBytes();
		virtual void discardSomeReadBytes();
		
		virtual shared_ptr<CharBuf> slice(int index, int length);
		virtual shared_ptr<CharBuf> retainedSlice(int index, int length);
		
	protected:
		virtual void _setChar(int index, char v) = 0;
		virtual void _setShort(int index, int16_t value) = 0;
		virtual void _setUShort(int index, uint16_t value) = 0;
		virtual void _setInt(int index, int32_t value) = 0;
		virtual void _setUInt(int index, uint32_t value) = 0;
		virtual void _setLong(int index, int64_t value) = 0;
		virtual void _setULong(int index, uint64_t value) = 0;
		virtual void _setChars(int index, const char* src, size_t len) = 0;
		
		virtual char _getChar(int index) = 0;
		virtual int16_t _getShort(int index) = 0;
		virtual uint16_t _getUShort(int index) = 0;
		virtual int32_t _getInt(int index) = 0;
		virtual uint32_t _getUInt(int index) = 0;
		virtual int64_t _getLong(int index) = 0;
		virtual uint64_t _getULong(int index) = 0;
		virtual void _getString(int index, string &dst, size_t len) = 0;
		
	protected:
		static void checkIndexBounds(int readerIndex, int writerIndex, int capacity);
		int calculateNewCapacity(int minNewCapacity, int maxCapacity);
		void ensureWritable(int minWritableBytes);
		void checkReadable(int minimumReadableBytes);
		void checkIndex(int index, int fieldLength) const  ;
		void checkRangeBounds(int index, int fieldLength, int capacity) const;
		void adjustMarkers(int decrement);
		void checkReadableBounds(CharBuf& src, int length);
		
		void setIndex0(int readerIndex, int writerIndex) {
	        readerIndex_ = readerIndex;
	        writerIndex_ = writerIndex;
	    }
		
	protected:
		volatile int readerIndex_;
	    volatile int writerIndex_;
	    volatile int markedReaderIndex_;
	    volatile int markedWriterIndex_;
	    volatile int maxCapacity_;
	};
	
}//namespace buffer
}//namespace comm

#endif
