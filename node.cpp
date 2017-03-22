#include "node.h"
#include "v8\libplatform\libplatform.h"

using namespace v8;

static int v8_thread_pool_size = 4;
static bool use_debug_agent = false;


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
		
		Mutex::ScopedLock scoped_lock();
	}
}
