#include "v8\v8.h"
#include "deps\uv\include\uv.h"

namespace node {
	_declspec(dllexport) int Start(int argc, char *argv[]);
}

enum NodeInstanceType{MAIN,WORKER, REMOTE_DEBUG_SERVER};

class NodeInstanceData
{
public:
	NodeInstanceData(NodeInstanceType type, uv_loop_t* event_loop, int argc, const char** argv, int exec_argc, const char** exec_argv, bool use_debug_agent_flag)
		:node_type(type), exit_code_(1), event_loop_(event_loop), argc_(argc), argv_(argv), exec_argc_(exec_argc), exec_argv_(exec_argv), debug_flag(use_debug_agent_flag) {};

private:
	const NodeInstanceType node_type;
	int exit_code_;
	uv_loop_t* const event_loop_;
	const int argc_;
	const char** argv_;
	const int exec_argc_;
	const char** exec_argv_;
	const bool debug_flag;
};

