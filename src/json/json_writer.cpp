/**
 * @file json_writer.cpp
 * @author Zhiwei Tan (zhiweix1988@gmail.com)
 * @brief JSON 写入器，用于将数据序列化为 JSON 格式
 * @version 0.1
 * @date 2025-07-05
 * 
 * @copyright Copyright (c) 2025
 */

#include "json_writer.h"

namespace csrl {
yyjson_mut_val* JsonWriter::SetArrayAsRoot(const size_t) noexcept 
{
    yyjson_mut_val* val = yyjson_mut_arr(m_doc);
    if (val) {
        yyjson_mut_doc_set_root(m_doc, val);
    }
    return val;
}

yyjson_mut_val* JsonWriter::SetObjectAsRoot(const size_t) noexcept 
{
    yyjson_mut_val* val = yyjson_mut_obj(m_doc);
    if (val) {
        yyjson_mut_doc_set_root(m_doc, val);
    }
    return val;
}

yyjson_mut_val* JsonWriter::SetNullAsRoot() noexcept 
{
    yyjson_mut_val* val = yyjson_mut_null(m_doc);
    if (val) {
        yyjson_mut_doc_set_root(m_doc, val);
    }
    return val;
}
} // namespace csrl