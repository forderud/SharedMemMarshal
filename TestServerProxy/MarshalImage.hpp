#pragma once

struct MarshalImage : Image2d {
    static constexpr unsigned int MarshalSize() {
        return sizeof(time) + sizeof(pix_size) + sizeof(dims) + sizeof(img_offset) + sizeof(img_size);
    }

    HRESULT Serialize(IStream* strm) {
        // serialize metadata
        RETURN_IF_FAILED(*strm << time);
        RETURN_IF_FAILED(*strm << pix_size);
        RETURN_IF_FAILED(*strm << dims);
        RETURN_IF_FAILED(*strm << img_offset);
        RETURN_IF_FAILED(*strm << img_size);
        return S_OK;
    }

    HRESULT DeSerialize(IStream* strm) {
        // deserialize metadata
        RETURN_IF_FAILED(*strm >> time);
        RETURN_IF_FAILED(*strm >> pix_size);
        RETURN_IF_FAILED(*strm >> dims);
        RETURN_IF_FAILED(*strm >> img_offset);
        RETURN_IF_FAILED(*strm >> img_size);
        return S_OK;
    }

    size_t img_offset = 0; // shared-mem segment offset
    size_t img_size = 0;
};
