#include "NioEventReactor.hpp"
#include <iostream>
#include <boost/bind.hpp>

namespace comm { 
namespace net {
	
NioEventReactor::NioEventReactor() : stop_(false), io_service_work_(io_service_) {
}

NioEventReactor::~NioEventReactor() {
	shutdownGracefully();
}

void NioEventReactor::start() {
	boost::lock_guard<boost::mutex> lock(mutex_);
	if( isInEventLoop() ) return;
	serviceRunThread_.reset( new boost::thread(boost::bind(&NioEventReactor::run, this)) );
}

bool NioEventReactor::isInEventLoop() {
	return  !!serviceRunThread_ ;
}

void NioEventReactor::shutdownGracefully() {
	if( stop_ || !serviceRunThread_ ) return;
	stop_ = true;
	io_service_.stop();
	serviceRunThread_->join();
}

void NioEventReactor::run() {
	while( !stop_ ) {
		try {
			io_service_.run();
		} catch(std::exception &ex) {
			std::cout << ex.what() << std::endl;
		}
	}
}

}//namespace net
}//namespace comm