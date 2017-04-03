
 代码都放在src目录下；deps目录包括v8的源码以及编译需要的工具等；

说明：这个项目是不能编译的，因为这个是我最开始的版本，v8等编译命令都没设置，我本来想上传一个新的GitHub的可是项目太大了，老上传失败；
      所以我就上传了个百度网盘https://pan.baidu.com/s/1jIC4xCy
      这个是可以直接编译的，有需要的可以去下载；
     

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
