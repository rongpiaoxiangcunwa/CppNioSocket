#ifndef _ChannelInboundInvoker_hpp_
#define _ChannelInboundInvoker_hpp_
#include "Event.hpp"

namespace comm { 
namespace net {
	class ChannelInboundInvoker {
	public:
		virtual ChannelInboundInvoker& fireChannelRegistered() = 0;
		virtual ChannelInboundInvoker& fireChannelUnregistered() = 0;
		virtual ChannelInboundInvoker& fireChannelActive() = 0;
		virtual ChannelInboundInvoker& fireChannelInactive() = 0;
		virtual ChannelInboundInvoker& fireExceptionCaught(const std::exception& cause) = 0;
   		virtual ChannelInboundInvoker& fireChannelRead(const shared_ptr<Event> &event) = 0;
	};
}//namespace net
}//namespace comm

#endif
