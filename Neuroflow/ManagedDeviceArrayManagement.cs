﻿using Neuroflow.Data;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Neuroflow
{
    internal sealed class ManagedDeviceArrayManagement : IDeviceArrayManagement
    {
        public IDeviceArray CreateArray(bool copyOptimized, int size)
        {
            return new ManagedArray(size);
        }

        public IDeviceArray2 CreateArray2(bool copyOptimized, int rowSize, int colSize)
        {
            return new ManagedArray2(rowSize, colSize);
        }

        public unsafe void Copy(IDeviceArray from, int fromIndex, IDeviceArray to, int toIndex, int size)
        {
            Array.Copy(from.ToManaged().InternalArray, fromIndex, to.ToManaged().InternalArray, toIndex, size);
        }
    }
}
