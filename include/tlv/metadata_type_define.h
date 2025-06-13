#ifndef __SERIALIZATION_DATA_TYPE_H__
#define __SERIALIZATION_DATA_TYPE_H__

#include <cstdint>
#include <sys/types.h>

#define SYSTEM_UUID_LENGTH 128
#define NAME_BUFFER_MAX_SIZE 16
#define DESCRIPTION_MAX_LENGTH 48
#define ALGORITHM_TYPE_MAX_SIZE 32
#define SYSTEM_DEVICE_ID_SIZE 128
#define COORDINATE_POINT_LIMIT 10
#define TRACKED_PERSON_LIMIT 10
#define DEVICE_ID_MAX_LENGTH 128

// 追踪目标分类
typedef enum {
    TRACKING_FACE_BODY_RECTANGLE = 0x00, // 人脸人体检测框
    TRACKING_MAXIMUM,
} TrackingType;

enum class DataContentFilterMode {
    FILTER_ALL_CONTENT = 0,
    FILTER_DETECTION_BOX = 0x01,   // 检测框
    FILTER_OVERVIEW_IMAGE = 0x02,  // 全景图
    FILTER_DETAILED_IMAGE = 0x04,  // 特写图
    FILTER_TARGET_FEATURES = 0x08, // 目标特征值
    FILTER_BOUNDARY_BOX = 0x10,    // 规则框
    FILTER_ANALYSIS_DATA = 0x20,   // 统计信息
    FILTER_COMPLETE_TYPES = 0xFF,  // 全部类型
    FILTER_EXCLUDE_IMAGES =
        FILTER_COMPLETE_TYPES & (~(uint32_t)FILTER_OVERVIEW_IMAGE) & (~(uint32_t)FILTER_DETAILED_IMAGE),
};

enum class DataStructureFormat : uint16_t {
    RAW_DATA_FORMAT = 0, // 智能原始裸数据类型
    CACHED_DATA_FORMAT,  // 智能序列化数据，用于进程间通信
    REST_API_V1,
    REST_API_V2,
    REST_API_HSAPI,
    SINGLE_LAYER_TLV,
    THREE_LAYER_TLV,
    ONVIF_FORMAT,
    GAT_1400_FORMAT,
    YIHUALU_FORMAT,
    GAT_1400_SHANGHAI_FORMAT,
    GAT_497_FORMAT,
    EHUALU_PROCESS_FORMAT,
    GB21255_FORMAT,
    HOLOSENS_REST_FORMAT,
    DATA_FORMAT_LIMIT
};

struct SystemCacheBuffer {
    void* virtualAddress;
    unsigned long physicalAddress;
    unsigned int bufferSize;
    unsigned int cookieValue;
};

typedef struct {
    uint64_t timestamp;
    TrackingType targetType;
    DataStructureFormat formatType;
    DataContentFilterMode filterMode;
} DataContentInfo;

typedef struct {
    DataContentInfo contentInfo;
    struct SystemCacheBuffer cacheBuffer;
} CachedMetaData;

typedef struct {
    char* dataBuffer;
    size_t bufferLength;
    DataContentInfo contentInfo;
} TransferBuffer;

namespace SerializationCore {
typedef enum {
    TIME_STAMP = 0x09000001,
    RULE_MASK = 0x09000007,
    PANORAMA_IMAGE = 0x0A00000A,
    OBJECT_ID = 0x07000021,
    PANORAMA_IMAGE_SIZE = 0x07000073,
    SMART_GLOBAL_OBJECT_ID = 0x09000082,
    OBJECT_RELATIVE_POSITION = 0x0B000035,
    SNAPSHOT_TIME_MILLISECONDS = 0x09000003,
    SNAPSHOT_TIME = 0x07000068,
    DEVICE_TIME_ZONE = 0x08000069,
    CAMERA_CHANNEL_NUMBER = 0x09000078,
    DAYLIGHT_SAVING_OFFSET = 0x08000085,
    CLOSE_UP_IMAGE = 0x0A00006B,
    ALGORITHM_CLASSIFICATION = 0x20000001,
    METADATA_NAME = 0x20000002,
    DATA_PRODUCER_NAME = 0x20000003,
    DESCRIPTION_INFORMATION = 0x20000004,
    CAMERA_UNIQUE_ID = 0x20000061,
    OBJECT_EXTENDED_ATTRIBUTES = 0x21000001,
    IMAGE_EXTENDED_ATTRIBUTES = 0x21000002,
    COMMON_ALARM_TYPE = 0x07000041,
    ALARM_NAME = 0x20000005,
    ALARM_SOURCE = 0x20000006,
    INPUT_ALARM_NAME = 0x20000007,
    METADATA_TYPE = 0x4154454D,
    OBJECT_UPPER_HALF_COLOR = 0x0F000026,
    OBJECT_LOWER_HALF_COLOR = 0x0F000027,
    RULE_TYPE = 0x07000031,
    RULE_LINE_DIRECTION = 0x07000033,
    OBJECT_STATUS = 0x06000022,
    OBJECT_POSITION = 0x0B000023,
    OBJECT_TYPE = 0x06000024,
    OBJECT_SPEED = 0x0C000025,
    RULE_LINE_POSITION = 0x0D000032,
    RULE_AREA_POSITION = 0x0E000034,
    OBJECT_RELATIVE_SPEED = 0x0C000036,
    RULE_LINE_RELATIVE_POSITION = 0x0D000037,
    RULE_AREA_RELATIVE_POSITION = 0x0E000038,
    SMART_BUSINESS_TARGET_TYPE = 0x07000023,
    PROCESSING_IMAGE_WIDTH = 0x07000100,
    PROCESSING_IMAGE_HEIGHT = 0x07000101,
    UTC_TIME_STAMP = 0x09000008,
    PEOPLE_ENTRY_COUNT = 0x07000709,
    PEOPLE_EXIT_COUNT = 0x0700070A,
    LINE_CROSSING_TOTAL_ENTRY = 0x07000900,
    LINE_CROSSING_TOTAL_EXIT = 0x07000901,
    LINE_CROSSING_ENTRY_INCREMENT = 0x07000902,
    LINE_CROSSING_EXIT_INCREMENT = 0x07000903,
    ABSENCE_DETECTION_REQUIRED_COUNT = 0x07000904,
    ABSENCE_DETECTION_CURRENT_COUNT = 0x07000905,
    ABSENCE_DETECTION_TARGET_BOX = 0x0B000066,
    COMMON = 0x00000001,
    TARGET = 0x00000002,
    RULE = 0x00000003,
    EXTERNAL_CAMERA_CHANNEL_ID = 0x09000092,
    INTELLIGENT_TARGET_INDEX = 0x09000095,
    TARGET_TIME_DOMAIN_INFO = 0x0300007D,
    OVERLAY_TARGET_BOX_ENABLE = 0x0100000C,
    HEAD_SHOULDER_POSITION = 0x0B000018,
    HEAD_SHOULDER_NUMBER = 0x06000001,
    QUEUE_TIME = 0x06000002,
    TIMING_START_TIME = 0x09000050,
    TIMING_END_TIME = 0x09000051,
    PEOPLE_NUMBER = 0X07000087,
    AREA_RATIO = 0X07000089,
    HEAD_SHOULDER_POS = 0X0B000088,
    SNAPSHOT_TIME_ZONE = 0x08000069,
} SERIALIZATION_ELEMENT_TYPE_E;

enum DataCategory {
    DATA_RECTANGLE_INFO = 0x00000001,  // 框数据     000... .0001
    DATA_PICTURE_INFO = 0x00000002,    // 图数据     000... .0010
    DATA_STATISTICS_INFO = 0x00000004, // 统计数据   000..  .0100
    DATA_HEARTBEAT_INFO = 0x00000008,  // 保活数据   000... .1000
    DATA_EXTERNAL_ALERT = 0x00000010,  // 第三方告警使用   000... 10000
    DATA_VEHICLE_ATTRS = 0x00000020,   // 全息相机的车辆车牌属性信息 000....100000
    DATA_AI_ALERT = 0x00000040,        // 智能告警   000   100000
    DATA_CATEGORY_LIMIT
};

typedef struct {
    int16_t coordinateX;
    int16_t coordinateY;
} GeometryPoint;

// 线段
typedef struct {
    GeometryPoint startPoint;
    GeometryPoint endPoint;
} GeometryLine;

// 多边形
typedef struct {
    uint32_t pointCount;
    GeometryPoint points[COORDINATE_POINT_LIMIT];
} GeometryPolygon;

typedef struct {
    char systemUuid[SYSTEM_UUID_LENGTH];
    char algorithmType[ALGORITHM_TYPE_MAX_SIZE];
    uint32_t channelIdentifier;
    char metadataName[NAME_BUFFER_MAX_SIZE];
    uint32_t dataTypeMask;
    uint32_t dataFormatType;
    char descriptionInfo[DESCRIPTION_MAX_LENGTH];
    uint32_t totalEnterCount;
    uint32_t totalExitCount;
    uint64_t alertStartTimestamp;
    uint32_t captureDstOffset;
    uint64_t captureTimestamp;
    uint32_t captureTimeZone;
    uint32_t enterIncrementCount;
    uint32_t exitIncrementCount;
    uint32_t alertAreaIdentifier;
    uint32_t alertCategory;
    uint32_t targetIdentifier;
} PersonCountAlert;

typedef struct {
    char systemUuid[SYSTEM_UUID_LENGTH];
    char deviceIdentifier[SYSTEM_DEVICE_ID_SIZE];
} CommonParameters;

typedef struct {
    uint64_t actionTargetId;
    uint32_t actionTargetType;
} AiActionTarget;

typedef struct {
    uint32_t currentEnterCount;
    uint32_t currentExitCount;
    GeometryLine boundaryLine;
    uint32_t actionTargetCount;
    AiActionTarget actionTargetInfo[TRACKED_PERSON_LIMIT];
} PersonCountTargetParams;

typedef struct {
    char systemUuid[SYSTEM_UUID_LENGTH];
    char algorithmType[ALGORITHM_TYPE_MAX_SIZE];
    uint32_t channelIdentifier;
    char metadataName[NAME_BUFFER_MAX_SIZE];
    uint32_t dataTypeMask;
    uint32_t dataFormatType;
    char descriptionInfo[DESCRIPTION_MAX_LENGTH];
    uint32_t alertAreaIdentifier;
    uint32_t alertCategory;
    uint32_t targetIdentifier;
    CommonParameters commonParams;
    uint32_t targetListCount;
    PersonCountTargetParams targetList[TRACKED_PERSON_LIMIT];
} QueueDetectionAlert;

typedef struct {
    CommonParameters commonParams;
    uint32_t targetListCount;
    PersonCountTargetParams targetList[TRACKED_PERSON_LIMIT];
} PersonCountSubAlert;

#endif // __SERIALIZATION_DATA_TYPE_H__