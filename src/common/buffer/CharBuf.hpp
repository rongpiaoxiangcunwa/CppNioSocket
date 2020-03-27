#ifndef _CharBuf_hpp_
#define _CharBuf_hpp_

#include <string>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <boost/smart_ptr.hpp>

using boost::shared_ptr;
using std::string;

#ifndef gettid
#include <sys/syscall.h>  
#define gettid() syscall(__NR_gettid)
#endif 

namespace comm {
namespace buffer {
	#define CHAR_BUF_MAX_CAPACITY 0x7FFFFFFF
	
	class CharBuf {
	public:
		~CharBuf() {}
		virtual int maxCapacity() const = 0;
		virtual void maxCapacity(int _maxCapacity) = 0;
		
		virtual int capacity() const = 0 ;
		virtual void capacity(int _capacity) = 0;

		virtual bool isReadable() = 0;
		virtual bool isWritable() = 0;
		virtual int readerIndex() = 0;
		virtual void readerIndex(int index) = 0;
		virtual void markReaderIndex() = 0;
   		virtual void resetReaderIndex() = 0;

		virtual int writerIndex() = 0;
		virtual void writerIndex(int index) = 0;
		virtual void markWriterIndex() = 0;
		virtual void resetWriterIndex() = 0;
		
		virtual int readableBytes() = 0;
		virtual int writableBytes() = 0;

		virtual void writeBool(bool v) = 0;
		virtual void writeChar(char v) = 0;
		virtual void writeChars(const char *src, int length) = 0;
		virtual void writeChars(CharBuf &src) = 0;
		virtual void writeChars(CharBuf &src, int length) = 0;
		virtual void writeChars(CharBuf &src, int srcIndex, int length) = 0;
		virtual void writeShort(int16_t v) = 0;
		virtual void writeUShort(uint16_t v) = 0;
		virtual void writeInt(int32_t v) = 0;
		virtual void writeUInt(uint32_t v) = 0;
		virtual void writeLong(int64_t v) = 0;
		virtual void writeULong(uint64_t v) = 0;
		virtual void writeFloat(float v) = 0;
		virtual void writeDouble(double v) = 0;
		virtual void writeString(const string& v) = 0;

		virtual void setBool(int index, bool value) = 0;
		virtual void setChar(int index, char value) = 0;
		virtual void setShort(int index, int16_t value) = 0;
		virtual void setUShort(int index, uint16_t value) = 0;
		virtual void setInt(int index, int32_t value) = 0;
		virtual void setUInt(int index, uint32_t value) = 0;
		virtual void setLong(int index, int64_t value) = 0;
		virtual void setULong(int index, uint64_t value) = 0;
		virtual void setFloat(int index, float value) = 0;
		virtual void setDouble(int index, double value) = 0;
		virtual void setString(int index, const string& value) = 0;
		virtual void setChars(int index, const char* src, int srcIndex, int length)  = 0 ;
		virtual void setChars(int index, const CharBuf &src, int srcIndex, int length) = 0;
		
		virtual bool readBool() = 0;
		virtual char readChar() = 0;
		virtual void readChars(char* dst, int dstIndex, int length) = 0;
		virtual int16_t readShort() = 0;
		virtual uint16_t readUShort() = 0;
		virtual int32_t readInt() = 0;
		virtual uint32_t readUInt() = 0;
		virtual int64_t readLong() = 0;
		virtual uint64_t readULong() = 0;
		virtual float readFloat() = 0;
		virtual double readDouble() = 0;
		virtual void readString(string& dst, int len) = 0;

		virtual bool getBool(int index) = 0;
		virtual char getChar(int index) = 0;
		virtual void getChars(int index, char* dst, int dstIndex, int length) const = 0 ;
		virtual int16_t getShort(int index) = 0;
		virtual uint16_t getUShort(int index) = 0;
		virtual int32_t getInt(int index) = 0;
		virtual uint32_t getUInt(int index) = 0;
		virtual int64_t getLong(int index) = 0;
		virtual uint64_t getULong(int index) = 0;
		virtual float getFloat(int index) = 0;
		virtual double getDouble(int index) = 0;
		virtual void getString(int index, string& dst, int len) = 0;

		virtual void skipBytes(int n) = 0;
		virtual void discardReadBytes() = 0;
		virtual void discardSomeReadBytes() = 0;

		// for cache slice
		virtual shared_ptr<CharBuf> slice(int index, int length) = 0;
		virtual shared_ptr<CharBuf> retainedSlice(int index, int length) = 0;
		
		virtual void retain() = 0;
		virtual void retain(int increment) =0;
		virtual void release() = 0;
		virtual void release(int decrement) = 0;
		virtual int refCnt() = 0;
		
		virtual CharBuf* unwrap() const = 0;
		virtual char* array() = 0;
		virtual shared_ptr<CharBuf> get_shared_ptr() = 0;
	};

}//namespace buffer
}//namespace comm

#endif
