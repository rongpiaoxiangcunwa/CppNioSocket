#ifndef COMM_NET_NIO_TCP_CONNECTION_ACCEPTOR_HPP
#define COMM_NET_NIO_TCP_CONNECTION_ACCEPTOR_HPP

#include "AbstractConnectionAcceptor.hpp"
#include "TcpConnection.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace comm {
namespace net {

class NioTcpConnectionAcceptor
: public AbstractConnectionAcceptor<boost::asio::ip::tcp>
, public boost::enable_shared_from_this<NioTcpConnectionAcceptor>
{
public:
	NioTcpConnectionAcceptor(NioEventReactor *reactor,NioEventReactorGroup *childGroup) 
	: AbstractConnectionAcceptor<boost::asio::ip::tcp>(reactor, childGroup) {
	}

protected:
	virtual shared_ptr<AbstractConnectionAcceptor> get_shared_ptr() {
		return shared_from_this();
	}
	
	virtual shared_ptr<Connection> newConnection(const boost::shared_ptr<boost::asio::ip::tcp::socket> &sock, NioEventReactor *childReactor) {
		return shared_ptr<Connection>(new TcpConnection(childReactor, sock));
	}
	
	virtual void bindAddress(std::vector<endpoint>& endpoints) {
		if (endpoints.empty()) return;
		acceptor_.reset(new boost::asio::ip::tcp::acceptor(reactor_->get_io_service(), endpoints.front()));
	}
};
}//namespace net
}//namespace comm

#endif
