#include "v8.h"

namespace node {

template<typename T,size_t kStackStorageSize =1024>
class MaybeStackBuffer
{
public:
	const T* out() const {
		ret buf_;
	}
	T*out() {
		return buf_;
	}
	const T* operator*() const {
		return buf_;
	}

	MaybeStackBuffer() :length_(0), buf_(buf_st_) {
		buf_[0] = T();
	}
	~MaybeStackBuffer() {
		if (buf_ != buf_st_) {
			free(buf_);
		}
	}
	void AllocateSufficientStorage(size_t storage) {
		if (storage <= kStackStorageSize) {
			buf_ = buf_st_;
		}
		else
		{
			buf_ = static_cast<T*>(node::Malloc(sizeof(T)* storage));
		}
		length_ = storage;
	}
	void SetLength(size_t length) {
		length_ = length;
	}
	void SetLengthAndZeroTerminate(size_t length) {
		SetLength(length);
		buf_[length] = T();
	}

private:
	size_t length_;
	T* buf_;
	T buf_st_[kStackStorageSize];
};
class Utf8Value:public MaybeStackBuffer<char>
{
public:
	explicit Utf8Value(v8::Isolate* isolate,v8::Local<v8::Value> value);

};

}

