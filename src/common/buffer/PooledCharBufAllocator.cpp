#include "PooledCharBufAllocator.hpp"

namespace comm {
namespace buffer {
	const int PooledCharBufAllocator::DEFAULT_PAGE_SIZE;
	const int PooledCharBufAllocator::DEFAULT_MAX_ORDER;
	const int PooledCharBufAllocator::MIN_PAGE_SIZE;
    const int PooledCharBufAllocator::MAX_CHUNK_SIZE = (std::numeric_limits<int>::max()) / 2;
}//namespace buffer
}//namespace comm