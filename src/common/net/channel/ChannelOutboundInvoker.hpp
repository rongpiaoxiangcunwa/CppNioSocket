#ifndef _ChannelOutboundInvoker_hpp_
#define _ChannelOutboundInvoker_hpp_

#include "Event.hpp"

namespace comm { 
namespace net {
	class ChannelOutboundInvoker {
	public:
		virtual void write(const shared_ptr<Event>& event, const EventCallBack& callback) = 0;
		virtual void close(const EventCallBack& callback) = 0;
	};
}//namespace net
}//namespace comm

#endif
