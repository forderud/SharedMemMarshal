#pragma once

struct MarshalImage : Image2d {
    static constexpr unsigned int MarshalSize() {
        return sizeof(time) + sizeof(pix_size) + sizeof(dims) + sizeof(img_offset) + sizeof(img_size);
    }

    MarshalImage(double time, unsigned char ps, USHORT dims[2]) : Image2d(time, pix_size, dims, /*allocate*/false) {
    }

    ~MarshalImage() {
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

    static std::unique_ptr<MarshalImage> DeSerialize(IStream* strm) {
        // deserialize metadata
        double time = 0;
        if (FAILED(*strm >> time))
            abort();
        unsigned char pix_size = 0;
        if (FAILED(*strm >> pix_size))
            abort();
        unsigned short dims[2] = {};
        if (FAILED(*strm >> dims))
            abort();
        size_t img_offset = 0;
        if (FAILED(*strm >> img_offset))
            abort();
        size_t img_size = 0;
        if (FAILED(*strm >> img_size))
            abort();
        return std::make_unique<MarshalImage>(time, pix_size, dims);
    }

    size_t img_offset = 0; // shared-mem segment offset
    size_t img_size = 0;
};
