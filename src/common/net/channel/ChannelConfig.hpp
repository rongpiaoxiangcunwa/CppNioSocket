#ifndef _ChannelConfig_hpp_
#define _ChannelConfig_hpp_

#include "RecvCharBufAllocator.hpp"

namespace comm { 
namespace net {
	class ChannelConfig {
	public:
		 virtual shared_ptr<RecvCharBufAllocator> getRecvCharBufAllocator() = 0;
		 virtual CharBufAllocator* getAllocator() = 0;
		 virtual void setAllocator(CharBufAllocator* allocator) = 0;
	};
	
}//namespace net
}//namespace comm

#endif
