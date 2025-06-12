# C++ Serialize Project

这是一个基于 CMake 的 C++14 项目。

## 项目结构

```
cpp_serialize/
├── CMakeLists.txt          # 主 CMake 配置文件
├── README.md               # 项目说明文档
├── include/                # 头文件目录
│   ├── core/              # 核心功能头文件
│   ├── json/              # JSON 序列化相关（待实现）
│   └── tlv/               # TLV 序列化相关（待实现）
├── src/                    # 源文件目录
│   └── main.cpp           # 主程序
├── test/                   # 测试文件目录
│   ├── CMakeLists.txt     # 测试 CMake 配置
│   └── test_*.cpp         # 各种测试文件
└── build/                 # 构建目录（生成）
```

## 构建要求

- CMake 3.14 或更高版本
- 支持 C++14 的编译器（如 GCC 5.0+ 或 Clang 3.4+）

## 构建步骤

1. 创建构建目录：
```bash
mkdir build
cd build
```

2. 配置项目：
```bash
cmake ..
```

3. 编译项目：
```bash
make
```

4. 运行程序：
```bash
./cpp_serialize
```

5. 运行测试：
```bash
./test/test_cpp_serialize
``` 