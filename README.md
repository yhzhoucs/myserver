# Introduction

一个用来练习 C++ 网络编程的服务器项目，实现了一个最简单的井字棋小游戏在线对战服务器。

另外又简单实现了两个与之匹配的客户端，分别使用 C++ 与 Rust 实现。其中 C++ 客户端只适用于类 Unix 平台，Rust 客户端可支持包括 Windows 在内的多个平台。



# Features

服务器实现了

- 基于 TCP 的 C/S 通信
- 数据库连接池，基于 SQLite
- 线程池
- 定时器
- 异步日志系统



目前支持

- 登录验证
- 战局匹配
- 输赢判定
- 超时断连



# Manual

## 服务器

为了运行服务器，你需要准备：

- 类 Unix 系统或 WSL2
- C/C++ 编译工具，支持 C++20
- CMake
- Sqlite, LibUUID

服务器的用户数据采用 Sqlite3 存储，并使用 LibUUID 来生成日志文件的唯一编号。代码里用到了 Concepts 相关的语法，因此**需要支持 C++20 的编译器**来编译。项目使用 FetchContent 来下载和安装依赖库，因此请确保你的 CMake 版本**支持 FetchContent**。我使用的版本是 CMake 3.30.2 。服务器使用 Unix 系统调用完成构建，因此只能在类 Unix 系统上运行。



下面演示了怎样在 Ubuntu 20.04 上运行这个服务器。

首先，安装必要的依赖：

```she	
sudo apt-get update
sudo apt-get install sqlite3 libsqlite3-dev
sudo apt-get install uuid-dev
```

下载本仓库到本地并编译服务器：

```shel
git clone https://github.com/yhzhoucs/myserver.git
cd myserver
mkdir build
cmake -S . -B build
cmake --build build --target myserver
```

运行服务器：

```shell
./build/src/myserver
```



## C++ 客户端

为了运行 C++ 客户端，你需要准备：

- 类 Unix 系统或 WSL2
- C/C++ 编译工具，支持 C++11
- CMake

客户端是一个 stand-alone 的项目，直接在 clients/cpp 目录下编译。

因为这个客户端使用了 Unix 的系统调用和套接字实现，所以只能在类 Unix 系统上运行。**如果你想在 Windows 上运行，请直接使用另一个由 Rust 实现的客户端。** 客户端本身使用纯 C 实现，但它依赖的 json 库需要 C++11 的支持，因此你需要一个支持 C++11 的编译器。客户端使用 FetchContent 来下载和安装 json 依赖库，因此请确保你的 CMake 版本**支持 FetchContent**。



下面演示了怎样在 Ubuntu 20.04 上运行 C++ 客户端。

首先，修改 clients/cpp/main.cpp 中的服务器地址。

然后，进入项目 clients/cpp 目录下进行编译：

```shell
cd clients/cpp
mkdir build
cmake -S . -B build
cmake --build build --target client
```

确保此时对应地址上运行了相应的服务器程序，接着就能运行客户端程序了：

```shell
./build/client
```



## Rust 客户端

为了运行 Rust 客户端，你需要准备：

- 类 Unix /WSL2/Windows 操作系统
- Rust 编译工具链（rustc, cargo）

这个客户端由 Rust 写成，支持跨平台。你可以参考 [官网教程](https://www.rust-lang.org/tools/install) 完成 Rust 编译工具链的安装。它也是一个 stand-alone 项目，直接在 clients/rust 目录下编译。



下面演示了怎样在 Windows 10 上运行 Rust 客户端。

首先，修改 clients/rust/src/main.rs 中的服务器地址。

然后，进行 clients/rust 目录下进行编译：

```shell
cd clients/rust
cargo build
```

确保此时对应地址上运行了相应的服务器程序，接着就能运行客户端程序了：

```shell
cargo run
```

