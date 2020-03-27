#include "ServerBootstrap.hpp"
#include "NioSctpConnectionAcceptor.hpp"
#include "NioTcpConnectionAcceptor.hpp"

namespace comm {
namespace net {
class ServerBootstrapAcceptor 
: public ChannelInboundHandlerAdapter, public boost::enable_shared_from_this<ServerBootstrapAcceptor>
{
public:
	ServerBootstrapAcceptor(NioEventReactorGroup *childGroup, const shared_ptr<ChannelHandler> &childHandler)
	: childGroup_(childGroup), childHandler_(childHandler) {
	}

	virtual shared_ptr<ChannelHandler> getSharedPtr() { return shared_from_this(); }
	virtual void channelRead(ChannelHandlerContextPtr ctx, const shared_ptr<Event> &event) {
		if (ConnectionAccepteddEvent::ID() != event->eventId()) {
			ctx->fireChannelRead(event);
			return;
		}

		shared_ptr<ConnectionAccepteddEvent> connEvent = boost::dynamic_pointer_cast<ConnectionAccepteddEvent>(event);
		if (connEvent) {
			Connection *conn = connEvent->getConnection().get();
			conn->pipeline()->addLast("", childHandler_);
			//conn->setEventReactor(childGroup_->next());
			conn->pipeline()->fireChannelRegistered(); // call user defined Handler
			conn->pipeline()->fireChannelActive();
			conn->read();
		}
	}
	
private:
	NioEventReactorGroup *childGroup_;
	shared_ptr<ChannelHandler> childHandler_;
};

////////////////////////////////////////// class ServerBootstrap ////////////////////////////////////////
ServerBootstrap::ServerBootstrap(): type_(COMM_NET_SERVER_SCTP), group_(0), childGroup_(0) {
}

ServerBootstrap& ServerBootstrap::connection(ServerBootstrap::ServerConnectionType type) {
	this->type_ = type;
	return *this;
}

ServerBootstrap& ServerBootstrap::group(NioEventReactorGroup *group, NioEventReactorGroup *childGroup) {
	group_ = group;
	childGroup_ = childGroup;
	return *this;
}

ServerBootstrap& ServerBootstrap::childHandler(const shared_ptr<ChannelHandler> &_childHandler) {
	childHandler_ = _childHandler;
	return *this;
}

ServerBootstrap& ServerBootstrap::bind(const std::vector<string>& hosts, const string& port) {
	connectionAcceptor_ = init();
	if (connectionAcceptor_) {
		connectionAcceptor_->bind(hosts, port);
		connectionAcceptor_->startAccept();
	}
	
	return *this;
}

ServerBootstrap& ServerBootstrap::bind(const string& host, const string& port) {
	connectionAcceptor_ = init();
	if (connectionAcceptor_) {
		connectionAcceptor_->bind(host, port);
		connectionAcceptor_->startAccept();
	}
	return *this;
}

shared_ptr<ConnectionAcceptor> ServerBootstrap::init() {
	shared_ptr<ConnectionAcceptor> acceptor;
	std::cout << "type_:" << (int)type_  << " " << COMM_NET_SERVER_TCP << std::endl;
    switch(type_) {
	    case COMM_NET_SERVER_TCP:
	   		acceptor.reset(new NioTcpConnectionAcceptor(group_->next(), childGroup_));
	    break;

	    case COMM_NET_SERVER_SCTP:
	    	acceptor.reset(new NioSctpConnectionAcceptor(group_->next(), childGroup_));
	    break;

	    default:
	    	throw std::runtime_error("Connection type is wrong");
    }
	
	if (acceptor) {
		shared_ptr<ChannelPipeline>& p = acceptor->pipeline();
		p->addLast("ServerBootstrapAcceptor", boost::shared_ptr<ChannelHandler>(new ServerBootstrapAcceptor(childGroup_, childHandler_)));
	}
	return acceptor;
}

}//namespace net
}//namespace comm
