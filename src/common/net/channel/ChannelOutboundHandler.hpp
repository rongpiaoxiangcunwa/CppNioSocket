#ifndef _ChannelOutboundHandler_hpp_
#define _ChannelOutboundHandler_hpp_

#include "Event.hpp"
#include "channel/ChannelHandler.hpp"
#include "channel/ChannelHandlerContext.hpp"

namespace comm { 
namespace net {
	class ChannelOutboundHandler : public virtual ChannelHandler {
	public:
		virtual ~ChannelOutboundHandler() {}
		virtual void write(ChannelHandlerContextPtr ctx, const shared_ptr<Event>& event, const EventCallBack& callback) = 0;
		virtual void close(ChannelHandlerContextPtr ctx, const EventCallBack& callback) = 0;
	};
}//namespace net
}//namespace comm
#endif
