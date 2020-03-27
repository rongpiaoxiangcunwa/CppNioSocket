#ifndef _MaxMessagesRecvCharBufAllocator_hpp_
#define _MaxMessagesRecvCharBufAllocator_hpp_

#include "RecvCharBufAllocator.hpp"

namespace comm { 
namespace net {
	class MaxMessagesRecvCharBufAllocator : public RecvCharBufAllocator {
	public:
		/**
	     * Returns the maximum number of messages to read per read loop.
	     */
		virtual int maxMessagesPerRead() = 0;

	       /**
	     * Sets the maximum number of messages to read per read loop.
	     * If this value is greater than 1, an event loop might attempt to read multiple times to procure multiple messages.
	     */
    		virtual MaxMessagesRecvCharBufAllocator& maxMessagesPerRead(int maxMessagesPerRead) = 0;
	};
}//namespace net
}//namespace comm

#endif
