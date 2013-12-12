BDK
===

BDK - Base Development Kit

1. BDK依赖的第三方库说明：
    - BOOST
      * 只使用无需编译的库
      * 主要使用的库如下：
        ** noncopyable -> 禁用拷贝构造函数和赋值运算符
        ** function    -> 回调函数
        ** bind        -> 回调函数
    - PTHREAD [win32]
      * 主要使用的库如下：
        ** thread
        ** mutex
        ** condition
        
    [Note] 
    - 上述库只需要在Visual Studio 2005中进行简单配置即可
      * Tool -> Options -> Projects and Solutions -> VC++ Directories
        ** Include files : 加入BOOST和PTHREAD头文件路径
        ** Library files : 加入PTHREAD库文件路径
