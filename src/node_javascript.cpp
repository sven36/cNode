#include "node_javascript.h"

namespace node {
	using v8::Local;
	using v8::NewStringType;
	using v8::Object;
	using v8::String;
	using node::Environment;
	//此为name的数组
	static const uint8_t internal_bootstrap_node_data[] = {
		105,110,116,101,114,110,97,108,47,98,111,111,116,115,116,114,97,112,95,110,
		111,100,101 };

	static struct :public String::ExternalOneByteStringResource{
		const char* data() const override {
			return reinterpret_cast<const char*>(internal_bootstrap_node_data);
		}
		size_t length() const override { return sizeof(internal_bootstrap_node_data); }
		void Dispose() override{}
	}internal_bootstrap_node_external_data;

	Local<String> MainSource(node::Environment* env) {
		auto maybe_string =String::NewExternalOneByte(env->isolate(),&internal_bootstrap_node_external_data);
		return maybe_string.ToLocalChecked();
	}
}
