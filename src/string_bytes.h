#include "v8.h"
#include "node.h"

namespace node {
	class StringBytes
	{
	public:
		StringBytes();
		~StringBytes();

		static size_t StorageSize(v8::Isolate* isolate,v8::Local<v8::Value> val,enum encoding enc);

	private:

	};

}
