#include "node_buffer.h"
#include "v8.h"

namespace node {
	using v8::Local;
	using v8::Value;
	using v8::Uint8Array;

	bool HasInstance(v8::Local<v8::Value> val) {
		return val->IsUint8Array();
	}
	size_t Length(Local<Value> val) {
		Local<Uint8Array> ui = val.As<Uint8Array>();
		return ui->ByteLength();
	}
}