#include "node.h"
#include "v8\libplatform\libplatform.h"

static int v8_thread_pool_size = 4;

static struct 
{
	void Initialize(int thread_pool_size) {
		//CreateDefaultPlatform是一个虚函数；重写它并初始化；
		platform_ = v8::platform::CreateDefaultPlatform(thread_pool_size);
		V8::InitializePlatform(platform_);
	}
	v8::Platform* platform_;
}v8_platform;


int node::Start(int argc, char * argv[])
{
	int exec_argc;
	const char** exec_argv;
	v8_platform.Initialize(v8_thread_pool_size);
	V8::Initialize();
	return 0;
}
