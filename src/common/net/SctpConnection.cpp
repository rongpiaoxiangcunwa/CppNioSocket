#include "SctpConnection.hpp"
#include "ChannelInboundEvent.hpp"
#include <boost/format.hpp>

namespace comm { 
namespace net {
 SctpConnection::SctpConnection(NioEventReactor* eventReactor) : AbstractConnection(eventReactor) {
 	init();
}

SctpConnection::SctpConnection(NioEventReactor* eventReactor, const string& remoteAddrs, const string& remotePort)
: AbstractConnection(eventReactor, remoteAddrs, remotePort) {
	init();
}

		
SctpConnection::SctpConnection(NioEventReactor *eventReactor, const std::vector<string>& remoteHost, const string& remotePort)
  : AbstractConnection(eventReactor) {
	init();
}

SctpConnection::SctpConnection(NioEventReactor *eventReactor, const boost::shared_ptr< boost::asio::ip::sctp::socket > &socket)
  : AbstractConnection(eventReactor), socket_(socket) {
	init();
	connected_ = socket_->is_open();
	if (connected_) {
		setSocketOption();
		getLocalAddress();
		getRemoteAddress();
	}
}

SctpConnection::~SctpConnection() {
	asyncWriter_->shutdown();
	std::cout << "SctpConnection::~SctpConnection() " <<  this << std::endl;
}

void SctpConnection::init() {
	if (!socket_) socket_.reset( new boost::asio::ip::sctp::socket(eventReactor_->get_io_service()) );
 	 asyncWriter_.reset( new BoostAsyncWriter<boost::asio::ip::sctp::socket>(socket_) );
}

void SctpConnection::getRemoteAddress() {
	boost::system::error_code ec;
	boost::asio::ip::sctp::endpoint endpoint = socket_->remote_endpoint(ec);
	if ( !ec ) {
		remoteHost_ = endpoint.address().to_string();
		remotePort_ = ( boost::format("%1%") % endpoint.port() ).str();
	}
}

void SctpConnection::getLocalAddress() {
	boost::system::error_code ec;
	boost::asio::ip::sctp::endpoint endpoint = socket_->local_endpoint(ec);
	if ( !ec ) {
		localHost_ = endpoint.address().to_string();
		localPort_ = ( boost::format("%1%") % endpoint.port() ).str();
	}
}

void SctpConnection::read() {
	startReadMessage();
}

void SctpConnection::connect(const string& host, const string& port) {
	remoteHost_ = host;
	remotePort_ = port;
	close();
	if (stopped_) return;
	boost::asio::ip::sctp::resolver::query query(remoteHost_, remotePort_);
	boost::shared_ptr<boost::asio::ip::sctp::resolver> resolver( new boost::asio::ip::sctp::resolver(eventReactor_->get_io_service()) );
	resolver->async_resolve(query, boost::bind(&SctpConnection::handleResolved, shared_from_this(), _1, _2, resolver) );
}

void SctpConnection::handleResolved(const boost::system::error_code &ec, const boost::asio::ip::sctp::resolver::iterator &iterator,
	const boost::shared_ptr<boost::asio::ip::sctp::resolver> &resolver) {
	if ( ec || iterator == boost::asio::ip::sctp::resolver::iterator() ) {
		std::cout << "fail to resolve address = " << remoteHost_<< ":" << remotePort_<< std::endl;
		fireConnectionInActvice();
		return;
	}

	std::vector<boost::asio::ip::sctp::endpoint> addrs;
	boost::asio::ip::sctp::endpoint ep = *iterator;
	addrs.push_back(ep);
	std::cout << "handleResolved " << ep.address().to_string() << ":" << ep.port() << std::endl;
	socket_->async_connect(addrs, boost::bind(&SctpConnection::handleConnected, shared_from_this(), _1));
}

void SctpConnection::handleConnected(const boost::system::error_code &ec) {
	if (ec) {
		std::cout << "fail connect to address  " << remoteHost_ << ":" << remotePort_ << " error:" << ec << std::endl;
		fireConnectionInActvice();
		return;
	}
	getLocalAddress();
	connected_ = true;
	setSocketOption();
	fireConnectionActvice();
	startReadMessage();
}

void SctpConnection::setSocketOption() {
	socket_->set_option(boost::asio::ip::sctp::sctp_ppid(29));
	socket_->set_option(boost::asio::socket_base::reuse_address(true));
	//socket_->set_option(boost::asio::socket_base::keep_alive(true)); 
	//socket_->set_option(boost::asio::ip::tcp::no_delay(true));
}

void SctpConnection::startReadMessage() {
	adjustReadBuffer();
	socket_->async_read_some(boost::asio::buffer(cumulation_->array() + cumulation_->writerIndex(), cumulation_->writableBytes()), 
		boost::bind(&SctpConnection::handleRead, shared_from_this(), _1, _2, _3,_4) );
}

void SctpConnection::handleRead(uint16_t stream_no, int flags, const boost::system::error_code &ec,  size_t bytes_transferred) {
	if (ec) {
		close();
		fireConnectionInActvice();
		return;
	}
	
	//std::cout << "stream_no:" << stream_no << " flag:" << flags  << " len:" << bytes_transferred << std::endl;
	cumulation_->writerIndex(cumulation_->writerIndex() + bytes_transferred);

	if (flags & MSG_EOR) {
		shared_ptr<CharBuf>  buf = getAndRemoveMessage(stream_no);
		if (!buf) {
			cumulation_->retain();
			messageReceived(bytes_transferred);
		} else {
			buf->writeChars(*cumulation_);
			record(bytes_transferred);
			pipeline_->fireChannelRead(shared_ptr<ChannelInboundEvent>(new ChannelInboundEvent(buf)));
			discardSomeBytes();
		}
	}  else {
		record(bytes_transferred);
		appendMessage(stream_no, cumulation_);
		discardSomeBytes();
	}
	
	startReadMessage();
}

void SctpConnection::close() {
	if (!connected_) return;
	boost::system::error_code ec;
	connected_  = false;
	socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket_->close(ec);
	freeBuffer();
	recvHandle.reset();
}

void SctpConnection::sendMessage(const string &msg, const boost::function<void(bool)> &callback) {
	asyncWriter_->write(msg, callback);
}

void SctpConnection::appendMessage(uint16_t stream_no, const shared_ptr<CharBuf>& buf) {
	boost::mutex::scoped_lock guard(mutex_);
	std::map<uint16_t,  shared_ptr<CharBuf> >::iterator iter = streamMsgs_.find(stream_no);
	if (iter != streamMsgs_.end()) {
		iter->second->writeChars(*buf);
		return;
	}
	shared_ptr<CharBuf> buffer (config()->getAllocator()->heapBuffer(buf->readableBytes() << 1));
	buffer->writeChars(*buf);
	streamMsgs_[ stream_no ] = buffer; 
}

shared_ptr<CharBuf> SctpConnection::getAndRemoveMessage(uint16_t stream_no) {
	boost::mutex::scoped_lock guard(mutex_);
	shared_ptr<CharBuf> buf;
	std::map<uint16_t, shared_ptr<CharBuf> >::iterator iter =  streamMsgs_.find(stream_no);
	if (iter != streamMsgs_.end()) {
		buf = iter->second;
		streamMsgs_.erase(iter);
	}
	return buf;
}

}//namespace net
}//namespace comm