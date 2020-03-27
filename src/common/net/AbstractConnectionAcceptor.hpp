#ifndef COMM_NET_ABSTRACT_CONNECTION_ACCEPTOR_HPP
#define COMM_NET_ABSTRACT_CONNECTION_ACCEPTOR_HPP

#include <iostream>
#include "NioEventReactor.hpp"
#include "ConnectionAcceptor.hpp"
#include "DefaultChannelPipeline.hpp"
#include "DefaultChannelConfig.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace comm {
namespace net {

class ConnectionAccepteddEvent : public Event {
public:
	static int ID() { return 3; }
	
	ConnectionAccepteddEvent(const shared_ptr<Connection>& newConn) : newConn_(newConn) {		
	}
	
	virtual int eventId() { return ID();}
	virtual const string& name() { return "ConnectionAccepteddEvent"; }

	shared_ptr<Connection>& getConnection() { return newConn_;}
	void setConnection(const shared_ptr<Connection>& newConn) { newConn_ = newConn; }
	
private:
	shared_ptr<Connection> newConn_;
};

/////////////////// class AbstractConnectionAcceptor //////////////////
template<typename Protocol>
class AbstractConnectionAcceptor : public ConnectionAcceptor
{
public:
	typedef typename Protocol::endpoint endpoint;
	typedef typename Protocol::acceptor acceptor;
	typedef typename Protocol::socket socket;
	
	AbstractConnectionAcceptor(NioEventReactor *reactor, NioEventReactorGroup *childGroup) : reactor_(reactor), childGroup_(childGroup) {
		init();
	}

	virtual shared_ptr<ChannelPipeline>& pipeline() { return pipeline_;}
	virtual NioEventReactor* getEventReactor() const { return reactor_; }
	virtual ChannelConfig* config() { return config_.get(); }
	
	virtual void bind(const std::vector<string>& hosts, const string& port) {
		hosts_ = hosts;
		port_ = port;
		doBind();
	}
	
	virtual void bind(const string& host, const string& port) {
		hosts_.push_back(host);
		port_ = port;
		doBind();
	}

	virtual void startAccept() {
		if (acceptor_) {
			NioEventReactor *childReactor = childGroup_->next();
			boost::shared_ptr<socket>  sock(new socket(childReactor->get_io_service()));
			acceptor_->async_accept(*sock, boost::bind(&AbstractConnectionAcceptor::handleAccept, get_shared_ptr(), _1, sock, childReactor));
		}
	}
	
protected:
	virtual shared_ptr<AbstractConnectionAcceptor> get_shared_ptr() = 0;
	virtual void bindAddress(std::vector<endpoint>& endpoints) = 0;
	virtual shared_ptr<Connection> newConnection(const boost::shared_ptr<socket> &sock, NioEventReactor *childReactor) = 0;

	void handleAccept(const boost::system::error_code &error, const boost::shared_ptr<socket> &sock, NioEventReactor *childReactor) {
		if (error) {
			if (doesNeedRetry(error)) {
				startAccept();
			} else {
				boost::system::error_code ec;
				acceptor_->cancel(ec);
				acceptor_->close(ec);
				restartBind();
			}
			return;
		}
		
		fireNewConnection(newConnection(sock, childReactor));
	}
	
	void init() {
		pipeline_.reset(new DefaultChannelPipeline(0));
		config_.reset(new DefaultChannelConfig());
	}

	void fireNewConnection(const shared_ptr<Connection> &conn) {
		pipeline()->fireChannelRead(shared_ptr<Event>(new ConnectionAccepteddEvent(conn)));
		startAccept();
	}
	
	void restartBind() {
		std::cout << "restartBind()" << std::endl;
		doBind();
		startAccept();
	}
	
	void doBind() {
		std::vector<endpoint> endpoints;
		resolverAddresses(endpoints);
		bindAddress(endpoints);
	}
	
	void resolverAddresses(std::vector<endpoint>& endpoints) {
		for(std::vector<string>::const_iterator iter = hosts_.begin(); iter != hosts_.end(); ++iter) {
			resolverAddress(endpoints, *iter, port_);
		}
		std::cout << "resolverAddresss size:" << endpoints.size() << std::endl;
	}

	void resolverAddress(std::vector<endpoint>& endpoints, const string& host, const string &port) {
		typename Protocol::resolver::query query(host, port_);
		typename Protocol::resolver resolver(reactor_->get_io_service());
		boost::system::error_code ec;
		typename Protocol::resolver::iterator iter = resolver.resolve(query, ec);
		if (!ec) {
			typename Protocol::resolver::iterator end;
			for (; iter != end; ++iter) {
				endpoints.push_back(*iter);
			}
		}
	}

	bool doesNeedRetry(const boost::system::error_code& error) {
		return error == boost::asio::error::would_block || error == boost::asio::error::try_again ||
			   error == boost::asio::error::connection_aborted || error == boost::asio::error::interrupted;
	}
	
protected:
	NioEventReactor *reactor_;
	NioEventReactorGroup *childGroup_;
	std::vector<string> hosts_;
	string port_;
 	shared_ptr<ChannelPipeline> pipeline_;
	shared_ptr<ChannelConfig> config_;
	shared_ptr<acceptor> acceptor_;
};

}//namespace net
}//namespace comm

#endif
