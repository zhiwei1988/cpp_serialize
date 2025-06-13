#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <typeindex>
#include <functional>
#include <type_traits>
#include <iostream>
#include <cxxabi.h>
#include <unordered_set>

class ByteStorage {
  public:
    explicit ByteStorage(const size_t& size) : m_size(size) {
        if (m_size) {
            m_data = new (std::nothrow) uint8_t[m_size]();
            if (m_data == nullptr) {
                return;
            }
        } else {
            m_data = nullptr;
        }
    }

    ByteStorage(const ByteStorage&) = delete;
    ByteStorage& operator=(const ByteStorage&) = delete;

    virtual ~ByteStorage() {
        if (m_data != nullptr) {
            delete[] m_data;
            m_data = nullptr;
        }
    }

    void resize(const size_t size) {
        uint8_t* newData = new (std::nothrow) uint8_t[size]();
        if (newData == nullptr) {
            m_size = 0;
            return;
        }

        auto func = [&newData] {
            if (newData != nullptr) {
                delete[] newData;
                newData = nullptr;
            }
        };

        size_t copyLength = std::min(size, m_size);
        auto result = memcpy(newData, m_data, copyLength);
        if (result != 0) {
            func();
            return;
        }

        delete[] m_data;
        m_data = newData;
        m_size = size;
    }

    const size_t size() const { return m_size; }

    uint8_t* data() const { return m_data; }

    uint8_t* m_data = nullptr;

  protected:
    size_t m_size = 0;
};

class TLVWriter : public ByteStorage {
  public:
    explicit TLVWriter(size_t size = 1024) : ByteStorage(size), m_currentIndex(0) {}

    // 递归计算各段长度（要求参数必须成对：指针和长度）
    static size_t TotalLength() { return 0; }

    template <typename Ptr, typename Len, typename... Rest>
    static size_t TotalLength(const Ptr /*buf*/, Len len, Rest&&... rest) {
        return len + TotalLength(std::forward<Rest>(rest)...);
    }

    // 递归写入各段数据，更新 currentIndex
    static void AppendSegments(uint8_t* /*dest*/, size_t& /*index*/) {}

    template <typename Ptr, typename Len, typename... Rest>
    static void AppendSegments(uint8_t* dest, size_t& index, const Ptr buf, Len len, Rest&&... rest) {
        if (len > 0 && buf != nullptr) {
            memcpy(dest + index, buf, len);
        }
        index += len;
        AppendSegments(dest, index, std::forward<Rest>(rest)...);
    }

    // 通用的追加接口：
    // 先写入 type (uint32_t) 和整个数据段的长度
    // (uint32_t)，再将各段数据拼接到一起
    template <typename... Args>
    int32_t Append(uint32_t type, Args&&... args) {
        static_assert(sizeof...(Args) % 2 == 0, "Buffer segments must come in pairs: pointer and length.");
        size_t segmentsLen = TotalLength(std::forward<Args>(args)...);
        size_t required = m_currentIndex + sizeof(type) + sizeof(uint32_t) + segmentsLen;
        if (required > m_size) {
            required = std::max(m_size * 2, required);
            resize(required);
        }
        // 写入 type
        memcpy(m_data + m_currentIndex, &type, sizeof(type));
        m_currentIndex += sizeof(type);
        // 写入整个数据段长度
        uint32_t valueLen = static_cast<uint32_t>(segmentsLen);
        memcpy(m_data + m_currentIndex, &valueLen, sizeof(valueLen));
        m_currentIndex += sizeof(valueLen);
        // 将各段数据依次复制
        AppendSegments(m_data, m_currentIndex, std::forward<Args>(args)...);
        return 0;
    }

    // AppendBuf 接口：仅传入一段数据
    int32_t AppendBuf(uint32_t type, const char* buf, size_t len) { return Append(type, buf, len); }

    // AppendPair 接口：传入两段数据，内部会拼接两段数据（总长度为两段之和）
    int32_t AppendPair(uint32_t type, const char* firstBuf, size_t firstLen, const char* secBuf, size_t secLen) {
        return Append(type, firstBuf, firstLen, secBuf, secLen);
    }

    size_t CurrentIndex() { return m_currentIndex; }

    void ZeroIndex() { m_currentIndex = 0; }

  private:
    size_t m_currentIndex;
};

// ========== 以下内容来自 serialize_to_tlv.h ==========

const uint32_t TLV_BUF_LEN = 1024;

#define LOG_DEBUG(msg) std::cout << "[DEBUG] " << __func__ << ":" << __LINE__ << " " << msg << std::endl

// 获取类型名称的辅助函数（用于调试模板实例化）
template <typename T>
std::string GetTypeName() {
    int status;
    char* demangledName = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
    std::string result = (status == 0 && demangledName) ? demangledName : typeid(T).name();
    free(demangledName);
    return result;
}

template <typename T>
void PrintType(const char* message) {
    std::cout << message << GetTypeName<T>() << std::endl;
}

// 反射系统
class MetaReflection {
  private:
    // 字段描述符
    struct FieldDescriptor {
        std::function<void(const void*, std::shared_ptr<TLVWriter>&)> serialize;
        std::string fieldName;
    };

    // 字段列表类型
    struct TypeFields {
        std::vector<FieldDescriptor> fields;
        std::unordered_set<std::string> fieldNameSet;
    };

    // 注册表类型
    using TypeFieldsMap = std::map<std::type_index, TypeFields>;

    // 单例注册表
    static TypeFieldsMap& Registry() {
        static TypeFieldsMap registry;
        return registry;
    }

    // 类型名称映射（用于调试）
    static std::map<std::type_index, std::string>& TypeNames() {
        static std::map<std::type_index, std::string> typeNames;
        return typeNames;
    }

  public:
    // 注册字段
    template <typename T>
    static void RegisterField(std::function<void(const T&, std::shared_ptr<TLVWriter>&)> serializeFunc,
                              const std::string& fieldName = "unknown") {
        auto& typeFields = Registry()[std::type_index(typeid(T))];

        if (typeFields.fieldNameSet.find(fieldName) != typeFields.fieldNameSet.end()) {
            return;
        }

        FieldDescriptor field;
        field.serialize = [serializeFunc](const void* obj, std::shared_ptr<TLVWriter>& buf) {
            serializeFunc(*static_cast<const T*>(obj), buf);
        };

        field.fieldName = fieldName;

        typeFields.fields.push_back(field);
        typeFields.fieldNameSet.insert(fieldName);
    }

    // 序列化字段
    template <typename T>
    static void SerializeFields(const T& obj, std::shared_ptr<TLVWriter>& buf) {
        auto it = Registry().find(std::type_index(typeid(T)));
        if (it != Registry().end()) {
            for (const auto& field : it->second.fields) {
                field.serialize(&obj, buf);
            }
        }
    }

    // 调试函数：打印已注册的所有类型
    static void PrintRegisteredTypes() {
        std::cout << "=== 已注册类型列表 ===" << std::endl;
        for (const auto& entry : Registry()) {
            std::string typeName = "未知类型";
            auto nameIt = TypeNames().find(entry.first);
            if (nameIt != TypeNames().end()) {
                typeName = nameIt->second;
            }

            std::cout << "类型: " << typeName << " (" << entry.first.name() << ")" << std::endl;
            std::cout << "  注册字段数: " << entry.second.fields.size() << std::endl;
            int fieldIndex = 0;
            for (const auto& field : entry.second.fields) {
                std::cout << "  字段 #" << fieldIndex << ": " << field.fieldName << std::endl;
                fieldIndex++;
            }
        }
        std::cout << "====================" << std::endl;
    }
};

// 类型特征 - 用于自动推断字段序列化方式
template <typename T, typename MemberPtr>
struct FieldTypeTraits;

template <typename T, size_t N>
struct FieldTypeTraits<T, char (T::*)[N]> {
    static void TLVSerialize(uint32_t tlvType, const T& obj, char (T::*member)[N], std::shared_ptr<TLVWriter>& buf) {
        LOG_DEBUG("字符数组序列化 - TLV类型: " << tlvType);
        const char (&arr)[N] = obj.*member;
        LOG_DEBUG("  数组值: \"" << arr << "\", 有效长度: " << strlen(arr));
        buf->Append(tlvType, arr, strlen(arr) + 1);
    }

    // 属性序列化
    static void TLVSerializeAttr(uint32_t tlvType, const char* keyName, const T& obj, char (T::*member)[N],
                                 std::shared_ptr<TLVWriter>& buf) {
        LOG_DEBUG("字符数组属性序列化 - TLV类型: " << tlvType << ", 键名: " << (keyName ? keyName : "NULL"));
        if (keyName == nullptr) {
            return;
        }

        const char (&arr)[N] = obj.*member;
        LOG_DEBUG("  数组值: \"" << arr << "\", 有效长度: " << strlen(arr));
        buf->Append(tlvType, keyName, strlen(keyName) + 1, arr, strlen(arr) + 1);
    }
};

template <typename T, typename MemberType>
struct FieldTypeTraits<T, MemberType T::*> {
    using type = MemberType;

    static void TLVSerialize(uint32_t tlvType, const T& obj, MemberType T::* member, std::shared_ptr<TLVWriter>& buf) {
        LOG_DEBUG("序列化成员 - TLV类型: " << tlvType << ", 复合类型: " << GetTypeName<T>()
                                           << ", 成员类型: " << GetTypeName<MemberType>());
        const MemberType& value = obj.*member;
        if constexpr (std::is_arithmetic_v<MemberType>) {
            LOG_DEBUG("  算术类型序列化，值: " << value);
            buf->Append(tlvType, &value, sizeof(value));
        } else {
            LOG_DEBUG("  复杂类型序列化，启动递归处理");
            SerializeStruct(tlvType, value, buf);
        }
    }

    // 以整体二进制序列化复合类型
    static void TLVSerializeStructAsBinary(uint32_t tlvType, const T& obj, MemberType T::* member,
                                           std::shared_ptr<TLVWriter>& buf) {
        const MemberType& value = obj.*member;
        LOG_DEBUG("  复合类型整体列化：" << GetTypeName<MemberType>());
        buf->Append(tlvType, &value, sizeof(MemberType));
    }

    static void TLVSerializeDigital(uint32_t tlvType, const T& obj, MemberType T::* member,
                                    std::shared_ptr<TLVWriter>& buf) {
        LOG_DEBUG("数字转字符串序列化 - TLV类型: " << tlvType);
        const MemberType& value = obj.*member;
        std::string valueStr = std::to_string(value);
        LOG_DEBUG("  值: " << value << ", 字符串表示: \"" << valueStr << "\"");
        buf->Append(tlvType, valueStr.c_str(), valueStr.length() + 1);
    }

    static void TLVSerializeAttr(uint32_t tlvType, const char* keyName, const T& obj, MemberType T::* member,
                                 std::shared_ptr<TLVWriter>& buf) {
        LOG_DEBUG("属性序列化 - TLV类型: " << tlvType << ", 键名: " << (keyName ? keyName : "NULL"));
        LOG_DEBUG("复合类型: " << GetTypeName<T>() << ", 成员类型: " << GetTypeName<MemberType>());
        if (keyName == nullptr) {
            // last_word
            return;
        }

        const MemberType& value = obj.*member;
        if constexpr (std::is_arithmetic_v<MemberType>) {
            LOG_DEBUG("  算术类型属性序列化，值: " << value);
            buf->Append(tlvType, keyName, strlen(keyName) + 1, &value, sizeof(value));
        } else {
            LOG_DEBUG("  复合类型属性序列化，启动递归处理");
            SerializeStructAttr(tlvType, keyName, value, buf);
        }
    }

    static void TLVSerializeDigitalAttr(uint32_t tlvType, const char* keyName, const T& obj, MemberType T::* member,
                                        std::shared_ptr<TLVWriter>& buf) {
        LOG_DEBUG("数字转字符串属性序列化 - TLV类型: " << tlvType << ", 键名: " << (keyName ? keyName : "NULL"));
        if (keyName == nullptr) {
            // last_word
            return;
        }

        const MemberType& value = obj.*member;
        std::string valueStr = std::to_string(value);
        LOG_DEBUG("  值: " << value << ", 字符串表示: \"" << valueStr << "\"");
        buf->Append(tlvType, keyName, strlen(keyName) + 1, valueStr.c_str(), valueStr.length() + 1);
    }

  private:
    // 递归序列化结构体
    template <typename StructType>
    static void SerializeStruct(uint32_t tlvType, const StructType& value, std::shared_ptr<TLVWriter>& buf) {
        LOG_DEBUG("开始递归序列化结构体 - 类型: " << GetTypeName<StructType>());

        // 创建临时缓冲区存储嵌套结构
        auto tempBuf = std::make_shared<TLVWriter>(TLV_BUF_LEN);

        // 序列化嵌套结构体的所有字段
        MetaReflection::SerializeFields(value, tempBuf);

        // 将嵌套结构体作为二进制数据附加到父TLV
        size_t len = tempBuf->CurrentIndex();
        if (len > 0) {
            buf->Append(tlvType, tempBuf->data(), len);
        }
    }
};

// 有条件的数组序列化（根据计数字段），添加了复合类型整体二进制序列化的支持
template <typename T, typename ElemType, size_t N, typename CountType, bool BinarySerializeComplex = false>
void SerializeArrayWithCount(uint32_t tlvType, const T& obj, ElemType (T::*arrayMember)[N], CountType T::* countMember,
                             std::shared_ptr<TLVWriter>& buf) {
    LOG_DEBUG("带计数的数组序列化 - TLV类型: " << tlvType << ", 元素类型: " << GetTypeName<ElemType>());

    const ElemType(&arr)[N] = obj.*arrayMember;
    const CountType& count = obj.*countMember;

    // 确保不超出数组边界
    size_t actualCount = (count < N) ? count : N;
    LOG_DEBUG("有效元素数量: " << actualCount << " (声明值: " << count << ", 数组大小: " << N << ")");

    // 序列化有效的数组元素
    for (size_t i = 0; i < actualCount; i++) {
        LOG_DEBUG("序列化数组元素 #" << i);
        if constexpr (std::is_arithmetic_v<ElemType> || BinarySerializeComplex) {
            // 基本类型或强制二进制序列化复合类型
            LOG_DEBUG("  " << (std::is_arithmetic_v<ElemType> ? "基本类型" : "复合类型(二进制)") << "元素");
            buf->Append(tlvType, &arr[i], sizeof(ElemType));
        } else {
            LOG_DEBUG("  复杂类型元素，开始递归序列化");
            auto tempBuf = std::make_shared<TLVWriter>(TLV_BUF_LEN);
            MetaReflection::SerializeFields(arr[i], tempBuf);
            size_t len = tempBuf->CurrentIndex();
            if (len > 0) {
                buf->Append(tlvType, tempBuf->data(), len);
            }
        }
    }
}

// 更多类型特化可以添加在这里...

// 基本字段注册宏
#define TLV_REGISTER_FIELD(TLVType, ObjType, Name)                                                                     \
    namespace {                                                                                                        \
    struct FieldRegistrar_##ObjType##_##Name {                                                                         \
        FieldRegistrar_##ObjType##_##Name() {                                                                          \
            MetaReflection::RegisterField<ObjType>(                                                                    \
                [](const ObjType& obj, std::shared_ptr<TLVWriter>& buf) {                                              \
                    FieldTypeTraits<ObjType, decltype(&ObjType::Name)>::TLVSerialize(TLVType, obj, &ObjType::Name,     \
                                                                                     buf);                             \
                },                                                                                                     \
                #ObjType "." #Name);                                                                                   \
        }                                                                                                              \
    };                                                                                                                 \
    static FieldRegistrar_##ObjType##_##Name fieldRegistrar_##ObjType##_##Name;                                        \
    }

// 作为属性的字段注册宏
#define TLV_REGISTER_ATTR_FIELD(TLVType, KeyName, ObjType, Name)                                                       \
    namespace {                                                                                                        \
    struct FieldRegistrar_##ObjType##_##Name {                                                                         \
        FieldRegistrar_##ObjType##_##Name() {                                                                          \
            MetaReflection::RegisterField<ObjType>(                                                                    \
                [](const ObjType& obj, std::shared_ptr<TLVWriter>& buf) {                                              \
                    FieldTypeTraits<ObjType, decltype(&ObjType::Name)>::TLVSerializeAttr(TLVType, KeyName, obj,        \
                                                                                         &ObjType::Name, buf);         \
                },                                                                                                     \
                #ObjType "." #Name);                                                                                   \
        }                                                                                                              \
    };                                                                                                                 \
    static FieldRegistrar_##ObjType##_##Name fieldRegistrar_##ObjType##_##Name;                                        \
    }

// 数字整型转字符串字段
#define TLV_REGISTER_DIGITAL_FIELD(TLVType, ObjType, Name)                                                             \
    namespace {                                                                                                        \
    struct FieldRegistrar_##ObjType##_##Name {                                                                         \
        FieldRegistrar_##ObjType##_##Name() {                                                                          \
            MetaReflection::RegisterField<ObjType>(                                                                    \
                [](const ObjType& obj, std::shared_ptr<TLVWriter>& buf) {                                              \
                    FieldTypeTraits<ObjType, decltype(&ObjType::Name)>::TLVSerializeDigital(TLVType, obj,              \
                                                                                            &ObjType::Name, buf);      \
                },                                                                                                     \
                #ObjType "." #Name);                                                                                   \
        }                                                                                                              \
    };                                                                                                                 \
    static FieldRegistrar_##ObjType##_##Name fieldRegistrar_##ObjType##_##Name;                                        \
    }

// 作为属性的数字整型字段
#define TLV_REGISTER_DIGITAL_ATTR_FIELD(TLVType, KeyName, ObjType, Name)                                               \
    namespace {                                                                                                        \
    struct FieldRegistrar_##ObjType##_##Name {                                                                         \
        FieldRegistrar_##ObjType##_##Name() {                                                                          \
            MetaReflection::RegisterField<ObjType>(                                                                    \
                [](const ObjType& obj, std::shared_ptr<TLVWriter>& buf) {                                              \
                    FieldTypeTraits<ObjType, decltype(&ObjType::Name)>::TLVSerializeDigitalAttr(TLVType, KeyName, obj, \
                                                                                                &ObjType::Name, buf);  \
                },                                                                                                     \
                #ObjType "." #Name);                                                                                   \
        }                                                                                                              \
    };                                                                                                                 \
    static FieldRegistrar_##ObjType##_##Name fieldRegistrar_##ObjType##_##Name;                                        \
    }

// 注册整体二进制序列化的复合类型字段
#define TLV_REGISTER_STRUCT_AS_BINARY_FIELD(TLVType, ObjType, Name)                                                    \
    namespace {                                                                                                        \
    struct FieldRegistrar_##ObjType##_##Name {                                                                         \
        FieldRegistrar_##ObjType##_##Name() {                                                                          \
            MetaReflection::RegisterField<ObjType>([](const ObjType& obj, std::shared_ptr<TLVWriter>& buf) {           \
                FieldTypeTraits<ObjType, decltype(&ObjType::Name)>::TLVSerializeStructAsBinary(TLVType, obj,           \
                                                                                               &ObjType::Name, buf);   \
            });                                                                                                        \
        }                                                                                                              \
    };                                                                                                                 \
    static FieldRegistrar_##ObjType##_##Name fieldRegistrar_##ObjType##_##Name;                                        \
    }

// 有条件的数组注册宏（根据计数字段）
#define TLV_REGISTER_COUNTED_ARRAY_FIELD(TLVType, ObjType, ArrayName, CountName)                                       \
    namespace {                                                                                                        \
    struct FieldRegistrar_##ObjType##_##ArrayName##_##CountName {                                                      \
        FieldRegistrar_##ObjType##_##ArrayName##_##CountName() {                                                       \
            MetaReflection::RegisterField<ObjType>(                                                                    \
                [](const ObjType& obj, std::shared_ptr<TLVWriter>& buf) {                                              \
                    SerializeArrayWithCount(TLVType, obj, &ObjType::ArrayName, &ObjType::CountName, buf);              \
                },                                                                                                     \
                #ObjType "." #ArrayName);                                                                              \
        }                                                                                                              \
    };                                                                                                                 \
    static FieldRegistrar_##ObjType##_##ArrayName##_##CountName fieldRegistrar_##ObjType##_##ArrayName##_##CountName;  \
    }

// 将复合类型数组元素作为整体二进制序列化
#define TLV_REGISTER_COUNTED_ARRAY_AS_BINARY_FIELD(TLVType, ObjType, ArrayName, CountName)                             \
    namespace {                                                                                                        \
    struct FieldRegistrar_Binary_##ObjType##_##ArrayName##_##CountName {                                               \
        FieldRegistrar_Binary_##ObjType##_##ArrayName##_##CountName() {                                                \
            MetaReflection::RegisterField<ObjType>(                                                                    \
                [](const ObjType& obj, std::shared_ptr<TLVWriter>& buf) {                                              \
                    SerializeArrayWithCount<ObjType, std::remove_extent_t<decltype(ObjType::ArrayName)>,               \
                                            std::extent_v<decltype(ObjType::ArrayName)>, decltype(ObjType::CountName), \
                                            true>(TLVType, obj, &ObjType::ArrayName, &ObjType::CountName, buf);        \
                },                                                                                                     \
                #ObjType "." #ArrayName " (binary)");                                                                  \
        }                                                                                                              \
    };                                                                                                                 \
    static FieldRegistrar_Binary_##ObjType##_##ArrayName##_##CountName                                                 \
        fieldRegistrar_Binary_##ObjType##_##ArrayName##_##CountName;                                                   \
    }

class MetaTransExecutor {
  public:
    MetaTransExecutor() = default;
    virtual ~MetaTransExecutor() = default;

    virtual uint32_t MetaTrans(const std::shared_ptr<void>& businessData, uint32_t contentFilter, TLVWriter& buf) = 0;
};

// 注意：以下代码依赖于 metadata_type_define.h 中定义的类型
// 需要在使用前包含相应的头文件

template <typename T>
struct DecodeCacheTraits { // 这是主模板的定义
    static void Decode(const srvfs_cache& cache, T& data) { memcpy(&data, cache.addr_virt, sizeof(T)); }
};

// 然后可以对特定类型进行特化
// template<>
// struct DecodeCacheTraits<HumanCountAlarm> {
//     static void Decode(const srvfs_cache& cache, HumanCountAlarm& data) {
//         // HumanCountAlarm 的特殊实现
//     }
// };

// 通用的MetaTrans执行器模板
template <typename T>
class GenericCacheToSingleTlv : public MetaTransExecutor {
  public:
    GenericCacheToSingleTlv() = default;
    virtual ~GenericCacheToSingleTlv() = default;

    uint32_t MetaTrans(const std::shared_ptr<void>& businessData, uint32_t contentFilter, TLVWriter& buf) override {
        std::shared_ptr<MetaCacheData> metaCacheData = std::static_pointer_cast<MetaCacheData>(businessData);

        T metaData = {};
        struct srvfs_cache& shm_cache = metaCacheData->cache;

        DecodeCacheData(shm_cache, metaData); // 调用特化或通用实现
        PublishAlarm(metaData, buf);

        buf.contentInfo.type = metaCacheData->contentInfo.type;
        buf.contentInfo.dataType = MetaStructType::LAYER_SINGLE_TLV;
        buf.contentInfo.contentType = MetaContentFilterType::CONTENT_STATISTICS;

        return 0;
    }

  protected:
    uint32_t PublishAlarm(const T& metaData, TransBuf& buf) {
        auto tlvBuf = std::make_shared<TLVWriter>(TLV_BUF_LEN);

        // 添加固定字段
        tlvBuf->Append(Oie::METADATA_TYPE, (const char*)0, 0);

        // 通过反射添加所有基本字段
        MetaReflection::SerializeFields(metaData, tlvBuf);

        // 复制数据到输出缓冲区
        size_t len = tlvBuf->CurrentIndex();
        buf.data = (char*)calloc(1, len);
        memcpy(buf.data, tlvBuf->data(), len);
        buf.len = len;

        return 0;
    }

    // 将 DecodeCacheData 移到私有方法
    void DecodeCacheData(const srvfs_cache& cache, T& data) { DecodeCacheTraits<T>::Decode(cache, data); }
};