#pragma once

#include "nfdev.h"
#include "layer_description.h"

namespace nf
{
    struct activation_description : layer_description
    {
        activation_description()
        {
        }

        activation_description(activation_function function, float alpha) :
            _function(function),
            _alpha(alpha)
        {
        }

        activation_function function() const
        {
            return _function;
        }

        float alpha() const
        {
            return _alpha;
        }

    private:
        activation_function _function = activation_function::sigmoid;
        float _alpha = 1.7f;
    };
}