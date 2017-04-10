
 代码都放在src目录下；deps目录包括v8的源码以及编译需要的工具等；

 说明：完整编译的文件太大，上传GitHub不成功，下载请用百度网盘https://pan.baidu.com/s/1jIC4xCy
 
 ------------------------------------------------------------------------------------------------------------------------
先说一下node.js启动过程：

       node.js的src目录下的源代码大部分都是node.js的模块文件；其实初始化node.js用到的文件只有：node.h , node.cc , env.h , env_inl.h , node_internals.h , node_javascript.h , node_javascript.cc , util.h , util.cc ,以及用js2c.py工具将内置JavaScript代码转成C++里面的数组，生成的node_natives.h文件;

我实现的过程是按照node.js的启动过程，需要哪个方法就实现哪个方法，能合并的方法都尽量合并，能忽略的细节都尽量忽略，下面简单说说node.js启动主要的方法和过程；

入口是在node_main.cc中，根据平台的不同会进入不同的Start方法，我的是windows平台，运行的wmain方法，然后调用了node::Start方法；

node::Start方法的具体实现是在node.cc中，node.cc也是node的核心代码；在启动过程中需要注意的有四个方法：StartNodeInstance，在StartNodeInstance里面调用的CreateEnvironment方法，

在CreateEnvironment方法里面调用的SetupProcessObject方法，以及CreateEnvironment结束之后调用的LoadEnvironment方法；

StartNodeInstance在初始化v8虚拟机，绑定作用域之后就会调用CreateEnvironment方法；CreateEnvironment会初始化Environment类，该方法定义在env_inl.h文件中；

CreateEnvironment在初始化Environment类之后，会先初始化v8的的CPU分析器，再初始化handle的回收方法，然后就会初始化全局process对象；

代码如下：

Local<FunctionTemplate> process_template = FunctionTemplate::New(isolate);
process_template->SetClassName(node::OneByteString(isolate, "process", sizeof("process") - 1));

Local<Object> process_object = process_template->GetFunction()->NewInstance(context).ToLocalChecked();
env->set_process_object(process_object);

在v8里面一个template是javascript函数的蓝图。你可以使用一个template来将c++函数和结构体包装到javascript对象中，让javascirpt脚本来使用它。所以我们经常调用的process.binding,process.cpuUsage,process.dlopen等方法，其实是在调用包装成js脚本的C++方法；

接下来的SetupProcessObject方法就是具体初始化process对象的方法了，除了只读属性process.versions,process.moduleLoadList等;更重要的是通过Environment::SetMethod方法，把C++方法包装成js脚本方法；比如:

  env->SetMethod(process, "binding", Binding);

就把process.binding方法绑定成C++里面的Binding方法；

初始化process对象之后就会调用LoadEnvironment方法，在该方法中我们可以看的：

Local<String> script_name = FIXED_ONE_BYTE_STRING(env->isolate(),
"bootstrap_node.js");
Local<Value> f_value = ExecuteString(env, MainSource(env), script_name);

其中ExecuteString方法会调用v8::Script::Compile方法来编译传入的js文件；那我们知道了bootstrap_node.js是一个被调用的js文件，在这个里面又发生了什么呢？

大概可以分为：初始化全局 process 对象上的部分属性 / 行为，初始化全局的一些 timer 方法，初始化全局 console 对象等一些方法；这里我们不展开了，我们看一看node的js模块是如何引入的；


function NativeModule(id) {
this.filename = `${id}.js`;
this.id = id;
this.exports = {};
this.loaded = false;
this.loading = false;
}

NativeModule._source = process.binding('natives');
NativeModule._cache = {};

我们看的原生模块会调用process.binding('natives')方法，我们找到node.cc里面的Binding方法看看会进行哪些操作；

else if (!strcmp(*module_v, "natives")) {
exports = Object::New(env->isolate());
DefineJavaScript(env, exports);
cache->Set(module, exports);

我们看到当传入的参数是natives的时候会调用DefineJavaScript方法，我们看看这个方法做了什么；

void DefineJavaScript(Environment* env, Local<Object> target) {
auto context = env->context();
#define V(id) \
do { \
auto key = \
String::NewFromOneByte( \
env->isolate(), id##_name, NewStringType::kNormal, \
sizeof(id##_name)).ToLocalChecked(); \
auto value = \
String::NewExternalOneByte( \
env->isolate(), &id##_external_data).ToLocalChecked(); \
CHECK(target->Set(context, key, value).FromJust()); \
} while (0);
NODE_NATIVES_MAP(V)
#undef V
}

是一个复杂的宏定义，看着不太好理解那我们自己抽离一个实现看看；

void DefineJavaScript(Environment* env, Local<Object> target){
auto context = env->context();
do {
auto key = String::NewFromOneByte(env->isolate(), buffer_name, NewStringType::kNormal, sizeof(buffer_name)).ToLocalChecked();
auto value = String::NewExternalOneByte(env->isolate(), &buffer_external_data).ToLocalChecked();
} while (0);

}

buffer_name是什么呢？在js2c.py把内置js文件转成C++数组的node_natives.h文件里面我们可以找到：

static const uint8_t buffer_name[] = {
98,117,102,102,101,114};

所以process.binding('natives')其实是使用v8引擎，编译我们内置的js文件；

到这里node.js的启动过程和文件模块机制基本上就说了个大概了，其它诸如非核心模块的引入，和buffer,stream等应C++完成核心部分，其它部分用js包装或导出的模块就需要大家自己去了解了；



我在过程中碰到一些问题和需要注意的地方：


--------------------------------------------------------------------------------------------------------------------------------------
前言：

最近在看Node.js，看了一段时间后便想着自己实现一个Node.js现在已经实现了个大概（绝大部分是模仿人家，不过自己实现一遍基本上就理解Node.js的原理了）下面便说说这个过程中的坑，以及一些需要注意的地方；

      Node.js需要一定C++基础，建议看完C++Primer再看，否则V8的好多表达方式，指针，引用，模板之类的会看不懂；

编译：我用的win10的环境，具体编译请参考Node.js的编译说明：https://github.com/nodejs/node/blob/master/BUILDING.md

        其中的坑：Python应该是2.6或2.7，不要装3.0或以上的，因为node.js有的py文件3.0编译出错； Visual C++ Build Tools必须是2015，安装官方的链接就是，因为Node.js在Windows平台的编译用的是vs2015的v140平台工具集，用低版本的或者vs2017的v141平台工具集都会报错；

 

        编译流程：安装完python和Visual C++ Build Tools2015之后，下载node.js的源码  node-v6.10.0.tar.gz  然后用Visual C++ Build Tools2015的命令行运行解压目录下的vcbuild.bat处理文件就会开始编译了；编译成功后解压目录下就会出现.sln文件就可以使用vs打开了（vs也需要是2015或2017，并且项目的平台工具集也需要是v140否则编译报错）；


自己实现一个Node.js的难点与思路：

V8引擎：

          因为node.js其实就是嵌入V8的一个C++程序；首先要对v8的Isolate，LocalHandle，Scope等概念有一个了解，此处不展开了，请参考这个文档：

           https://github.com/Chunlin-Li/Chunlin-Li.github.io/blob/master/blogs/javascript/V8_Embedder's_Guide_CHS.md

          在我的代码里面我也加了一些注释，在src目录下的node.cpp文件内，可以参考；

 

C++编译与平常的面向对象编译方式的不同：

          在.NET或Java之类的语言中，可以不必关注方法的声明顺序，比如：

          private void a(){

               b();

         }

         private void b(){

               console.log(1);

         }

         不过在C++中这样是不行的，调用b之前，必须完全声明b；也就是把b放在方法a之前(这也是node.cpp文件中为什么把开始方法Start放在最下端)；

     

为什么Node.js的头文件要用namespace node包起来：

                 是为了更好的解耦node.js的各个模块；在C++中命名空间相同而内部成员名字不同，它们会自动合并为同一个命名空间，可以理解为追加；

 

Node.js里面比较复杂的宏定义：

    Node.js和V8用了很多复杂的宏定义，如果不理解它们看起来会很费力；在C++中，宏定义里面的##符号是连接字符串的意思；

    比如：env-inl.h文件下的宏定义：

#define VP(PropertyName, StringValue) V(v8::Private, PropertyName, StringValue)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName, StringValue)
#define V(TypeName, PropertyName, StringValue)                                \
  inline                                                                      \
  v8::Local<TypeName> Environment::IsolateData::PropertyName() const {        \
    /* Strings are immutable so casting away const-ness here is okay. */      \
    return const_cast<IsolateData*>(this)->PropertyName ## _.Get(isolate());  \
  }
    PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(VP)
        PER_ISOLATE_STRING_PROPERTIES(VS)
#undef V
#undef VS
#undef VP

它最后的编译形式是：

    inline v8::Local<v8::String> Environment::IsolateData::async_queue_string() const {
        return const_cast<IsolateData*>(this)->async_queue_string_.Get(isolate());
    }

这种地方多一些耐心仔细分析一下就会懂了；我写的代码里面基本上这种宏定义第一个字符串我都是这种手写的，其余的是按照原先的宏定义的方式声明，可以参考；

 

使用VS编译需要注意的地方：

      node.js项目文件其实是用google的GYP工具生产的，所以VS项目编译的过程也在各个项目下的.gyp文件内。

      了解gyp工具请参考：http://www.cnblogs.com/nanvann/p/3913880.html#conditions
