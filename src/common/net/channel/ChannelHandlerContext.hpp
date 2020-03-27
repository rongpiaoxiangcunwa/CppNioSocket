#ifndef _ChannelHandlerContext_hpp_
#define _ChannelHandlerContext_hpp_

#include <string>
#include <boost/shared_ptr.hpp>
#include "NioEventReactor.hpp"
#include "ChannelOutboundInvoker.hpp"
#include "ChannelInboundInvoker.hpp"
#include "CharBufAllocator.hpp"

using std::string;
using boost::shared_ptr;
using comm::buffer::CharBufAllocator;

namespace comm { 
namespace net {
	class ChannelPipeline;
	class ChannelHandler;
	class Connection;
	
	class ChannelHandlerContext :  
		public ChannelInboundInvoker, 
		public ChannelOutboundInvoker {
	public:
		virtual ~ChannelHandlerContext() {}	
		virtual const string& name() = 0;
		virtual Connection* connection() = 0;
		virtual NioEventReactor* reactor() = 0;
		virtual shared_ptr<ChannelHandler> handler() = 0;
		virtual ChannelPipeline* pipeline() = 0;
		virtual CharBufAllocator* alloc() = 0;
	};

	typedef ChannelHandlerContext* ChannelHandlerContextPtr;

}//namespace net
}//namespace comm
#endif
