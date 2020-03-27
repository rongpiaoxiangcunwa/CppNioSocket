#ifndef _AutorecoveringConnection_HPP_
#define _AutorecoveringConnection_HPP_

#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "AbstractConnection.hpp"
#include "ChannelInboundHandlerAdapter.hpp"

namespace comm { 
namespace net {
	
	class AutorecoveringConnection: 
		public Connection, 
		public ChannelInboundHandlerAdapter,
		public boost::enable_shared_from_this<AutorecoveringConnection> 
	{
	public:
		AutorecoveringConnection(const shared_ptr<Connection> &delegate) : delegate_(delegate), stopped_(false), retry_(false) {
		}

		~AutorecoveringConnection() {	
			pipeline()->remove(shared_from_this());
		}
		
		virtual shared_ptr<ChannelPipeline>& pipeline() { return delegate_->pipeline(); }
		virtual NioEventReactor* getEventReactor() { return delegate_->getEventReactor(); }
		virtual NioEventReactor* setEventReactor(NioEventReactor* reactor) {  delegate_->setEventReactor(reactor); }
		virtual ChannelConfig* config() { return delegate_->config(); }
		
		virtual const string& getLocalAddress() const { return delegate_->getLocalAddress(); } 
		virtual const string& getLocalPort() const { return delegate_->getLocalPort(); }
		virtual const string& getRemoteAddress() const { return delegate_->getRemoteAddress(); }
		virtual const string& getRemotePort() const { return delegate_->getRemotePort(); }
		virtual bool isConnected() { return delegate_->isConnected(); }
		
		virtual void read() {
			delegate_->read(); 
		} 
		
		virtual void connect(const string& host, const string& port){ 
			if (!retry_) pipeline()->addLast("AutorecoveringConnectionHandler", shared_from_this());
			delegate_->connect(host, port); 
		}
		
		virtual void close() { delegate_->close(); }	
		virtual void stop() { 
			stopped_ = true;
			delegate_->stop(); 
		}
		
		virtual void sendMessage(const string &msg, const boost::function<void(bool)> &callback) {
			delegate_->sendMessage(msg, callback) ;
		}

		virtual void channelInactive(ChannelHandlerContextPtr ctx) {
			if (!stopped_) connectToServerAfterSeconds(1);
			ChannelInboundHandlerAdapter::channelInactive(ctx);
		}
		virtual shared_ptr<Connection> get_shared() { return shared_from_this(); }
		virtual shared_ptr<ChannelHandler> getSharedPtr() { return shared_from_this(); }
		
	private:
		void connectToServerAfterSeconds(int seconds) {
			std::cout << "connect to server after " << seconds << " seconds" << std::endl;
			retry_ = true;
			if( seconds <= 0) {
				connect(delegate_->getRemoteAddress(), delegate_->getRemotePort());
			} else {
				boost::shared_ptr<boost::asio::deadline_timer> timer(new boost::asio::deadline_timer( getEventReactor()->get_io_service() ) );
				timer->expires_from_now( boost::posix_time::seconds( seconds ) );
				timer->async_wait( boost::bind(&AutorecoveringConnection::doReconnect, shared_from_this(), timer) );
			}
		}

		void doReconnect(const boost::shared_ptr<boost::asio::deadline_timer> &timer) {
			connect(delegate_->getRemoteAddress(), delegate_->getRemotePort());
		}
		
	private:
		shared_ptr<Connection> delegate_;
		volatile bool stopped_;
		volatile bool retry_;
	};
		
}//namespace net
}//namespace comm

#endif
