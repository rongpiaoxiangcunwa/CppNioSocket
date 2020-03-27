#ifndef _CharBufAllocator_hpp_
#define _CharBufAllocator_hpp_

#include <boost/shared_ptr.hpp>
#include "CharBuf.hpp"

using boost::shared_ptr;

namespace comm {
namespace buffer {
	
	class CharBufAllocator {
	public:
		static CharBufAllocator* DEFAULT_ALLOCATOR; 

		virtual shared_ptr<CharBuf> ioBuffer() = 0;
		virtual shared_ptr<CharBuf> ioBuffer(int initialCapacity) = 0;
		virtual shared_ptr<CharBuf> ioBuffer(int initialCapacity, int maxCapaity) = 0;

		virtual shared_ptr<CharBuf> buffer(int initialCapacity) = 0;
		virtual shared_ptr<CharBuf> heapBuffer() = 0;
		virtual shared_ptr<CharBuf> heapBuffer(int initialCapacity) = 0;
		virtual shared_ptr<CharBuf> heapBuffer(int initialCapacity, int maxCapacity) = 0;
		
		virtual ~CharBufAllocator() {}
	};
	
}//namespace buffer
}//namespace comm

#endif
