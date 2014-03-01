#pragma once

#include "nfdev.h"

namespace nf
{
    struct learning_impl_factory : virtual nf_object
    {
        virtual learning_impl_ptr create_impl(const learning_behavior_ptr& learningBehavior, const training_node_collection_t& nodes) = 0;
    };
}