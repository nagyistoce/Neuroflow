#pragma once

#include "nfdev.h"
#include "device_info.h"

namespace nf
{
    struct computation_context : virtual nf_object
    {
        const neural_network_factory_ptr& neural_network_factory();

        virtual const nf::device_info& device_info() const = 0;
        virtual const properties_t& properties() const = 0;
        virtual device_array_management_ptr device_array_management() = 0;
        virtual data_array_factory_ptr data_array_factory() = 0;
        virtual utils_ptr utils() = 0;
        virtual compute_activation_ptr compute_activation() = 0;

    private:
        neural_network_factory_ptr _neuralNetworkFactory;
    };
}