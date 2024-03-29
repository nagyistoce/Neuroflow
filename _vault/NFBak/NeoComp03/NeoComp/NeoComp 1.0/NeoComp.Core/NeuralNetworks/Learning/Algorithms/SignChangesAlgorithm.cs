﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NeoComp.NeuralNetworks;
using System.Diagnostics;
using System.Diagnostics.Contracts;

namespace NeoComp.NeuralNetworks.Learning.Algorithms
{
    public sealed class SignChangesAlgorithm : LocalAdaptiveGDAlgorithm<SignChangesRule>
    {
        protected override double CalculateCurrentStepSize(IBackwardConnection connection, SignChangesRule rule, LocalAdaptiveDelta delta, double signState, bool useAverageError)
        {
            if (signState >= 0.0)
            {
                return delta.CurrentStepSize * rule.U;
            }
            else
            {
                return delta.CurrentStepSize * rule.D;
            }
        }
    }
}
