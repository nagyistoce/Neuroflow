#pragma once

#include "ocl_nf.h"
#include "device_array_management.h"
#include "ocl_contexted.h"

namespace nf
{
    struct ocl_device_array_management : ocl_contexted, _implements device_array_management
    {
        friend struct ocl_device_array_pool;

        ocl_device_array_management(const ocl_internal_context_ptr& context, const ocl_utils_ptr& utils);

        device_array_ptr create_array(bool copyOptimized, idx_t size) override;
        device_array2_ptr create_array2(bool copyOptimized, idx_t rowSize, idx_t colSize) override;
        void copy(device_array_ptr from, idx_t fromIndex, device_array_ptr to, idx_t toIndex, idx_t size) override;
        device_array_pool_ptr create_pool() override;

    private:
        ocl_utils_ptr utils;

        cl::Buffer create_buffer(bool copyOptimized, idx_t sizeInBytes);
    };
}