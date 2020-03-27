#ifndef _ChannelOutboundHandlerAdapter_hpp_
#define  _ChannelOutboundHandlerAdapter_hpp_

#include "ChannelOutboundHandler.hpp"
#include "ChannelHandlerAdapter.hpp"
namespace comm { 
namespace net {
class ChannelOutboundHandlerAdapter
: public virtual ChannelOutboundHandler
  /*,public virtual ChannelHandlerAdapter*/ {
public:
	virtual void write(ChannelHandlerContextPtr ctx, const shared_ptr<Event>& event, const EventCallBack& callback) {
		ctx->write(event, callback);
	}
	
	virtual void close(ChannelHandlerContextPtr ctx, const EventCallBack& callback) {
		ctx->close(callback);
	}
	
	// ChannelHandler
	virtual void exceptionCaught(ChannelHandlerContextPtr ctx, const std::exception &ex) {
		ctx->fireExceptionCaught(ex);
	}
};
}//namespace net
}//namespace comm

#endif
