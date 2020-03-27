#ifndef _AbstractDerivedCharBuf_hpp_
#define _AbstractDerivedCharBuf_hpp_

#include "AbstractCharBuf.hpp"

namespace comm { 
namespace buffer {
	class AbstractDerivedCharBuf : public AbstractCharBuf {
	public:
		AbstractDerivedCharBuf(int _maxCapacity) : AbstractCharBuf(_maxCapacity){
		}

		virtual void retain() {
			unwrap()->retain();
		}
		
		virtual void retain(int increment) {
			unwrap()->retain(increment);
		}

		virtual void release() {
			unwrap()->release();
		}
		
		virtual void release(int decrement) {
			unwrap()->release(decrement);
		}
		
		virtual int refCnt() {
			return unwrap()->refCnt();
		}
	};
}//namespace net
}//namespace comm

#endif