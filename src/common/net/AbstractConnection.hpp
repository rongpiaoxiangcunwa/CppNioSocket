#ifndef _AbstractConnection_HPP_
#define _AbstractConnection_HPP_

#include <boost/shared_array.hpp>
#include "Connection.hpp"
#include "ConnectionListenerPipeline.hpp"
#include "CharBufAllocator.hpp"
#include "DefaultChannelPipeline.hpp"
#include "ChannelConfig.hpp"

using comm::buffer::CharBufAllocator;
using comm::buffer::CharBuf;

namespace comm { 
namespace net {
	
class AbstractConnection : public Connection {
public:
	AbstractConnection();
	AbstractConnection(NioEventReactor* eventReactor);
	AbstractConnection(NioEventReactor* eventReactor, const string& remoteHost, const string& remotePort);
	virtual ~AbstractConnection();
	virtual shared_ptr<ChannelPipeline>& pipeline() { return pipeline_; }
	virtual NioEventReactor* getEventReactor() { return eventReactor_ ; }
	virtual NioEventReactor* setEventReactor(NioEventReactor* reactor) { eventReactor_ = reactor; }
	virtual ChannelConfig* config() { return channelConfig.get(); }
	
	virtual const string& getLocalAddress() const;
	virtual const string& getLocalPort() const;
	virtual const string& getRemoteAddress() const;
	virtual const string& getRemotePort() const;
	virtual bool isConnected();
	virtual void stop() { stopped_ = true;}	
	
protected:
	void init();
	void messageReceived(size_t len);

	void fireConnectionActvice() {
		pipeline()->fireChannelActive();
	}

	void fireConnectionInActvice() {
		pipeline()->fireChannelInactive();
	}

	shared_ptr<ChannelPipeline> newChannelPipeline() {
		return shared_ptr<ChannelPipeline>(new DefaultChannelPipeline(this));
	}

	shared_ptr<RecvCharBufAllocator::Handle>& recvBufAllocHandle() {
		if (!recvHandle) {
			recvHandle.reset( channelConfig->getRecvCharBufAllocator()->newHandle() );
		}
		return recvHandle;
	}

	void freeBuffer();
	void adjustReadBuffer();
	void record(size_t len);
	void discardSomeBytes();
	
protected:
	NioEventReactor *eventReactor_;
	string remoteHost_;
	string remotePort_;
	volatile bool connected_;
	string localHost_;
	string localPort_;
	shared_ptr<ChannelPipeline> pipeline_;
	shared_ptr<ChannelConfig> channelConfig;
	shared_ptr<RecvCharBufAllocator::Handle> recvHandle;
	int readBufferLen_;//8192
	shared_ptr<CharBuf> cumulation_;
	volatile bool stopped_;
};
		
}//namespace net
}//namespace comm

#endif
