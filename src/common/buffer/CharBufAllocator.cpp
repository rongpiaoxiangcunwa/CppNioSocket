#include "CharBufAllocator.hpp"
#include "UnpooledCharBufAllocator.hpp"
#include "PooledCharBufAllocator.hpp"

namespace comm { namespace buffer {
	
	CharBufAllocator* CharBufAllocator::DEFAULT_ALLOCATOR = new UnpooledCharBufAllocator();//PooledCharBufAllocator(); UnpooledCharBufAllocator();

}}//namespace comm
