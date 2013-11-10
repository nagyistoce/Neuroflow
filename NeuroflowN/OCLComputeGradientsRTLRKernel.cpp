#include "stdafx.h"
#include "OCLComputeGradientsRTLRKernel.h"
#include "OCLIntCtx.h"
#include "OCLProgram.h"
#include "OCLVault.h"
#include "OCLComputationState.h"
#include "GetVectorSize.h"
#include "OCLDeviceArrayManagement.h"

using namespace std;
using namespace cl;
using namespace NeuroflowN;

OCLComputeGradientsRTLRKernel::OCLComputeGradientsRTLRKernel(const OCLIntCtxSPtrT& ctx, const OCLVaultSPtrT& vault) :
OCLVersionableKernelBase(ctx, "ComputeGradientsRTLR")
{
    Build(vault);
}

void OCLComputeGradientsRTLRKernel::Build(const OCLVaultSPtrT& vault)
{
    program = make_shared<OCLProgram>(ctx, "ComputeGradientsRTLRPrg");
    if (ctx->IsCPU()) program->Using(vault->GetNetCode()); else program->Using(vault->GetCommonCode());
    program->Using(vault->GetReduceCode());

    ADD_OCL_CODE(program,
        void ComputeGradinetsRTLR_SetGradients(local float* tmpGradients, global float* gradients, global float* gradientSums, int gradientsIndex)
        {
            Reduce_Sum(tmpGradients);

            if (get_local_id(0) == 0)
            {
                if (gradients != null) gradients[gradientsIndex] = tmpGradients[0];
                if (gradientSums != null) gradientSums[gradientsIndex] += tmpGradients[0];
            }
        }
    );

    if (ctx->IsCPU())
    {
        auto cpuCode = CreateCPUKernelCode();
        program->AddCode(cpuCode);
    }
    else
    {
        auto gpuCode = CreateGPUKernelCode();
        program->AddCode(gpuCode);
    }
}

std::string OCLComputeGradientsRTLRKernel::GetKernelHeader(const char* name)
{
    string layerParsTmpl1 =
        "global float$* p_i_j_l_Values_{i}_{l}\n"
        ",int p_i_j_l_ValuesSize_{i}_{l}\n"
        ",global float$* weights_{i}_{l}\n";

    string layerParsTmpl2 =
        ",global float* p_i_j_k_Values_{l}\n"
        ",int p_i_j_k_ValuesSize_{l}\n"
        ",global float* netDerivValues_{l}\n";

    stringstream hdr;
    hdr << "kernel void " << name << "$(\n";

    for (unsigned layerIndex = 0; layerIndex < ctx->GetMaxLayerCount(); layerIndex++)
    {
        for (unsigned inputIndex = 0; inputIndex < ctx->GetMaxConnectionCount(); inputIndex++)
        {
            if (!(inputIndex == 0 && layerIndex == 0)) hdr << ',';
            hdr << ReplaceIndexesInTemplate(layerParsTmpl1, inputIndex, layerIndex);
        }

        hdr << ReplaceIndexesInTemplate(layerParsTmpl2, layerIndex);
    }

    hdr << ",int iLayerIndex\n";
    hdr << ",int iValueIndex\n";
    hdr << ",global float* inputs\n";
    hdr << ",int inputIndex\n";
    hdr << ",global float* outputs\n";
    hdr << ",global float* desiredOutputs\n";
    hdr << ",local float* tmpGradients\n";
    hdr << ",global float* gradients\n";
    hdr << ",global float* gradientSums\n";
    hdr << ",int gradientIndex)\n";

    string hdrStr = hdr.str();
    return hdrStr;
}

std::string OCLComputeGradientsRTLRKernel::CreateCode_ComputeGradinetsRTLR_Layer_CPU()
{
    string tmpl = 
        "global float$* p_i_j_l_Values_{i}\n"
        ",int p_i_j_l_ValuesSize_{i}\n"
        ",global float$* weights_{i}\n";

    stringstream code;
    code << "void ComputeGradinetsRTLR_Layer_CPU$(\n";
    for (unsigned inputIndex = 0; inputIndex < ctx->GetMaxConnectionCount(); inputIndex++)
    {
        if (inputIndex != 0) code << ",";
        code << ReplaceIndexesInTemplate(tmpl, inputIndex, null);
    }
    code << 
        ",global float* p_i_j_k_Values\n"
        ",int p_i_j_k_ValuesSize\n"
        ",global float* netDerivValues\n"
        ",int iValueIndex\n"
        ",global float* inputs\n"
        ",int inputIndex\n"
        ",local float* tmpGradients\n"
        ",global float* outputs\n"
        ",global float* desiredOutputs)\n";
    code <<
        "{\n"
        "int localSize = get_local_size(0);\n"
        "int localId = get_local_id(0);\n"
        "barrier(CLK_LOCAL_MEM_FENCE);\n"
        "int block = p_i_j_k_ValuesSize / localSize + (p_i_j_k_ValuesSize % localSize != 0 ? 1 : 0);\n"
        "int kValueIndex = localId * block;\n"
        "int max = kValueIndex + block;\n"
        "if (max > p_i_j_k_ValuesSize) max = p_i_j_k_ValuesSize;\n"
        "while (kValueIndex < max)\n"
        "{\n"
        "float sum = iValueIndex == kValueIndex ? (inputs != null ? inputs[inputIndex] : 1.0f) : 0.0f; \n";
    
    for (unsigned inputIndex = 0; inputIndex < ctx->GetMaxConnectionCount(); inputIndex++)
    {
        if (inputIndex != 0) code << "if (p_i_j_l_Values_" << inputIndex << " != null) ";
        code << "sum += ComputeForward_Sum$(p_i_j_l_Values_" << inputIndex << ", p_i_j_l_ValuesSize_" << inputIndex << ", weights_" << inputIndex << ", kValueIndex);\n";
    };
    
    code <<
        "float p = netDerivValues[kValueIndex] * sum;\n"
        "p_i_j_k_Values[kValueIndex] = p;\n"
        "if (tmpGradients != null) tmpGradients[localId] += (desiredOutputs[kValueIndex] - outputs[kValueIndex]) * p;\n"
        "kValueIndex++;\n"
        "}\n"
        "}\n";

    return code.str();
}

std::string OCLComputeGradientsRTLRKernel::CreateCallCode_ComputeGradinetsRTLR_Layer_CPU(unsigned layerIndex)
{
    string tmpl = 
        "p_i_j_l_Values_{i}_{l}\n"
        ",p_i_j_l_ValuesSize_{i}_{l}\n"
        ",weights_{i}_{l}\n";

    stringstream code;
    code << "ComputeGradinetsRTLR_Layer_CPU$(\n";
    for (unsigned inputIndex = 0; inputIndex < ctx->GetMaxConnectionCount(); inputIndex++)
    {
        if (inputIndex != 0) code << ',';
        code << ReplaceIndexesInTemplate(tmpl, inputIndex, layerIndex);
    }

    code << ReplaceIndexesInTemplate(",p_i_j_k_Values_{l}\n,p_i_j_k_ValuesSize_{l}\n,netDerivValues_{l}\n", layerIndex);

    code <<
        ",iLayerIndex == kLayerIndex ? iValueIndex : -1\n"
        ",inputs\n"
        ",inputIndex\n"
        ",isLastLayer ? tmpGradients : null\n"
        ",isLastLayer ? outputs : null\n"
        ",isLastLayer ? desiredOutputs : null);\n";

    return code.str();
}

std::string OCLComputeGradientsRTLRKernel::CreateCPUKernelCode()
{
    auto& names = GetCPUNames();
    stringstream code;
    code << CreateCode_ComputeGradinetsRTLR_Layer_CPU();

    auto createCode = [=](const char* name)
    {
        stringstream code;
        code << GetKernelHeader(name);
        code << "{\n";
        code << "tmpGradients[get_global_id(0)] = 0.0f;\n";
        code << "int kLayerIndex;\n";
        code << "bool isLastLayer;\n";

        for (int layerIndex = 0; layerIndex < ctx->GetMaxLayerCount(); layerIndex++)
        {
            if (layerIndex != 0)
            {
                code << 
                    "if (!isLastLayer)\n"
                    "{\n";
            }

            code << "kLayerIndex = " << layerIndex << ";\n";
            if (layerIndex == ctx->GetMaxLayerCount() - 1)
            {
                code << "isLastLayer =  true;\n";
            }
            else
            {
                code << "isLastLayer =  p_i_j_k_Values_" << layerIndex + 1 << " == null;\n";
            }
            code << CreateCallCode_ComputeGradinetsRTLR_Layer_CPU(layerIndex);

            if (layerIndex != 0)
            {
                code << "};\n";
            }
        }

        code << "ComputeGradinetsRTLR_SetGradients(tmpGradients, gradients, gradientSums, gradientIndex);\n";
        code << "}\n";

        return code.str();
    };

    code << createCode(names.GetVersion()->GetName().c_str());
    string codeStr = code.str();
    
    return codeStr;
}

std::string OCLComputeGradientsRTLRKernel::CreateGPUKernelCode()
{
    return "kernel lofasz(global int* foo) { }";
}

void OCLComputeGradientsRTLRKernel::Exec(NfObject* state, RTLRLayerInfoVecVecT* inputLayerInfos, DeviceArrayVecT* netValueDerivates, RTLRComputationData* data, DeviceArrayVecT* valueRelatedPBuffs, IDeviceArray* outputs, IDeviceArray* desiredOutputs)
{
    auto cState = (OCLComputationState*)state;

    auto init = [=](Kernel& kernel)
    {
    };

    auto exec = cState->GetExec(0);

    unsigned vectorSize = 1;

    if (ctx->IsCPU())
    {
        unsigned workSize = 64;
        exec->Execute(program, (*GetCPUNames().GetVersion())(vectorSize), vectorSize, init, workSize);
    }

    /*auto cState = (OCLComputationState*)state;

    unsigned kLayerSize = valueRelatedPBuffs->size();
    unsigned outputLayerIndex = kLayerSize - 1;

    ctx->GetQueue().finish();*/
}

void OCLComputeGradientsRTLRKernel::AnalyzeInfos(const RTLRLayerInfoVecT& infos, unsigned& vectorSize, unsigned& uCount) const
{
    vectorSize = 16;
    uCount = 0;
    for (auto& i : infos)
    {
        vectorSize = GetVectorSize(vectorSize, i.Size);
        if (i.IsElementOfU) uCount++;
    }
}
