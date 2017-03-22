#include "v8\v8.h"
#include "deps\uv\include\uv.h"
#include "env.h"


#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                    \
  void operator=(const TypeName&) = delete;                                   \
  void operator=(TypeName&&) = delete;                                        \
  TypeName(const TypeName&) = delete;                                         \
  TypeName(TypeName&&) = delete

namespace node {
	_declspec(dllexport) int Start(int argc, char *argv[]);
	void* Realloc(void* pointer, size_t size) {
		if (size == 0) {
			free(pointer);
			return nullptr;
		}
		return realloc(pointer, size);
	}

	void* Malloc(size_t size) {
		if (size == 0)
			size == 1;
		return Realloc(nullptr, size);
	}
	template <typename Traits> class ConditionVariableBase;
	template <typename Traits> class MutexBase;

	struct LibuvMutexTraits;
	using ConditionVariable = ConditionVariableBase<LibuvMutexTraits>;
	using Mutex = MutexBase<LibuvMutexTraits>;

	template<typename Traits>
	class MutexBase
	{
	public:
		inline	MutexBase();
		inline	~MutexBase();
		inline void Lock();
		inline void UnLock();
		class ScopedLock;
		class ScopedUnLock;

		class ScopedLock
		{
		public:
			inline	explicit ScopedLock(const MutexBase& mutex);
			inline	explicit ScopedLock(const ScopedUnLock& scoped_unlock);
			inline ~ScopedLock();

		private:
			//template <typename> friend class 
			friend class ScopedUnLock;
			const MutexBase& mutex_;
			DISALLOW_COPY_AND_ASSIGN(ScopedLock);
		};
		class ScopedUnLock
		{
		public:
			inline explicit	ScopedUnLock(const ScopedLock& scoped_lock);
			inline ~ScopedUnLock();

		private:
			friend class ScopedLock;
			const MutexBase& mutex_;
			DISALLOW_COPY_AND_ASSIGN(ScopedUnLock);
		};


	private:
		mutable typename Traits::MutexT mutex_;
		DISALLOW_COPY_AND_ASSIGN(MutexBase);
	};

	template<typename Traits>
	class ConditionVariableBase
	{
	public:
		using ScopedLock = typename MutexBase<Traits>::ScopedLock;
		inline ConditionVariableBase();
		inline ~ConditionVariableBase();

	private:

	};


	struct LibuvMutexTraits {
		using CondT = uv_cond_t;
		using MutexT = uv_mutex_t;
	};
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

