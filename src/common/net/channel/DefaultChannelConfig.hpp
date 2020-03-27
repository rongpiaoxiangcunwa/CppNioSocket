#ifndef _DefaultChannelConfig_hpp_
#define _DefaultChannelConfig_hpp_

#include "ChannelConfig.hpp"
#include "CharBufAllocator.hpp"

namespace comm { 
namespace net {
	class DefaultChannelConfig : public ChannelConfig {
	public:
		DefaultChannelConfig() ;
		
		virtual shared_ptr<RecvCharBufAllocator> getRecvCharBufAllocator() ;

		virtual CharBufAllocator* getAllocator() ;
		
		virtual void setAllocator(CharBufAllocator* _allocator) ;
		 
	private:
		shared_ptr<RecvCharBufAllocator> recvAlloctor;
		CharBufAllocator* allocator;
	};
}//namespace net
}//namespace comm

#endif
