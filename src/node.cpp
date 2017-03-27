#include "node.h"
#include "libplatform\libplatform.h"
#include "src\handles.h"
#include "src\api.h"
#include "env.h"
#include "env-inl.h"
#include "node-internals.h"

using namespace v8;

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

	void Dispose() {
		delete platform_;
		platform_ = nullptr;
	}

	Platform* platform_;
}v8_platform;

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
		Environment* env = CreateEnvironment(isolate, context, instance_data);
	}
}

void SetupProcessObject(Environment* env, int argc, const char* const* argv, int exec_argc, const char* const* exec_argv) {
	HandleScope scope(env->isolate());
	Local<Object> process = env->process_object();
	auto title_string = node::OneByteString(env->isolate(), "title", sizeof("title") - 1);
	env->context();
	//process.version
	READONLY_PROPERTY(process, "version", FIXED_ONE_BYTE_STRING(env->isolate(), "cNode1.0~"));
	//process.moduleLoadList
	READONLY_PROPERTY(process, "moduleLoadList", env->module_load_list_array());
}

static Environment* CreateEnvironment(Isolate* isolate, Local<Context> context, node::NodeInstanceData* instance_data) {
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




	
