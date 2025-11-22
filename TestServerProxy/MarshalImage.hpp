#pragma once

struct MarshalImage : Image2d {
    static constexpr unsigned int MarshalSize() {
        return sizeof(time) + sizeof(pix_size) + sizeof(dims) + sizeof(m_img_offset);
    }

    MarshalImage(double time, unsigned char ps, USHORT dims[2], bool do_allocate) : Image2d(time, pix_size, dims, /*allocate*/false) {
        CHECK(SafeArrayAllocDescriptorEx(VT_UI1, 1, &data));
        data->cbElements = 1;
        data->fFeatures |= FADF_AUTO;  // prevent data from being deleted
        data->pvData = nullptr;        // assign later
        data->rgsabound[0] = { 0, 0 }; // assign later

        auto new_size = size();
        if (do_allocate && new_size) {
            // allocate image pointer in shared mem.
            assert(data->pvData == nullptr);
            data->pvData = static_cast<unsigned char*>(SharedMem::Allocate(new_size));
            data->rgsabound[0] = { new_size, 0 };
            m_img_offset = SharedMem::GetOffset((BYTE*)data->pvData);
            m_owns_data = true;
        }
    }

    ~MarshalImage() {
        if (m_owns_data)
            SharedMem::Free((BYTE*)data->pvData);
        data->pvData = nullptr;
        m_owns_data = false;

        CHECK(SafeArrayDestroyDescriptor(data)); // only delete descriptor, and not data
        data = nullptr;
    }

    HRESULT Serialize(IStream* strm) {
        // serialize metadata
        RETURN_IF_FAILED(*strm << time);
        RETURN_IF_FAILED(*strm << pix_size);
        RETURN_IF_FAILED(*strm << dims);
        RETURN_IF_FAILED(*strm << m_img_offset);
        return S_OK;
    }

    static std::unique_ptr<MarshalImage> DeSerialize(IStream* strm) {
        // deserialize metadata
        double time = 0;
        CHECK(*strm >> time);
        unsigned char pix_size = 0;
        CHECK(*strm >> pix_size);
        unsigned short dims[2] = {};
        CHECK(*strm >> dims);
        size_t img_offset = 0;
        CHECK(*strm >> img_offset);
        auto frame = std::make_unique<MarshalImage>(time, pix_size, dims, false);

        // access image data from shared memory
        frame->m_img_offset = img_offset;
        frame->data->pvData = SharedMem::GetPointer(img_offset);
        frame->data->rgsabound[0] = { frame->size(), 0 };

        return frame;
    }

    size_t m_img_offset = 0; // shared-mem segment offset
    bool   m_owns_data = false;
};
