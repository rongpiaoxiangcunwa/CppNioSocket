#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <ConnectionFactory.hpp>
#include "ChannelInitializer.hpp"
#include "ChannelInboundEvent.hpp"
#include "ChannelOutboundEvent.hpp"
#include "UnpooledHeapCharBuf.hpp"
#include "codec/LengthFieldBasedFrameDecoder.hpp"
#include "ExecutorService.hpp"
#include "AtomicInteger.hpp"
#include "SystemUtil.hpp"

using std::cout;
using std::endl;
using std::string;
using namespace comm::net;
AtomicInteger<int> toatlaMsgs(0);

static void sendComplete(bool ok) { 
	if (!ok) std::cout << "sendComplete failed" << std::endl; 
}

class ClientMessageHandler : 
	public ChannelInboundHandlerAdapter,
	public boost::enable_shared_from_this<ClientMessageHandler>
{
public:
	ClientMessageHandler(const boost::function<void(bool)> &connListener): connListener_(connListener) {
		cout << "ClientMessageHandler::ClientMessageHandler " << this << endl;
	}
	
	virtual void channelActive(ChannelHandlerContextPtr ctx) {
		Connection *conn = ctx->connection();
		cout << "ClientMessageHandler::channelActive() " << conn << " conection with remote[" << conn->getRemoteAddress() << ":" << conn->getRemotePort() << "] local["<< conn->getLocalAddress() << ":" << conn->getLocalPort() << "] is successful" << endl;
		//ctx->fireChannelActive();
		std::string tmpMsg("connection is ready");
		shared_ptr<comm::buffer::UnpooledHeapCharBuf> charBuf(new comm::buffer::UnpooledHeapCharBuf());
		charBuf->writeUInt(tmpMsg.length());
		charBuf->writeString(tmpMsg);
		ctx->write(shared_ptr<ChannelOutboundEvent>(new ChannelOutboundEvent(charBuf)), boost::bind(&::sendComplete, _1));
		connListener_(true);
	}
	
	virtual void channelInactive(ChannelHandlerContextPtr ctx) {
		Connection *conn = ctx->connection();
		cout << "ClientMessageHandler::channelInactive() " << conn << " conection with remote[" << conn->getRemoteAddress() << ":" << conn->getRemotePort() << "] local["<< conn->getLocalAddress() << ":" << conn->getLocalPort() << "] is failed" << endl;
		//ctx->fireChannelActive();connListener_(false);
		ctx->fireChannelInactive();
		connListener_(false);
	}
		
	virtual void channelRead(ChannelHandlerContextPtr ctx, const boost::shared_ptr<Event> &event) {
		//cout << "ClientMessageHandler::channelRead:" <<  event->eventId() << endl;
		if (event->eventId() == ChannelInboundEvent::ID()) {
			boost::shared_ptr<ChannelInboundEvent> inEvent = boost::dynamic_pointer_cast<ChannelInboundEvent>(event);
			handle(inEvent->getMessage());
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
	void handle(const boost::shared_ptr<CharBuf> &charBuf) {
		int len = charBuf->readInt();
		string msg;
		charBuf->readString(msg, len);
		toatlaMsgs.incrementAndGet();
		if (charBuf->readableBytes() != 0)cout << "message length:"  << len  << " charBuf still has:" << charBuf->readableBytes() << endl;
	}

private:
	 boost::function<void(bool)>  connListener_;
};

///////////////////// class AbstractClient /////////////////////////////////////////
class AbstractClient : public ChannelInitializer
{
public:
	AbstractClient(const string& addr, const string& port): addr_(addr), port_(port), group_(2), service(1) {
	}

	virtual void start() {
		conn_ = connectionFactory.group(&group_)
							     .connection(getType())
						         .remoteAddress(addr_, port_)
							     .handler(getSharedPtr())
							     .connect();
	}

	void sendMessage(const string& msg) {
		conn_->sendMessage(msg, boost::bind(&::sendComplete, _1) );
	}

protected:
	virtual ConnectionFactory::ConnectionType getType() {return ConnectionFactory::COMM_NET_SCTP;}

	void channelStatusChanged(bool success) {
		cout << "AbstractClient::channelStatusChanged:" << success << endl;
		if ( success ) {
			service.submit(boost::bind(&AbstractClient::doSendMessages, this));
		}
	}
	
	void doSendMessages() {
		sleep(1);
		cout << "input [msg size] [TPS] [iterator numbers]: " ;
		int size = 0, tps = 0, iterNum = 0;
		while(std::cin >> size >> tps >> iterNum) {
			std::string tmpMsg(size, '1');
			comm::buffer::UnpooledHeapCharBuf charBuf(size * 2);
			charBuf.writeUInt(size);
			charBuf.writeString(tmpMsg);
			string msg;
			charBuf.readString(msg, 4 + size);
			std::cout << "message size:" << msg.length() << " max iterations:" << iterNum << std::endl;
			
			while(iterNum-- > 0) {
				int tmpTps = tps;
				uint64_t st = SystemUtil::currentTimeMillis();
				while (tmpTps-- > 0) sendMessage(msg);
				uint64_t ed = SystemUtil::currentTimeMillis();
				int cost = ed - st;
				if (cost < 1000) {
					usleep(1000*(1000 - cost));
				} else {
					cout << "cost " << cost << endl ;
				}
			}
			
			cout << "input [msg size] [TPS] [iterator numbers]: " ;
		} 
	}
	
protected:
	string addr_;
	string port_;
	boost::shared_ptr<Connection> conn_;
	NioEventReactorGroup group_;
	ConnectionFactory connectionFactory;
	ExecutorService service;
};

///////////////////// class SctpClient /////////////////////////////////////////
class SctpClient
: public AbstractClient,
  public boost::enable_shared_from_this<SctpClient> {
public:
	SctpClient(const string& addr, const string& port): AbstractClient(addr, port) {
	}
	
	virtual boost::shared_ptr<ChannelHandler> getSharedPtr() {
		return shared_from_this();
	}
	
protected:
	virtual void initChannel(Connection *conn) {
		cout << "SctpClient::initChannel " << this << endl;
		ChannelPipeline& pipeline = *(conn->pipeline());
		pipeline.addLast("ClientMessageHandler", boost::shared_ptr<ClientMessageHandler>(new ClientMessageHandler(boost::bind(&SctpClient::channelStatusChanged, this, _1))))
		 		;
	}
};

///////////////////// class TcpClient /////////////////////////////////////////
class TcpClient
: public AbstractClient,
  public boost::enable_shared_from_this<TcpClient> {
public:
	TcpClient(const string& addr, const string& port): AbstractClient(addr, port) {
	}
	
	virtual boost::shared_ptr<ChannelHandler> getSharedPtr() {
		return shared_from_this();
	}
	
protected:
	virtual ConnectionFactory::ConnectionType getType() {return ConnectionFactory::COMM_NET_TCP;}
	
	virtual void initChannel(Connection *conn) {
		cout << "TcpClient::initChannel " << this << endl;
		ChannelPipeline& pipeline = *(conn->pipeline());
		pipeline.addLast("LengthFieldBasedFrameDecoder", boost::shared_ptr<ChannelHandler>(new LengthFieldBasedFrameDecoder(8192, 0, 4)));
		pipeline.addLast("ClientMessageHandler", boost::shared_ptr<ChannelHandler>(new ClientMessageHandler(boost::bind(&TcpClient::channelStatusChanged, this, _1))));
	}
};

int main(int argc, char* argv[])
{
	if (argc < 4) {
		cout << "Usage: " << argv[0] << "host port tcp/sctp" << endl;
		return -1;
	}
	
	boost::shared_ptr<AbstractClient> c;
	if (boost::iequals("sctp", argv[3])) {
		c.reset(new SctpClient(argv[1], argv[2]));
	} else {
		c.reset(new TcpClient(argv[1], argv[2]));
	}
	
	c->start();
	
	while(1) { 
		sleep(10);
		cout << "Total messages:" << toatlaMsgs.get() << endl;
	}
}
