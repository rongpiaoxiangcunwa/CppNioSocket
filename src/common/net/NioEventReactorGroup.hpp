#ifndef _NioEventReactorGroup_HPP_
#define _NioEventReactorGroup_HPP_

#include <vector>
#include <AtomicInteger.hpp>
#include "NioEventReactor.hpp"

namespace comm { 
namespace net {
	
class NioEventReactorChooser;
	
class NioEventReactorGroup  {
public:
	NioEventReactorGroup();
	NioEventReactorGroup(int nThreads);
	~NioEventReactorGroup();
	NioEventReactor* next();
	void shutdownGracefully();
	
private:
	static bool isPowerOfTwo(int val);
	void init(int threads);
	
private:
	std::vector<NioEventReactor*> eventReators_;
	NioEventReactorChooser *chooser_;
};

class NioEventReactorChooser {
public:
	NioEventReactorChooser(const std::vector<NioEventReactor*> &eventReators) : eventReators_(eventReators) {
		size_ = eventReators_.size();
	}

	virtual ~NioEventReactorChooser() {}
	virtual NioEventReactor* next() = 0;
	
protected:
	const std::vector<NioEventReactor*> &eventReators_;
	int size_;
	AtomicInteger<int> childIndex_;
};

class PowerOfTwoEventReactorChooser : public NioEventReactorChooser {
public:
	PowerOfTwoEventReactorChooser(const std::vector<NioEventReactor*> &eventReators) 
	: NioEventReactorChooser(eventReators) {
	}
	
	virtual NioEventReactor* next() {
		return eventReators_[childIndex_.getAndIncrement() & (size_ - 1)];
	}
};

class GenericEventReactorChooser : public NioEventReactorChooser {
public:
	GenericEventReactorChooser(const std::vector<NioEventReactor*> &eventReators) 
	: NioEventReactorChooser(eventReators) {
	}
	
	virtual NioEventReactor* next() {
		return eventReators_[childIndex_.getAndIncrement() % size_];
	}
};

}//namespace net
}//namespace common
#endif
