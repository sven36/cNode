#include "node.h"
#include "string_bytes.h"
#include "v8.h"
#include "node_buffer.h"
#include "base64.h"

namespace node {
	using v8::Isolate;
	using v8::Local;
	using v8::Value;
	using v8::String;
	using v8::HandleScope;
	size_t StringBytes::StorageSize(Isolate* isolate,Local<Value> val,enum encoding encoding) {
		HandleScope scope(isolate);
		size_t data_size = 0;
		bool is_buffer = Buffer::HasInstance(val);

		if (is_buffer&&(encoding==BUFFER||encoding==LATIN1)) {
			return Buffer::Length(val);
		}
		Local<String> str = val->ToString();
		switch (encoding)
		{
		case node::ASCII:
		case node::LATIN1:
			data_size = str->Length();
			break;
		case node::BUFFER:
		case node::UTF8:
			data_size =3* str->Length();
			break;
		case node::BASE64:
			data_size = base64_decoded_size_fast(str->Length());
			break;

		case node::UCS2:
			data_size = str->Length()*sizeof(uint16_t);
			break;
		case node::HEX:
			data_size = str->Length() / 2;
			break;
		}
		return data_size;
	}
}
