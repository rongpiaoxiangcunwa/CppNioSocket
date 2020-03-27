#ifndef _AbstractReferenceCountedCharBuf_hpp_
#define _AbstractReferenceCountedCharBuf_hpp_

#include <AtomicInteger.hpp>
#include "AbstractCharBuf.hpp"
#include <iostream>

namespace comm { 
namespace buffer {
	class AbstractReferenceCountedCharBuf : public AbstractCharBuf {
	public:
		
		AbstractReferenceCountedCharBuf(int maxCapacity) : AbstractCharBuf(maxCapacity), refCnt_(1) {
		}

		virtual void retain() {
			retain(1);
		}
		
		virtual void retain(int increment) {
			if (increment > 0) {
				refCnt_.incrementAndGet(increment);
			}
		}

		virtual void release() {
			release(1);
		}
		
		virtual void release(int decrement) {
			if (decrement > 0) {
				refCnt_.descrementAndGet(decrement);
			}
			
			if (refCnt_.get() < 0) {
				std::cout <<  "release:" << refCnt_.get() << std::endl;
			}
			
			if (refCnt_.get() == 0)  {
				deallocate();
			}
		}
		
		virtual int refCnt() {
			return refCnt_.get();
		}

		virtual void deallocate() { }

		void resetRefCnt() { refCnt_.set(1); }
		
	private:
		AtomicInteger<int> refCnt_;
	};
}//namespace net
}//namespace comm

#endif