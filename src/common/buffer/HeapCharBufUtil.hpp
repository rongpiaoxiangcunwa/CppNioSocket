#ifndef _HeapCharBufUtil_hpp_
#define _HeapCharBufUtil_hpp_
#include <string>

using std::string;

namespace comm {
namespace buffer {
	
	class HeapCharBufUtil {
	public:
		static void setChar(char memory[], int index, char value) {
			memory[index] = value;
		}

		static void setChars(char memory[],  int index, const char* src, size_t len) {
			memcpy(memory + index, src, len);
		}
		
		static void setShort(char memory[], int index, int16_t value) {
			memory[index] = (char)( (value >> 8) & 0xff);
			memory[index + 1] = (char)( value & 0xff);
		}

		static void setUShort(char memory[], int index, uint16_t value) {
			memory[index] = (char)( (value >> 8) & 0xff);
			memory[index + 1] = (char)( value & 0xff);
		}
		
		static void setInt(char memory[], int index, int32_t value) {
			memory[index] = (char)( (value >> 24) & 0xff);
			memory[index + 1] = (char)( (value >> 16) & 0xff);
			memory[index + 2] = (char)( (value >> 8) & 0xff);
			memory[index + 3] = (char)( value & 0xff);
		}

		static void setUInt(char memory[], int index, uint32_t value) {
			memory[index] = (char)( (value >> 24) & 0xff);
			memory[index + 1] = (char)( (value >> 16) & 0xff);
			memory[index + 2] = (char)( (value >> 8) & 0xff);
			memory[index + 3] = (char)( value & 0xff);
		}

		static void setLong(char memory[], int index, int64_t value) {
			memory[index] = (char)( (value >> 56) & 0xff);
			memory[index + 1] = (char)( (value >> 48) & 0xff);
			memory[index + 2] = (char)( (value >> 40) & 0xff);
			memory[index + 3] = (char)( (value >> 32)& 0xff);
			memory[index + 4] = (char)( (value >> 24) & 0xff); 
			memory[index + 5] = (char)( (value >> 16) & 0xff);
			memory[index + 6] = (char)( (value >> 8) & 0xff);
			memory[index + 7] = (char)( value & 0xff);
		}

		static void setULong(char memory[], int index, uint64_t value) {
			memory[index] = (char)( (value >> 56) & 0xff);
			memory[index + 1] = (char)( (value >> 48) & 0xff);
			memory[index + 2] = (char)( (value >> 40) & 0xff);
			memory[index + 3] = (char)( (value >> 32)& 0xff);
			memory[index + 4] = (char)( (value >> 24) & 0xff); 
			memory[index + 5] = (char)( (value >> 16) & 0xff);
			memory[index + 6] = (char)( (value >> 8) & 0xff);
			memory[index + 7] = (char)( value & 0xff);
		}

		static char getChar(char memory[], int index) {
			return memory[index];
		}

		static int16_t getShort(char memory[], int index) {
			return static_cast<int16_t>( (memory[index] & 0xff) << 8  | (memory[index + 1] & 0xff) );
		}

		static uint16_t getUShort(char memory[], int index) {
			return static_cast<uint16_t>( (memory[index] & 0xff) << 8  | (memory[index + 1] & 0xff) );
		}

		static int32_t getInt(char memory[], int index) {
			return static_cast<int32_t>( (memory[index] & 0xff) << 24  | (memory[index + 1] & 0xff) << 16  | 
				(memory[index + 2] & 0xff) << 8  | (memory[index + 3] & 0xff) );
		}

		static uint32_t getUInt(char memory[], int index) {
			return static_cast<uint32_t>( (memory[index] & 0xff) << 24  | (memory[index + 1] & 0xff) << 16  |
				(memory[index + 2] & 0xff) << 8  | (memory[index + 3] & 0xff) );
		}

		static int64_t getLong(char memory[], int index) {
			return static_cast<int64_t>( ((int64_t)memory[index] & 0xff) << 56  | ((int64_t)memory[index + 1] & 0xff) << 48  | 
				((int64_t)memory[index + 2] & 0xff) << 40  | ((int64_t)memory[index + 3] & 0xff) << 32 |
				((int64_t)memory[index + 4] & 0xff) << 24  | ((int64_t)memory[index + 5] & 0xff) << 16 | 
				((int64_t)memory[index + 6] & 0xff) << 8  | ((int64_t)memory[index + 7] & 0xff) );
		}

		static uint64_t getULong(char memory[], int index) {
			return static_cast<uint64_t>( ((uint64_t)memory[index] & 0xff) << 56  | ((uint64_t)memory[index + 1] & 0xff) << 48  | 
				((uint64_t)memory[index + 2] & 0xff) << 40  | ((uint64_t)memory[index + 3] & 0xff) << 32 |
				((uint64_t)memory[index + 4] & 0xff) << 24  | ((uint64_t)memory[index + 5] & 0xff) << 16 | 
				((uint64_t)memory[index + 6] & 0xff) << 8  | ((uint64_t)memory[index + 7] & 0xff) );
		}


		static void getString(char memory[],  int index, string& dst, size_t len) {
			dst.assign(memory + index, len);
		}

		static void getChars(const char memory[], int index, char dst[], int dstIndex, int length)  {
			memcpy(dst + dstIndex,  memory + index, length);
		}
		
	};
	
}//namespace buffer
}//namespace comm

#endif
