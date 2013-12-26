#pragma once

#include "nf.h"
#include "ocl.h"

namespace nf
{
    struct ocl_device_array;
    typedef std::shared_ptr<ocl_device_array> ocl_device_array_ptr;

    struct ocl_device_array2;
    typedef std::shared_ptr<ocl_device_array2> ocl_device_array2_ptr;

    struct ocl_device_array_pool;
    typedef std::shared_ptr<ocl_device_array_pool> ocl_device_array_pool_ptr;
}