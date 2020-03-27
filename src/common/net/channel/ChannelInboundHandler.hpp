#ifndef _ChannelInboundHandler_hpp_
#define _ChannelInboundHandler_hpp_

#include "Event.hpp"
#include "channel/ChannelHandler.hpp"
#include "channel/ChannelHandlerContext.hpp"

namespace comm { 
namespace net {
	class ChannelInboundHandler : 
		public virtual ChannelHandler 
	{
	public:
		virtual ~ChannelInboundHandler() {}
		virtual void channelRegistered(ChannelHandlerContextPtr ctx) = 0;
		virtual void channelUnregistered(ChannelHandlerContextPtr ctx) = 0;
		virtual void channelActive(ChannelHandlerContextPtr ctx) = 0;
		virtual void channelInactive(ChannelHandlerContextPtr ctx) = 0;
		virtual void channelRead(ChannelHandlerContextPtr ctx, const shared_ptr<Event> &event) = 0;
	};
}//namespace net
}//namespace comm
#endif
