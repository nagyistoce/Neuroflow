import ncomputeactivation;
import tonative;
import activationdescription;
import nhelpers;

void ncaBackward(MLPBackwardNode[] nodes, in GradientComputationPhase phase, in size_t internalIterationCount)
{
    foreach (ref node; nodes)
    {
        if (node.isLast) computeLastErrors(&node); else computeInnerErrors(&node);
        computeGradients(&node, phase, internalIterationCount);
    }
}

private void computeLastErrors(MLPBackwardNode* node)
{
    assert(!node.netOutputs.isNull);
    auto outputs = toArray(node.netOutputs.get.outputs());
    auto desiredOutputs = toArray(node.netOutputs.get.desiredOutputs());
    auto errors = toArray(node.errors);
    assert(outputs);
    assert(desiredOutputs);
    assert(errors);
    size_t size = errors.length;
    assert(outputs.length == size);
    assert(desiredOutputs.length == size);
    float alpha = node.activation.alpha();
    
    if (node.activation.activationFunction == ActivationFunction.sigmoid)
    {
        for (size_t i = 0; i < size; i++)
        {
            errors[i] = (desiredOutputs[i] - outputs[i]) * sigmoidDeriv(outputs[i], alpha);
        }
    }
    else
    {
        for (size_t i = 0; i < size; i++)
        {
            errors[i] = (desiredOutputs[i] - outputs[i]) * alpha;
        }
    }
}

private void computeInnerErrors(MLPBackwardNode* node)
{
    assert(node.lowerErrors.length > 0);
    auto errors = toArray(node.errors);
    assert(errors);
    size_t size = errors.length;
    float alpha = node.activation.alpha();
    
    for (size_t valueIdx = 0; valueIdx < size; valueIdx++)
    {
        float sum = 0.0f;
        foreach (ref weightedError; node.lowerErrors)
        {
            auto lowerErrors = toArray(weightedError.errors);
            assert(lowerErrors);
            auto lowerWeights = toArray(weightedError.weights);
            assert(lowerWeights);
            size_t lowerErrorsSize = lowerErrors.length;
            
            for (size_t lowerErrorIdx = 0; lowerErrorIdx < lowerErrorsSize; lowerErrorIdx++)
            {
                size_t lwidx = getIndex2(valueIdx, lowerErrorIdx, size);
                assert(lwidx >= 0 && lwidx < lowerWeights.length);
                sum += lowerErrors[lowerErrorIdx] * lowerWeights[lwidx];
            }
        }
        
        if (node.activation.activationFunction == ActivationFunction.sigmoid)
        {
            assert(node.outputs);
            auto outputs = toArray(node.outputs());
            assert(outputs);
            assert(outputs.length == size);
            
            errors[valueIdx] = sum * sigmoidDeriv(outputs[valueIdx], alpha);
        }
        else
        {
            errors[valueIdx] = sum * alpha;
        }
    }
}

private void computeGradients(MLPBackwardNode* node, in GradientComputationPhase phase, in size_t internalIterationCount)
{
    final switch (phase)
    {
        case GradientComputationPhase.ff:
            computeGradientsFF(node);
            break;
        case GradientComputationPhase.bpttPhase1:
            computeGradientsBPTTPhase1(node);
            break;
        case GradientComputationPhase.bpttPhase2:
            computeGradientsBPTTPhase2(node, internalIterationCount);
            break;
    }
}

private void computeGradientsFF(MLPBackwardNode* node)
{
    bool hasGradients = node.hasGradients;
    bool hasGradientSums = node.hasGradientSums;
    assert(hasGradients || hasGradientSums);
    auto errors = toNativeDeviceArray(node.errors);
    assert(errors);
    float[] errorsArr = errors.array;
    size_t size = errors.size;
    
    float[] pBiasGradients = null;
    float[] pBiasGradientSums = null;
    if (hasGradients)
    {
        auto biasGradients = toNativeDeviceArray(node.biasGradients);
        assert(biasGradients);
        pBiasGradients = biasGradients.array;
    }
    if (hasGradientSums)
    {
        auto biasGradientSums = toNativeDeviceArray(node.biasGradientSums);
        assert(biasGradientSums);
        pBiasGradientSums = biasGradientSums.array;
    }
    
    for (size_t valueIdx = 0; valueIdx < size; valueIdx++)
    {
        if (hasGradients) pBiasGradients[valueIdx] = errorsArr[valueIdx];
        if (hasGradientSums) pBiasGradientSums[valueIdx] += errorsArr[valueIdx];
        
        size_t inputLayersCount = node.inputs.length;
        for (size_t ilidx = 0; ilidx < inputLayersCount; ilidx++)
        {
            auto inputs = toNativeDeviceArray(node.inputs[ilidx]());
            assert(inputs);
            float[] inputsArr = inputs.array;
            size_t inputSize = inputs.size;
            if (hasGradients && hasGradientSums)
            {
                auto gradients = toNativeDeviceArray2(node.gradients[ilidx]);
                assert(gradients);
                float[] gradientsArr = gradients.array;
                auto gradientSums = toNativeDeviceArray2(node.gradientSums[ilidx]);
                assert(gradientSums);
                float[] gradientSumsArr = gradientSums.array;
                
                for (size_t inputIndex = 0; inputIndex < inputSize; inputIndex++)
                {
                    size_t gidx = getIndex2(inputIndex, valueIdx, inputSize);
                    assert(gidx >= 0 && gidx < gradients.size && gidx < gradientSums.size);
                    gradientSumsArr[gidx] += (gradientsArr[gidx] = inputsArr[inputIndex] * errorsArr[valueIdx]);
                }
            }
            else if (hasGradients)
            {
                auto gradients = toNativeDeviceArray2(node.gradients[ilidx]);
                assert(gradients);
                float[] gradientsArr = gradients.array;
                
                for (size_t inputIndex = 0; inputIndex < inputSize; inputIndex++)
                {
                    size_t gidx = getIndex2(inputIndex, valueIdx, inputSize);
                    assert(gidx >= 0 && gidx < gradients.size);
                    gradientsArr[gidx] = inputsArr[inputIndex] * errorsArr[valueIdx];
                }
            }
            else
            {
                assert(hasGradientSums);
                auto gradientSums = toNativeDeviceArray2(node.gradientSums[ilidx]);
                assert(gradientSums);
                float[] gradientSumsArr = gradientSums.array;
                
                for (size_t inputIndex = 0; inputIndex < inputSize; inputIndex++)
                {
                    size_t gidx = getIndex2(inputIndex, valueIdx, inputSize);
                    assert(gidx >= 0 && gidx < gradientSums.size);
                    gradientSumsArr[gidx] += (inputsArr[inputIndex] * errorsArr[valueIdx]);
                }
            }
        }
    }
}

private void computeGradientsBPTTPhase1(MLPBackwardNode* node)
{
    auto errors = toNativeDeviceArray(node.errors);
    assert(errors);
    float[] errorsArr = errors.array;
    size_t size = errors.size;
    
    auto biasGradients = toNativeDeviceArray(node.biasGradients);
    assert(biasGradients);
    float[] pBiasGradients = biasGradients.array;
    
    for (size_t valueIdx = 0; valueIdx < size; valueIdx++)
    {
        pBiasGradients[valueIdx] += errorsArr[valueIdx];
        
        size_t inputLayersCount = node.inputs.length;
        for (size_t ilidx = 0; ilidx < inputLayersCount; ilidx++)
        {
            auto inputs = toNativeDeviceArray(node.inputs[ilidx]());
            assert(inputs);
            float[] inputsArr = inputs.array;
            size_t inputSize = inputs.size;
            auto gradients = toNativeDeviceArray2(node.gradients[ilidx]);
            assert(gradients);
            float[] gradientsArr = gradients.array;
            
            for (size_t inputIndex = 0; inputIndex < inputSize; inputIndex++)
            {
                size_t gidx = getIndex2(inputIndex, valueIdx, inputSize);
                gradientsArr[gidx] += inputsArr[inputIndex] * errorsArr[valueIdx];
            }
        }
    }
}

private void computeGradientsBPTTPhase2(MLPBackwardNode* node, in size_t internalIterationCount)
{
    bool hasGradients = node.hasGradients;
    bool hasGradientSums = node.hasGradientSums;
    assert(hasGradients || hasGradientSums);
    auto errors = toNativeDeviceArray(node.errors);
    assert(errors);
    float[] errorsArr = errors.array;
    size_t size = errors.size;
    float by = internalIterationCount;
    
    auto biasGradients = toNativeDeviceArray(node.biasGradients);
    assert(biasGradients);
    float[] pBiasGradients = biasGradients.array;
    
    float[] pBiasGradientSums = null;
    if (hasGradientSums)
    {
        auto biasGradientSums = toNativeDeviceArray(node.biasGradientSums);
        assert(biasGradientSums);
        pBiasGradientSums = biasGradientSums.array;
    }
    
    for (size_t valueIdx = 0; valueIdx < size; valueIdx++)
    {
        pBiasGradients[valueIdx] += errorsArr[valueIdx];
        pBiasGradients[valueIdx] /= by;
        if (hasGradientSums) pBiasGradientSums[valueIdx] += pBiasGradients[valueIdx];
        
        size_t inputLayersCount = node.inputs.length;
        for (size_t ilidx = 0; ilidx < inputLayersCount; ilidx++)
        {
            auto inputs = toNativeDeviceArray(node.inputs[ilidx]());
            assert(inputs);
            float[] inputsArr = inputs.array;
            size_t inputSize = inputs.size;
            if (hasGradientSums)
            {
                auto gradients = toNativeDeviceArray2(node.gradients[ilidx]);
                assert(gradients);
                float[] gradientsArr = gradients.array;
                auto gradientSums = toNativeDeviceArray2(node.gradientSums[ilidx]);
                assert(gradientSums);
                float[] gradientSumsArr = gradientSums.array;
                
                for (size_t inputIndex = 0; inputIndex < inputSize; inputIndex++)
                {
                    size_t gidx = getIndex2(inputIndex, valueIdx, inputSize);
                    gradientsArr[gidx] += inputsArr[inputIndex] * errorsArr[valueIdx];
                    gradientsArr[gidx] /= by;
                    gradientSumsArr[gidx] += gradientsArr[gidx];
                }
            }
            else
            {
                auto gradients = toNativeDeviceArray2(node.gradients[ilidx]);
                assert(gradients);
                float[] gradientsArr = gradients.array;
                
                for (size_t inputIndex = 0; inputIndex < inputSize; inputIndex++)
                {
                    size_t gidx = getIndex2(inputIndex, valueIdx, inputSize);
                    gradientsArr[gidx] += inputsArr[inputIndex] * errorsArr[valueIdx];
                    gradientsArr[gidx] /= by;
                }
            }
        }
    }
}
