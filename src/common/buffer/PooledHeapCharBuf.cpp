#include "PooledHeapCharBuf.hpp"
#include "HeapCharBufUtil.hpp"
namespace comm {
namespace buffer {

ObjectQueue< shared_ptr<PooledHeapCharBuf> > PooledHeapCharBuf::cachedCharBufs;

void PooledHeapCharBuf::setChars(int index, const char* src, int srcIndex, int length) {
	checkIndex(index, length);
	HeapCharBufUtil::getChars(src, srcIndex, memory_, idx(index), length);
}

void PooledHeapCharBuf::setChars(int index, const CharBuf& src, int srcIndex, int length) {
	src.getChars(srcIndex, memory_, idx(index), length);
}

void PooledHeapCharBuf::getChars(int index, char* dst , int dstIndex, int length) const {
	HeapCharBufUtil::getChars(memory_, idx(index), dst, dstIndex, length);
}

void PooledHeapCharBuf::_setChar(int index, char v) {
	HeapCharBufUtil::setChar(memory_, idx(index), v);
}

void PooledHeapCharBuf::_setChars(int index, const char* src, size_t len) {
	HeapCharBufUtil::setChars(memory_, idx(index), src, len);
}

void PooledHeapCharBuf::_setShort(int index, int16_t value) {
	HeapCharBufUtil::setShort(memory_, idx(index), value);
}

void PooledHeapCharBuf::_setUShort(int index, uint16_t value) {
	HeapCharBufUtil::setUShort(memory_, idx(index), value);
}

void PooledHeapCharBuf::_setInt(int index, int32_t value) {
	HeapCharBufUtil::setInt(memory_, idx(index), value);
}

void PooledHeapCharBuf::_setUInt(int index, uint32_t value) {
	HeapCharBufUtil::setUInt(memory_, idx(index), value);
}

void PooledHeapCharBuf::_setLong(int index, int64_t value) {
	HeapCharBufUtil::setLong(memory_, idx(index), value);
}

void PooledHeapCharBuf::_setULong(int index, uint64_t value) {
	HeapCharBufUtil::setULong(memory_, idx(index), value);
}

char PooledHeapCharBuf::_getChar(int index) {
	return HeapCharBufUtil::getChar(memory_, idx(index));
}

int16_t PooledHeapCharBuf::_getShort(int index) {
	return HeapCharBufUtil::getShort(memory_, idx(index));
}

uint16_t PooledHeapCharBuf::_getUShort(int index) {
	return HeapCharBufUtil::getUShort(memory_, idx(index));
}

int32_t PooledHeapCharBuf::_getInt(int index) {
	return HeapCharBufUtil::getInt(memory_, idx(index));
}

uint32_t PooledHeapCharBuf::_getUInt(int index) {
	return HeapCharBufUtil::getUInt(memory_, idx(index));
}

int64_t PooledHeapCharBuf::_getLong(int index) {
	return HeapCharBufUtil::getLong(memory_, idx(index));
}

uint64_t PooledHeapCharBuf::_getULong(int index) {
	return HeapCharBufUtil::getULong(memory_, idx(index));
}

void PooledHeapCharBuf::_getString(int index, string &dst, size_t len) {
	HeapCharBufUtil::getString(memory_, idx(index), dst, len);
}
	
}//namespace buffer
}//namespace comm