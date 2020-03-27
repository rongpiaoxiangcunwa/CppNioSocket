#ifndef _AbstractCharBufAllocator_hpp_
#define _AbstractCharBufAllocator_hpp_

#include "CharBufAllocator.hpp"

namespace comm {
namespace buffer {
	class AbstractCharBufAllocator: public CharBufAllocator {
	public:
		virtual shared_ptr<CharBuf> ioBuffer() { return ioBuffer(256); }
		
		virtual shared_ptr<CharBuf> ioBuffer(int initialCapacity) { return heapBuffer(initialCapacity); }
		
		virtual shared_ptr<CharBuf> ioBuffer(int initialCapacity, int maxCapaity) { return heapBuffer(initialCapacity, maxCapaity); }

		virtual shared_ptr<CharBuf> buffer(int initialCapacity) {return heapBuffer(initialCapacity);}
		
		virtual shared_ptr<CharBuf> heapBuffer() { return heapBuffer(256); }
		
		virtual shared_ptr<CharBuf> heapBuffer(int initialCapacity)  { return heapBuffer(initialCapacity, CHAR_BUF_MAX_CAPACITY); }
		
		virtual shared_ptr<CharBuf> heapBuffer(int initialCapacity, int maxCapacity) {
			if (0 == initialCapacity || 0 == maxCapacity || initialCapacity > maxCapacity) return shared_ptr<CharBuf>();
			return newHeapBuffer(initialCapacity, maxCapacity);
		}

	protected:
		virtual shared_ptr<CharBuf> newHeapBuffer(int initialCapacity, int maxCapacity) = 0; 
	};
}//namespace buffer
}//namespace comm

#endif
