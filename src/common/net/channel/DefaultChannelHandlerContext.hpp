#ifndef _DefaultChannelHandlerContext_hpp_
#define _DefaultChannelHandlerContext_hpp_

#include "AbstractChannelHandlerContext.hpp"
#include <typeinfo>

namespace comm { 
namespace net {
	class DefaultChannelHandlerContext : public AbstractChannelHandlerContext {
	public:
		DefaultChannelHandlerContext(const string& name, ChannelPipeline* pipeline, const shared_ptr<ChannelHandler>& handler) 
		: AbstractChannelHandlerContext(name, pipeline, isInbound(handler), isOutbound(handler)), 
		  handler_(handler){
		}
		
		virtual shared_ptr<ChannelHandler> handler()  { return handler_; }
		
	private:
		static bool isInbound( const shared_ptr<ChannelHandler>& handler) {
			return !!boost::dynamic_pointer_cast<ChannelInboundHandler>(handler);
		}

		static bool isOutbound( const shared_ptr<ChannelHandler>& handler) {
			return !!boost::dynamic_pointer_cast<ChannelOutboundHandler>(handler);
		}
		
	private:
		shared_ptr<ChannelHandler> handler_;
	};
}//namespace net
}//namespace comm

#endif
