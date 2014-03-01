#include "stdafx.h"
#include "CppUnitTest.h"

#include <nf.h>
#include <layer_order_comparer.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace nf;
using namespace linqlike;

namespace NeuroflowNativeUT
{
	TEST_CLASS(multilayer_perceptron_tests)
	{
	public:

        BEGIN_TEST_METHOD_ATTRIBUTE(layer_order_comparer_test)
            TEST_METHOD_ATTRIBUTE(L"Category", L"MLP")
            TEST_METHOD_ATTRIBUTE(L"Platform", L"CPP")
        END_TEST_METHOD_ATTRIBUTE()
        TEST_METHOD(layer_order_comparer_test)
        {
            vector<layer_ptr> layers =
            {
                make_layer(2),
                make_layer(4),
                make_layer(1)
            };

            layers[0]->output_connections().add_one_way(layers[1]);
            layers[1]->output_connections().add_one_way(layers[2]);

            Assert::AreEqual(idx_t(0), layers[0]->input_connections().connected_layers(flow_direction::all) | size());
            Assert::AreEqual(idx_t(1), layers[0]->output_connections().connected_layers(flow_direction::all) | size());

            auto sorted = layers | sort(layer_order_comparer()) | row_num() | to_vector();

            Assert::AreEqual(idx_t(3), sorted.size());
            Assert::AreEqual(idx_t(0), sorted[0].row_num());
            Assert::AreEqual(idx_t(1), sorted[1].row_num());
            Assert::AreEqual(idx_t(2), sorted[2].row_num());
        }
		
        BEGIN_TEST_METHOD_ATTRIBUTE(cpp_get_and_set_weights)
            TEST_METHOD_ATTRIBUTE(L"Category", L"MLP")
            TEST_METHOD_ATTRIBUTE(L"Platform", L"CPP")
            END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(cpp_get_and_set_weights)
		{
            try
            {
                auto ctx = computation_context_factory().create_context(cpp_context);
                do_get_and_set_weights(ctx);
            }
            catch (exception& ex)
            {
                Logger::WriteMessage(ex.what());
                throw;
            }
		}

        BEGIN_TEST_METHOD_ATTRIBUTE(ocl_get_and_set_weights_cpu)
            TEST_METHOD_ATTRIBUTE(L"Category", L"MLP")
            TEST_METHOD_ATTRIBUTE(L"Platform", L"OCL")
            TEST_METHOD_ATTRIBUTE(L"Device", L"OCL CPU")
        END_TEST_METHOD_ATTRIBUTE()
        TEST_METHOD(ocl_get_and_set_weights_cpu)
        {
            try
            {
                auto ctx = computation_context_factory().create_context(ocl_context, L"CPU");
                do_get_and_set_weights(ctx);
            }
            catch (exception& ex)
            {
                Logger::WriteMessage(ex.what());
                throw;
            }
        }

        BEGIN_TEST_METHOD_ATTRIBUTE(ocl_get_and_set_weights_gpu)
            TEST_METHOD_ATTRIBUTE(L"Category", L"MLP")
            TEST_METHOD_ATTRIBUTE(L"Platform", L"OCL")
            TEST_METHOD_ATTRIBUTE(L"Device", L"OCL GPU")
        END_TEST_METHOD_ATTRIBUTE()
        TEST_METHOD(ocl_get_and_set_weights_gpu)
        {
            try
            {
                auto ctx = computation_context_factory().create_context(ocl_context, L"GPU");
                do_get_and_set_weights(ctx);
            }
            catch (exception& ex)
            {
                Logger::WriteMessage(ex.what());
                throw;
            }
        }

        BEGIN_TEST_METHOD_ATTRIBUTE(cpu_compute)
            TEST_METHOD_ATTRIBUTE(L"Category", L"MLP")
            TEST_METHOD_ATTRIBUTE(L"Platform", L"CPP")
        END_TEST_METHOD_ATTRIBUTE()
        TEST_METHOD(cpu_compute)
        {
            try
            {
                auto ctx = computation_context_factory().create_context(cpp_context);
                do_compute(ctx);
            }
            catch (exception& ex)
            {
                Logger::WriteMessage(ex.what());
                throw;
            }
        }

        void do_get_and_set_weights(const computation_context_ptr& ctx)
        {
            auto mlp = create_mlp(ctx);
            idx_t numWeights = mlp->number_of_weights();
            auto weights = ctx->data_array_factory()->create(numWeights);
            vector<float> weightValues(numWeights);
            mlp->get_weights(weights);
            weights->read(0, numWeights, &weightValues[0], 0).wait();
            for (float v : weightValues) Assert::AreEqual(0.0f, v);

            for (idx_t i = 0; i < numWeights; i++) weightValues[i] = 0.11f;
            weights->write(&weightValues[0], 0, numWeights, 0).wait();
            mlp->set_weights(weights);

            for (idx_t i = 0; i < numWeights; i++) weightValues[i] = 0.99f;
            weights->write(&weightValues[0], 0, numWeights, 0).wait();
            for (idx_t i = 0; i < numWeights; i++) weightValues[i] = 0.0f;
            weights->read(0, numWeights, &weightValues[0], 0).wait();
            for (float v : weightValues) Assert::AreEqual(0.99f, v);

            mlp->get_weights(weights);
            weights->read(0, numWeights, &weightValues[0], 0).wait();
            for (float v : weightValues) Assert::AreEqual(0.11f, v);
        }

        void do_compute(const computation_context_ptr& ctx)
        {
            auto mlp = create_mlp(ctx);
            idx_t numWeights = mlp->number_of_weights();
            auto weights = ctx->data_array_factory()->create(numWeights);
            vector<float> weightValues(numWeights);
            for (idx_t i = 0; i < numWeights; i++) weightValues[i] = 0.11f;
            weights->write(&weightValues[0], 0, numWeights, 0).wait();
            mlp->set_weights(weights);

            idx_t inputsSize = mlp->input_size();
            auto inputs = ctx->data_array_factory()->create(inputsSize);
            vector<float> inputValues(inputsSize);
            for (idx_t i = 0; i < inputsSize; i++) inputValues[i] = 0.22f;
            inputs->write(&inputValues[0], 0, inputsSize, 0).wait();

            idx_t outputsSize = mlp->output_size();
            auto outputs = ctx->data_array_factory()->create(outputsSize);
            vector<float> outputValues(outputsSize);
            
            outputs->read(0, outputsSize, &outputValues[0], 0).wait();
            for (idx_t i = 0; i < outputsSize; i++)
            {
                Assert::AreEqual(0.0f, outputValues[i]);
            }

            mlp->compute(inputs, outputs);

            outputs->read(0, outputsSize, &outputValues[0], 0).wait();
            for (idx_t i = 0; i < outputsSize; i++)
            {
                Assert::AreNotEqual(0.0f, outputValues[i]);
            }
        }

        multilayer_perceptron_ptr create_mlp(const computation_context_ptr& ctx)
        {
            vector<layer_ptr> layers =
            {
                make_layer(2),
                make_layer(4, make_activation_description(activation_function::sigmoid, 1.7f)),
                make_layer(1, make_activation_description(activation_function::linear, 1.1f))
            };
            layers[0]->output_connections().add_one_way(layers[1]);
            layers[1]->output_connections().add_one_way(layers[2]);

            auto mlp = ctx->neural_network_factory()->create_multilayer_perceptron(layers);
            idx_t numWeights = mlp->number_of_weights();
            Assert::AreEqual(idx_t((2 * 4 + 4) + (4 * 1 + 1)), numWeights);

            return move(mlp);
        }
	};
}