## 1. 目录与文件说明

```shell
.
├── CMakeLists.txt
├── config.json
├── config.json.in
├── extension_include
│   └── ptrace_tool.h
├── extension_src
│   ├── config.cpp -> ../src/config.cpp
│   ├── expception.cpp -> ../src/expception.cpp
│   ├── format.cpp -> ../src/format.cpp
│   ├── log.cpp -> ../src/log.cpp
│   ├── logger.cpp -> ../src/logger.cpp
│   ├── main.cpp
│   ├── ptrace_tool.cpp
│   └── tool.cpp -> ../src/tool.cpp
├── include
│   ├── config.h
│   ├── exception.h
│   ├── format.h
│   ├── logger.h
│   ├── log.h
│   ├── option.h
│   ├── option.h.in
│   └── tool.h
├── README.md
├── src
│   ├── config.cpp
│   ├── expception.cpp
│   ├── format.cpp
│   ├── libhook.cpp
│   ├── log.cpp
│   ├── logger.cpp
│   └── tool.cpp
└── test
    ├── config.json -> ../config.json
    ├── hook_run.sh
    ├── src
    │   ├── build.sh
    │   ├── connect.c
    │   └── open.c
    ├── test_black_list.txt
    └── test_white_list.txt
```

1. `src`和`include`下为构建共享库的代码文件。

2. `extension_src`和`extension_include`为构建扩展的代码文件。（扩展使用了部分`src`的代码，以符号链接引入）

3. `test`为辅助测试文件，与测试用的`config.json`配合使用，以符号链接形式引入。

4. cmake利用`config.json.in`生成测试用的`config.json`。


## 2. 构建

依赖：cjson

```shell
mkdir build && cd build
cmake ..
cmake --build .
```

构建成功后build目录下会得到共享库`libhook.so`和可执行程序`static_hook`。

可选选项（在CMakeLists.txt中修改，或者命令行中`-D`指定）：
- `CONFIG_FILE_PATH`
    配置文件路径
- `LOG_FILE_PATH`
    日志文件路径
- `ENABLE_ALLOW_LOG`
    - 是否开启`ALLOW`日志
    - 可选值为`true`或`false`。
    - 默认为`false`。
- `ENABLE_DENY_LOG`
    - 是否开启`DENY`日志。
    - 值为`true`或`false`。
    - 默认为`true`。
    

## 3. 测试与使用

### 方法一

将编译出的共享库写入`/etc/ld.so.preload`，并重启进程。

```shell
sudo echo `<dir>/build/libhook.so` > /etc/ld.so.preload
```
#### 方法二

将共享库加入`LD_PRELOAD`环境变量。

或者以`test`目录下的`hook_run.sh`运行。

例如测试`cat`命令访问白名单测试文件`test_white_list.txt`：
```shell
cd test
./hook_run.sh cat test_white_list.txt
```

####  配置文件说明

只有在白名单中的程序 才能访问 白名单保护的文件。
只有在黑名单中的程序 不能访问 黑名单保护的文件。

运行时会解析符号链接，找到真正的可执行程序。

### 注意

1. shell脚本通过exec调起可执行程序时，可以绕过拦截，所以配置文件中**请写入真正的可执行程序**，或可执行程序的符号链接。
2. 测试网络时请关闭代理。且使用无痕浏览模式效果更真实。
   
除给出示例外，文件拦截可以`vim`、`cat`、`cp`等进行测试，ip拦截可以谷歌浏览器、`ping`、`ssh`等为例。

## BUG

1. 静态拦截程序`static_detect`仅作原理展示和可行性验证，实际用处不大。
2. 获取当前时间时，部分情况为*epoch*，原因不明。