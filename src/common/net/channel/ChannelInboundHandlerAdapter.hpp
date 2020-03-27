#ifndef _ChannelInboundHandlerAdapter_hpp_
#define _ChannelInboundHandlerAdapter_hpp_

#include "ChannelInboundHandler.hpp"
#include "ChannelHandlerAdapter.hpp"
#include <iostream>

namespace comm { 
namespace net {
class ChannelInboundHandlerAdapter : 
	public virtual ChannelInboundHandler/*,public virtual ChannelHandlerAdapter*/
{
public:
	virtual void channelRegistered(ChannelHandlerContextPtr ctx) {
		ctx->fireChannelRegistered();
	}
	
	virtual void channelUnregistered(ChannelHandlerContextPtr ctx) {
		ctx->fireChannelUnregistered();
	}
	
	virtual void channelActive(ChannelHandlerContextPtr ctx) {
		ctx->fireChannelActive();
	}
	
	virtual void channelInactive(ChannelHandlerContextPtr ctx) {
		ctx->fireChannelInactive();
	}
	
	virtual void channelRead(ChannelHandlerContextPtr ctx, const shared_ptr<Event> &event) {
		ctx->fireChannelRead(event);
	}

	virtual void exceptionCaught(ChannelHandlerContextPtr ctx, const std::exception &ex) {
		ctx->fireExceptionCaught(ex);
	}
};
}//namespace net
}//namespace comm

#endif
