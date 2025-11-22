#pragma once

struct MarshalImage {
    static constexpr unsigned int MarshalSize() {
        return sizeof(value);
    }

    HRESULT Serialize(IStream* strm) {
        // serialize metadata
        RETURN_IF_FAILED(*strm << value);
        return S_OK;
    }

    HRESULT DeSerialize(IStream* strm) {
        // deserialize metadata
        RETURN_IF_FAILED(*strm >> value);
        return S_OK;
    }

    size_t value = 0;
};
