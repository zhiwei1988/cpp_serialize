## 基本运行

```cpp
add_test(NAME <name> COMMAND <command> [arg1 [arg2 ...]])
```

- NAME: 测试的名称，必须唯一
- COMMAND: 要执行的命令或可执行文件
- 参数: 传递给命令的参数

```cmake
# 1. 基本用法 - 运行可执行文件
add_test(NAME my_test COMMAND my_test_executable)

# 2. 带参数的测试
add_test(NAME test_with_args COMMAND my_program --input file.txt --verbose)

# 3. 使用目标名称（推荐）
add_test(NAME unit_tests COMMAND $<TARGET_FILE:my_test_target>)

# 4. 设置工作目录
add_test(NAME test_in_dir COMMAND my_test)
set_tests_properties(test_in_dir PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_data)

# 5. 设置环境变量
set_tests_properties(my_test PROPERTIES 
    ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}")

# 6. 设置超时时间
set_tests_properties(my_test PROPERTIES TIMEOUT 30)

# 7. 预期失败的测试
set_tests_properties(my_test PROPERTIES WILL_FAIL TRUE)
```

// 运行所有测试
ctest

// 详细输出
ctest -V
ctest --verbose

// 仅显示失败的测试
ctest --output-on-failure

## 测试选择

// 运行特定测试
ctest -R "test_name"
ctest --tests-regex "pattern"

// 排除特定测试
ctest -E "test_name"
ctest --exclude-regex "pattern"

// 运行指定标签的测试
ctest -L "label"

## 并行执行

// 并行运行（4个线程）
ctest -j4
ctest --parallel 4

## 重复和调试

// 重复运行测试
ctest --repeat until-fail:3

// 停止在第一个失败
ctest --stop-on-failure

// 超时设置
ctest --timeout 30

## 高级配置

### 设置测试属性

```cmake
# 设置测试属性
set_tests_properties(my_test PROPERTIES
    TIMEOUT 30                    # 超时时间
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}  # 工作目录
    ENVIRONMENT "VAR=value"       # 环境变量
    PASS_REGULAR_EXPRESSION "PASS"  # 成功的正则表达式
    FAIL_REGULAR_EXPRESSION "FAIL"  # 失败的正则表达式
    WILL_FAIL TRUE               # 预期失败的测试
    LABELS "unit;fast"           # 测试标签
)
```

### 设置测试依赖

```cmake
# 设置测试依赖关系
set_tests_properties(test2 PROPERTIES DEPENDS test1)
```

### 自定义测试命令

```cmake
# 使用脚本作为测试
add_test(NAME script_test 
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/test_script.cmake)

# 使用 Python 脚本
add_test(NAME python_test 
    COMMAND python3 ${CMAKE_SOURCE_DIR}/test.py)
```