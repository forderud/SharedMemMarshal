#pragma once

struct MarshalData {
    static constexpr unsigned int MarshalSize() {
        return sizeof(offset) + sizeof(size);
    }

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
