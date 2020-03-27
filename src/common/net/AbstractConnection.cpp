#include "AbstractConnection.hpp"
#include "DefaultChannelConfig.hpp"
#include "ChannelInboundEvent.hpp"
#include <iostream>
using std::cout;
using std::endl;

namespace comm { 
namespace net {
AbstractConnection::AbstractConnection() : eventReactor_(0) {
	init();
}

AbstractConnection::AbstractConnection(NioEventReactor* eventReactor) 
: eventReactor_(eventReactor) {
	init();
}

AbstractConnection::AbstractConnection(NioEventReactor* eventReactor, const string& remoteHost, const string& remotePort) 
: eventReactor_(eventReactor), remoteHost_(remoteHost), remotePort_(remotePort) {
	init();
}

AbstractConnection::~AbstractConnection() { 
	freeBuffer();	
}

void AbstractConnection::freeBuffer() {
	if (cumulation_) {
		cumulation_->release();
		cumulation_.reset();
	}
}


void AbstractConnection::init() {
	readBufferLen_= 0;
	connected_ = false;
	stopped_ = false;
	channelConfig.reset(new DefaultChannelConfig());
	pipeline_ = newChannelPipeline();
	adjustReadBuffer();
}

void AbstractConnection::adjustReadBuffer() {
	shared_ptr<RecvCharBufAllocator::Handle>& allocHandle = recvBufAllocHandle();
	if (!cumulation_ || readBufferLen_ != allocHandle->guess() || !cumulation_->isWritable()) {
		if (allocHandle->guess() < readBufferLen_ && cumulation_) {
			cumulation_->capacity(allocHandle->guess());
		} else {
			if (cumulation_ && readBufferLen_ != 0) cout << "adjust readBuffer from " << readBufferLen_ << " to " << allocHandle->guess() << endl;
			freeBuffer();
			readBufferLen_ = allocHandle->guess();
			cumulation_ = config()->getAllocator()->ioBuffer(readBufferLen_);
			//std::cout << "AbstractConnection::adjustReadBuffer() "<< cumulation_.get() << " refCnt:" << cumulation_->refCnt() << " free len:" << cumulation_->writableBytes() << std::endl;
		}
	}
	allocHandle->attemptedBytesRead(cumulation_->writableBytes());
	allocHandle->reset(config());
}

const string& AbstractConnection::getLocalAddress() const {
	return localHost_;
}

const string& AbstractConnection::getLocalPort() const {
	return localPort_;
}
		
const string& AbstractConnection::getRemoteAddress() const { 
	return remoteHost_;
}

const string& AbstractConnection::getRemotePort() const {
	return remotePort_; 
}

bool AbstractConnection::isConnected() {
	return connected_;
}

void AbstractConnection::messageReceived(size_t len) {
	record(len);
	pipeline_->fireChannelRead(shared_ptr<ChannelInboundEvent>(new ChannelInboundEvent(cumulation_)));
	discardSomeBytes();
}

void AbstractConnection::record(size_t len) {
	shared_ptr<RecvCharBufAllocator::Handle>& allocHandle = recvBufAllocHandle();
	if (len > 0) {
		allocHandle->lastBytesRead(len);
	}
	allocHandle->readComplete();
}

void AbstractConnection::discardSomeBytes() {
	if (cumulation_->refCnt() > 1) {
		//std::cout << "AbstractConnection::discardSomeBytes " << cumulation_.get() << " refCnt:" << cumulation_->refCnt() << std::endl;
		freeBuffer();
	} else if (cumulation_->refCnt() <= 1) {
		//cout << "AbstractConnection::messageReceived() discardSomeReadBytes " << cumulation_->readerIndex() << endl;
		cumulation_->discardSomeReadBytes();
	}
}
	
}//namespace net
}//namespace comm
