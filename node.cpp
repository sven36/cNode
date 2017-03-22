#include "node.h"
#include "v8\libplatform\libplatform.h"

using namespace v8;

static int v8_thread_pool_size = 4;
static bool use_debug_agent = false;
static Isolate* node_isolate;

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





int node::Start(int argc, char * argv[])
{
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
static void StartNodeInstance(void* arg) {
	NodeInstanceData* instance_data = static_cast<NodeInstanceData*>(arg);
	Isolate::CreateParams params;
	ArrayBufferAllocator* array_buffer_allocator = new ArrayBufferAllocator();
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
		Environment* env = CreateEnvironment(isolate,context,instance_data);
	}
}
static Environment* CreateEnvironment(Isolate* isolate,Local<Context> context,NodeInstanceData* instance_data) {
	HandleScope handle_sope(isolate);
	Context::Scope context_scope(context);
	Environment* env = Environment::New(context,instance_data->event_loop());

}
inline Environment* Environment::New(v8::Local<v8::Context> context,uv_loop_t* loop) {
	Environment* env = new Environment(context, loop);

}
inline Environment::Environment(v8::Local<v8::Context> context, uv_loop_t* loop)
	:isolate_(context->)
