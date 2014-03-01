#pragma once

#include "nfdev.h"

namespace nf
{
    struct compute_activation : virtual nf_object
    {
        virtual nf_object_ptr create_operation_context() = 0;
        virtual void compute_forward(const nf_object_ptr& context, const std::vector<mlp_forward_node>& nodes, idx_t offset) = 0;
        virtual void compute_backward(const nf_object_ptr& context, const std::vector<mlp_backward_node>& nodes, idx_t offset, gradient_computation_formula gcf, idx_t internalIterationCount) = 0;
    };
}