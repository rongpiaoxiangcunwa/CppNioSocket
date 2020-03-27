#include "DefaultChannelConfig.hpp"
#include "AdaptiveRecvByteBufAllocator.hpp"

namespace comm {  namespace net {
	
	DefaultChannelConfig::DefaultChannelConfig() {
		recvAlloctor.reset(new AdaptiveRecvByteBufAllocator());
		allocator = CharBufAllocator::DEFAULT_ALLOCATOR;
	}
	
	shared_ptr<RecvCharBufAllocator> DefaultChannelConfig::getRecvCharBufAllocator() {
		return recvAlloctor;
	}

	CharBufAllocator* DefaultChannelConfig::getAllocator() {
		return allocator;
	}
	
	void DefaultChannelConfig::setAllocator(CharBufAllocator* _allocator) {
		allocator = _allocator;
	}
	
}}//namespace comm