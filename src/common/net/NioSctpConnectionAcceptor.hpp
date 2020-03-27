#ifndef COMM_NET_NIO_SCTP_CONNECTION_ACCEPTOR_HPP
#define COMM_NET_NIO_SCTP_CONNECTION_ACCEPTOR_HPP

#include "AbstractConnectionAcceptor.hpp"
#include "SctpConnection.hpp"
#include <boostsctp/asio/ip/sctp.hpp>

namespace comm {
namespace net {

class NioSctpConnectionAcceptor 
: public AbstractConnectionAcceptor<boost::asio::ip::sctp>,
  public boost::enable_shared_from_this<NioSctpConnectionAcceptor>
{
public:
	NioSctpConnectionAcceptor(NioEventReactor *reactor, NioEventReactorGroup *childGroup) 
	: AbstractConnectionAcceptor<boost::asio::ip::sctp>(reactor, childGroup) {
	}
	
protected:
	virtual shared_ptr<AbstractConnectionAcceptor> get_shared_ptr() {
		return shared_from_this();
	}
	
	virtual shared_ptr<Connection> newConnection(const boost::shared_ptr<boost::asio::ip::sctp::socket> &sock, NioEventReactor *childReactor) {
		return shared_ptr<Connection>(new SctpConnection(childReactor, sock));
	}
	
	virtual void bindAddress(std::vector<boost::asio::ip::sctp::endpoint>& endpoints) {
		acceptor_.reset(new boost::asio::ip::sctp::acceptor(reactor_->get_io_service(), endpoints));
	}
};

}//namespace net
}//namespace comm

#endif
