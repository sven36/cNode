#include "v8.h"
#include "env.h"

using namespace v8;

namespace node {

	inline v8::Local<v8::String> OneByteString(v8::Isolate* isolate, const char* data, int length) {
		return v8::String::NewFromOneByte(isolate, reinterpret_cast<const uint8_t*>(data), v8::NewStringType::kNormal, length).ToLocalChecked();
	}

#define READONLY_PROPERTY(obj, str, var)                                      \
  do {                                                                        \
    obj->DefineOwnProperty(env->context(),                                    \
                           node::OneByteString(env->isolate(), str),                \
                           var,                                               \
                           v8::ReadOnly).FromJust();                          \
  } while (0)

#define READONLY_DONT_ENUM_PROPERTY(obj, str, var)                            \
  do {                                                                        \
    obj->DefineOwnProperty(env->context(),                                    \
                            node::OneByteString(env->isolate(), str),                \
                           var,                                               \
                           static_cast<v8::PropertyAttribute>(v8::ReadOnly |  \
                                                              v8::DontEnum))  \
        .FromJust();                                                          \
  } while (0)

#define FIXED_ONE_BYTE_STRING(isolate, string)                                \
  ( node::OneByteString((isolate), (string), sizeof(string) - 1))

	inline Environment* Environment::New(v8::Local<v8::Context> context, uv_loop_t* loop) {
		Environment* env = new Environment(context, loop);
		env->AssignToContext(context);
		return env;
	}
	inline Isolate* Environment::isolate() const {
		return isolate_;
	}

	template <class TypeName>
	inline v8::Local<TypeName> StrongPersistentToLocal(const v8::Persistent<TypeName>& persistent) {
		return *reinterpret_cast<v8::Local<TypeName>*>(const_cast<v8::Local<TypeName*>>(&persistent));
	}

	inline v8::Local<v8::External> Environment::as_external() const {
		return StrongPersistentToLocal(as_external_);                        \
	}

	inline void Environment::set_as_external(v8::Local<v8::External> value) {
		as_external_.Reset(isolate(), value);
	}
	inline Environment::IsolateData* Environment::isolate_data() const {
		return isolate_data_;
	}
	inline uv_loop_t* Environment::event_loop() const {
		return isolate_data()->event_loop();
	}
	inline void Environment::AssignToContext(v8::Local<v8::Context> context) {
		context->SetAlignedPointerInEmbedderData(kContextEmbedderDataIndex, this);
	}
	inline uv_check_t* Environment::immediate_check_handle() {
		return &immediate_check_handle_;
	}
	inline uv_idle_t* Environment::immediate_idle_handle() {
		return &immediate_idle_handle_;
	}
	inline uv_prepare_t* Environment::idle_prepare_handle() {
		return &idle_prepare_handle_;
	}
	inline uv_check_t* Environment::idle_check_handle() {
		return &idle_check_handle_;
	}
	inline void Environment::FinishHandleCleanup(uv_handle_t* handle) {
		handle_cleanup_waiting_--;
	}

	void* ArrayBufferAllocator::Allocate(size_t size) {
		if (env_ == nullptr || !env_->array_buffer_allocator_info()->no_zero_fill()) {
			return node::Calloc(size, 1);
		}
		env_->array_buffer_allocator_info()->reset_fill_flag();
		return node::Malloc(size);
	}

	inline Environment::ArrayBufferAllocatorInfo*
		Environment::array_buffer_allocator_info() {
		return &array_buffer_allocator_info_;
	}

	inline bool Environment::ArrayBufferAllocatorInfo::no_zero_fill() const {
		return fields_[kNoZeroFill] != 0;
	}
	inline int Environment::ArrayBufferAllocatorInfo::fields_count() const {
		return kFieldsCount;
	}
	inline uint32_t* Environment::ArrayBufferAllocatorInfo::fields() {
		return fields_;
	}
	inline void Environment::ArrayBufferAllocatorInfo::reset_fill_flag() {
		fields_[kNoZeroFill] = 0;
	}
	inline void Environment::RegisterHandleCleanup(uv_handle_t* handle,
		HandleCleanupCb cb,
		void *arg) {
		handle_clean_queue_.PushBack(new HandleCleanUp(handle, cb, arg));
	}

	inline Environment::Environment(v8::Local<v8::Context> context, uv_loop_t* loop)
		:isolate_(context->GetIsolate()),
		isolate_data_(IsolateData::GetOrCreate(context->GetIsolate(), loop)),
		timer_base_(uv_now(loop)),
		using_domains_(false),
		printed_error_(false),
		trace_sync_io_(false),
		http_parser_buffer_(nullptr),
		context_(context->GetIsolate(), context) {
		HandleScope handle_scope(isolate());
		Context::Scope context_scope(context);
		set_as_external(v8::External::New(isolate(), this));
		set_binding_cache_object(v8::Object::New(isolate()));
		set_module_load_list_array(v8::Array::New(isolate()));

		v8::Local<v8::FunctionTemplate> fn = v8::FunctionTemplate::New(isolate());
		fn->SetClassName(OneByteString(isolate(), "InternalFieldObject", sizeof("InternalFieldObject") - 1));
		v8::Local<v8::ObjectTemplate> obj = fn->InstanceTemplate();
		obj->SetInternalFieldCount(1);
		set_generic_internal_field_template(obj);

		RB_INIT(&cares_task_list_);
		handle_cleanup_waiting_ = 0;
		destroy_ids_list_.reserve(512);
	}

#define V(PropertyName, TypeName)                                             \
  inline v8::Local<TypeName> Environment::PropertyName() const {              \
    return StrongPersistentToLocal(PropertyName ## _);                        \
  }                                                                           \
  inline void Environment::set_ ## PropertyName(v8::Local<TypeName> value) {  \
    PropertyName ## _.Reset(isolate(), value);                                \
  }
	ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES(V)
#undef V;
}
