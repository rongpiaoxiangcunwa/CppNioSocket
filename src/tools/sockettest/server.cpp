#include <iostream>
#include <string>
#include <signal.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <ServerBootstrap.hpp>
#include "ChannelInitializer.hpp"
#include "ChannelInboundEvent.hpp"
#include "ChannelOutboundEvent.hpp"
#include "UnpooledHeapCharBuf.hpp"
#include "codec/LengthFieldBasedFrameDecoder.hpp"
#include "ExecutorService.hpp"
#include "AtomicInteger.hpp"
#include "ChannelOutboundEvent.hpp"

using std::cout;
using std::endl;
using std::string;
using namespace comm::net;
AtomicInteger<int> toatlaMsgs(0);

static void sendComplete(bool ok) { 
	#ifdef DEBUG_LOG
	if (!ok) std::cout << "sendComplete" << std::endl; 
	#endif
}

class ClientMessageHandler : 
	public ChannelInboundHandlerAdapter,
	public boost::enable_shared_from_this<ClientMessageHandler>
{
public:
	ClientMessageHandler(const boost::function<void(bool,const shared_ptr < Connection > & )> &connListener): connListener_(connListener) {
		//cout << "ClientMessageHandler::ClientMessageHandler " << this << endl;
	}
	
	virtual void channelActive(ChannelHandlerContextPtr ctx) {
		Connection *conn = ctx->connection();
		//cout << "ClientMessageHandler::channelActive() " << conn << " conection with remote[" << conn->getRemoteAddress() << ":" << conn->getRemotePort() << "] local["<< conn->getLocalAddress() << ":" << conn->getLocalPort() << "] is successful" << endl;
		connListener_(true, conn->get_shared());
	}
	
	virtual void channelInactive(ChannelHandlerContextPtr ctx) {
		Connection *conn = ctx->connection();
		cout << "ClientMessageHandler::channelInactive() " << conn << " conection with remote[" << conn->getRemoteAddress() << ":" << conn->getRemotePort() << "] local["<< conn->getLocalAddress() << ":" << conn->getLocalPort() << "] is failed" << endl;
		conn->close();
		connListener_(false, conn->get_shared());
		ctx->fireChannelInactive();
	}
		
	virtual void channelRead(ChannelHandlerContextPtr ctx, const boost::shared_ptr<Event> &event) {
		if (event->eventId() == ChannelInboundEvent::ID()) {
			boost::shared_ptr<ChannelInboundEvent> inEvent = boost::dynamic_pointer_cast<ChannelInboundEvent>(event);
			handle(inEvent->getMessage());
			sendAck(ctx);
		}
		ChannelInboundHandlerAdapter::channelRead(ctx, event);
	}
	
	virtual boost::shared_ptr<ChannelHandler> getSharedPtr() {
		return shared_from_this();
	}

	virtual void exceptionCaught(ChannelHandlerContextPtr ctx, const std::exception &ex) {
		cout << "ClientMessageHandler::exceptionCaught:" <<  ex.what() << endl;
	}
	
protected:
	void sendAck(ChannelHandlerContextPtr ctx) {
		std::string tmpMsg("ACK");
		shared_ptr<comm::buffer::CharBuf> charBuf = ctx->alloc()->heapBuffer(1024);
		//shared_ptr<comm::buffer::UnpooledHeapCharBuf> charBuf(new comm::buffer::UnpooledHeapCharBuf(10));
		charBuf->writeUInt(tmpMsg.length());
		charBuf->writeString(tmpMsg);
		ctx->write(shared_ptr<ChannelOutboundEvent>(new ChannelOutboundEvent(charBuf)), boost::bind(&::sendComplete, _1));
	}
	
	void handle(const boost::shared_ptr<CharBuf> &charBuf) {
		//int len = charBuf->readInt();
		//string msg;
		//charBuf->readString(msg, len);
		toatlaMsgs.incrementAndGet();
		//if (charBuf->readableBytes() != 0 )
		//cout << "message length:"  << len  << " charBuf still has:" << charBuf->readableBytes() << endl;
	}

private:
	 boost::function<void(bool, const shared_ptr < Connection > & )>  connListener_;
};
	
///////////////////// class ConnectionCache /////////////////////////////////////////
class ConnectionCache {
public:
	void add(const boost::shared_ptr<Connection> & conn) {
		boost::lock_guard<boost::recursive_mutex> guard(mutex_);
		conns_.insert(conn);
	}

	void remove(const boost::shared_ptr<Connection>& conn) {
		boost::lock_guard<boost::recursive_mutex> guard(mutex_);
		conns_.erase(conn);
	}

	size_t size() {
		boost::lock_guard<boost::recursive_mutex> guard(mutex_);
		return conns_.size();
	}
	
private:
	boost::recursive_mutex mutex_;
	std::set<boost::shared_ptr<Connection> > conns_;
};
///////////////////// class AbstractServer /////////////////////////////////////////
class AbstractServer : public ChannelInitializer
{
public:
	AbstractServer(const string& addr, const string& port) : addr_(addr), port_(port), group_(2), childGroup_(2), service(1) {
	}
	
	virtual void start() {
		booststrap.group(&group_, &childGroup_)
			      .connection(getType())
			      .childHandler(getSharedPtr())
			      .bind(addr_, port_);
	}

protected:
	virtual ServerBootstrap::ServerConnectionType getType() { return ServerBootstrap::COMM_NET_SERVER_SCTP; }
	
	void channelStatusChanged(bool success, const shared_ptr < Connection > & conn) {
		if (!success) {
			conns_.remove(conn);
			cout << "[TID#" << gettid() << "] total connections:" << conns_.size() << endl;
		}
	}
	
protected:
	string addr_;
	string port_;
	ConnectionCache conns_;
	NioEventReactorGroup group_;
	NioEventReactorGroup childGroup_;
	ServerBootstrap booststrap;
	ExecutorService service;	
};

///////////////////// class SctpServer /////////////////////////////////////////
class SctpServer
: public AbstractServer ,
  public boost::enable_shared_from_this<SctpServer> {
public:
	SctpServer(const string& addr, const string& port) : AbstractServer(addr, port) {
	}
	
	virtual boost::shared_ptr<ChannelHandler> getSharedPtr() {
		return shared_from_this();
	}
	
protected:
	virtual void initChannel(Connection *conn) {
		ChannelPipeline& pipeline = *(conn->pipeline());
		pipeline.addLast("ClientMessageHandler", boost::shared_ptr<ClientMessageHandler>(new ClientMessageHandler(boost::bind(&SctpServer::channelStatusChanged, this, _1, _2))));
		conns_.add(conn->get_shared());
		cout  << time(0) << " total connections:" << conns_.size() << endl;
	}
};


///////////////////// class SctpServer /////////////////////////////////////////
class TcpServer
: public AbstractServer ,
  public boost::enable_shared_from_this<TcpServer> {
public:
	TcpServer(const string& addr, const string& port) : AbstractServer(addr, port) {
	}
	
	virtual boost::shared_ptr<ChannelHandler> getSharedPtr() {
		return shared_from_this();
	}
	
protected:
	virtual ServerBootstrap::ServerConnectionType getType() { return ServerBootstrap::COMM_NET_SERVER_TCP; }
	
	virtual void initChannel(Connection *conn) {
		ChannelPipeline& pipeline = *(conn->pipeline());
		pipeline.addLast("LengthFieldBasedFrameDecoder", boost::shared_ptr<ChannelHandler>(new LengthFieldBasedFrameDecoder(8192, 0, 4)));
		pipeline.addLast("ClientMessageHandler", boost::shared_ptr<ChannelHandler>(new ClientMessageHandler(boost::bind(&TcpServer::channelStatusChanged, this, _1, _2))));
		conns_.add(conn->get_shared());
		cout  << time(0) << " total connections:" << conns_.size() << endl;
	}
};


void handleStopSignal( int sig ) {
	cout << " handleStopSignal " << sig << endl;
	if (SIGPIPE == sig) {
		::signal(SIGPIPE, handleStopSignal);
	} 
}


int main(int argc, char* argv[])
{
	if (argc < 4) {
		cout << "Usage: " << argv[0] << "host port tcp/sctp" << endl;
		return -1;
	}
	
	::signal( SIGPIPE, handleStopSignal );
	
	boost::shared_ptr<AbstractServer> s;
	if (boost::iequals("sctp", argv[3])) {
		s.reset(new SctpServer(argv[1], argv[2]));
	} else {
		s.reset(new TcpServer(argv[1], argv[2]));
	}
	
	s->start();
	
	while(1) { 
		sleep(10);
		cout << time(0) << " Total messages:" << toatlaMsgs.get() << endl;
	}
}
