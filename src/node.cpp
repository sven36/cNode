#include "node.h"
#include "libplatform\libplatform.h"
#include "handles.h"
#include "api.h"
#include "env.h"
#include "env-inl.h"
#include "node-internals.h"
#include "node_javascript.h"


using namespace v8;
using node::Environment;

static int v8_thread_pool_size = 4;
static bool use_debug_agent = false;
static v8::Isolate* node_isolate;
static const int kContextEmbedderDataIndex = 32;//NODE_CONTEXT_EMBEDDER_DATA_INDEX;


static void HandleCloseCb(uv_handle_t* handle) {
	Environment* env = reinterpret_cast<Environment*>(handle->data);
	env->FinishHandleCleanup(handle);
}

static void HandleCleanup(Environment* env,
	uv_handle_t* handle,
	void* arg) {
	handle->data = env;
	uv_close(handle, HandleCloseCb);
}

static struct 
{
	void Initialize(int thread_pool_size) {
		//CreateDefaultPlatform是一个虚函数；重写它并初始化；
		platform_ = v8::platform::CreateDefaultPlatform(thread_pool_size);
		V8::InitializePlatform(platform_);
	}
	void PumpMessageLoop(Isolate* isolate) {
		v8::platform::PumpMessageLoop(platform_, isolate);
	}

	void Dispose() {
		delete platform_;
		platform_ = nullptr;
	}

	Platform* platform_;
}v8_platform;

Local<Value> MakeCallback(Environment* env,Local<Object> recv,const char* method,int argc,Local<Value> argv[]) {
	Local<String> method_string = node::OneByteString(env->isolate(), method,-1);
	Local<Value> cb_v = recv->Get(method_string);
	//env, recv.As<Value>(), cb_v.As<Function>(), argc, argv);
	recv = recv.As<Value>();
	const Local<Function> callback = cb_v.As<Function>();
	Local<Function> pre_fn = env->async_hooks_pre_function();
	Local<Function> post_fn = env->async_hooks_post_function();
	Local<Object> object, domain;
	bool ran_init_callback = false;
	bool has_domain = false;
	Environment::AsyncCallbackScope callback_scope(env);

	if (recv->IsObject()) {
		object = recv.As<Object>();
		Local<Value> async_queue_v = object->Get(env->async_queue_string());
		if (async_queue_v->IsObject()) {
			ran_init_callback = true;
		}
	}
	if (env->using_domains()) {
		Local<Value> domain_v = object->Get(env->domain_string());
		has_domain = domain_v->IsObject();
		if (has_domain) {
			domain = domain_v.As<Object>();
			if (domain->Get(env->disposed_string())->IsTrue())
				return Undefined(env->isolate());
		}
	}
	Local<Value> ret = callback->Call(recv, argc, argv);
	if (ret.IsEmpty()) {
		return callback_scope.in_makecallback() ? ret : Undefined(env->isolate()).As<Value>();
	}
	if (callback_scope.in_makecallback()) {
		return ret;
	}
	Environment::TickInfo* tick_info = env->tick_info();
	if (tick_info->length()==0) {
		env->isolate()->RunMicrotasks();
	}
	Local<Object> process = env->process_object();
	if (tick_info->length() == 0) {
		tick_info->set_index(0);
	}
	if (env->tick_callback_function()->Call(process,0,nullptr).IsEmpty()) {
		return Undefined(env->isolate());
	}
	return ret;
}

void EmitBeforeExit(Environment* env) {
	HandleScope handle_scope(env->isolate());
	Context::Scope context_scope(env->context());
	Local<Object> process_object = env->process_object();
	Local<String> exit_code = FIXED_ONE_BYTE_STRING(env->isolate, "exitCode");
	Local<Value> args[] = {
		FIXED_ONE_BYTE_STRING(env->isolate(),"beforeExit"),
		process_object->Get(exit_code)->ToInteger(env->isolate())
	};
	MakeCallback(env,process_object,"emit",arraysize(args),args);
}

static Local<Value> ExecuteString(Environment* env,Local<String> source, Local<String> filename) {
	EscapableHandleScope scope(env->isolate());
	ScriptOrigin origin(filename);
	MaybeLocal<Script> script = Script::Compile(env->context(), source, &origin);
	Local<Value> result = script.ToLocalChecked()->Run();
	return scope.Escape(result);
}

void LoadEnvironment(Environment* env) {
	HandleScope handle_scope(env->isolate());
	//env->isolate()->SetFatalErrorHandler(node::Onf);
	Local<String> script_name = FIXED_ONE_BYTE_STRING(env->isolate(), "cNodeStart.js");
	Local<Value> f_value = ExecuteString(env, node::MainSource(env), script_name);
	Local<Function> f = Local<Function>::Cast(f_value);
	Local<Object> global = env->context()->Global();
	Local<Value> arg = env->process_object();
	f->Call(Null(env->isolate()), 1, &arg);
}

static void StartNodeInstance(void* arg) {
	node::NodeInstanceData* instance_data = static_cast<node::NodeInstanceData*>(arg);
	Isolate::CreateParams params;
	node::ArrayBufferAllocator* array_buffer_allocator = new node::ArrayBufferAllocator();
	params.array_buffer_allocator = array_buffer_allocator;

	Isolate* isolate = Isolate::New(params);
	{
		//Mutex::ScopedLock scoped_lock();
		if (instance_data->is_main()) {
			node_isolate = isolate;
		}
	}
	{
		/* 在V8中，内存分配都是在V8的Heap中进行分配的，JavaScript的值和对象也都存放在V8的Heap中。
		这个Heap由V8独立的去维护，失去引用的对象将会被V8的GC掉并可以重新分配给其他对象。而Handle即是对Heap中对象的引用。
		V8为了对内存分配进行管理，GC需要对V8中的所有对象进行跟踪，而对象都是用Handle方式引用的，所以GC需要对Handle进行管理，
		这样GC就能知道Heap中一个对象的引用情况，当一个对象的Handle引用为发生改变的时候，GC即可对该对象进行回收（gc）或者移动。
		因此，V8编程中必须使用Handle去引用一个对象，而不是直接通过C++的方式去获取对象的引用，直接通过C++的方式去直接去引用一个对象，
		会使得该对象无法被V8管理。*/
		/*一个函数中，可以有很多Handle，而HandleScope则相当于用来装Handle（Local）的容器，当HandleScope生命周期结束的时候，
		Handle也将会被释放，会引起Heap中对象引用的更新。HandleScope是分配在栈上，不能通过New的方式进行创建。
		对于同一个作用域内可以有多个HandleScope，新的HandleScope将会覆盖上一个HandleScope，并对Local Handle进行管理。*/
		Locker locker(isolate);
		Isolate::Scope isolate_scope(isolate);
		HandleScope handle_sope(isolate);
		Local<Context> context = Context::New(isolate);
		node::Environment* env = CreateEnvironment(isolate, context, instance_data);
		array_buffer_allocator->set_env(env);
		Context::Scope content_scope(context);

		//isolate->SetAbortOnUncaughtExceptionCallback();
		{
			node::Environment::AsyncCallbackScope callback_scope(env);
			LoadEnvironment(env);
		}
		//env->set_trac
		{
			SealHandleScope seal(isolate);
			bool more;
			do {
				v8_platform.PumpMessageLoop(isolate);
				more = uv_run(env->event_loop(), UV_RUN_ONCE);
				if (more==false) {
					v8_platform.PumpMessageLoop(isolate);
					EmitBeforeExit(env);

					more = uv_loop_alive(env->event_loop());
					if (uv_run(env->event_loop(),UV_RUN_NOWAIT)!=0) {
						more = true;
					}
				}
			} while (more == true);
		}

		//int exit_code=
	}
}

void SetupProcessObject(Environment* env, int argc, const char* const* argv, int exec_argc, const char* const* exec_argv) {
	HandleScope scope(env->isolate());
	Local<Object> process = env->process_object();
	auto title_string = node::OneByteString(env->isolate(), "title", sizeof("title") - 1);
	env->context();
	//process.version
	//READONLY_PROPERTY(process, "version", FIXED_ONE_BYTE_STRING(env->isolate(), "cNode1.0~"));
	////process.moduleLoadList
	//READONLY_PROPERTY(process, "moduleLoadList", env->module_load_list_array());
}

static node::Environment* CreateEnvironment(Isolate* isolate, Local<Context> context, node::NodeInstanceData* instance_data) {
	HandleScope handle_sope(isolate);
	Context::Scope context_scope(context);
	Environment* env = Environment::New(context, instance_data->event_loop());
	isolate->SetAutorunMicrotasks(false);
	uv_check_init(env->event_loop(), env->immediate_check_handle());
	uv_unref(
		reinterpret_cast<uv_handle_t*>(env->immediate_check_handle()));

	uv_idle_init(env->event_loop(), env->immediate_idle_handle());
	//当cpu空闲的时候，通知v8的的CPU分析器，记录下epoll_wait()的时间来使分析工具可以找到它；
	uv_prepare_init(env->event_loop(), env->idle_prepare_handle());
	uv_check_init(env->event_loop(), env->idle_check_handle());
	uv_unref(reinterpret_cast<uv_handle_t*>(env->idle_prepare_handle()));
	uv_unref(reinterpret_cast<uv_handle_t*>(env->idle_check_handle()));

	//注册handle的回收方法；
	env->RegisterHandleCleanup(
		reinterpret_cast<uv_handle_t*>(env->immediate_check_handle()),
		HandleCleanup,
		nullptr);
	env->RegisterHandleCleanup(
		reinterpret_cast<uv_handle_t*>(env->immediate_idle_handle()),
		HandleCleanup,
		nullptr);
	env->RegisterHandleCleanup(
		reinterpret_cast<uv_handle_t*>(env->idle_prepare_handle()),
		HandleCleanup,
		nullptr);
	env->RegisterHandleCleanup(
		reinterpret_cast<uv_handle_t*>(env->idle_check_handle()),
		HandleCleanup,
		nullptr);

	Local<FunctionTemplate> process_template = FunctionTemplate::New(isolate);
	process_template->SetClassName(node::OneByteString(isolate, "process", sizeof("process") - 1));

	Local<Object> process_object = process_template->GetFunction()->NewInstance(context).ToLocalChecked();
	env->set_process_object(process_object);

	SetupProcessObject(env, instance_data->argc(), instance_data->argv(), instance_data->exec_argc(), instance_data->exec_argv());

}

int node::Start(int argc, char * argv[])
{
	return 0;
	int exec_argc;
	int exit_code = 1;
	const char** exec_argv;
	v8_platform.Initialize(v8_thread_pool_size);
	V8::Initialize();
	{
		NodeInstanceData instance_data(NodeInstanceType::MAIN,uv_default_loop(),argc,const_cast<const char**>(argv),exec_argc, exec_argv, use_debug_agent);
		StartNodeInstance(&instance_data);
		exit_code = instance_data.exit_code();
	}
	V8::Dispose();
	v8_platform.Dispose();
	delete[] exec_argv;
	exec_argv = nullptr;
	return exit_code;
}




	
