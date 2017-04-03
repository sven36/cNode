#include "v8.h"
#include "uv.h"

#ifdef _WIN32
# ifndef BUILDING_NODE_EXTENSION
#   define NODE_EXTERN __declspec(dllexport)
# else
#   define NODE_EXTERN __declspec(dllimport)
# endif
#else
# define NODE_EXTERN /* nothing */
#endif

namespace node {

	enum encoding { ASCII, UTF8, BASE64, UCS2, BINARY, HEX, BUFFER, LATIN1 = BINARY };

#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                    \
  void operator=(const TypeName&) = delete;                                   \
  void operator=(TypeName&&) = delete;                                        \
  TypeName(const TypeName&) = delete;                                         \
  TypeName(TypeName&&) = delete

	typedef void(*addon_context_register_func)(
		v8::Local<v8::Object> exports,
		v8::Local<v8::Value> module,
		v8::Local<v8::Context> context,
		void *priv
		);
	struct node_module {
		int nm_version;
		unsigned int nm_flags;
		void* nm_dso_handle;
		const char* nm_filename;
		const char* nm_modulename;
		addon_context_register_func nm_context_register_func;
		struct node_module* nm_link;
		void* nm_priv;
	};
	node_module* get_builtin_module(const char* name);
	node_module* get_linked_module(const char* name);

	_declspec(dllexport) int Start(int argc, char *argv[]);

	
	template<class TypeName>
	inline v8::Local<TypeName> StrongPersistentToLocal(const v8::Persistent<TypeName>& persistent);

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