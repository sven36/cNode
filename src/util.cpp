#include "util.h"
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
		const size_t storage=StringBytes::
	}

	Utf8Value::Utf8Value(Isolate* isolate, Local<Value> value) {
		if (value.IsEmpty()) 
			return;

		Make
	}
}