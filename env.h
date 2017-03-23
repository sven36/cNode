
//V(as_external, v8::External)
#define ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES(V)                           \
                                              \
  V(async_hooks_destroy_function, v8::Function)                               \
  V(async_hooks_init_function, v8::Function)                                  \
  V(async_hooks_post_function, v8::Function)                                  \
  V(async_hooks_pre_function, v8::Function)                                   \
  V(binding_cache_object, v8::Object)                                         \
  V(buffer_constructor_function, v8::Function)                                \
  V(buffer_prototype_object, v8::Object)                                      \
  V(context, v8::Context)                                                     \
  V(domain_array, v8::Array)                                                  \
  V(domains_stack_array, v8::Array)                                           \
  V(fs_stats_constructor_function, v8::Function)                              \
  V(generic_internal_field_template, v8::ObjectTemplate)                      \
  V(jsstream_constructor_template, v8::FunctionTemplate)                      \
  V(module_load_list_array, v8::Array)                                        \
  V(pipe_constructor_template, v8::FunctionTemplate)                          \
  V(process_object, v8::Object)                                               \
  V(promise_reject_function, v8::Function)                                    \
  V(push_values_to_array_function, v8::Function)                              \
  V(script_context_constructor_template, v8::FunctionTemplate)                \
  V(script_data_constructor_function, v8::Function)                           \
  V(secure_context_constructor_template, v8::FunctionTemplate)                \
  V(tcp_constructor_template, v8::FunctionTemplate)                           \
  V(tick_callback_function, v8::Function)                                     \
  V(tls_wrap_constructor_function, v8::Function)                              \
  V(tls_wrap_constructor_template, v8::FunctionTemplate)                      \
  V(tty_constructor_template, v8::FunctionTemplate)                           \
  V(udp_constructor_function, v8::Function)                                   \
  V(write_wrap_constructor_function, v8::Function)                            \


class Environment
{
public:
	class  AsyncHooks
	{
	public:
		inline unsigned int* fileds();
		inline int fields_count() const;
		inline bool callbacks_enabled();
		inline void set_enable_callbacks(unsigned int flag);

	private:
		friend class Environment;
		inline AsyncHooks();
		enum Fields {
			kEnableCallbacks,
			kFieldCount
		};
	};
	static inline Environment* New(v8::Local<v8::Context> context, uv_loop_t* loop);
	inline Environment(v8::Local<v8::Context> context, uv_loop_t* loop);
	inline ~Environment();
	inline v8::Isolate* isolate() const;
	inline v8::Local<v8::External> as_external() const;
	inline void set_as_external(v8::Local<v8::External> value);

#define V(PropertyName, TypeName)                                             \
  inline v8::Local<TypeName> PropertyName() const;                            \
  inline void set_ ## PropertyName(v8::Local<TypeName> value);
	ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES(V)
#undef V
private:
	v8::Isolate* const isolate_;
	class IsolateData;
	IsolateData* const isolate_data_;
	const uint64_t timer_base_;
	bool using_domains_;
	bool printed_error_;
	bool trace_sync_io_;
	size_t makecallback_cntr_;
	int64_t async_wrap_uid_;
	std::vector<int64_t> destroy_ids_list_;
	//Agent debugger_agent_;
	char* http_parser_buffer_;
	//v8::Persistent<v8::Context> context_;
	class IsolateData {
	public:
		static inline IsolateData* GetOrCreate(v8::Isolate* isolate, uv_loop_t* loop);
	};
	v8::Persistent<v8::External> as_external_;

#define V(PropertyName, TypeName)                                             \
  v8::Persistent<TypeName> PropertyName ## _;
	ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES(V)
#undef V
};

