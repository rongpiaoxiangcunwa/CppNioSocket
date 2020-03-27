#ifndef COMM_NET_CONNECTION_ACCEPTOR_HPP
#define COMM_NET_CONNECTION_ACCEPTOR_HPP

#include "NioEventReactor.hpp"
#include "ChannelConfig.hpp"
#include "ChannelPipeline.hpp"

namespace comm {
namespace net {

class ConnectionAcceptor
{
public:
	virtual ~ConnectionAcceptor() {}
	virtual shared_ptr<ChannelPipeline>& pipeline() = 0;
	virtual NioEventReactor* getEventReactor() const = 0;		
	virtual ChannelConfig* config() = 0;
	virtual void bind(const std::vector<string>& hosts, const string& port) = 0;
	virtual void bind(const string& hosts, const string& port) = 0;
	virtual void startAccept() = 0;
};

}//namespace net
}//namespace comm

#endif
