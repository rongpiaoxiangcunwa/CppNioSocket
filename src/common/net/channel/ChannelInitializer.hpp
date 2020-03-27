#ifndef _ChannelInitializer_hpp_
#define _ChannelInitializer_hpp_

#include "Connection.hpp"
#include "ChannelInboundHandlerAdapter.hpp"

namespace comm { 
namespace net {
class ChannelInitializer : public ChannelInboundHandlerAdapter {
public:
	virtual void channelRegistered(ChannelHandlerContextPtr  ctx) {
		ChannelPipeline* pipeline = ctx->pipeline();
		if (initChannel(ctx)) {
			pipeline->fireChannelRegistered();
		} else {
			ctx->fireChannelRegistered();
		}
	}

	virtual void exceptionCaught(ChannelHandlerContextPtr ctx, const std::exception &ex) {
		ctx->close(boost::bind(&ChannelInitializer::closeCallback, _1));
	}

	static void closeCallback(bool) {}
	
protected:
	virtual void initChannel(Connection *conn) = 0;
	
	bool initChannel(ChannelHandlerContextPtr ctx) {
		try {
			initChannel(ctx->connection());
		} catch (std::exception& cause) {
			exceptionCaught(ctx, cause);
			return false;
		} 
		remove(ctx);
		return true;
	}

	void remove(ChannelHandlerContextPtr ctx) {
		ChannelPipeline* pipeline = ctx->pipeline();
		if (pipeline->context(getSharedPtr())) {
			pipeline->remove(getSharedPtr());
		}
	}
};
}//namespace net
}//namespace comm

#endif
