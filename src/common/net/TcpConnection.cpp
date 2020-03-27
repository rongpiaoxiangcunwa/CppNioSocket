#include "TcpConnection.hpp"
#include <boost/format.hpp>

namespace comm { 
namespace net {
TcpConnection::TcpConnection(NioEventReactor* eventReactor) : AbstractConnection(eventReactor) {
	init();
}
	
TcpConnection::TcpConnection(NioEventReactor *eventReactor, const string& remoteHost, const string& remotePort)
: AbstractConnection(eventReactor, remoteHost, remotePort) {
	init();
}

TcpConnection::TcpConnection(NioEventReactor *eventReactor, const boost::shared_ptr< boost::asio::ip::tcp::socket > &socket)
: AbstractConnection(eventReactor), socket_(socket) {
  	init();
	connected_ = socket_->is_open();
	if (connected_) {
		setSocketOption();
		getLocalAddress();
		getRemoteAddress();
	}
}

TcpConnection::~TcpConnection() {
	asyncWriter_->shutdown();
	std::cout << "TcpConnection::~TcpConnection() " <<  this << std::endl;
}

void TcpConnection::init() {
	if (!socket_) socket_.reset(new boost::asio::ip::tcp::socket(eventReactor_->get_io_service()));
	asyncWriter_.reset( new BoostAsyncWriter<boost::asio::ip::tcp::socket>(socket_) );
}

void TcpConnection::getRemoteAddress() {
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint endpoint = socket_->remote_endpoint(ec);
	if ( !ec ) {
		remoteHost_ = endpoint.address().to_string();
		remotePort_ = ( boost::format("%1%") % endpoint.port() ).str();
	}
}

void TcpConnection::getLocalAddress() {
	boost::system::error_code ec;
	boost::asio::ip::tcp::endpoint endpoint = socket_->local_endpoint(ec);
	if ( !ec ) {
		localHost_ = endpoint.address().to_string();
		localPort_ = ( boost::format("%1%") % endpoint.port() ).str();
	}
}

void TcpConnection::read() {
	startReadMessage();
}

void TcpConnection::connect(const string& host, const string& port) {
	remoteHost_ = host;
	remotePort_ = port;
	close();
	if (stopped_) return;
	boost::asio::ip::tcp::resolver::query query(remoteHost_, remotePort_);
	boost::shared_ptr<boost::asio::ip::tcp::resolver> resolver( new boost::asio::ip::tcp::resolver(eventReactor_->get_io_service()) );
	resolver->async_resolve(query, boost::bind(&TcpConnection::handleResolved, shared_from_this(), _1, _2, resolver) );
}

void TcpConnection::handleResolved(const boost::system::error_code &ec, const boost::asio::ip::tcp::resolver::iterator &iterator,
	const boost::shared_ptr<boost::asio::ip::tcp::resolver> &resolver) {
	if ( ec || iterator == boost::asio::ip::tcp::resolver::iterator() ) {
		//std::cout << "fail to resolve address = " << remoteHost_<< ":" << remotePort_<< std::endl;
		fireConnectionInActvice();
		return;
	}
	socket_->async_connect(*iterator, boost::bind(&TcpConnection::handleConnected, shared_from_this(), _1, true));
}

void TcpConnection::handleConnected(const boost::system::error_code &ec,  bool fire) {
	if (ec) {
		fireConnectionInActvice();
		return;
	}
	getLocalAddress();
	connected_ = true;
	setSocketOption();
	if (fire) fireConnectionActvice();
	startReadMessage();
}

void TcpConnection::setSocketOption() {
	socket_->set_option(boost::asio::ip::tcp::no_delay(true));
	socket_->set_option(boost::asio::socket_base::keep_alive(true)); 
	socket_->set_option(boost::asio::socket_base::reuse_address(true));
}

void TcpConnection::startReadMessage() {
	adjustReadBuffer();
	socket_->async_read_some(boost::asio::buffer(cumulation_->array() + cumulation_->writerIndex(), cumulation_->writableBytes()), 
		boost::bind(&TcpConnection::handleRead, shared_from_this(), _1, _2) );
}

void TcpConnection::handleRead(const boost::system::error_code &ec,  size_t bytes_transferred) {
	if (ec) {
		std::cout << "TcpConnection::handleRead() " << ec << std::endl;
		close();
		fireConnectionInActvice();
		return;
	}
	cumulation_->writerIndex(cumulation_->writerIndex() + bytes_transferred);
	messageReceived(bytes_transferred);
	startReadMessage();
}

void TcpConnection::close() {
	if (!connected_) return;
	boost::system::error_code ec;
	connected_  = false;
	socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket_->close(ec);
	std::cout << "TcpConnection::close() " << cumulation_.get() << " refCnt:" << cumulation_->refCnt() << std::endl;
	freeBuffer();
	recvHandle.reset();
}

void TcpConnection::sendMessage(const string &msg, const boost::function<void(bool)> &callback) {
	asyncWriter_->write(msg, callback);
}

}//namespace net
}//namespace comm
