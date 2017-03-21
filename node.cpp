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
	Platform* platform_;
}v8_platform;


int node::Start(int argc, char * argv[])
{
	int exec_argc;
	const char** exec_argv;
	v8_platform.Initialize(v8_thread_pool_size);
	V8::Initialize();
	{
		NodeInstanceData instance_data(NodeInstanceType::MAIN,uv_default_loop(),argc,const_cast<const char**>(argv),exec_argc, exec_argv, use_debug_agent);
	}
	return 0;
}
static void StartNodeInstance(void* arg) {

}
