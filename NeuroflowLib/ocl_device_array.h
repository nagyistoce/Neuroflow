#pragma once

#include "ocl_nf.h"
#include "device_array.h"

namespace nf
{
    struct ocl_device_array : _implements device_array
    {
        ocl_device_array(const cl::Buffer& buffer);
        ocl_device_array(const ocl_device_array_pool_ptr& pool, idx_t beginOffset, idx_t arraySize);

        idx_t size() const override;
        cl::Buffer& buffer();

    private:
        cl::Buffer _buffer;
        idx_t beginIndex = 0;
        idx_t arraySize = 0;
        ocl_device_array_pool_ptr pool;
    };
}