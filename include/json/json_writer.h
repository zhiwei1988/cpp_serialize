/**
 * @file json_writer.h
 * @author Zhiwei Tan (zhiweix1988@gmail.com)
 * @brief JSON 写入器，用于将数据序列化为 JSON 格式
 * @version 0.1
 * @date 2025-07-05
 *
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <string>
#include <type_traits>
#include <utility>

#include "define_type_traits.h"
#include "yyjson.h"

namespace csrl {

class JsonWriter {
  public:
    JsonWriter(yyjson_mut_doc* doc) { m_doc = doc; }

    // yyjson_mut_doc_free 内部会判断指针是否为空，所以这里不需要判断
    ~JsonWriter() { yyjson_mut_doc_free(m_doc); }

    yyjson_mut_val* SetArrayAsRoot(const size_t) noexcept;

    yyjson_mut_val* SetObjectAsRoot(const size_t) noexcept;

    yyjson_mut_val* SetNullAsRoot() noexcept;

    template <typename T>
    yyjson_mut_val* ValueAsRoot(const T& value) noexcept {
        const auto val = FromBasicValue(value);
        if (val) {
            yyjson_mut_doc_set_root(m_doc, val);
        }
        return val;
    }

    yyjson_mut_val* AddArrayToArray(yyjson_mut_val* array) noexcept {
        const auto val = yyjson_mut_arr(m_doc);
        if (val) {
            yyjson_mut_arr_add_val(array, val);
        }
        return val;
    }

    yyjson_mut_val* AddArrayToObject(const std::string& key, yyjson_mut_val* object) noexcept {
        const auto val = yyjson_mut_arr(m_doc);
        if (val) {
            yyjson_mut_obj_add(object, yyjson_mut_strcpy(m_doc, key.c_str()), val);
        }
        return val;
    }

    yyjson_mut_val* AddObjectToArray(yyjson_mut_val* array) noexcept {
        const auto val = yyjson_mut_obj(m_doc);
        if (val) {
            yyjson_mut_arr_add_val(array, val);
        }
        return val;
    }

    template <typename T>
    yyjson_mut_val* AddValueToArray(const T& value, yyjson_mut_val* array) noexcept {
        const auto val = FromBasicValue(value);
        if (val) {
            yyjson_mut_arr_add_val(array, val);
        }
        return val;
    }

    template <typename T>
    yyjson_mut_val* AddValueToObject(const std::string& key, const T& value, yyjson_mut_val* object) noexcept {
        const auto val = FromBasicValue(value);
        if (val) {
            yyjson_mut_obj_add(object, yyjson_mut_strcpy(m_doc, key.c_str()), val);
        }
        return val;
    }

  private:
    // 处理 std::string 类型
    template <typename T>
    typename std::enable_if<std::is_same<csrl::remove_cvref_t<T>, std::string>::value, yyjson_mut_val*>::type
    FromBasicValue(const T& str) noexcept {
        return yyjson_mut_strcpy(m_doc, str.c_str());
    }

    // 处理 bool 类型
    template <typename T>
    typename std::enable_if<std::is_same<csrl::remove_cvref_t<T>, bool>::value, yyjson_mut_val*>::type
    FromBasicValue(const T& value) noexcept {
        return yyjson_mut_bool(m_doc, value);
    }

    // 处理浮点数类型
    template <typename T>
    typename std::enable_if<std::is_floating_point<csrl::remove_cvref_t<T>>::value, yyjson_mut_val*>::type
    FromBasicValue(const T& value) noexcept {
        return yyjson_mut_real(m_doc, static_cast<double>(value));
    }

    // 处理无符号整数类型
    template <typename T>
    typename std::enable_if<std::is_unsigned<csrl::remove_cvref_t<T>>::value &&
                                std::is_integral<csrl::remove_cvref_t<T>>::value &&
                                !std::is_same<csrl::remove_cvref_t<T>, bool>::value,
                            yyjson_mut_val*>::type
    FromBasicValue(const T& value) noexcept {
        return yyjson_mut_uint(m_doc, static_cast<uint64_t>(value));
    }

    // 处理有符号整数类型
    template <typename T>
    typename std::enable_if<std::is_signed<csrl::remove_cvref_t<T>>::value &&
                                std::is_integral<csrl::remove_cvref_t<T>>::value,
                            yyjson_mut_val*>::type
    FromBasicValue(const T& value) noexcept {
        return yyjson_mut_int(m_doc, static_cast<int64_t>(value));
    }

    yyjson_mut_doc* m_doc;
};
} // namespace csrl
