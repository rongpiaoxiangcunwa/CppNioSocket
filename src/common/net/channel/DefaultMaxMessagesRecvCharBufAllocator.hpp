#ifndef _DefaultMaxMessagesRecvCharBufAllocator_hpp_
#define _DefaultMaxMessagesRecvCharBufAllocator_hpp_

#include <exception>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/limits.hpp>
#include "ChannelConfig.hpp"
#include "MaxMessagesRecvCharBufAllocator.hpp"

namespace comm { 
namespace net {
	class DefaultMaxMessagesRecvCharBufAllocator : public MaxMessagesRecvCharBufAllocator {
	public:
		DefaultMaxMessagesRecvCharBufAllocator() : maxMessagesPerRead_(1),  respectMaybeMoreData_(true)  {
		}

		DefaultMaxMessagesRecvCharBufAllocator(int maxMessagesPerRead) : maxMessagesPerRead_(maxMessagesPerRead),  respectMaybeMoreData_(true)  {
		}

		virtual int maxMessagesPerRead() { return maxMessagesPerRead_ ;}
		
		virtual MaxMessagesRecvCharBufAllocator& maxMessagesPerRead(int maxMessagesPerRead)  {
			if (maxMessagesPerRead <= 0) {
				throw std::runtime_error( (boost::format("maxMessagesPerRead : %1% (expected: > 0)")  % maxMessagesPerRead).str().c_str());
			}
			maxMessagesPerRead_ = maxMessagesPerRead;
			return *this;
		}

		DefaultMaxMessagesRecvCharBufAllocator& respectMaybeMoreData(bool respectMaybeMoreData) {
			respectMaybeMoreData_ = respectMaybeMoreData;
			return *this;
		}

		bool respectMaybeMoreData() { return respectMaybeMoreData_; }

		class MaxMessageHandle : public  RecvCharBufAllocator::Handle  {				
		public:
			MaxMessageHandle(DefaultMaxMessagesRecvCharBufAllocator* _parent)  : parent(_parent), config(0), maxMessagePerRead_(1),
				totalMessages(0), totalBytesRead_(0), attemptedBytesRead_(0), lastBytesRead_(0) {
				respectMaybeMoreData_ = parent->respectMaybeMoreData();
			}
			
			virtual shared_ptr<CharBuf> allocate(CharBufAllocator *alloc) {
				return alloc->ioBuffer(guess());
			}

			virtual void reset(ChannelConfig *_config)  {
				config = _config;
				maxMessagePerRead_ = parent->maxMessagesPerRead();
				totalMessages = totalBytesRead_ = 0;
			}

			virtual void incMessagesRead(int amt) { 
				totalMessages += amt;
			}
			
			virtual void lastBytesRead(int bytes)  {
				lastBytesRead_ = bytes;
				if (bytes > 0) {
					totalBytesRead_ += bytes;
				}
			}

			virtual int lastBytesRead() { 
				return lastBytesRead_; 
			}

			virtual void attemptedBytesRead(int bytes) {
				attemptedBytesRead_ = bytes;
			}

			virtual int attemptedBytesRead() { return attemptedBytesRead_; }

			virtual bool continueReading()  {
				return  (!respectMaybeMoreData_ || attemptedBytesRead_ == lastBytesRead_) 
					&& totalMessages < maxMessagePerRead_ && totalBytesRead_ > 0;
			}

			virtual void readComplete()  { }
			
		protected:
	        int totalBytesRead() {
	        	return totalBytesRead_ < 0 ? std::numeric_limits<int>::max() : totalBytesRead_;
	        }
				
		protected:
			DefaultMaxMessagesRecvCharBufAllocator *parent;
			ChannelConfig* config;
         	int maxMessagePerRead_;
        	int totalMessages;
        	int totalBytesRead_;
        	int attemptedBytesRead_;
        	int lastBytesRead_;
         	bool respectMaybeMoreData_;
		};

	private:
		volatile int maxMessagesPerRead_;
		volatile bool respectMaybeMoreData_;
	};
}//namespace net
}//namespace comm
#endif