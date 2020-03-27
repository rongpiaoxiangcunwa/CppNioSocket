#include "ConnectionFactory.hpp"
#include "AutorecoveringConnection.hpp"
#include "TcpConnection.hpp"
#include "SctpConnection.hpp"
#include <stdexcept>

namespace comm { 
namespace net {
	ConnectionFactory::ConnectionFactory() : type_(COMM_NET_TCP), automaticRecovery_(true), group_(0), listener_(0)/*, handler_(0)*/ {
	}
	
	ConnectionFactory& ConnectionFactory::connection(ConnectionType type) {
		type_ = type;
		return *this;
	}

	ConnectionFactory& ConnectionFactory::setAutomaticRecovery(bool automaticRecovery) {
		automaticRecovery_ = automaticRecovery;
		return *this;
	}
	
	ConnectionFactory& ConnectionFactory::group(NioEventReactorGroup *group) {
		group_ = group;
		return *this;
	}
	
	ConnectionFactory& ConnectionFactory::localAddress(const string& host, const string& port) {
		localHost_ = host;
		localPort_ = port;
		return *this;
	}
	
	ConnectionFactory& ConnectionFactory::remoteAddress(const string& host, const string& port) {
		remoteHost_ = host;
		remotePort_ = port;
		return *this;
	}
	
	ConnectionFactory& ConnectionFactory::connectionListener(ConnectionListener *listener) {
		listener_ = listener;
		return *this;
	}
	
	ConnectionFactory& ConnectionFactory::handler(const shared_ptr<ChannelHandler> &handler) {
		channelHandler_ = handler;
		return *this;
	}
	 
	shared_ptr<Connection> ConnectionFactory::connect() {
		return connect(remoteHost_, remotePort_);
	}
	
	shared_ptr<Connection> ConnectionFactory::connect(const string& remoteHost, const string& remotePort) {
		return doConnect(remoteHost, remotePort, localHost_, localPort_);
	}
	
	shared_ptr<Connection> ConnectionFactory::doConnect(const string& remoteHost, const string& remotePort, 
	  const string& localHost, const string& localPort) {
		shared_ptr<Connection> connection = initAndRegister();
		if (connection) {
			connection->connect(remoteHost, remotePort);
		}
		return connection;
	}
	
	shared_ptr<Connection> ConnectionFactory::createAutorecoveringConnection(const shared_ptr<Connection> &delegate) {
		if (delegate) return shared_ptr<Connection>( new AutorecoveringConnection(delegate) );
		return shared_ptr<Connection>();
	}
	
	shared_ptr<Connection> ConnectionFactory::initAndRegister() {
		shared_ptr<Connection> connection;
		switch(type_) {
			case COMM_NET_TCP:
				connection.reset( new TcpConnection(group_->next()));
			break;

			case COMM_NET_SCTP:
				connection.reset(new SctpConnection(group_->next()));
			break;

			default:
				throw std::runtime_error("Connection type is wrong");
		}
		
		if (automaticRecovery_) connection = createAutorecoveringConnection(connection);
		//it generate name automaticly
		connection->pipeline()->addLast("", channelHandler_);
		connection->pipeline()->fireChannelRegistered();
		return connection;
	}
}//namespace net
}//namespace comm
