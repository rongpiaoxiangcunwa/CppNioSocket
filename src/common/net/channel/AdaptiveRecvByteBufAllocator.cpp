#include "AdaptiveRecvByteBufAllocator.hpp"

namespace comm { 
namespace net {
	const int AdaptiveRecvByteBufAllocator::DEFAULT_MINIMUM;
	const int AdaptiveRecvByteBufAllocator::DEFAULT_INITIAL;
	const int AdaptiveRecvByteBufAllocator::DEFAULT_MAXIMUM;
	const int AdaptiveRecvByteBufAllocator::INDEX_INCREMENT;
	const int AdaptiveRecvByteBufAllocator::INDEX_DECREMENT;
	
	AdaptiveRecvByteBufAllocator::SizeTable AdaptiveRecvByteBufAllocator::HandleImpl::sizeTable;

	void AdaptiveRecvByteBufAllocator::SizeTable::init() {
		for(int i = 16; i < 512; i += 16) {
			sizes.push_back(i);
		}

		for(int i = 512; i > 0; i <<= 1) {
			sizes.push_back(i);
		}
	}
	
	int AdaptiveRecvByteBufAllocator::SizeTable::getSizeTableIndex(int size) {
		int low = 0, high = sizes.size() - 1;
		while (low < high) {
			int mid = low + high >> 1;
			int a = sizes[mid], b = sizes[mid + 1];
			if (size == a) {
				return mid;
			} else if (size < a) {
				high = mid - 1;
			} else if (size > b){
				low= mid + 1;
			} else {
				return mid + 1;
			}
		}
		return low;
	}
	

}//namespace net
}//namespace comm