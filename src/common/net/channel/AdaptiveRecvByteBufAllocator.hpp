#ifndef _AdaptiveRecvByteBufAllocator_hpp_
#define _AdaptiveRecvByteBufAllocator_hpp_

#include <vector>
#include <algorithm>
#include <iostream>
#include "DefaultMaxMessagesRecvCharBufAllocator.hpp"
using std::vector;

namespace comm { 
namespace net {
	class AdaptiveRecvByteBufAllocator  :  public DefaultMaxMessagesRecvCharBufAllocator {
	public:
		static const int DEFAULT_MINIMUM = 1024;//64
		static const int DEFAULT_INITIAL = 8192;//1024
		static const int DEFAULT_MAXIMUM = 65536;
		static const int INDEX_INCREMENT = 4;
		static const int INDEX_DECREMENT = 1;
		
		////////////////////////////////////////////////////////////////////////////////////
		class SizeTable {
		public:
			SizeTable() {
				init();
			}
			
			int getSizeTableIndex(int size);
			
			int getSize(int index) {
				if (index <= 0) return sizes.front();
				return index >= sizes.size() ? sizes.back() : sizes[index] ;
			}
			
		private:
			void init();
		private:
			vector<int> sizes;
		};
	
		////////////////////////////////////////////////////////////////////////////////////
		class HandleImpl : public DefaultMaxMessagesRecvCharBufAllocator::MaxMessageHandle {
		public:
			typedef DefaultMaxMessagesRecvCharBufAllocator::MaxMessageHandle BASE;
			HandleImpl(AdaptiveRecvByteBufAllocator* _parent, int _minIndex, int _maxIndex, int _initial) : 
				BASE(_parent), minIndex(_minIndex), maxIndex(_maxIndex), decreaseNow(false) {
				index = sizeTable.getSizeTableIndex(_initial);
				nextReceiveBufferSize = sizeTable.getSize(index);
			}
				
			virtual void lastBytesRead(int bytes) {
				if (bytes == attemptedBytesRead()) {
					record(bytes);
				}
				BASE::lastBytesRead(bytes);
			}

			virtual int guess() {
				return nextReceiveBufferSize;
			}

			virtual void readComplete() {
				record(totalBytesRead());
			}
			
		private:
			void record(int actualReadBytes) {
				if (actualReadBytes <= sizeTable.getSize(std::max(0, index - INDEX_DECREMENT - 1))) {
					if (decreaseNow) {
						index = std::max(index - INDEX_DECREMENT, minIndex);
						nextReceiveBufferSize = sizeTable.getSize(index);
						decreaseNow = false;
					} else {
						decreaseNow = true;
					}
				} else if (actualReadBytes >= nextReceiveBufferSize) {
					int old = nextReceiveBufferSize;
					index = std::min(index + INDEX_INCREMENT, maxIndex);
					nextReceiveBufferSize = sizeTable.getSize(index);
					//std::cout << "actualReadBytes:" << actualReadBytes  << " old:" << old << " nextReceiveBufferSize:" << nextReceiveBufferSize << std::endl;
					decreaseNow = false;
				}
			}
			
		private:	
			int minIndex;
			int maxIndex;
			int index;
			int nextReceiveBufferSize;
			bool decreaseNow;
		public:
			static SizeTable sizeTable;
		};

	///////////////////////////////////////////////////////////
	public:
		AdaptiveRecvByteBufAllocator() {
			init(DEFAULT_MINIMUM, DEFAULT_INITIAL, DEFAULT_MAXIMUM);
		}

		AdaptiveRecvByteBufAllocator(int _minimum, int _initial, int _maximum)  {
			init(_minimum, _initial, _maximum);
		}

		virtual Handle* newHandle()  {
			return new HandleImpl(this, minIndex_, maxIndex_, initial_);
		}
		
	protected:	
		void init(int minimum, int initial, int maximum) {
			if (minimum <= 0) {
				throw std::runtime_error((boost::format("minimum: %1%") % minimum).str().c_str());
			}
			
			if (initial < minimum) {
				throw std::runtime_error((boost::format("initial: %1%") % initial).str().c_str());
			}
			
			if (maximum < initial) {
				throw std::runtime_error((boost::format("maximum: %1%") % maximum).str().c_str());
			}

			int minIndex = HandleImpl::sizeTable.getSizeTableIndex(minimum);
			if (HandleImpl::sizeTable.getSize(minIndex) < minimum) {
				minIndex_ = minIndex + 1;
			} else {
				minIndex_ = minIndex;
			}

			int maxIndex = HandleImpl::sizeTable.getSizeTableIndex(maximum);
			if (HandleImpl::sizeTable.getSize(maxIndex) > maximum) {
				maxIndex_ = maxIndex - 1;
			} else {
				maxIndex_ = maxIndex;
			}

			initial_ = initial;
		}
		
	private:
		int minIndex_;
		int maxIndex_;
		int initial_;
	};

}//namespace net
}//namespace comm

#endif
