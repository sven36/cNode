#include "string_bytes.h"
#include "v8.h"

namespace node {
	using v8::Isolate;
	using v8::Local;
	using v8::Value;
	using v8::HandleScope;
	size_t StringBytes::StorageSize(Isolate* isolate,Local<Value> val,enum encoding encoding) {
		HandleScope scope(isolate);
		size_t data_size = 0;
		bool is_buffer=Buffer::
	}
}
