
#include <xe/DataType.h>

namespace xe {
    int bytesize(DataType dataType) {
        switch (dataType) {
        case DataType::UInt8:
        case DataType::Int8:
            return 1;

        case DataType::UInt16:
        case DataType::Int16:
        case DataType::Float16:
            return 2;

        case DataType::UInt32:
        case DataType::Int32:
        case DataType::Float32:
            return 4;

        case DataType::Float64:
        case DataType::Int64:
        case DataType::UInt64:
            return 8;

        default:
            return 0;
        }
    }
} // namespace xe
