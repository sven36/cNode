

namespace node {


#define PER_ISOLATE_STRING_PROPERTIES(V)                                      \
  V(domain_string, "domain")                                                  \
  V(disposed_string, "_disposed")                                             \
  V(enter_string, "enter")                                                    \



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

#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                    \
  void operator=(const TypeName&) = delete;                                   \
  void operator=(TypeName&&) = delete;                                        \
  TypeName(const TypeName&) = delete;                                         \
  TypeName(TypeName&&) = delete

#include "ares.h"
#include "uv.h"
#include "tree.h"

	class Environment;

	struct node_ares_task {
		Environment* env;
		ares_socket_t soket;
		uv_poll_t pool_watcher;
		RB_ENTRY(node_ares_task) node;
	};
	RB_HEAD(node_ares_task_list, node_ares_task);


	template <typename T>
	class ListNode {
	public:
		inline ListNode();
		inline ~ListNode();
		inline void Remove();
		inline bool IsEmpty() const;

	private:
		template <typename U, ListNode<U>(U::*M)> friend class ListHead;
		ListNode* prev_;
		ListNode* next_;
		DISALLOW_COPY_AND_ASSIGN(ListNode);
	};

	template <typename T, ListNode<T>(T::*M)>
	class ListHead {
	public:
		class Iterator {
		public:
			inline T* operator*() const;
			inline const Iterator& operator++();
			inline bool operator!=(const Iterator& that) const;

		private:
			friend class ListHead;
			inline explicit Iterator(ListNode<T>* node);
			ListNode<T>* node_;
		};

		inline ListHead() = default;
		inline ~ListHead();
		inline void MoveBack(ListHead* that);
		inline void PushBack(T* element);
		inline void PushFront(T* element);
		inline bool IsEmpty() const;
		inline T* PopFront();
		inline Iterator begin() const;
		inline Iterator end() const;

	private:
		ListNode<T> head_;
		DISALLOW_COPY_AND_ASSIGN(ListHead);
	};

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

		class TickInfo {
		public:
			inline uint32_t index() const;
			inline uint32_t length() const;
			inline void set_index(uint32_t value);
		private:
			friend class Environment;
			inline TickInfo();
			enum Fields {
				kIndex,kLength,kFieldsCount
			};
			uint32_t fields_[kFieldsCount];

			DISALLOW_COPY_AND_ASSIGN(TickInfo);
		};

		class AsyncCallbackScope
		{
		public:
			explicit AsyncCallbackScope(Environment * env);
			~AsyncCallbackScope();

			inline bool in_makecallback();
		private:
			Environment* env_;
			DISALLOW_COPY_AND_ASSIGN(AsyncCallbackScope);
		};


		typedef void(*HandleCleanupCb)(Environment* ebv, uv_handle_t* handle, void* arg);
		class HandleCleanUp
		{
		private:
			friend class Environment;

			HandleCleanUp(uv_handle_t* handle, HandleCleanupCb cb, void* arg) :handle_(handle), cb_(cb), arg_(arg) {}

			uv_handle_t* handle_;
			HandleCleanupCb cb_;
			void* arg_;
			ListNode<HandleCleanUp> handle_clean_queue_;

		};

		class ArrayBufferAllocatorInfo {
		public:
			inline uint32_t* fields();
			inline int fields_count() const;
			inline bool no_zero_fill() const;
			inline void reset_fill_flag();

		private:
			friend class Environment;  // So we can call the constructor.
			inline ArrayBufferAllocatorInfo();

			enum Fields {
				kNoZeroFill,
				kFieldsCount
			};

			uint32_t fields_[kFieldsCount];

			DISALLOW_COPY_AND_ASSIGN(ArrayBufferAllocatorInfo);
		};

		inline ArrayBufferAllocatorInfo* array_buffer_allocator_info();

		static inline Environment* New(v8::Local<v8::Context> context, uv_loop_t* loop);
		inline Environment(v8::Local<v8::Context> context, uv_loop_t* loop);
		inline ~Environment();
		inline v8::Isolate* isolate() const;
		inline v8::Local<v8::External> as_external() const;
		inline void set_as_external(v8::Local<v8::External> value);
		inline uv_loop_t* event_loop() const;

		void AssignToContext(v8::Local<v8::Context> context);

		inline uv_check_t* immediate_check_handle();
		inline uv_idle_t* immediate_idle_handle();
		inline uv_prepare_t* idle_prepare_handle();
		inline uv_check_t* idle_check_handle();

		//当env->Dispose()时回收handle;
		inline void RegisterHandleCleanup(uv_handle_t* handle,
			HandleCleanupCb cb,
			void *arg);
		inline void FinishHandleCleanup(uv_handle_t* handle);

		inline v8::Local<v8::String> Environment::async_queue_string() const;

		inline bool using_domains() const;
		inline TickInfo* tick_info();

#define VP(PropertyName, StringValue) V(v8::Private, PropertyName, StringValue)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName, StringValue)
#define V(TypeName, PropertyName, StringValue)                                \
  inline v8::Local<TypeName> PropertyName() const;
		//PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(VP)
			PER_ISOLATE_STRING_PROPERTIES(VS)
#undef V
#undef VS
#undef VP

#define V(PropertyName, TypeName)                                             \
  inline v8::Local<TypeName> PropertyName() const;                            \
  inline void set_ ## PropertyName(v8::Local<TypeName> value);
		ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES(V)
#undef V
	private:
		v8::Isolate* const isolate_;
		class IsolateData;
		inline IsolateData* isolate_data() const;
		IsolateData* const isolate_data_;
		const uint64_t timer_base_;
		bool printed_error_;
		bool trace_sync_io_;
		size_t makecallback_cntr_;
		int64_t async_wrap_uid_;
		std::vector<int64_t> destroy_ids_list_;
		node_ares_task_list cares_task_list_;
		int handle_cleanup_waiting_;

		ArrayBufferAllocatorInfo array_buffer_allocator_info_;

		uv_check_t immediate_check_handle_;
		uv_idle_t immediate_idle_handle_;
		uv_prepare_t idle_prepare_handle_;
		uv_check_t idle_check_handle_;
		TickInfo tick_info_;

		ListHead<HandleCleanUp,
			&HandleCleanUp::handle_clean_queue_> handle_clean_queue_;

		//Agent debugger_agent_;
		char* http_parser_buffer_;
		//v8::Persistent<v8::Context> context_;
		class IsolateData {
		public:
			static inline IsolateData* GetOrCreate(v8::Isolate* isolate, uv_loop_t* loop);
			inline uv_loop_t* event_loop() const;
			inline v8::Local<v8::String> async_queue_string() const;
#define VP(PropertyName, StringValue) V(v8::Private, PropertyName, StringValue)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName, StringValue)
#define V(TypeName, PropertyName, StringValue)                                \
    inline v8::Local<TypeName> PropertyName() const;
			//PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(VP)
				PER_ISOLATE_STRING_PROPERTIES(VS)
#undef V
#undef VS
#undef VP
		private:
			uv_loop_t* const event_loop_;
			v8::Eternal<v8::String> async_queue_string_;
#define VP(PropertyName, StringValue) V(v8::Private, PropertyName, StringValue)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName, StringValue)
#define V(TypeName, PropertyName, StringValue)                                \
    v8::Eternal<TypeName> PropertyName ## _;
			//PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(VP)
			PER_ISOLATE_STRING_PROPERTIES(VS)
#undef V
#undef VS
#undef VP
		};
		v8::Persistent<v8::External> as_external_;
		v8::Eternal<v8::String> async_queue_string_;
		//bool using_domains_;



#define V(PropertyName, TypeName)                                             \
  v8::Persistent<TypeName> PropertyName ## _;
		ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES(V)
#undef V
	};
}

