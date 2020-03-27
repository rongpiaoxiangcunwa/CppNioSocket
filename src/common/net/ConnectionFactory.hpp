#ifndef COMM_NET_CONNECTION_FACTORY_HPP
#define	COMM_NET_CONNECTION_FACTORY_HPP

#include "NioEventReactorGroup.hpp"
#include "Connection.hpp"
#include "ConnectionListener.hpp"
#include "ChannelHandler.hpp"

namespace comm { 
namespace net {
	class ConnectionFactory {
	public:
		enum ConnectionType{COMM_NET_TCP, COMM_NET_SCTP} ;
		ConnectionFactory();
		ConnectionFactory& connection(ConnectionType type);
		ConnectionFactory& setAutomaticRecovery(bool automaticRecovery);
		ConnectionFactory& group(NioEventReactorGroup *group);
		ConnectionFactory& localAddress(const string& host, const string& port);
		ConnectionFactory& remoteAddress(const string& host, const string& port);
		ConnectionFactory& connectionListener(ConnectionListener *listener);
		ConnectionFactory& handler(const shared_ptr<ChannelHandler> &handler);
		shared_ptr<Connection> connect();
		shared_ptr<Connection> connect(const string& remoteHost, const string& remotePort);

	private:
		shared_ptr<Connection> doConnect(const string& remoteHost, const string& remotePort, const string& localHost, const string& localPort);
		shared_ptr<Connection> createAutorecoveringConnection(const shared_ptr<Connection> &delegate);
		shared_ptr<Connection>  initAndRegister();
	private:
		ConnectionType type_;
		volatile bool automaticRecovery_;
		NioEventReactorGroup *group_;
		ConnectionListener *listener_;
		string localHost_;
		string localPort_;
		string remoteHost_;
		string remotePort_;
		shared_ptr<ChannelHandler> channelHandler_;
	};
		
}//namespace net
}//namespace comm
#endif
