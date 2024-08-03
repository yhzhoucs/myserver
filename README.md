# Introduction

这是一个用来练习 C++ 网络编程的服务器项目，实现了一个最简单的井字棋小游戏在线对战服务器。另又简单实现了两个与之匹配的客户端，分别使用 C++ 与 Rust 实现。其中 C++ 客户端只适用于 Linux 平台，Rust 客户端可支持包括 Windows 在内的多个平台。

# Features

服务器的实现了

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

为了运行服务器，你需要准备：

- Linux 系统或 WSL2
- C/C++ 编译工具，支持 C++20
- CMake
- Sqlite, LibUUID



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



为了运行 C++ 客户端，你需要准备：

- Linux 系统或 WSL2
- C/C++ 编译工具
- CMake

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



为了运行 Rust 客户端，你需要准备：

- Linux/WSL2/Windows 操作系统
- Rust 编译工具链（rustc, cargo）



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

