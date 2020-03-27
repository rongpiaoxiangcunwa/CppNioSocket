#include "NioEventReactorGroup.hpp"
#include <boost/foreach.hpp>
#include <iostream>

namespace comm { 
namespace net {
	
bool NioEventReactorGroup::isPowerOfTwo(int val) {
    return (val & -val) == val;
}

NioEventReactorGroup::NioEventReactorGroup() {
	init(0);
}

NioEventReactorGroup::NioEventReactorGroup(int nThreads) {
	init(nThreads);
}

NioEventReactorGroup::~NioEventReactorGroup() {
	shutdownGracefully();
}

void NioEventReactorGroup::init(int nThreads) {
	if( 0 == nThreads) {
		nThreads = boost::thread::hardware_concurrency() * 2;
	}
	
	for(int i = 0; i < nThreads; ++i) {
		eventReators_.push_back( new NioEventReactor() );
	}

	if(isPowerOfTwo(nThreads)) {
		chooser_ = new PowerOfTwoEventReactorChooser(eventReators_);
	} else {
		chooser_ = new GenericEventReactorChooser(eventReators_);
	}
}

NioEventReactor* NioEventReactorGroup::next() {
	NioEventReactor* reactor = chooser_->next();
	if( !reactor->isInEventLoop() ) {
		reactor->start();
	}
	//std::cout << "NioEventReactorGroup::next() " << reactor << std::endl;
	return reactor;
}

void NioEventReactorGroup::shutdownGracefully() {
	BOOST_FOREACH(NioEventReactor* eventReator, eventReators_) {
		eventReator->shutdownGracefully();
		delete eventReator;
	}
	eventReators_.clear();
}

}//namespace net
}//namespace common