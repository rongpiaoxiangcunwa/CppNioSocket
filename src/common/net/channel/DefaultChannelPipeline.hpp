#ifndef _DefaultChannelPipeline_hpp_
#define _DefaultChannelPipeline_hpp_

#include <typeinfo>
#include <map>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/tss.hpp>
#include "Connection.hpp"
#include "DefaultChannelHandlerContext.hpp"
#include "ChannelInboundHandlerAdapter.hpp"
#include "ChannelOutboundHandlerAdapter.hpp"


using boost::thread_specific_ptr;

namespace comm { 
namespace net {
	class DefaultChannelPipeline : 
		public ChannelPipeline, 
		public boost::enable_shared_from_this<DefaultChannelPipeline> 
	{
	public:
		DefaultChannelPipeline(Connection* _connection) : connection_(_connection) {
			headRef_.reset(new HeadContext(this));
			tailRef_.reset(new TailContext(this));
			head_ = headRef_.get();
			tail_ = tailRef_.get();
			head_->next = tail_;
			tail_->prev = head_;
			
		}

		~DefaultChannelPipeline() {
			AbstractChannelHandlerContext* cur = head_->next;
			while (cur != NULL && cur != tail_) {
				head_ = cur->next;
				delete cur;
				cur = head_;
			}
		}
		
		virtual shared_ptr<ChannelPipeline> getSharedPtr() {
			return shared_from_this();
		}
		
		virtual ChannelPipeline& addLast(const string& name,  const shared_ptr<ChannelHandler> &handler) {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			AbstractChannelHandlerContext  *newCtx = new DefaultChannelHandlerContext(filterName(name, handler), this,  handler);
			addLast0(newCtx);
			return *this;
		}
		
		virtual void remove(const string& name) {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			AbstractChannelHandlerContext* ctx = context0(name) ;
			if  (ctx) remove0(ctx);
		}
		
		virtual void remove(const shared_ptr<ChannelHandler> &handler) {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			AbstractChannelHandlerContext* ctx = context0(handler) ;
			if (ctx) remove0(ctx);
		}
		
		virtual shared_ptr<ChannelHandler>  get(const string& name) {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			AbstractChannelHandlerContext* ctx = context0(name) ;
			if (ctx) return ctx->handler();
			return shared_ptr<ChannelHandler>();
		}
		
		virtual shared_ptr<ChannelHandler>  first() {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			AbstractChannelHandlerContext* ctx =head_->next;
			if (ctx != tail_) return ctx->handler();
			return shared_ptr<ChannelHandler>();
		}
		
		virtual shared_ptr<ChannelHandler>  last() {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			AbstractChannelHandlerContext* ctx =tail_->prev;
			if (ctx != head_) return ctx->handler();
			return shared_ptr<ChannelHandler>();
		}
		
		virtual ChannelHandlerContext* context(const string &name)  {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			return context0(name);
		}

		virtual ChannelHandlerContext* context(const shared_ptr<ChannelHandler> &handler) {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			return context0(handler);
		}
		
		virtual ChannelHandlerContext* firstContext() {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			AbstractChannelHandlerContext* ctx =head_->next;
			if (ctx != tail_) return ctx;
			return NULL;
		}
		
		virtual ChannelHandlerContext* lastContext() {
			boost::lock_guard<boost::recursive_mutex> guard(lock_);
			AbstractChannelHandlerContext* ctx =tail_->prev;
			if (ctx != head_) return ctx;
			return NULL;
		}
		
		virtual Connection* connection() {
			return connection_;
		}

		virtual ChannelPipeline& fireChannelRegistered() {
			AbstractChannelHandlerContext::invokeChannelRegistered(head_);
       		return *this;
		}
		
		virtual ChannelPipeline& fireChannelUnregistered() {
			AbstractChannelHandlerContext::invokeChannelUnregistered(head_);
		}
		
		virtual ChannelPipeline& fireChannelActive() {
			 AbstractChannelHandlerContext::invokeChannelActive(head_);
			 return *this;
		}
		
		virtual ChannelPipeline& fireChannelInactive() {
			AbstractChannelHandlerContext::invokeChannelInactive(head_);
			return *this;
		}
		
		virtual ChannelPipeline& fireExceptionCaught(const std::exception& cause) {
			return *this;
		}
		
   		virtual ChannelPipeline& fireChannelRead(const shared_ptr<Event> &event) {
			AbstractChannelHandlerContext::invokeChannelRead(head_, event);
			return *this;
   		}

		virtual void write(const shared_ptr<Event>& event, const EventCallBack& callback) {
			tail_->write(event, callback);
		}
		
		virtual void close(const EventCallBack& callback) {
			tail_->close(callback);
		}

	private:
		string filterName(const string &name, const shared_ptr<ChannelHandler> &handler) {
			if (name.empty()) {
				return generateName(handler);
			}
			checkDuplicateName(name);
			return name;
		}

		void checkDuplicateName(const string &name) ;
		string generateName(const shared_ptr<ChannelHandler> &handler) ;
		static string generateName0(const string& clzName);
		
		void addLast0(AbstractChannelHandlerContext *ctx) {
			AbstractChannelHandlerContext* prev = tail_->prev;
			ctx->prev = prev;
			ctx->next = tail_;
			prev->next = ctx;
			tail_->prev = ctx;
		}

		AbstractChannelHandlerContext* context0(const string& name) {
			AbstractChannelHandlerContext* ctx = head_->next;
			while (ctx != NULL) {
				if (ctx->name() == name) return ctx;
				ctx = ctx->next;
			}
			return NULL;
		}

		AbstractChannelHandlerContext* context0(const shared_ptr<ChannelHandler> &handler) {
			AbstractChannelHandlerContext *ctx = head_->next;
			while (ctx != NULL) {
				if (ctx->handler() == handler) return ctx;
				ctx = ctx->next;
			}
			return NULL;
		}

		void remove0(AbstractChannelHandlerContext *ctx)  {
			AbstractChannelHandlerContext* prev = ctx->prev;
	        AbstractChannelHandlerContext* next = ctx->next;
	        prev->next = next;
	        next->prev = prev;
			delete ctx;
		}
		
	private:
		boost::recursive_mutex lock_;
		Connection* connection_;
		AbstractChannelHandlerContext *head_;
		AbstractChannelHandlerContext *tail_;
		shared_ptr<AbstractChannelHandlerContext> headRef_;
		shared_ptr<AbstractChannelHandlerContext> tailRef_;
		static boost::thread_specific_ptr< std::map<string, string>  > nameCaches;

	private:
		////////////////////////////////////////////////////////////////////////////////////////////////////////
		class HeadContext : 
			public AbstractChannelHandlerContext,
			public virtual ChannelInboundHandlerAdapter, 
			public virtual ChannelOutboundHandlerAdapter,
			public boost::enable_shared_from_this<HeadContext > {
		public:
			HeadContext(DefaultChannelPipeline * pipeline)  
			: AbstractChannelHandlerContext(DefaultChannelPipeline::generateName0(typeid(HeadContext).name()) , pipeline, true, true) {
				//std::cout << "HeadContext " << this << std::endl;
			}
			
			//ChannelHandlerContext
			virtual shared_ptr<ChannelHandler> handler() { return getSharedPtr();}

			//ChannelHandler
			virtual shared_ptr<ChannelHandler> getSharedPtr() { return shared_from_this();}

			virtual void exceptionCaught(ChannelHandlerContextPtr ctx, const std::exception &ex)  {
				ctx->fireExceptionCaught(ex);
			}

			//ChannelOutboundHandler			
			virtual void write(ChannelHandlerContextPtr ctx, const shared_ptr<Event>& event, const EventCallBack& callback);

			virtual void close(ChannelHandlerContextPtr ctx, const EventCallBack& callback) {
				try {
					connection()->close();
					callback(true);
				} catch(const std::exception &ex) {
					callback(false);
				}
			}
		};

		class TailContext  : 
			public AbstractChannelHandlerContext, 
			public ChannelInboundHandlerAdapter,  
			public boost::enable_shared_from_this<TailContext> 
		{
		public:
			TailContext(DefaultChannelPipeline *pipeline) 
			: AbstractChannelHandlerContext(DefaultChannelPipeline::generateName0(typeid(TailContext).name()) , pipeline, true,  false){
				//std::cout << "TailContext " << this << std::endl;
			}

			//ChannelHandlerContext
			virtual shared_ptr<ChannelHandler> handler() { return getSharedPtr();}

			//ChannelHandler
			virtual shared_ptr<ChannelHandler> getSharedPtr() { return shared_from_this();}
	
			//ChannelInboundHandler
			virtual void channelRegistered(ChannelHandlerContextPtr ctx) {}
			virtual void channelUnregistered(ChannelHandlerContextPtr ctx) {}
			virtual void channelActive(ChannelHandlerContextPtr ctx) {}
			virtual void channelInactive(ChannelHandlerContextPtr ctx) {}
			virtual void channelRead(ChannelHandlerContextPtr ctx, const shared_ptr<Event> &event);
			virtual void exceptionCaught(ChannelHandlerContextPtr ctx, const std::exception &ex) { }
		};
		
	};
}//namespace net
}//namespace comm

#endif
