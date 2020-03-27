#ifndef _UnpooledCharBufAllocator_hpp_
#define _UnpooledCharBufAllocator_hpp_

#include "AbstractCharBufAllocator.hpp"
#include "UnpooledHeapCharBuf.hpp"

namespace comm {
namespace buffer {
	
	class UnpooledCharBufAllocator: public AbstractCharBufAllocator {
	protected:
		virtual shared_ptr<CharBuf> newHeapBuffer(int initialCapacity, int maxCapacity) {
			return shared_ptr<CharBuf>( new UnpooledHeapCharBuf(initialCapacity, maxCapacity) ); 
		}
	};
	
}//namespace buffer
}//namespace comm

#endif
