#ifndef _RecvCharBufAllocator_hpp_
#define  _RecvCharBufAllocator_hpp_

#include "CharBufAllocator.hpp"

using namespace comm::buffer;

namespace comm { 
namespace net {
	class ChannelConfig;
	class RecvCharBufAllocator {
	public:
		class Handle {
		public:
			/**
			 * Creates a new receive buffer whose capacity is probably large enough to read all inbound data and small
			 * enough not to waste its space.
			 */
			virtual shared_ptr<CharBuf> allocate(CharBufAllocator *alloc) = 0;

			/**
			 * Similar to {@link #allocate(CharBufAllocator)} except that it does not allocate anything but just tells the capacity.
			 */
			virtual int guess() = 0;

			/**
			 * Reset any counters that have accumulated and recommend how many messages/bytes should be read for the next read loop.
			 */
			virtual void reset(ChannelConfig *config) = 0;

			/**
			 * Increment the number of messages that have been read for the current read loop.
			 */
			virtual void incMessagesRead(int numMessages) = 0;

			/**
			 * Set the bytes that have been read for the last read operation.
			 * This may be used to increment the number of bytes that have been read.
			 */
			virtual void lastBytesRead(int bytes) = 0;

			/**
			 * Get the amount of bytes for the previous read operation.
			 * @return The amount of bytes for the previous read operation.
			 */
			virtual int lastBytesRead() = 0;

			/**
			 * Set how many bytes the read operation will (or did) attempt to read.
			 */
			virtual void attemptedBytesRead(int bytes) = 0;

			/**
			 * Get how many bytes the read operation will (or did) attempt to read.
			 */
			virtual int attemptedBytesRead() = 0;

			/**
			 * Determine if the current read loop should continue.
			 */
			virtual bool continueReading() = 0;

			/**
			 * The read has completed.
			 */
			virtual void readComplete() = 0;
		};		
	public:
		virtual Handle*  newHandle() = 0;
	};
}//namespace net
}//namespace comm

#endif
