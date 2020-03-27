#ifndef _CommUtil_hpp_
#define _CommUtil_hpp_

class CommUtil {
public:
	#define ALIGN_SIZE 16
	#define MAXIMUM_CAPACITY 1 << 31
	
	static size_t roundUpSize(size_t size) {
		return (size + (size_t)ALIGN_SIZE - 1) & ~((size_t)ALIGN_SIZE - 1);
	}

	static size_t roundUpPower2(size_t cap) {
		size_t n = cap - 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return (n <= 0) ? 1 : (n >= MAXIMUM_CAPACITY) ? MAXIMUM_CAPACITY : n + 1;
	}
	
};
#endif
