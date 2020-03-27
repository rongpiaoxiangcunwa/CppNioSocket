#ifndef _UnpooledSlicedCharBuf_hpp
#define _UnpooledSlicedCharBuf_hpp

#include "AbstractUnpooledSlicedCharBuf.hpp"

namespace comm { 
namespace buffer {
	class UnpooledSlicedCharBuf : 
		public AbstractUnpooledSlicedCharBuf, 
		public boost::enable_shared_from_this<UnpooledSlicedCharBuf>
	{
	public:
		UnpooledSlicedCharBuf(const shared_ptr<CharBuf>&  buffer, int index, int length)  
		: AbstractUnpooledSlicedCharBuf(buffer, index, length)  {
		}

		virtual shared_ptr<CharBuf> get_shared_ptr() {
			return shared_from_this();
		}
		
	};
}//namespace net
}//namespace comm

#endif