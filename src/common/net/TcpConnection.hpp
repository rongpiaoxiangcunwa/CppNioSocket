#ifndef _TcpConnection_HPP_
#define _TcpConnection_HPP_

#include "AbstractConnection.hpp"
#include "BoostAsyncWriter.hpp"

namespace comm { 
namespace net {
	
class TcpConnection : public AbstractConnection, 
							public boost::enable_shared_from_this<TcpConnection>  {
public:
	TcpConnection();
	TcpConnection(NioEventReactor* eventReactor);
	TcpConnection(NioEventReactor* eventReactor, const string& remoteHost, const string& remotePort);
	TcpConnection(NioEventReactor* eventReactor, const boost::shared_ptr< boost::asio::ip::tcp::socket > &socket);
	~TcpConnection();
	virtual void read();
	virtual void connect(const string& host, const string& port);
	virtual void close();
	virtual void sendMessage(const string &msg, const boost::function<void(bool)> &callback) ;
	virtual shared_ptr<Connection> get_shared() { return shared_from_this(); }
	
protected:
	void init();
	void getLocalAddress();
	void getRemoteAddress();
	void handleResolved(const boost::system::error_code &error, const boost::asio::ip::tcp::resolver::iterator &iterator,
		const boost::shared_ptr<boost::asio::ip::tcp::resolver> &resolver);
	void handleConnected(const boost::system::error_code &ec, bool fire = true) ;
	void setSocketOption();
	void startReadMessage();	
	void handleRead(const boost::system::error_code &ec, size_t bytes_transferred);
	
private:
	boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;
	boost::shared_ptr< BoostAsyncWriter<boost::asio::ip::tcp::socket> > asyncWriter_;
};
		
}//namespace net
}//namespace comm

#endif
