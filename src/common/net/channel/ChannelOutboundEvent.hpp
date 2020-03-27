#ifndef _ChannelOutboundEvent_hpp_
#define _ChannelOutboundEvent_hpp_

#include "CharBuf.hpp"
namespace comm { 
namespace net {
	class ChannelOutboundEvent : public Event {
	public:
		static int ID() { return 2; }
		
		ChannelOutboundEvent(const shared_ptr<CharBuf>& msg) : msg_(new string()) {	
			msg->readString(*msg_, msg->readableBytes());
			msg->release();
		}

		ChannelOutboundEvent(const shared_ptr<string>& msg) : msg_(msg) {		
		}

		ChannelOutboundEvent(const string& msg) : msg_(new string()) {	
			*msg_ = msg;
		}
		
		virtual int eventId() { return ID();}
		virtual const string& name() { return "ChannelOutboundEvent";}
		const shared_ptr<string>& getMessage() const {return msg_;}
		
	private:
		shared_ptr<string> msg_;
	};
}//namespace net
}//namespace comm

#endif