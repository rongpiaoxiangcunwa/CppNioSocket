#ifndef COMMON_NET_SCTPCONNECTION_HPP
#define COMMON_NET_SCTPCONNECTION_HPP

#include <boostsctp/asio/ip/sctp.hpp>
#include "AbstractConnection.hpp"
#include "BoostAsyncWriter.hpp"

using std::string;

namespace comm { 
namespace net {
	
class SctpConnection  : public AbstractConnection, public boost::enable_shared_from_this<SctpConnection>
{
public:
	SctpConnection() {}
	SctpConnection(NioEventReactor* eventReactor);
	SctpConnection(NioEventReactor* eventReactor, const string& remoteAddrs, const string& remotePort);
	SctpConnection(NioEventReactor* eventReactor, const std::vector<string>& remoteAddrs, const string& remotePort);
	SctpConnection(NioEventReactor* eventReactor, const boost::shared_ptr< boost::asio::ip::sctp::socket > &socket);
	~SctpConnection();
	virtual void read();
	virtual void connect(const string& host, const string& port);
	virtual void close();
	virtual void sendMessage(const string &msg, const boost::function<void(bool)> &callback) ;
	virtual shared_ptr<Connection> get_shared() { return shared_from_this(); }
	
protected:
	void init();
	void getLocalAddress();
	void getRemoteAddress();
	void handleResolved(const boost::system::error_code &error, const boost::asio::ip::sctp::resolver::iterator &iterator,
		const boost::shared_ptr<boost::asio::ip::sctp::resolver> &resolver);
	void handleConnected(const boost::system::error_code &ec) ;
	void setSocketOption();
	void startReadMessage();	
	void handleRead(uint16_t stream_no, int flags, const boost::system::error_code &ec, size_t bytes_transferred);
	void appendMessage(uint16_t stream_no, const shared_ptr<CharBuf>& buf);
	shared_ptr<CharBuf> getAndRemoveMessage(uint16_t stream_no);
	
private:
	boost::shared_ptr<boost::asio::ip::sctp::socket> socket_;
	boost::shared_ptr< BoostAsyncWriter<boost::asio::ip::sctp::socket> > asyncWriter_;
	boost::mutex mutex_;
	std::map<uint16_t,  shared_ptr<CharBuf> > streamMsgs_;
};
		
}//namespace net
}//namespace comm

#endif
