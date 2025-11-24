#pragma once


/** Serialize/deserialize an arbitrary POD type. */
template <class T> HRESULT operator<< (IStream& stream, const T& data) {
    unsigned long bytes_written = 0;
    RETURN_IF_FAILED(stream.Write(&data, sizeof(data), &bytes_written));
    if (bytes_written != sizeof(data))
        return E_FAIL;
    return S_OK;
}
template <class T> HRESULT operator>> (IStream& stream, T& data) {
    unsigned long bytes_read = 0;
    RETURN_IF_FAILED(stream.Read(&data, sizeof(data), &bytes_read));
    if (bytes_read != sizeof(data))
        return E_FAIL;
    return S_OK;
}


/** Image2d serialization wrapper. */
struct MarshalImage : Image2d {
    static constexpr unsigned int MarshalSize() {
        return sizeof(time) + sizeof(format) + sizeof(dims) + sizeof(m_img_offset);
    }

    /** Create Image2d with new buffer. */
    MarshalImage(double time, unsigned int format, USHORT dims[2]) : Image2d(time, format, dims, /*allocate*/false) {
        CHECK(SafeArrayAllocDescriptorEx(VT_UI1, 1, &data));
        data->cbElements = 1;
        data->fFeatures |= FADF_AUTO;  // prevent data and descriptor from being automatically deleted
        data->pvData = nullptr;        // assign later
        data->rgsabound[0] = { 0, 0 }; // assign later

        auto new_size = size();
        if (new_size) {
            // allocate image pointer in shared mem.
            data->pvData = SharedMem::Allocate(new_size);
            data->rgsabound[0] = { new_size, 0 };
            m_img_offset = SharedMem::GetOffset(data->pvData);
            m_owns_data = true;
        }
    }

    /** Create shallow Image2d based on existing buffer. */
    MarshalImage(double time, unsigned int format, USHORT dims[2], size_t img_offset) : Image2d(time, format, dims, /*allocate*/false) {
        CHECK(SafeArrayAllocDescriptorEx(VT_UI1, 1, &data));
        data->cbElements = 1;
        data->fFeatures |= FADF_AUTO;  // prevent data and descriptor from being automatically deleted
        data->pvData = nullptr;        // assign later
        data->rgsabound[0] = { 0, 0 }; // assign later

        auto new_size = size();
        // use already allocated image data from shared memory
        data->pvData = SharedMem::GetPointer(img_offset);
        data->rgsabound[0] = { new_size, 0 };
        m_img_offset = img_offset;
        m_owns_data = false;
    }

    ~MarshalImage() {
        if (m_owns_data)
            SharedMem::Free(data->pvData);
        data->pvData = nullptr;
        m_owns_data = false;

        CHECK(SafeArrayDestroyDescriptor(data)); // only delete descriptor, and not data
        data = nullptr;
    }

    HRESULT Serialize(IStream* strm) const {
        // serialize metadata
        RETURN_IF_FAILED(*strm << time);
        RETURN_IF_FAILED(*strm << format);
        RETURN_IF_FAILED(*strm << dims);
        RETURN_IF_FAILED(*strm << m_img_offset);
        // Don't serialize the image-data, since it might be large.
        // Instead, configure zero-copy exchange through a shared-memory segment.
        return S_OK;
    }

    static std::unique_ptr<MarshalImage> DeSerialize(IStream* strm) {
        // deserialize metadata
        double time = 0;
        CHECK(*strm >> time);
        unsigned int format = 0;
        CHECK(*strm >> format);
        unsigned short dims[2] = {};
        CHECK(*strm >> dims);
        size_t img_offset = 0;
        CHECK(*strm >> img_offset);

        // use already allocated image data from shared memory
        return std::make_unique<MarshalImage>(time, format, dims, img_offset);
    }

    size_t m_img_offset = 0; // shared-mem segment offset
    bool   m_owns_data = false;
};
