#pragma once

struct MarshalImage : Image2d {
    static constexpr unsigned int MarshalSize() {
        return sizeof(time) + sizeof(pix_size) + sizeof(dims) + sizeof(offset);
    }

    HRESULT Serialize(IStream* strm) {
        // serialize metadata
        RETURN_IF_FAILED(*strm << time);
        RETURN_IF_FAILED(*strm << pix_size);
        RETURN_IF_FAILED(*strm << dims);
        RETURN_IF_FAILED(*strm << offset);
        return S_OK;
    }

    HRESULT DeSerialize(IStream* strm) {
        // deserialize metadata
        RETURN_IF_FAILED(*strm >> time);
        RETURN_IF_FAILED(*strm >> pix_size);
        RETURN_IF_FAILED(*strm >> dims);
        RETURN_IF_FAILED(*strm >> offset);
        return S_OK;
    }

    size_t offset = 0; // shared-mem segment offset
};
