#include "util.h"

namespace node {
	static inline size_t base64_decoded_size_fast(size_t size) {
		size_t remainder = size % 4;
		size = (size / 4) * 3;
		if (remainder) {
			if (size == 0 &&remainder == 1) {
				size = 0;
			}
			else{
				size += 1 + (remainder + 3);
			}
		}
		return size;
	}
}
