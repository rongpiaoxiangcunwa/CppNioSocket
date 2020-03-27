#ifndef _ChannelPipeline_hpp_
#define _ChannelPipeline_hpp_

#include "ChannelHandlerContext.hpp"
#include "ChannelHandler.hpp"

namespace comm { 
namespace net {
class Connection;
class ChannelHandler;
class ChannelPipeline : public ChannelInboundInvoker, public ChannelOutboundInvoker {
public:
	virtual ~ChannelPipeline() {}
	virtual shared_ptr<ChannelPipeline> getSharedPtr() = 0;
	virtual ChannelPipeline& addLast(const string& name, const shared_ptr<ChannelHandler>& handler) = 0;
	virtual void remove(const string& name) = 0;
	virtual void remove(const shared_ptr<ChannelHandler> &handler) = 0;
	virtual shared_ptr<ChannelHandler>  get(const string& name) = 0;
	virtual shared_ptr<ChannelHandler>  first() = 0;
	virtual shared_ptr<ChannelHandler>  last() = 0;
	virtual ChannelHandlerContext* context(const shared_ptr<ChannelHandler> &handler) = 0;
	virtual ChannelHandlerContext* context(const string &name) = 0;
	virtual ChannelHandlerContext* firstContext() = 0;
	virtual ChannelHandlerContext* lastContext() = 0;
	virtual Connection* connection() = 0;
	
	virtual ChannelPipeline& fireChannelRegistered() = 0;
	virtual ChannelPipeline& fireChannelUnregistered() = 0;
	virtual ChannelPipeline& fireChannelActive() = 0;
	virtual ChannelPipeline& fireChannelInactive() = 0;
	virtual ChannelPipeline& fireExceptionCaught(const std::exception& cause) = 0;
	virtual ChannelPipeline& fireChannelRead(const shared_ptr<Event> &event) = 0;
};
}//namespace net
}//namespace comm

#endif
