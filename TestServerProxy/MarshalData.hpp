#pragma once

struct MarshalData {
    static const size_t MARSHAL_SIZE = sizeof(size_t) + sizeof(size_t); // offset + size

    HRESULT Serialize(IStream* strm) {
        // serialize metadata
        RETURN_IF_FAILED(*strm << offset);
        RETURN_IF_FAILED(*strm << size);
        return S_OK;
    }

    HRESULT DeSerialize(IStream* strm) {
        // deserialize metadata
        RETURN_IF_FAILED(*strm >> offset);
        RETURN_IF_FAILED(*strm >> size);
        return S_OK;
    }

public:
    size_t offset = 0;
    size_t size = 0;
};
