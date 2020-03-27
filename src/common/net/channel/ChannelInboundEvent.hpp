#ifndef _ChannelInboundEvent_hpp_
#define _ChannelInboundEvent_hpp_

#include "CharBuf.hpp"
namespace comm { 
namespace net {
	class ChannelInboundEvent : public Event {
	public:
		static int ID() { return 1; }
		
		ChannelInboundEvent(const shared_ptr<CharBuf>& msg) : msg_(msg) {		
		}
		
		virtual int eventId() {
			return ID();
		}
		
		virtual const string& name() {
			return "ChannelInboundEvent";
		}

		shared_ptr<CharBuf>& getMessage() {return msg_;}
		void setMessage(const shared_ptr<CharBuf>& msg) { msg_ = msg; }
		
	private:
		shared_ptr<CharBuf> msg_;
	};
}//namespace net
}//namespace comm

#endif