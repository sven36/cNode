#include "util.h"
#include "string_bytes.h"

namespace node {
	using v8::Isolate;
	using v8::Local;
	using v8::Value;
	using v8::String;

	template <typename T>
	static void MakeUtf8String(Isolate* isolate,Local<Value> value,T* target) {
		Local<String> string = value->ToString(isolate);
		if (string.IsEmpty())
			return;
		const size_t storage = StringBytes::StorageSize(isolate, string, UTF8) + 1;
		target->AllocateSufficientStorage(storage);
		const int flags = String::NO_NULL_TERMINATION | String::REPLACE_INVALID_UTF8;
		const int length = string->WriteUtf8(target->out(), storage, 0, flags);
		target->SetLengthAndZeroTerminate(length);
	}

	Utf8Value::Utf8Value(Isolate* isolate, Local<Value> value) {
		if (value.IsEmpty()) 
			return;

		MakeUtf8String(isolate, value, this);
	}
}