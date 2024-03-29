﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Neuroflow.Core.Vectors;
using System.ComponentModel;
using Neuroflow.Core.ComponentModel;

namespace Neuroflow.Networks.Neural.Learning
{
    public enum LearningMode : byte { Stochastic, Batch }
    
    public abstract class BackwardLearningRule : LearningRule
    {
        protected internal abstract LearningMode GetMode();

        [Category(PropertyCategories.Math)]
        public WeightDecay WeightDecay { get; set; }

        [Category(PropertyCategories.Algorithm)]
        public IterationRepeatPars IterationRepeat { get; set; }
    }
}
