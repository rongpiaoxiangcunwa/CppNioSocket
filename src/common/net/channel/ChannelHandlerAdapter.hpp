#ifndef _ChannelHandlerAdapter_hpp_
#define _ChannelHandlerAdapter_hpp_

#include "ChannelHandler.hpp"

namespace comm { 
namespace net {
	class ChannelHandlerAdapter : 
		public virtual ChannelHandler {
	public:
		virtual void exceptionCaught(ChannelHandlerContextPtr ctx, const std::exception &ex) {
			ctx->fireExceptionCaught(ex);
		}
	};
}//namespace net
}//namespace comm

#endif

