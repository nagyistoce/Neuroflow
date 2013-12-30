#pragma once

#include "nf.h"
#include "device_array.h"

namespace nf
{
    _interface data_array : _implements device_array
    {
        virtual bool is_read_only() const = 0;
        virtual concurrency::task<void> read(idx_t sourceBeginIndex, idx_t count, float* targetPtr, idx_t targetBeginIndex) const = 0;
        virtual concurrency::task<void> write(float* sourceArray, idx_t sourceBeginIndex, idx_t count, idx_t targetBeginIndex) = 0;
    };
}