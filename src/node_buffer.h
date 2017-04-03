#include "v8.h"
#include "node.h"

namespace node {
	extern bool zero_fill_all_buffers;

	namespace Buffer {
		NODE_EXTERN bool HasInstance(v8::Local<v8::Value> val);
		NODE_EXTERN size_t Length(v8::Local<v8::Value> val);
	}
}
