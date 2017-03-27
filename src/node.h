#include "v8.h"
#include "uv\include\uv.h"

namespace node {
	_declspec(dllexport) int Start(int argc, char *argv[]);

	//
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


