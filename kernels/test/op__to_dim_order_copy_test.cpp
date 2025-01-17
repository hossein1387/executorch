/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdint>
#include <map>
#include <typeindex>
#include <variant>

#include <executorch/kernels/test/FunctionHeaderWrapper.h> // Declares the operator
#include <executorch/kernels/test/TestUtil.h>
#include <executorch/kernels/test/supported_features.h>
#include <executorch/runtime/core/exec_aten/exec_aten.h>
#include <executorch/runtime/core/exec_aten/testing_util/tensor_factory.h>
#include <executorch/runtime/core/exec_aten/testing_util/tensor_util.h>

#include <gtest/gtest.h>

using namespace ::testing;
using exec_aten::ArrayRef;
using exec_aten::optional;
using exec_aten::ScalarType;
using exec_aten::Tensor;
using torch::executor::testing::TensorFactory;

// To further emphasize the accuracy of our op_to, we TEST_F the conversion
// from floating-point types to signed int types directly by the TEST_F cases
// generated by core Pytorch directly. Such data is random generated in [-5, 5].

// clang-format off
typedef std::map<
          std::type_index,
          std::variant<
            std::vector<float>,
            std::vector<double>,
            std::vector<exec_aten::Half>,
            std::vector<exec_aten::BFloat16>>>
        FloatingTypeToDataMap;

typedef std::map<
          std::type_index,
          std::variant<
              std::vector<int64_t>,
              std::vector<int32_t>,
              std::vector<int16_t>,
              std::vector<int8_t>,
              std::vector<uint8_t>>>
        IntTypeToDataMap;
// clang-format on

class OpToDimOrderCopyTest : public OperatorTest {
 protected:
  Tensor& op__to_dim_order_copy_out(
      const Tensor& self,
      bool non_blocking,
      exec_aten::optional<ArrayRef<int64_t>> dim_order,
      Tensor& out) {
    return torch::executor::dim_order_ops::_to_dim_order_copy_outf(
        context_, self, non_blocking, dim_order, out);
  }
  // Cast float vector to OUTPUT_CTYPE vector
  template <typename INPUT_CTYPE, typename OUTPUT_CTYPE>
  std::vector<OUTPUT_CTYPE> vector_type_cast(std::vector<INPUT_CTYPE> input) {
    std::vector<OUTPUT_CTYPE> output(input.size());
    std::transform(
        input.begin(), input.end(), output.begin(), [](INPUT_CTYPE x) {
          return static_cast<OUTPUT_CTYPE>(x);
        });
    return output;
  }

  template <typename INPUT_CTYPE, typename OUTPUT_CTYPE>
  struct ToTestCase {
    const std::vector<int32_t> sizes;
    const std::vector<INPUT_CTYPE> data_in;
    const std::vector<OUTPUT_CTYPE> data_out;
  };

  // Each TEST_F has different combination of input and output types. Therefore
  // it is a little bit mess if create template TEST_F case and custom data
  // types for both input data and output data. We choose another way: for all
  // TEST_F cases, their data are all in double. And we are gonna cast them into
  // desired type when delievering them into tf.make function. Based on our
  // experiments, type cast of core PyTorch is same as static_cast in c++ in the
  // representable scope, so here we believe using static_cast to generate
  // ground truth is reasonable.
  template <
      typename INPUT_CTYPE,
      ScalarType INPUT_DTYPE,
      typename OUTPUT_CTYPE,
      ScalarType OUTPUT_DTYPE>
  void test_runner_static_cast(
      std::vector<ToTestCase<double, double>> test_cases) {
    TensorFactory<INPUT_DTYPE> tf_in;
    TensorFactory<OUTPUT_DTYPE> tf_out;

    for (const auto& test_case : test_cases) {
      auto data_in = vector_type_cast<double, INPUT_CTYPE>(test_case.data_in);
      auto data_out = vector_type_cast<INPUT_CTYPE, OUTPUT_CTYPE>(data_in);

      Tensor input = tf_in.make(test_case.sizes, data_in);
      Tensor output = tf_out.zeros_like(input);

      std::vector<int64_t> dim_order_vec;
      for (int64_t i = 0; i < input.dim(); i++) {
        dim_order_vec.push_back(i);
      }
      ArrayRef<int64_t> dim_order(dim_order_vec.data(), dim_order_vec.size());

      Tensor ret = op__to_dim_order_copy_out(
          /*self=*/input,
          /*non_blocking=*/false,
          dim_order,
          output);

      Tensor expected = tf_out.make(test_case.sizes, data_out);

      // The original tensor a should share same value with the out variable and
      // return variable of to function
      EXPECT_TENSOR_EQ(ret, output);
      EXPECT_TENSOR_EQ(ret, expected);
    }
  }

  template <typename INPUT_CTYPE, ScalarType INPUT_DTYPE>
  void test_runner_to_bool(
      std::vector<double> test_case,
      std::vector<uint8_t> data_out) {
    TensorFactory<INPUT_DTYPE> tf_in;
    TensorFactory<ScalarType::Bool> tf_out;

    auto data_in = vector_type_cast<double, INPUT_CTYPE>(test_case);

    Tensor input = tf_in.make({(int)test_case.size()}, data_in);
    Tensor output = tf_out.zeros_like(input);

    std::vector<int64_t> dim_order_vec;
    for (int i = 0; i < input.dim(); i++) {
      dim_order_vec.push_back(i);
    }
    ArrayRef<int64_t> dim_order(dim_order_vec.data(), dim_order_vec.size());

    Tensor ret = op__to_dim_order_copy_out(
        /*self=*/input,
        /*non_blocking=*/false,
        dim_order,
        output);

    Tensor expected = tf_out.make({(int)data_out.size()}, data_out);

    // The return value of op__to_dim_order_copy_out and the values written to
    // output should be the same.
    EXPECT_TENSOR_EQ(ret, output);
    // The return value of op__to_dim_order_copy_out and the values in expected
    // which are the reference values should be the same.
    EXPECT_TENSOR_EQ(ret, expected);
  }

  template <typename OUT_CTYPE, ScalarType OUT_DTYPE>
  void test_runner_from_bool(
      std::vector<uint8_t> test_case,
      std::vector<double> out) {
    TensorFactory<ScalarType::Bool> tf_in;
    TensorFactory<OUT_DTYPE> tf_out;

    auto data_out = vector_type_cast<double, OUT_CTYPE>(out);

    Tensor input = tf_in.make({(int)test_case.size()}, test_case);
    Tensor output = tf_out.zeros_like(input);

    std::vector<int64_t> dim_order_vec;
    for (int64_t i = 0; i < input.dim(); i++) {
      dim_order_vec.push_back(i);
    }
    ArrayRef<int64_t> dim_order(dim_order_vec.data(), dim_order_vec.size());

    Tensor ret = op__to_dim_order_copy_out(
        /*self=*/input,
        /*non_blocking=*/false,
        dim_order,
        output);

    Tensor expected = tf_out.make({(int)data_out.size()}, data_out);

    // The return value of op__to_dim_order_copy_out and the values written to
    // output should be the same.
    EXPECT_TENSOR_EQ(ret, output);
    // The return value of op__to_dim_order_copy_out and the values in expected
    // which are the reference values should be the same.
    EXPECT_TENSOR_EQ(ret, expected);
  }

  /* %python
  import torch
  torch.manual_seed(0)
  x = torch.rand(2, 3)
  res = x.to(non_blocking = False, memory_format = torch.preserve_format)
  op = "op__to_dim_order_copy_out"
  opt_setup_params = """
    bool non_blocking = false;
    optional<MemoryFormat> memory_format;
  """
  opt_extra_params = "non_blocking, memory_format,"
  out_args = "out_shape, dynamism"
  dtype = "ScalarType::Float"
  check = "EXPECT_TENSOR_EQ" */

  void test_dynamic_shape(
      const std::vector<int32_t>& out_shape,
      enum torch::executor::TensorShapeDynamism dynamism) {
    /* %python
    %rewrite(unary_op) */

    TensorFactory<ScalarType::Float> tf;

    Tensor x = tf.make(
        {2, 3},
        {0.49625658988952637,
         0.7682217955589294,
         0.08847743272781372,
         0.13203048706054688,
         0.30742281675338745,
         0.6340786814689636});
    Tensor expected = tf.make(
        {2, 3},
        {0.49625658988952637,
         0.7682217955589294,
         0.08847743272781372,
         0.13203048706054688,
         0.30742281675338745,
         0.6340786814689636});

    bool non_blocking = false;

    Tensor out = tf.zeros(out_shape, dynamism);

    std::vector<int64_t> dim_order_vec;
    for (int64_t i = 0; i < x.dim(); i++) {
      dim_order_vec.push_back(i);
    }
    ArrayRef<int64_t> dim_order(dim_order_vec.data(), dim_order_vec.size());

    Tensor ret = op__to_dim_order_copy_out(
        /*self=*/x, non_blocking, dim_order, out);

    EXPECT_TENSOR_EQ(out, expected);
    EXPECT_TENSOR_EQ(ret, expected);
  }

  template <
      typename INPUT_CTYPE,
      ScalarType INPUT_DTYPE,
      typename OUTPUT_CTYPE,
      ScalarType OUTPUT_DTYPE>
  void test_runner_hardcode_data(
      FloatingTypeToDataMap floating_point_data,
      IntTypeToDataMap int_data) {
    TensorFactory<INPUT_DTYPE> tf_in;
    TensorFactory<OUTPUT_DTYPE> tf_out;

    if (typeid(OUTPUT_CTYPE) == typeid(uint8_t)) {
      // Would cause underflow when testing uint8_t.
      return;
    }

    ToTestCase<INPUT_CTYPE, OUTPUT_CTYPE> test_case = {
        /*sizes=*/{3, 5}, /*data_in=*/
        std::get<std::vector<INPUT_CTYPE>>(
            floating_point_data[typeid(INPUT_CTYPE)]),
        /*data_out=*/
        std::get<std::vector<OUTPUT_CTYPE>>(int_data[typeid(OUTPUT_CTYPE)])};

    Tensor input = tf_in.make(test_case.sizes, test_case.data_in);
    Tensor output = tf_out.zeros_like(input);

    std::vector<int64_t> dim_order_vec;
    for (int64_t i = 0; i < input.dim(); i++) {
      dim_order_vec.push_back(i);
    }
    ArrayRef<int64_t> dim_order(dim_order_vec.data(), dim_order_vec.size());

    Tensor ret = op__to_dim_order_copy_out(
        /*self=*/input,
        /*non_blocking=*/false,
        dim_order,
        output);

    Tensor expected = tf_out.make(test_case.sizes, test_case.data_out);

    // The original tensor a should share same value with the out variable and
    // return variable of to function
    EXPECT_TENSOR_EQ(ret, output);
    EXPECT_TENSOR_EQ(ret, expected);
  }
};

/* Here we temporary not try to implement or TEST_F the behavior about casting a
 * number can not be represented in some type to this type (e.g. inf to
 * int32_t nan to int64_t or 2147483648 to int32_t), because
 * - a. The result of such kind of cast is undefined according to c++
 * standard;
 * - b. No explicit rules can be found in core pytorch for such transaction
 * (not same as static_cast or any other casting function in c++);
 * - c. If user tries to cast a unrepresentable value to certain type, they
 *      should take the risk;
 * - d. Even though we can always use if/switch to cover these boundry cases,
 *      the code will be lengthy and jumbled. I believe using these disordered
 *      code to meet some undefine behavior is meaningless, and we can not
 *      cover all such cases.
 */

// Regular TEST_F for to_copy.out
// TEST_F if to_copy.out works well under all kinds of data pairs
TEST_F(OpToDimOrderCopyTest, AllDtypesSupported) {
  std::vector<ToTestCase<double, double>> test_cases = {
      {
          /*sizes=*/{2, 4}, /*data_in=*/
          {2.11, 3.2, 2.3, 4.0, 1.1, 5.2, 1.1, 6.3}, /*data_out=*/
          {}, // data_out shouldn't be used in test_runner_static_cast
      },
      {
          /*sizes=*/{3, 4, 0, 5},
          /*data_in=*/{},
          /*data_out=*/{},
      },
      {
          /*sizes=*/{},
          /*data_in=*/{10.0},
          /*data_out=*/{}, // data_out shouldn't be used in
                           // test_runner_static_cast
      },
  };

#define TEST_KERNEL(INPUT_CTYPE, INPUT_DTYPE, OUTPUT_CTYPE, OUTPUT_DTYPE) \
  test_runner_static_cast<                                                \
      INPUT_CTYPE,                                                        \
      ScalarType::INPUT_DTYPE,                                            \
      OUTPUT_CTYPE,                                                       \
      ScalarType::OUTPUT_DTYPE>(test_cases);

#define TEST_ENTRY(INPUT_CTYPE, INPUT_DTYPE) \
  ET_FORALL_REAL_TYPES_WITH2(INPUT_CTYPE, INPUT_DTYPE, TEST_KERNEL);

  ET_FORALL_REAL_TYPES(TEST_ENTRY);

#undef TEST_ENTRY
#undef TEST_KERNEL
}

TEST_F(OpToDimOrderCopyTest, BoolTests) {
  std::vector<double> test_case_to_bool = {1.1, 2.2, 0};
  std::vector<uint8_t> result_to_bool = {true, true, false};
#define TEST_TO_BOOL(INPUT_CTYPE, INPUT_DTYPE)               \
  test_runner_to_bool<INPUT_CTYPE, ScalarType::INPUT_DTYPE>( \
      test_case_to_bool, result_to_bool);
  ET_FORALL_REAL_TYPES(TEST_TO_BOOL);

  std::vector<uint8_t> test_case_from_bool = {true, true, false};
  std::vector<double> result_from_bool = {1.0, 1.0, 0};
#define TEST_FROM_BOOL(OUTPUT_CTYPE, OUTPUT_DTYPE)               \
  test_runner_from_bool<OUTPUT_CTYPE, ScalarType::OUTPUT_DTYPE>( \
      test_case_from_bool, result_from_bool);
  ET_FORALL_REAL_TYPES(TEST_FROM_BOOL);
}

TEST_F(OpToDimOrderCopyTest, NanInfSupported) {
  constexpr auto floatInfinity = std::numeric_limits<float>::infinity();
  std::vector<ToTestCase<double, double>> test_cases = {{
      /*sizes=*/{2, 4},
      /*data_in=*/{2, 3, NAN, 4, floatInfinity, 5, -floatInfinity, 6},
      /*data_out=*/{2, 3, NAN, 4, floatInfinity, 5, -floatInfinity, 6},
  }};

#define TEST_KERNEL(INPUT_CTYPE, INPUT_DTYPE, OUTPUT_CTYPE, OUTPUT_DTYPE) \
  test_runner_static_cast<                                                \
      INPUT_CTYPE,                                                        \
      ScalarType::INPUT_DTYPE,                                            \
      OUTPUT_CTYPE,                                                       \
      ScalarType::OUTPUT_DTYPE>(test_cases);

#define TEST_ENTRY(INPUT_CTYPE, INPUT_DTYPE) \
  ET_FORALL_FLOATHBF16_TYPES_WITH2(INPUT_CTYPE, INPUT_DTYPE, TEST_KERNEL);

  ET_FORALL_FLOATHBF16_TYPES(TEST_ENTRY);

#undef TEST_ENTRY
#undef TEST_KERNEL
}

TEST_F(OpToDimOrderCopyTest, HardcodeFloatConvertInt) {
  // Hardcode input and output generated from core PyTorch
  // clang-format off
  std::vector<float> float_data = {
      -1.47900056838989257812, -4.59277725219726562500,
       2.15365791320800781250, -2.55494546890258789062,
       3.06999135017395019531,  3.27460670471191406250,
      -3.98865103721618652344, -4.81065988540649414062,
       3.67902207374572753906,  3.72226405143737792969,
       0.80567771196365356445,  2.23788332939147949219,
      -0.52035576105117797852, -1.58493483066558837891,
      -0.30919688940048217773};

  std::vector<double> double_data = {
      -1.47900053955270172068, -4.59277735274143061872,
       2.15365796963871947156, -2.55494554556038755422,
       3.06999137834642255029,  3.27460679459944969949,
      -3.98865109243288795682, -4.81065977167646074975,
       3.67902198302105531980,  3.72226414774102742911,
       0.80567768667100203572,  2.23788335717029518435,
      -0.52035578832931150828, -1.58493480710766210251,
      -0.30919688936285893988};
  // clang-format on

  std::vector<exec_aten::Half> half_data;
  std::vector<exec_aten::BFloat16> bf16_data;
  for (auto d : double_data) {
    half_data.emplace_back(d);
    bf16_data.emplace_back(d);
  }

  std::vector<int64_t> int64_data = {
      -1, -4, 2, -2, 3, 3, -3, -4, 3, 3, 0, 2, 0, -1, 0};
  std::vector<int32_t> int32_data = {
      -1, -4, 2, -2, 3, 3, -3, -4, 3, 3, 0, 2, 0, -1, 0};
  std::vector<int16_t> int16_data = {
      -1, -4, 2, -2, 3, 3, -3, -4, 3, 3, 0, 2, 0, -1, 0};
  std::vector<int8_t> int8_data = {
      -1, -4, 2, -2, 3, 3, -3, -4, 3, 3, 0, 2, 0, -1, 0};

  // Gathering all floating point data together for better traversial
  FloatingTypeToDataMap floating_point_data;
  floating_point_data[typeid(float)] = float_data;
  floating_point_data[typeid(double)] = double_data;
  floating_point_data[typeid(exec_aten::Half)] = half_data;
  floating_point_data[typeid(exec_aten::BFloat16)] = bf16_data;

  // Gathering all int data together for better traversial
  IntTypeToDataMap int_data;
  int_data[typeid(int64_t)] = int64_data;
  int_data[typeid(int32_t)] = int32_data;
  int_data[typeid(int16_t)] = int16_data;
  int_data[typeid(int8_t)] = int8_data;

#define TEST_KERNEL(INPUT_CTYPE, INPUT_DTYPE, OUTPUT_CTYPE, OUTPUT_DTYPE) \
  test_runner_hardcode_data<                                              \
      INPUT_CTYPE,                                                        \
      ScalarType::INPUT_DTYPE,                                            \
      OUTPUT_CTYPE,                                                       \
      ScalarType::OUTPUT_DTYPE>(floating_point_data, int_data);

#define TEST_ENTRY(INPUT_CTYPE, INPUT_DTYPE) \
  ET_FORALL_INT_TYPES_WITH2(INPUT_CTYPE, INPUT_DTYPE, TEST_KERNEL);

  ET_FORALL_FLOATHBF16_TYPES(TEST_ENTRY);
}

TEST_F(OpToDimOrderCopyTest, MismatchedSizesDie) {
  if (torch::executor::testing::SupportedFeatures::get()->is_aten) {
    GTEST_SKIP() << "ATen kernel can handle mismatched sizes";
  }
  TensorFactory<ScalarType::Int> tf;
  Tensor input = tf.make(/*sizes=*/{3, 1, 1, 2}, /*data=*/{1, 2, 3, 4, 5, 6});
  Tensor out = tf.zeros({3, 2, 1, 1});
  std::vector<int64_t> dim_order_vec;
  for (int64_t i = 0; i < input.dim(); i++) {
    dim_order_vec.push_back(i);
  }
  ArrayRef<int64_t> dim_order(dim_order_vec.data(), dim_order_vec.size());

  ET_EXPECT_KERNEL_FAILURE(
      context_,
      op__to_dim_order_copy_out(
          /*self=*/input,
          /*non_blocking=*/false,
          dim_order,
          out));
}

// Only contiguous memory is supported, the memory type MemoryFormat::Contiguous
// should not be allowed. The function is expected death if using the illegal
// memory format.
TEST_F(OpToDimOrderCopyTest, MismatchedMemoryFormatDies) {
  if (torch::executor::testing::SupportedFeatures::get()->is_aten) {
    GTEST_SKIP() << "ATen kernel can handle non contiguous memory formats";
  }
  TensorFactory<ScalarType::Float> tf_in;
  TensorFactory<ScalarType::Float> tf_out;
  Tensor input =
      tf_in.make(/*sizes=*/{3, 1, 1, 2}, /*data=*/{1, 2, 3, 4, 5, 6});
  Tensor out = tf_out.zeros({3, 1, 1, 2});

  std::vector<int64_t> dim_order_vec;
  for (int64_t i = 0; i < input.dim(); i++) {
    dim_order_vec.push_back(i);
  }

  // mutate dim_order_vec to create a illegal one.
  dim_order_vec[1] = 3;
  dim_order_vec[3] = 1;
  ArrayRef<int64_t> dim_order(dim_order_vec.data(), dim_order_vec.size());

  ET_EXPECT_KERNEL_FAILURE(
      context_,
      op__to_dim_order_copy_out(
          /*self=*/input,
          /*non_blocking=*/false,
          dim_order,
          out));
}

// Only blocking data transfer supported
TEST_F(OpToDimOrderCopyTest, MismatchedBlockingDie) {
  if (torch::executor::testing::SupportedFeatures::get()->is_aten) {
    GTEST_SKIP() << "ATen kernel can handle non blocking data transfer";
  }
  TensorFactory<ScalarType::Int> tf;
  Tensor input = tf.make(/*sizes=*/{3, 1, 1, 2}, /*data=*/{1, 2, 3, 4, 5, 6});
  Tensor out = tf.zeros(/*sizes=*/{3, 1, 1, 2});

  std::vector<int64_t> dim_order_vec;
  for (int64_t i = 0; i < input.dim(); i++) {
    dim_order_vec.push_back(i);
  }
  ArrayRef<int64_t> dim_order(dim_order_vec.data(), dim_order_vec.size());

  ET_EXPECT_KERNEL_FAILURE(
      context_,
      op__to_dim_order_copy_out(
          /*self=*/input,
          /*non_blocking=*/true,
          dim_order,
          out));
}

TEST_F(OpToDimOrderCopyTest, DynamicShapeUpperBoundSameAsExpected) {
  test_dynamic_shape(
      {2, 3}, torch::executor::TensorShapeDynamism::DYNAMIC_BOUND);
}

TEST_F(OpToDimOrderCopyTest, DynamicShapeUpperBoundLargerThanExpected) {
  test_dynamic_shape(
      {10, 10}, torch::executor::TensorShapeDynamism::DYNAMIC_BOUND);
}

TEST_F(OpToDimOrderCopyTest, DynamicShapeUnbound) {
  if (!torch::executor::testing::SupportedFeatures::get()->output_resize) {
    GTEST_SKIP() << "Dynamic shape unbound not supported";
  }
  test_dynamic_shape(
      {1, 1}, torch::executor::TensorShapeDynamism::DYNAMIC_UNBOUND);
}

TEST_F(OpToDimOrderCopyTest, ContiguousToChannelsLast) {
  TensorFactory<ScalarType::Float> tf;

  Tensor x = tf.make_with_dimorder(
      {3, 5, 2, 2},
      {0.2432, 0.5248, 0.5361, 0.8513, 0.8184, 0.8206, 0.7357, 0.9655, 0.6138,
       0.1112, 0.2799, 0.1079, 0.9680, 0.2548, 0.0393, 0.6002, 0.2257, 0.8766,
       0.2715, 0.1595, 0.2029, 0.7026, 0.6982, 0.8529, 0.4405, 0.6560, 0.9217,
       0.6372, 0.2446, 0.6590, 0.3866, 0.7185, 0.4439, 0.5346, 0.3179, 0.4492,
       0.3491, 0.6970, 0.8456, 0.2516, 0.2345, 0.2924, 0.7695, 0.0911, 0.8530,
       0.8560, 0.6909, 0.7719, 0.8923, 0.5546, 0.6978, 0.8151, 0.3007, 0.3961,
       0.8416, 0.4296, 0.7203, 0.8963, 0.3597, 0.5552});

  Tensor out = tf.full_channels_last({3, 5, 2, 2}, 0.0);
  Tensor expected = tf.make_with_dimorder(
      {3, 5, 2, 2},
      {0.2432, 0.8184, 0.6138, 0.9680, 0.2257, 0.5248, 0.8206, 0.1112, 0.2548,
       0.8766, 0.5361, 0.7357, 0.2799, 0.0393, 0.2715, 0.8513, 0.9655, 0.1079,
       0.6002, 0.1595, 0.2029, 0.4405, 0.2446, 0.4439, 0.3491, 0.7026, 0.6560,
       0.6590, 0.5346, 0.6970, 0.6982, 0.9217, 0.3866, 0.3179, 0.8456, 0.8529,
       0.6372, 0.7185, 0.4492, 0.2516, 0.2345, 0.8530, 0.8923, 0.3007, 0.7203,
       0.2924, 0.8560, 0.5546, 0.3961, 0.8963, 0.7695, 0.6909, 0.6978, 0.8416,
       0.3597, 0.0911, 0.7719, 0.8151, 0.4296, 0.5552},
      /*dim_order=*/{0, 2, 3, 1});

  std::vector<int64_t> dim_order_vec = {0, 2, 3, 1};
  exec_aten::ArrayRef<int64_t> dim_order(
      dim_order_vec.data(), dim_order_vec.size());
  Tensor ret = op__to_dim_order_copy_out(
      /*self*/ x, /*non_blocking*/ false, /*dim_order*/ dim_order, out);

  EXPECT_TENSOR_EQ(out, expected);
  EXPECT_TENSOR_EQ(ret, expected);
}

TEST_F(OpToDimOrderCopyTest, ChannelsLastToContiguous) {
  TensorFactory<ScalarType::Float> tf;

  Tensor out = tf.full({3, 5, 2, 2}, 0.0);
  Tensor x = tf.make_with_dimorder(
      {3, 5, 2, 2},
      {0.2432, 0.8184, 0.6138, 0.9680, 0.2257, 0.5248, 0.8206, 0.1112, 0.2548,
       0.8766, 0.5361, 0.7357, 0.2799, 0.0393, 0.2715, 0.8513, 0.9655, 0.1079,
       0.6002, 0.1595, 0.2029, 0.4405, 0.2446, 0.4439, 0.3491, 0.7026, 0.6560,
       0.6590, 0.5346, 0.6970, 0.6982, 0.9217, 0.3866, 0.3179, 0.8456, 0.8529,
       0.6372, 0.7185, 0.4492, 0.2516, 0.2345, 0.8530, 0.8923, 0.3007, 0.7203,
       0.2924, 0.8560, 0.5546, 0.3961, 0.8963, 0.7695, 0.6909, 0.6978, 0.8416,
       0.3597, 0.0911, 0.7719, 0.8151, 0.4296, 0.5552},
      /*dim_order=*/{0, 2, 3, 1});

  Tensor expected = tf.make_with_dimorder(
      {3, 5, 2, 2},
      {0.2432, 0.5248, 0.5361, 0.8513, 0.8184, 0.8206, 0.7357, 0.9655, 0.6138,
       0.1112, 0.2799, 0.1079, 0.9680, 0.2548, 0.0393, 0.6002, 0.2257, 0.8766,
       0.2715, 0.1595, 0.2029, 0.7026, 0.6982, 0.8529, 0.4405, 0.6560, 0.9217,
       0.6372, 0.2446, 0.6590, 0.3866, 0.7185, 0.4439, 0.5346, 0.3179, 0.4492,
       0.3491, 0.6970, 0.8456, 0.2516, 0.2345, 0.2924, 0.7695, 0.0911, 0.8530,
       0.8560, 0.6909, 0.7719, 0.8923, 0.5546, 0.6978, 0.8151, 0.3007, 0.3961,
       0.8416, 0.4296, 0.7203, 0.8963, 0.3597, 0.5552});

  std::vector<int64_t> dim_order_vec = {0, 1, 2, 3};
  exec_aten::ArrayRef<int64_t> dim_order(
      dim_order_vec.data(), dim_order_vec.size());
  Tensor ret = op__to_dim_order_copy_out(
      /*self*/ x, /*non_blocking*/ false, /*dim_order*/ dim_order, out);

  EXPECT_TENSOR_EQ(out, expected);
  EXPECT_TENSOR_EQ(ret, expected);
}

TEST_F(OpToDimOrderCopyTest, PreserveChanneslLast) {
  TensorFactory<ScalarType::Float> tf;

  Tensor out = tf.full_channels_last({3, 5, 2, 2}, 0.0);
  Tensor x = tf.make_with_dimorder(
      {3, 5, 2, 2},
      {0.2432, 0.8184, 0.6138, 0.9680, 0.2257, 0.5248, 0.8206, 0.1112, 0.2548,
       0.8766, 0.5361, 0.7357, 0.2799, 0.0393, 0.2715, 0.8513, 0.9655, 0.1079,
       0.6002, 0.1595, 0.2029, 0.4405, 0.2446, 0.4439, 0.3491, 0.7026, 0.6560,
       0.6590, 0.5346, 0.6970, 0.6982, 0.9217, 0.3866, 0.3179, 0.8456, 0.8529,
       0.6372, 0.7185, 0.4492, 0.2516, 0.2345, 0.8530, 0.8923, 0.3007, 0.7203,
       0.2924, 0.8560, 0.5546, 0.3961, 0.8963, 0.7695, 0.6909, 0.6978, 0.8416,
       0.3597, 0.0911, 0.7719, 0.8151, 0.4296, 0.5552},
      /*dim_order=*/{0, 2, 3, 1});

  Tensor expected = tf.make_with_dimorder(
      {3, 5, 2, 2},
      {0.2432, 0.8184, 0.6138, 0.9680, 0.2257, 0.5248, 0.8206, 0.1112, 0.2548,
       0.8766, 0.5361, 0.7357, 0.2799, 0.0393, 0.2715, 0.8513, 0.9655, 0.1079,
       0.6002, 0.1595, 0.2029, 0.4405, 0.2446, 0.4439, 0.3491, 0.7026, 0.6560,
       0.6590, 0.5346, 0.6970, 0.6982, 0.9217, 0.3866, 0.3179, 0.8456, 0.8529,
       0.6372, 0.7185, 0.4492, 0.2516, 0.2345, 0.8530, 0.8923, 0.3007, 0.7203,
       0.2924, 0.8560, 0.5546, 0.3961, 0.8963, 0.7695, 0.6909, 0.6978, 0.8416,
       0.3597, 0.0911, 0.7719, 0.8151, 0.4296, 0.5552},
      /*dim_order=*/{0, 2, 3, 1});

  Tensor ret = op__to_dim_order_copy_out(
      /*self*/ x,
      /*non_blocking*/ false,
      /*dim_order*/ exec_aten::nullopt,
      out);

  EXPECT_TENSOR_EQ(out, expected);
  EXPECT_TENSOR_EQ(ret, expected);
}
