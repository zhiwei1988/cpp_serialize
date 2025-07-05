## 关键特性

yyjson 提供了一套全面的 JSON 处理功能：

- 高性能：在现代 CPU 上能够以每秒千兆字节的速度读取和写入 JSON 数据
- 标准合规：完全符合 RFC 8259 JSON 标准
- 可移植性：使用 ANSI C (C89) 编写，以实现最大的跨平台兼容性
- 可扩展性：支持注释、尾随逗号、NaN/Infinity 值和自定义内存分配器
- 数值精度：精确处理 int64_t 、 uint64_t 和 double 数字
- 灵活的字符串处理：支持无限的 JSON 嵌套层级、 \u0000 个字符以及非空终止字符串
- JSON 操作：提供对 JSON 指针、JSON 补丁和 JSON 合并补丁的支持
- 开发者友好：只需一个头文件和一个源文件即可轻松集成

## 库架构

![](https://asserts.taneq.me/2025-07-05_12-35-06.jpg)

## 数据流

![](https://asserts.taneq.me/2025-07-05_12-35-54.jpg)

## 核心概念

1. 文档类型：该库同时提供不可变 ( yyjson_doc , yyjson_val ) 和可变 ( yyjson_mut_doc , yyjson_mut_val ) 文档类型。不可变文档在读取 JSON 时创建，并针对读取操作进行了优化。可变文档用于构建或修改 JSON。
2. 值类型：JSON 值由特定的类型和子类型表示：
   - YYJSON_TYPE_NULL: Null value
   - YYJSON_TYPE_BOOL: Boolean value (subtypes: TRUE, FALSE)
   - YYJSON_TYPE_NUM: Number value (subtypes: UINT, SINT, REAL)
   - YYJSON_TYPE_STR: String value
   - YYJSON_TYPE_ARR: Array value
   - YYJSON_TYPE_OBJ: Object value
   - YYJSON_TYPE_RAW: Raw string value (used with specific read flags)
3. 内存管理：该库对所有内存操作都使用内存分配器。用户可以提供自定义分配器，也可以使用内置的分配器（默认、池、动态）。
4. SON 处理流程涉及读取（解析）、操作和写入（序列化）操作，每个操作都有可配置的选项。

## 使用方式

Reading JSON

```c
// Read JSON from string
const char *json = "{\"name\":\"Mash\",\"star\":4,\"hits\":[2,2,1,3]}";
yyjson_doc *doc = yyjson_read(json, strlen(json), 0);
yyjson_val *root = yyjson_doc_get_root(doc);

// Access values
yyjson_val *name = yyjson_obj_get(root, "name");
printf("name: %s\n", yyjson_get_str(name));

// Free the document when done
yyjson_doc_free(doc);
```

Writing JSON

```c
// Create a mutable document
yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
yyjson_mut_val *root = yyjson_mut_obj(doc);
yyjson_mut_doc_set_root(doc, root);

// Add values
yyjson_mut_obj_add_str(doc, root, "name", "Mash");
yyjson_mut_obj_add_int(doc, root, "star", 4);

// Create and add an array
int hits_arr[] = {2, 2, 1, 3};
yyjson_mut_val *hits = yyjson_mut_arr_with_sint32(doc, hits_arr, 4);
yyjson_mut_obj_add_val(doc, root, "hits", hits);

// Write to string
const char *json = yyjson_mut_write(doc, 0, NULL);

// Free resources
free((void *)json);
yyjson_mut_doc_free(doc);
```

## Reference

- [yyjson](https://github.com/ibireme/yyjson)
- [yyjson deekwiki](https://deepwiki.com/ibireme/yyjson)