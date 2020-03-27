#ifndef _Connection_HPP_
#define _Connection_HPP_

#include <string>
#include <boost/function.hpp>
#include "ConnectionListener.hpp"
#include "NioEventReactor.hpp"
#include "ChannelPipeline.hpp"
#include "ChannelConfig.hpp"

using std::string;

namespace comm { 
namespace net {
	class Connection {
	public:
		virtual ~Connection() {}
		virtual shared_ptr<ChannelPipeline>& pipeline() = 0;
		virtual NioEventReactor* getEventReactor() = 0;	
		virtual NioEventReactor* setEventReactor(NioEventReactor* reactor) = 0;
		virtual ChannelConfig* config() = 0;
		virtual const string& getLocalAddress() const = 0;
		virtual const string& getLocalPort() const = 0 ;
		virtual const string& getRemoteAddress() const = 0;
		virtual const string& getRemotePort() const = 0 ;
		virtual bool isConnected() = 0;
		virtual void read() = 0;
		virtual void connect(const string& host, const string& port) = 0;
		virtual void close() = 0;
		virtual void sendMessage(const string &msg, const boost::function<void(bool)> &callback) = 0;
		virtual void stop() = 0;
		virtual shared_ptr<Connection> get_shared() = 0;
	};	
}//namespace net
}//namespace comm

#endif
