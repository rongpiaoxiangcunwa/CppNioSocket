#ifndef COMM_NET_SERVER_BOOST_STRAP_HPP 
#define COMM_NET_SERVER_BOOST_STRAP_HPP

#include "NioEventReactorGroup.hpp"
#include "Connection.hpp"
#include "ChannelHandler.hpp"
#include "ConnectionAcceptor.hpp"

namespace comm {
namespace net {

class ServerBootstrap
{
public:
	enum ServerConnectionType{COMM_NET_SERVER_TCP, COMM_NET_SERVER_SCTP} ;
	ServerBootstrap();
	ServerBootstrap& connection(ServerBootstrap::ServerConnectionType type);
	ServerBootstrap& group(NioEventReactorGroup *group, NioEventReactorGroup *childGroup);
	ServerBootstrap& childHandler(const shared_ptr<ChannelHandler> &_childHandler);
	ServerBootstrap& bind(const std::vector<string>& hosts, const string& port);
	ServerBootstrap& bind(const string& hosts, const string& port);

protected:
	shared_ptr<ConnectionAcceptor> init();
	
private:
	ServerConnectionType type_;
	NioEventReactorGroup *group_;
	NioEventReactorGroup *childGroup_;
	shared_ptr<ChannelHandler> childHandler_;
	shared_ptr<ConnectionAcceptor>  connectionAcceptor_;
};

}//namespace net
}//namespace comm

#endif
