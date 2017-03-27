
#include "v8.h"
#include "uv\include\uv.h"
#include "env.h"


namespace node {

	void* Realloc(void* pointer, size_t size) {
		if (size == 0) {
			free(pointer);
			return nullptr;
		}
		return realloc(pointer, size);
	}

	void* Malloc(size_t size) {
		if (size == 0)
			size = 1;
		return Realloc(nullptr, size);
	}
	void* Calloc(size_t n, size_t size) {
		if (n == 0) n = 1;
		if (size == 0) size = 1;
		return calloc(n, size);
	}

	enum NodeInstanceType { MAIN, WORKER, REMOTE_DEBUG_SERVER };

	class NodeInstanceData
	{
	public:
		NodeInstanceData(NodeInstanceType type, uv_loop_t* event_loop, int argc, const char** argv, int exec_argc, const char** exec_argv, bool use_debug_agent_flag)
			:node_type(type), exit_code_(1), event_loop_(event_loop), argc_(argc), argv_(argv), exec_argc_(exec_argc), exec_argv_(exec_argv), debug_flag(use_debug_agent_flag) {};
		int exit_code() {
			return exit_code_;
		}
		bool is_main() {
			return node_type == MAIN;
		}
		uv_loop_t* event_loop() {
			return event_loop_;
		}
		int argc() {
			return argc_;
		}

		const char** argv() {
			return argv_;
		}

		int exec_argc() {
			return exec_argc_;
		}

		const char** exec_argv() {
			return exec_argv_;
		}

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
	class ArrayBufferAllocator :public v8::ArrayBuffer::Allocator
	{
	public:
		ArrayBufferAllocator() :env_(nullptr) {};
		inline void set_env(Environment* env) { env_ = env; }
		virtual void* Allocate(size_t size);
		virtual void* AllocateUninitialized(size_t size) {
			return node::Malloc(size);
		}
		virtual void Free(void*data, size_t) { free(data); }

	private:
		Environment* env_;
	};
}
