
# Network Test Project

基于C/C++的网络功能测试项目，使用CMake构建系统。

## 项目结构
```
network_test/
├── cmake-build-debug/    # 构建目录
├── CMakeLists.txt        # CMake主配置文件
├── client.cpp            # 客户端实现
├── server.cpp            # 服务端实现
├── util.cpp              # 工具函数实现
└── util.h                # 工具函数声明
```

## 开发环境
- **操作系统**：WSL Ubuntu
- **编译器**：GCC 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)
- **构建工具**：CMake 3.28+
- **目标架构**：x86_64-linux-gnu

## 构建步骤
```bash
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

## 运行测试
```bash
# 启动服务端
./cmake-build-debug/server

# 新终端启动客户端
./cmake-build-debug/client
```

## 依赖项安装
```bash
# 安装编译器依赖
sudo apt-get install libstdc++-dev

# 安装CUDA驱动（可选）
sudo apt-get install cuda-drivers
```

## 特性
- 支持SIGINT/SIGTERM信号优雅退出
- 非阻塞IO读取机制
- 跨平台构建支持