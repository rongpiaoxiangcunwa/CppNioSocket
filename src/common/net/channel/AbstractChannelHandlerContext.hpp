#ifndef _AbstractChannelHandlerContext_hpp_
#define _AbstractChannelHandlerContext_hpp_

#include <iostream>
#include "ChannelHandlerContext.hpp"
#include "ChannelPipeline.hpp"
#include "Connection.hpp"
#include "ChannelInboundHandler.hpp"
#include "ChannelOutboundHandler.hpp"
#include "CharBufAllocator.hpp"

namespace comm { 
namespace net {
	class AbstractChannelHandlerContext : 
		public ChannelHandlerContext 
	{
	public:
		static void invokeChannelRegistered(AbstractChannelHandlerContext *next) {
			if (next) next->invokeChannelRegistered();
		}

		static void invokeChannelUnregistered(AbstractChannelHandlerContext *next) {
			if (next) next->invokeChannelUnregistered();
		}

		static void invokeChannelActive(AbstractChannelHandlerContext *next) {
			if (next) next->invokeChannelActive();
		}

		static void invokeChannelInactive(AbstractChannelHandlerContext *next) {
			if (next) next->invokeChannelInactive();
		}

		static void invokeChannelRead(AbstractChannelHandlerContext *next, const shared_ptr<Event> &event) {
			next->invokeChannelRead(event);
		}

		static void invokeExceptionCaught(AbstractChannelHandlerContext *next, const std::exception& cause) {
			if (next != NULL) next->invokeExceptionCaught(cause);
		}

		static void invokeWrite(AbstractChannelHandlerContext *next, const shared_ptr<Event>& event, const EventCallBack& callback)  {
			if (next) next->invokeWrite(event, callback);
		}
		
		static void invokeClose(AbstractChannelHandlerContext *next, const EventCallBack& callback)  {
			if (next) next->invokeClose(callback);
		}
		
		AbstractChannelHandlerContext(const string& name,  ChannelPipeline* _pipeline,  bool inbound,  bool outbound) 
		: next(0),  prev(0),  name_(name), pipeline_(_pipeline), inbound_(inbound), outbound_(outbound) {
		}

		virtual const string& name() { return name_; }
		virtual Connection* connection() { return pipeline_->connection(); }
		virtual NioEventReactor* reactor()  { return connection()->getEventReactor(); }
		virtual ChannelPipeline* pipeline() { return pipeline_; }
		virtual CharBufAllocator* alloc() { return connection()->config()->getAllocator();}
		
		//ChannelInboundInvoker
		virtual ChannelHandlerContext& fireChannelRegistered() {
			 invokeChannelRegistered(findContextInbound());
			 return *this;
		}
		
		virtual ChannelHandlerContext& fireChannelUnregistered() {
			invokeChannelUnregistered(findContextInbound());
			return *this;
		}
		
		virtual ChannelHandlerContext& fireChannelActive() {
			invokeChannelActive(findContextInbound());
			return *this;
		}
		
		virtual ChannelHandlerContext& fireChannelInactive() {
			invokeChannelInactive(findContextInbound());
			return *this;
		}
		
		virtual ChannelHandlerContext& fireExceptionCaught(const std::exception& cause) {
			invokeExceptionCaught(next, cause);
			return *this;
		}
		
   		virtual ChannelHandlerContext& fireChannelRead(const shared_ptr<Event> &event) {
			 invokeChannelRead(findContextInbound(), event);
			 return *this;
		}

		//ChannelOutboundInvoker
		virtual void write(const shared_ptr<Event>& event, const EventCallBack& callback)  {
			invokeWrite(findContextOutbound(), event, callback);
		}
		
		virtual void close(const EventCallBack& callback)  {
			invokeClose(findContextOutbound(), callback);
		}
		
	private:
		void invokeChannelRegistered() {
			shared_ptr<ChannelInboundHandler> inboundHandler = boost::dynamic_pointer_cast<ChannelInboundHandler>(handler());
			if (inboundHandler) inboundHandler->channelRegistered(this);
		}

		void invokeChannelUnregistered() {
			shared_ptr<ChannelInboundHandler> inboundHandler = boost::dynamic_pointer_cast<ChannelInboundHandler>(handler());
			if (inboundHandler) inboundHandler->channelUnregistered(this);
		}

		void invokeChannelActive() {
			shared_ptr<ChannelInboundHandler> inboundHandler = boost::dynamic_pointer_cast<ChannelInboundHandler>(handler());
			if (inboundHandler) inboundHandler->channelActive(this);
		}

		void invokeChannelInactive() {
			shared_ptr<ChannelInboundHandler> inboundHandler = boost::dynamic_pointer_cast<ChannelInboundHandler>(handler());
			if (inboundHandler) inboundHandler->channelInactive(this);
		}

		void invokeChannelRead(const shared_ptr<Event> &event) {
			try {
				shared_ptr<ChannelInboundHandler> inboundHandler = boost::dynamic_pointer_cast<ChannelInboundHandler>(handler());
				if (inboundHandler) inboundHandler->channelRead(this, event);
			} catch(const std::exception &ex) {
				invokeExceptionCaught(ex);
			}
		}

		void invokeExceptionCaught(const std::exception& cause) {
			shared_ptr<ChannelInboundHandler> inboundHandler = boost::dynamic_pointer_cast<ChannelInboundHandler>(handler());
			if (inboundHandler) inboundHandler->exceptionCaught(this, cause);
		}

		void invokeWrite(const shared_ptr<Event>& event, const EventCallBack& callback) {
			shared_ptr<ChannelOutboundHandler> outboundHandler = boost::dynamic_pointer_cast<ChannelOutboundHandler>(handler());
			if (outboundHandler) outboundHandler->write(this, event, callback);
		}

		void invokeClose(const EventCallBack& callback) {
			shared_ptr<ChannelOutboundHandler> outboundHandler = boost::dynamic_pointer_cast<ChannelOutboundHandler>(handler());
			if (outboundHandler) outboundHandler->close(this, callback);
		}
		
		AbstractChannelHandlerContext* findContextInbound() {
			AbstractChannelHandlerContext *ctx = this;
			do {
				ctx = ctx->next;
			} while(ctx && !ctx->inbound_);
			return ctx;
		}

		AbstractChannelHandlerContext* findContextOutbound() {
			AbstractChannelHandlerContext *ctx = this;
			do {
				ctx = ctx->prev;
			} while(ctx && !ctx->outbound_);
			return ctx;
		}
		
	public:
		AbstractChannelHandlerContext *next;
		AbstractChannelHandlerContext *prev;
		
	private:
		string name_;
		ChannelPipeline *pipeline_;
		bool inbound_;
		bool outbound_;
	};
}//namespace net
}//namespace comm

#endif
