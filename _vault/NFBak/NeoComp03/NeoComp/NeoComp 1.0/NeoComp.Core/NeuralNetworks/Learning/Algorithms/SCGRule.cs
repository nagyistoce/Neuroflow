﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NeoComp.NeuralNetworks.Learning.Algorithms
{
    public sealed class SCGRule : LearningRule, IWeightDecayedLearningRule
    {
        public WeightDecay WeightDecay { get; set; }
        
        protected override Type AlgorithmType
        {
            get { return typeof(SCGAlgorithm); }
        }
    }
}
