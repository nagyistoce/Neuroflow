﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using Neuroflow.Core.ComponentModel;

namespace Neuroflow.Networks.Neural.Managed
{
    public sealed class ManagedNNInitParameters : NNInitParameters<ManagedNeuralNetwork>
    {
        [Category(PropertyCategories.Behavior)]
        [DisplayName("Run Parallel")]
        [InitValue(false)]
        [DefaultValue(false)]
        public bool RunParallel { get; set; }
    }
}
