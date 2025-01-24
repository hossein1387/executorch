/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <executorch/kernels/test/FunctionHeaderWrapper.h> // Declares the operator
#include <executorch/kernels/test/TestUtil.h>
#include <executorch/kernels/test/supported_features.h>
#include <executorch/runtime/core/exec_aten/exec_aten.h>
#include <executorch/runtime/core/exec_aten/testing_util/tensor_factory.h>
#include <executorch/runtime/core/exec_aten/testing_util/tensor_util.h>

#include <gtest/gtest.h>

using namespace ::testing;
using executorch::aten::ScalarType;

class OpAvgPool2DOutTest : public OperatorTest {
 protected:
  executorch::aten::Tensor& op_avg_pool2d_out(
      const executorch::aten::Tensor& self,
      executorch::aten::ArrayRef<int64_t> kernel_size,
      executorch::aten::ArrayRef<int64_t> stride,
      executorch::aten::ArrayRef<int64_t> padding,
      bool ceil_mode,
      bool count_include_pad,
      executorch::aten::optional<int64_t> divisor_override,
      executorch::aten::Tensor& out) {
    return torch::executor::aten::avg_pool2d_outf(
        context_,
        self,
        kernel_size,
        stride,
        padding,
        ceil_mode,
        count_include_pad,
        divisor_override,
        out);
  }

  template <ScalarType DTYPE>
  void test_4d_dtype() {
    torch::executor::testing::TensorFactory<DTYPE> tf_dtype;

    executorch::aten::Tensor self = tf_dtype.make(
        {2, 3, 8, 8},
        {-81.875, -37.25,  -99.75,  -60.5,   -1.375,  25.625,  -54.25,
         -95.875, 48.0,    26.125,  -14.625, -5.0,    98.25,   1.75,
         5.875,   -71.25,  -26.5,   36.0,    32.375,  -87.875, -43.125,
         -40.875, -49.0,   -33.0,   16.0,    -1.875,  -48.0,   77.0,
         87.0,    58.25,   20.0,    10.875,  -37.875, -88.25,  97.75,
         -98.25,  -52.0,   -92.75,  -89.375, -31.5,   -1.625,  -50.0,
         -6.625,  -62.5,   -86.5,   -96.5,   85.0,    -94.375, -3.625,
         94.125,  12.25,   -33.875, -25.625, -20.625, -56.5,   -78.5,
         8.25,    38.875,  74.5,    -22.75,  14.125,  46.5,    -28.625,
         -52.0,   -36.75,  -3.75,   48.875,  2.875,   76.125,  -42.125,
         -71.125, 22.5,    9.25,    98.5,    41.5,    65.625,  -82.25,
         -85.75,  -20.75,  82.375,  3.25,    -74.5,   -14.25,  18.75,
         60.25,   -21.875, 7.0,     -44.0,   -97.0,   -71.625, 65.75,
         -89.5,   26.375,  14.125,  -99.75,  42.625,  -11.25,  56.625,
         -63.0,   -34.25,  -76.125, 69.25,   22.875,  43.875,  -7.125,
         -16.875, 93.875,  -48.25,  -57.375, -30.625, 11.75,   73.0,
         -96.5,   14.5,    50.375,  -65.875, 73.625,  88.625,  -11.375,
         41.75,   7.5,     62.125,  -1.25,   81.125,  -78.375, 62.875,
         75.125,  -34.75,  50.5,    -2.125,  64.125,  -74.375, 22.875,
         39.625,  -91.125, 96.125,  -31.75,  -70.875, 12.75,   6.5,
         30.125,  12.75,   -77.375, -56.5,   -24.5,   -59.25,  88.125,
         -26.375, -46.0,   62.75,   39.25,   -78.375, 67.875,  -57.75,
         15.5,    80.5,    -70.5,   96.875,  -18.625, -53.625, 70.75,
         70.375,  -68.125, -41.0,   -59.875, 74.625,  -82.125, 35.125,
         -24.25,  33.25,   45.875,  -74.75,  37.0,    -0.875,  86.625,
         -14.375, 45.25,   41.0,    -10.625, -49.875, 61.5,    -10.375,
         -39.0,   75.5,    -72.625, -29.0,   -53.875, -31.375, -57.375,
         79.5,    -25.0,   -83.0,   -49.875, -74.875, 50.25,   8.25,
         -35.5,   -0.875,  27.125,  85.875,  -95.625, 17.875,  45.25,
         -56.75,  60.5,    -61.75,  47.625,  50.5,    83.0,    -63.25,
         40.375,  -61.75,  -89.0,   -66.5,   6.0,     46.0,    -20.625,
         -15.625, -58.0,   88.25,   31.25,   -37.5,   -38.25,  68.5,
         11.625,  34.125,  -39.5,   84.25,   27.0,    -85.625, 7.625,
         86.375,  41.625,  28.0,    -41.125, 44.625,  19.625,  -43.125,
         -23.875, 54.5,    50.0,    -88.25,  29.0,    -77.5,   -82.125,
         -84.0,   8.75,    -32.875, 27.75,   -88.75,  -33.25,  -58.25,
         41.5,    -84.0,   -53.375, -85.5,   -15.625, 39.75,   29.375,
         -45.375, 96.5,    65.125,  34.875,  75.375,  -32.75,  -9.75,
         -55.0,   38.0,    31.125,  -35.0,   -74.375, -61.0,   -63.75,
         4.75,    88.75,   -83.25,  -19.75,  5.875,   88.375,  52.25,
         70.125,  -81.5,   -56.375, -98.375, 97.625,  88.375,  22.625,
         -100.0,  -75.0,   10.0,    41.0,    40.375,  12.0,    72.125,
         31.875,  -22.25,  -63.875, 10.5,    -81.25,  4.25,    43.5,
         -44.0,   71.5,    -29.625, -3.5,    -91.5,   45.375,  88.875,
         -93.125, -50.25,  -6.375,  -88.875, -2.375,  -17.75,  49.25,
         -14.75,  14.75,   -2.25,   51.25,   -57.875, 43.875,  87.0,
         86.25,   -95.125, 11.75,   -26.5,   29.875,  89.25,   -18.5,
         55.375,  74.25,   -64.0,   51.875,  78.0,    82.625,  -34.0,
         -0.875,  -69.375, -90.875, -83.5,   13.625,  46.875,  8.375,
         16.875,  96.75,   31.25,   45.625,  -2.625,  -71.0,   -62.375,
         31.25,   -23.5,   66.0,    51.375,  -45.25,  43.375,  49.5,
         12.625,  -73.875, 26.375,  29.0,    -86.5,   -55.375, 88.75,
         20.0,    90.0,    28.75,   -12.875, -37.75,  -1.875,  -28.125,
         96.75,   -66.75,  48.375,  -79.25,  8.0,     -14.25,  -8.0,
         51.75,   28.375,  32.0,    -50.875, 53.0,    -81.75});
    ::std::vector<int64_t> kernel_size_vec = {2, 3};
    executorch::aten::ArrayRef<int64_t> kernel_size = executorch::aten::ArrayRef<int64_t>(
        kernel_size_vec.data(), kernel_size_vec.size());
    ::std::vector<int64_t> stride_vec = {3, 2};
    executorch::aten::ArrayRef<int64_t> stride =
        executorch::aten::ArrayRef<int64_t>(stride_vec.data(), stride_vec.size());
    ::std::vector<int64_t> padding_vec = {1, 1};
    executorch::aten::ArrayRef<int64_t> padding =
        executorch::aten::ArrayRef<int64_t>(padding_vec.data(), padding_vec.size());
    bool ceil_mode = false;
    bool count_include_pad = true;
    executorch::aten::optional<int64_t> divisor_override;
    executorch::aten::Tensor out = tf_dtype.zeros({2, 3, 3, 4});
    executorch::aten::Tensor out_expected = tf_dtype.make(
        {2, 3, 3, 4},
        {-19.85416603088379,
         -32.91666793823242,
         -6.041666507720947,
         -20.75,
         3.9375,
         1.2708333730697632,
         8.395833015441895,
         -5.625,
         6.479166507720947,
         -7.770833492279053,
         -54.27083206176758,
         -43.58333206176758,
         -6.75,
         8.0,
         6.145833492279053,
         -15.125,
         -39.97916793823242,
         -27.5625,
         1.3541666269302368,
         -16.97916603088379,
         -17.66666603088379,
         4.625,
         -6.645833492279053,
         28.85416603088379,
         8.0625,
         -2.0625,
         -1.9791666269302368,
         7.4375,
         -12.270833015441895,
         6.791666507720947,
         16.20833396911621,
         8.041666984558105,
         15.875,
         -2.5208332538604736,
         -6.229166507720947,
         16.25,
         -20.79166603088379,
         -2.7291667461395264,
         -4.6875,
         18.6875,
         -2.75,
         -11.666666984558105,
         -22.54166603088379,
         -3.625,
         5.229166507720947,
         -17.54166603088379,
         -37.08333206176758,
         -20.10416603088379,
         4.020833492279053,
         3.9583332538604736,
         19.375,
         29.22916603088379,
         -11.729166984558105,
         -37.66666793823242,
         -1.5833333730697632,
         26.25,
         -24.72916603088379,
         -3.9583332538604736,
         -8.458333015441895,
         -24.60416603088379,
         21.8125,
         13.020833015441895,
         -18.3125,
         15.4375,
         9.625,
         -28.25,
         -26.5,
         2.9166667461395264,
         -16.1875,
         2.2708332538604736,
         46.1875,
         13.833333015441895});
    op_avg_pool2d_out(
        self,
        kernel_size,
        stride,
        padding,
        ceil_mode,
        count_include_pad,
        divisor_override,
        out);
    if constexpr (DTYPE == ScalarType::Half || DTYPE == ScalarType::BFloat16) {
      // op requires wide tolerance to pass test, but at least we
      // verify that it supports these dtypes.
      EXPECT_TENSOR_CLOSE_WITH_TOL(
          out,
          out_expected,
          5e-2,
          executorch::runtime::testing::internal::kDefaultAtol);
    } else {
      EXPECT_TENSOR_CLOSE(out, out_expected);
    }
  }

  template <ScalarType DTYPE>
  void test_4d_divisor_override_dtype() {
    torch::executor::testing::TensorFactory<ScalarType::Float> tfFloat;

    executorch::aten::Tensor self = tfFloat.make(
        {2, 3, 8, 8},
        {13.25,   87.125,  -61.875, 52.875,  -74.5,   37.5,    -62.125, 25.375,
         -34.375, 68.5,    87.875,  91.125,  -22.75,  96.875,  79.25,   38.125,
         6.5,     27.625,  58.875,  41.5,    -82.875, -77.875, -3.25,   22.0,
         -73.375, -56.875, 56.25,   -6.375,  -16.375, 40.0,    22.625,  61.0,
         -78.375, -87.5,   23.25,   -55.625, 92.75,   -90.0,   -55.625, 16.25,
         -46.5,   91.75,   74.0,    -45.0,   -34.875, -46.375, 38.0,    63.375,
         22.625,  16.25,   91.125,  68.375,  98.25,   34.0,    91.25,   89.375,
         1.0,     22.875,  -93.75,  7.5,     -81.375, 1.125,   -15.875, 16.75,
         29.25,   -14.0,   77.375,  -64.625, -22.375, -97.25,  -26.75,  9.125,
         -3.5,    -64.5,   35.625,  58.25,   4.125,   85.0,    -39.125, -98.25,
         -0.625,  -29.5,   -35.375, 90.375,  37.625,  57.875,  -59.625, -81.875,
         73.5,    -37.375, 56.75,   88.25,   -52.75,  -4.75,   80.625,  -38.375,
         -55.375, -76.625, -59.875, -41.5,   76.875,  45.375,  24.625,  57.5,
         58.25,   64.375,  -38.125, -39.875, 71.125,  2.5,     -14.75,  21.75,
         -8.375,  5.0,     71.625,  29.875,  79.375,  -13.125, -22.625, 96.875,
         7.5,     64.5,    1.0,     -40.75,  -56.0,   -94.75,  -89.5,   -71.25,
         78.125,  -5.875,  93.125,  -20.125, -73.875, -34.625, 51.5,    46.5,
         -88.25,  3.25,    26.0,    -32.375, -77.625, -51.875, 0.75,    85.75,
         -15.875, 84.625,  -44.75,  -87.625, 17.25,   -59.25,  40.25,   -0.5,
         14.625,  -80.125, -58.125, 17.25,   -7.5,    68.875,  -99.5,   -82.75,
         75.5,    -95.25,  -63.25,  55.25,   -17.125, 64.875,  -56.25,  39.75,
         -35.25,  18.0,    21.125,  -61.125, -35.5,   12.75,   -94.125, -17.125,
         -41.375, 22.625,  -69.25,  51.375,  7.25,    14.625,  70.25,   -63.5,
         -10.0,   53.625,  -46.75,  77.5,    -78.5,   -67.25,  -53.375, -33.5,
         23.375,  77.625,  64.125,  -56.375, -59.25,  -89.125, 16.25,   -2.125,
         -82.125, 72.75,   45.5,    -36.375, -1.125,  -7.25,   -44.5,   27.125,
         -1.75,   16.5,    71.0,    81.25,   4.125,   74.0,    45.0,    -4.75,
         68.125,  30.625,  11.625,  -88.5,   48.125,  36.875,  -32.0,   9.0,
         -67.5,   -17.125, -87.5,   29.875,  23.875,  -33.0,   -6.625,  0.5,
         28.25,   54.75,   87.5,    28.0,    -48.5,   -98.0,   -55.0,   62.25,
         -93.125, -41.5,   94.25,   -28.375, -23.75,  -50.625, 90.375,  -91.0,
         -67.75,  -11.625, 24.5,    49.5,    21.0,    70.25,   66.0,    69.75,
         43.25,   -14.875, -93.875, -15.75,  94.5,    69.125,  -67.625, 15.875,
         -26.875, 41.625,  -64.375, 23.0,    13.375,  78.5,    -88.875, 17.625,
         -52.375, 55.25,   41.875,  62.625,  29.0,    53.875,  -93.625, -93.375,
         -70.375, -56.125, 9.0,     60.625,  4.375,   66.625,  -46.25,  43.125,
         93.625,  85.125,  -91.375, 49.375,  40.25,   -19.75,  13.0,    69.5,
         -42.375, 47.25,   7.0,     -93.625, -80.0,   -57.875, -94.875, 9.375,
         -2.5,    -42.25,  -8.0,    92.5,    6.875,   -11.0,   80.625,  27.125,
         -75.875, -57.375, -36.625, 17.375,  33.125,  -49.0,   -84.0,   -45.5,
         -9.625,  -16.125, 8.75,    -1.0,    -16.625, 2.5,     -56.0,   -52.0,
         85.0,    63.5,    -50.625, 7.375,   -90.0,   -75.875, -82.0,   82.75,
         16.125,  -35.25,  -42.875, -1.375,  39.875,  -39.625, -66.875, 29.875,
         -80.75,  66.5,    -50.125, 13.5,    -77.875, 6.125,   -91.625, -44.125,
         -50.125, 59.625,  -32.5,   -2.0,    -83.375, 21.0,    49.875,  48.125,
         -93.375, -54.875, -61.125, 96.375,  91.25,   -2.375,  -33.25,  48.125,
         -58.125, -50.75,  -50.875, 8.375,   35.625,  -72.5,   -76.125, -33.25,
         -18.75,  -71.0,   76.625,  -11.25,  -3.0,    -38.625, -66.375, -25.0});
    ::std::vector<int64_t> kernel_size_vec = {2, 3};
    executorch::aten::ArrayRef<int64_t> kernel_size = executorch::aten::ArrayRef<int64_t>(
        kernel_size_vec.data(), kernel_size_vec.size());
    ::std::vector<int64_t> stride_vec = {3, 2};
    executorch::aten::ArrayRef<int64_t> stride =
        executorch::aten::ArrayRef<int64_t>(stride_vec.data(), stride_vec.size());
    ::std::vector<int64_t> padding_vec = {1, 1};
    executorch::aten::ArrayRef<int64_t> padding =
        executorch::aten::ArrayRef<int64_t>(padding_vec.data(), padding_vec.size());
    bool ceil_mode = false;
    bool count_include_pad = true;
    executorch::aten::optional<int64_t> divisor_override =
        executorch::aten::optional<int64_t>(10);
    executorch::aten::Tensor out = tfFloat.zeros({2, 3, 3, 4});
    executorch::aten::Tensor out_expected = tfFloat.make(
        {2, 3, 3, 4},
        {10.037500381469727,
         7.8125,
         1.587499976158142,
         0.07500000298023224,
         -9.612500190734863,
         12.100000381469727,
         -10.199999809265137,
         6.449999809265137,
         8.412500381469727,
         29.649999618530273,
         7.4375,
         26.962499618530273,
         1.524999976158142,
         -0.125,
         -18.424999237060547,
         -11.487500190734863,
         0.6000000238418579,
         13.3125,
         21.662500381469727,
         -4.612500190734863,
         11.925000190734863,
         9.287500381469727,
         12.987500190734863,
         7.0625,
         7.224999904632568,
         6.712500095367432,
         -12.862500190734863,
         6.337500095367432,
         0.32499998807907104,
         -16.875,
         -5.099999904632568,
         -13.287500381469727,
         -3.5999999046325684,
         -1.725000023841858,
         -1.0625,
         -7.712500095367432,
         10.100000381469727,
         8.537500381469727,
         -20.475000381469727,
         -7.5,
         11.350000381469727,
         12.25,
         15.587499618530273,
         12.8125,
         -5.162499904632568,
         19.462499618530273,
         -22.125,
         -14.199999809265137,
         2.8375000953674316,
         -12.449999809265137,
         14.787500381469727,
         1.7374999523162842,
         -12.362500190734863,
         17.325000762939453,
         27.712499618530273,
         -6.962500095367432,
         -3.987499952316284,
         0.2874999940395355,
         -14.3125,
         -4.662499904632568,
         -2.575000047683716,
         -0.8374999761581421,
         -1.5125000476837158,
         -10.550000190734863,
         -3.3375000953674316,
         -4.962500095367432,
         -5.9375,
         -20.625,
         -25.712499618530273,
         -11.287500381469727,
         15.675000190734863,
         -16.9375});
    op_avg_pool2d_out(
        self,
        kernel_size,
        stride,
        padding,
        ceil_mode,
        count_include_pad,
        divisor_override,
        out);
    EXPECT_TENSOR_CLOSE(out, out_expected);
  }

  template <ScalarType DTYPE>
  void test_4d_ceil_mode_no_include_padding_dtype() {
    torch::executor::testing::TensorFactory<ScalarType::Float> tfFloat;

    executorch::aten::Tensor self = tfFloat.make(
        {2, 3, 14, 12},
        {26.375,  -17.0,   63.5,    83.0,    21.375,  -46.5,   -69.125,
         99.875,  -67.125, -76.0,   -1.125,  -2.625,  -48.0,   -1.5,
         -92.75,  95.5,    91.5,    32.625,  -35.25,  12.125,  55.0,
         -64.875, -53.125, -20.0,   -5.875,  -0.25,   -3.5,    4.875,
         81.625,  92.25,   98.25,   -47.375, 78.875,  -97.5,   10.375,
         55.75,   65.375,  97.375,  -93.625, 51.5,    62.75,   -37.625,
         1.875,   -73.125, 78.75,   -62.25,  20.625,  89.375,  16.75,
         43.0,    98.375,  17.0,    40.125,  -78.0,   -24.375, 9.875,
         2.0,     -57.125, -65.375, -86.0,   -98.375, 68.75,   70.375,
         16.25,   -24.625, 12.0,    -55.0,   -17.0,   81.375,  12.0,
         62.875,  -79.0,   46.0,    -12.625, -42.0,   3.125,   -6.0,
         88.5,    -42.25,  82.625,  -96.875, 51.375,  41.875,  36.125,
         -27.375, 33.375,  -81.75,  6.875,   -39.5,   -57.75,  24.0,
         35.375,  -22.875, 96.375,  -65.0,   -67.0,   -64.875, 86.625,
         70.625,  16.5,    -45.0,   55.375,  88.5,    -42.0,   -38.5,
         -71.5,   -66.125, -30.5,   59.375,  -8.25,   8.375,   85.5,
         20.625,  -58.875, -35.5,   -80.125, 93.25,   -15.5,   40.125,
         54.25,   -40.25,  85.125,  61.375,  84.25,   58.5,    -60.375,
         -58.375, -81.5,   -80.125, -84.0,   31.75,   55.125,  8.125,
         -56.625, -7.0,    12.625,  75.75,   -88.125, -5.375,  60.0,
         21.25,   62.125,  2.125,   -3.375,  -26.75,  44.25,   -96.5,
         -87.5,   -36.625, -36.125, 1.125,   -42.0,   -48.125, 24.0,
         80.375,  -38.5,   50.75,   -0.625,  35.625,  31.875,  -32.75,
         70.375,  -41.0,   -20.75,  27.5,    60.75,   -10.5,   54.875,
         68.25,   40.0,    0.875,   39.375,  67.375,  31.25,   79.25,
         37.625,  33.0,    10.625,  -47.5,   -21.0,   -2.625,  81.0,
         -42.25,  23.625,  10.375,  1.625,   11.0,    14.625,  19.5,
         -36.75,  99.875,  86.125,  -70.25,  26.375,  -86.375, -21.0,
         -25.0,   51.75,   -11.375, 58.875,  74.75,   77.625,  90.0,
         -40.5,   -32.625, 40.5,    33.0,    -80.625, -33.875, 38.5,
         30.625,  -55.375, 53.875,  -1.25,   75.0,    -55.375, 71.125,
         -34.625, -29.75,  -92.75,  -34.125, -17.25,  -72.0,   79.5,
         81.5,    -14.375, -35.625, -38.875, 97.625,  -3.0,    -31.125,
         31.25,   13.5,    -3.625,  -64.0,   72.0,    64.5,    6.875,
         -4.875,  19.875,  50.25,   80.75,   76.875,  91.75,   -26.375,
         94.125,  19.0,    4.375,   -48.625, -69.375, 83.0,    6.75,
         -85.5,   -64.75,  -99.875, -88.375, 36.875,  24.375,  96.0,
         24.25,   -10.0,   39.75,   -8.75,   -76.25,  -20.75,  -96.25,
         75.5,    23.625,  50.875,  76.375,  -74.25,  2.0,     57.125,
         31.875,  -67.375, 78.375,  -25.125, -27.75,  -82.375, 37.25,
         84.875,  8.25,    53.75,   -14.375, -68.875, -7.875,  -5.25,
         50.25,   -92.75,  -90.625, 64.0,    -32.0,   -20.75,  47.625,
         35.5,    -17.875, 44.0,    35.625,  -38.0,   -80.5,   74.375,
         -11.125, 85.625,  47.25,   -92.375, 2.375,   -5.375,  31.875,
         -83.5,   -41.0,   -72.25,  49.75,   72.125,  -92.875, -39.125,
         -19.75,  -79.25,  90.375,  -67.625, 71.375,  -9.75,   -80.125,
         -56.875, 27.0,    93.75,   55.75,   85.0,    -60.5,   -11.625,
         11.5,    66.0,    7.875,   72.125,  -15.25,  60.25,   58.0,
         -16.125, -41.875, -16.875, 32.25,   -69.75,  81.5,    -65.125,
         -28.0,   59.25,   2.375,   -8.25,   60.0,    -69.625, 3.0,
         59.875,  93.25,   41.875,  -38.625, -86.375, -18.25,  86.375,
         29.875,  5.375,   -63.0,   -40.875, 42.0,    51.125,  -35.125,
         3.125,   31.875,  29.75,   6.5,     -12.125, -86.875, 62.5,
         6.75,    -43.125, 37.0,    58.625,  -89.5,   -62.125, -21.0,
         -95.875, -42.125, -42.125, -52.125, -22.875, 78.375,  -53.375,
         -85.625, 19.375,  -28.0,   47.375,  63.875,  21.75,   85.75,
         74.375,  27.75,   46.0,    41.375,  55.125,  -4.375,  -59.375,
         -93.375, -95.375, 98.125,  -30.875, 69.25,   69.75,   -78.375,
         71.125,  -71.25,  -73.75,  69.0,    -68.125, -49.0,   50.0,
         51.25,   -9.625,  -4.625,  -85.75,  -99.125, -99.875, -65.5,
         28.75,   95.25,   79.625,  16.5,    37.125,  -62.75,  -62.0,
         65.25,   95.5,    -15.5,   -42.375, 35.75,   18.5,    89.25,
         35.5,    -90.375, 3.625,   -2.875,  55.25,   63.5,    1.25,
         2.625,   -18.375, -69.0,   9.5,     -31.375, -44.375, 35.125,
         -37.0,   20.875,  98.5,    -43.0,   -35.625, -8.125,  40.25,
         -89.125, 41.5,    -100.0,  -39.625, -76.5,   39.0,    6.125,
         22.75,   83.75,   -62.375, 31.75,   -86.625, -26.125, 87.25,
         -69.0,   47.0,    99.125,  73.5,    24.875,  84.875,  13.625,
         -57.875, 67.125,  -10.375, -67.75,  -92.0,   52.125,  66.875,
         -62.25,  61.125,  -46.875, -35.875, 35.625,  -91.5,   -95.25,
         11.625,  51.125,  38.875,  -1.875,  1.75,    11.125,  61.125,
         -58.25,  60.125,  -62.25,  -83.75,  -41.375, -14.75,  48.25,
         -83.25,  -91.75,  13.625,  -12.75,  30.25,   -54.125, 60.0,
         -25.875, -45.0,   -33.75,  85.875,  46.0,    -74.75,  -93.375,
         64.25,   -31.5,   -30.125, 85.25,   87.0,    -1.75,   -55.5,
         32.0,    -67.25,  3.125,   -66.0,   16.75,   -62.25,  -59.375,
         91.375,  -49.625, -74.125, 52.5,    -9.125,  70.5,    69.375,
         -86.25,  -67.75,  82.625,  37.5,    1.5,     73.125,  -68.875,
         -1.5,    42.875,  94.125,  98.125,  9.5,     99.375,  35.0,
         -17.625, -22.375, 75.0,    67.5,    47.25,   -12.625, 30.375,
         -16.625, 97.125,  86.5,    61.375,  -94.875, 99.125,  62.25,
         -83.875, -65.0,   69.125,  84.125,  -84.75,  90.25,   -77.0,
         29.625,  58.375,  8.625,   76.875,  67.375,  -89.875, 62.5,
         51.0,    77.75,   -0.375,  45.5,    86.875,  -49.5,   -71.375,
         -32.75,  -76.375, -29.375, 69.25,   -13.375, 95.875,  -58.375,
         12.75,   95.375,  83.375,  51.0,    40.75,   58.125,  -26.875,
         29.375,  81.625,  37.25,   -91.375, 1.25,    20.625,  -46.5,
         24.875,  12.25,   -73.25,  -1.625,  -69.625, -79.0,   60.125,
         -0.125,  41.875,  -24.375, 66.5,    9.25,    -72.625, -66.375,
         -29.5,   -51.5,   28.75,   -79.75,  16.125,  -55.5,   80.25,
         -14.25,  37.875,  98.625,  -6.875,  67.75,   90.875,  -80.375,
         71.625,  58.875,  -63.0,   -71.625, 33.5,    -30.125, -33.25,
         94.75,   -30.5,   -45.75,  62.875,  67.75,   -22.125, 22.0,
         48.75,   -35.75,  83.625,  -53.375, 55.875,  53.875,  38.0,
         0.375,   15.125,  -29.25,  55.0,    -79.875, 40.5,    65.25,
         -67.625, -43.125, -47.625, -81.375, 40.5,    -94.375, 26.0,
         70.0,    50.25,   53.0,    26.75,   0.875,   4.0,     17.625,
         -74.25,  -28.5,   56.5,    71.25,   -67.0,   -9.875,  15.25,
         -38.125, -61.375, -22.25,  14.625,  -47.25,  -7.125,  34.75,
         10.625,  -10.25,  0.5,     77.5,    36.0,    -51.875, -59.5,
         13.375,  45.75,   -71.625, -29.75,  -90.75,  -38.25,  52.0,
         77.875,  21.125,  56.625,  17.875,  -32.625, -39.0,   -68.75,
         -82.875, -1.25,   96.75,   -91.375, -22.5,   27.5,    -30.125,
         -23.875, -76.0,   -93.5,   37.5,    78.375,  -12.5,   -84.875,
         -21.5,   29.25,   -4.375,  -48.875, 90.75,   -77.375, -23.625,
         -5.75,   -75.125, -92.125, 9.0,     96.5,    76.125,  47.5,
         15.875,  21.0,    18.625,  94.25,   8.5,     -67.75,  69.25,
         -72.0,   -14.5,   90.5,    -31.875, -20.875, -93.625, 42.875,
         -54.375, -70.5,   -75.375, -6.75,   -48.875, 21.75,   1.75,
         45.5,    60.75,   -9.75,   13.625,  -87.125, 12.625,  71.125,
         -46.5,   -31.25,  -15.375, -42.75,  13.0,    26.25,   61.625,
         81.0,    18.125,  84.5,    31.75,   -14.125, -80.875, 18.875,
         51.125,  0.625,   41.5,    -81.625, 32.625,  -19.375, 76.875,
         54.0,    -40.375, -78.25,  48.0,    77.5,    21.0,    71.25,
         34.875,  73.25,   71.75,   87.25,   -89.375, 93.625,  64.25,
         -81.375, -15.0,   -58.125, 64.75,   -49.125, 72.125,  -62.0,
         0.125,   -56.0,   -6.875,  98.25,   -24.75,  48.375,  -85.25,
         -62.25,  98.25,   12.75,   -67.25,  72.125,  9.625,   8.5,
         -43.375, 15.25,   -50.875, 7.875,   -21.625, 97.25,   48.5,
         33.25,   -15.125, 41.125,  69.25,   -35.875, 64.5,    98.75,
         -78.125, 44.625,  75.75,   41.625,  -39.375, -4.875,  87.5,
         -67.25,  -23.25,  -52.875, -44.875, -70.75,  -63.75,  86.25,
         -44.0,   -92.375, -22.75,  -70.75,  -84.875, -98.125, -86.75,
         35.25,   -77.5,   -78.0,   -62.5,   -88.5,   58.25,   -65.125,
         70.25,   -5.875,  11.25,   47.125,  10.25,   -91.75,  -50.125,
         45.125,  49.625,  57.25,   26.375,  -74.75,  70.25,   38.625,
         51.25,   12.375,  -79.875, -76.25,  12.0,    -15.625, 50.25,
         72.125,  30.5,    -68.625, -22.875, 5.125,   -59.5,   -82.75,
         -64.5,   46.75,   -87.75,  17.875,  50.875,  69.625,  24.125,
         -25.0,   -25.0,   -5.0,    21.0,    51.875,  91.625,  50.0,
         77.375,  -51.0,   64.25,   41.0,    -20.875, -89.75,  -74.75,
         48.375,  -41.75,  99.25,   36.125,  10.75,   -49.625, 57.375,
         19.625,  -43.75,  16.625,  -48.375, 84.375,  31.125,  93.375,
         76.125,  -12.25,  98.5,    15.0,    71.625,  -87.75,  98.625,
         -74.5,   -42.75,  -73.5,   -30.875, -44.625, 70.5,    96.0,
         21.5,    48.375,  12.75,   -65.625, 56.5,    97.375,  29.5,
         -19.375, 94.625,  11.25,   -0.375,  -96.5,   -48.75,  -40.375,
         98.0,    -43.0,   20.75,   50.375,  97.875,  -72.875, 92.125,
         98.5,    -74.0,   -57.875, 56.375,  -34.0,   6.125,   -66.125,
         71.75,   -67.125, -79.0,   -85.875, -88.625, 97.375,  16.875,
         -18.5,   -75.125, 22.625,  -21.125, 3.875,   57.25,   97.75,
         -11.0,   -46.875, 16.75,   -69.25,  99.75,   -68.25,  99.75,
         -17.25,  -3.125,  34.25,   -54.125, -93.125, 65.0,    -76.375,
         -20.625, -77.875, -65.625, -79.875, 28.75,   58.25,   -25.25});
    ::std::vector<int64_t> kernel_size_vec = {4, 2};
    executorch::aten::ArrayRef<int64_t> kernel_size = executorch::aten::ArrayRef<int64_t>(
        kernel_size_vec.data(), kernel_size_vec.size());
    ::std::vector<int64_t> stride_vec = {1, 2};
    executorch::aten::ArrayRef<int64_t> stride =
        executorch::aten::ArrayRef<int64_t>(stride_vec.data(), stride_vec.size());
    ::std::vector<int64_t> padding_vec = {1, 1};
    executorch::aten::ArrayRef<int64_t> padding =
        executorch::aten::ArrayRef<int64_t>(padding_vec.data(), padding_vec.size());
    bool ceil_mode = true;
    bool count_include_pad = false;
    executorch::aten::optional<int64_t> divisor_override;
    executorch::aten::Tensor out = tfFloat.zeros({2, 3, 13, 7});
    executorch::aten::Tensor out_expected = tfFloat.make(
        {2, 3, 13, 7},
        {-9.166666984558105,
         -8.583333015441895,
         62.97916793823242,
         12.041666984558105,
         21.89583396911621,
         -47.04166793823242,
         11.041666984558105,
         9.46875,
         -5.96875,
         61.515625,
         4.5625,
         17.125,
         -40.484375,
         30.625,
         7.0625,
         5.890625,
         55.609375,
         6.21875,
         14.515625,
         -46.15625,
         9.78125,
         -5.53125,
         35.0625,
         31.1875,
         1.171875,
         14.171875,
         -22.046875,
         -4.96875,
         7.4375,
         28.703125,
         20.015625,
         -16.859375,
         8.453125,
         0.5,
         -9.875,
         -15.75,
         22.1875,
         1.65625,
         -16.609375,
         9.3125,
         9.625,
         -48.96875,
         -36.15625,
         24.171875,
         -9.046875,
         14.171875,
         -2.234375,
         7.734375,
         -35.09375,
         3.28125,
         6.796875,
         5.265625,
         7.75,
         -8.640625,
         1.453125,
         -1.78125,
         -18.28125,
         31.9375,
         23.46875,
         -12.875,
         -27.0625,
         -16.734375,
         2.96875,
         -9.40625,
         30.03125,
         38.59375,
         -20.34375,
         -18.46875,
         -12.625,
         18.875,
         0.125,
         3.84375,
         26.640625,
         -42.703125,
         -19.671875,
         17.625,
         16.875,
         -2.03125,
         8.203125,
         13.265625,
         -27.234375,
         -20.46875,
         20.828125,
         17.03125,
         10.708333015441895,
         -13.479166984558105,
         -6.104166507720947,
         -16.52083396911621,
         -0.3541666567325592,
         36.47916793823242,
         4.333333492279053,
         -1.5416666269302368,
         3.2708332538604736,
         15.791666984558105,
         27.25,
         39.72916793823242,
         32.3125,
         8.208333015441895,
         -9.3125,
         11.640625,
         -2.46875,
         29.078125,
         29.609375,
         33.453125,
         -7.6875,
         -8.59375,
         -1.515625,
         -31.671875,
         4.109375,
         40.90625,
         31.8125,
         -12.15625,
         16.46875,
         -10.625,
         -30.328125,
         -5.921875,
         53.703125,
         24.171875,
         -28.71875,
         46.59375,
         16.578125,
         -16.40625,
         3.171875,
         31.46875,
         4.921875,
         -16.90625,
         33.375,
         -13.1875,
         -8.53125,
         9.578125,
         33.4375,
         -0.421875,
         -22.125,
         10.40625,
         -7.734375,
         16.640625,
         21.0,
         20.703125,
         1.390625,
         7.1875,
         -20.28125,
         -17.234375,
         26.3125,
         37.203125,
         -6.765625,
         -0.5,
         14.78125,
         -56.03125,
         -40.265625,
         11.546875,
         33.453125,
         2.03125,
         -2.5,
         -7.03125,
         -16.0625,
         -10.375,
         12.34375,
         18.03125,
         -6.203125,
         -20.53125,
         24.46875,
         7.15625,
         -24.28125,
         -9.34375,
         20.609375,
         -5.890625,
         -33.21875,
         11.625,
         36.875,
         7.078125,
         -33.625,
         22.546875,
         14.515625,
         -25.953125,
         13.5625,
         80.08333587646484,
         13.875,
         -36.04166793823242,
         16.20833396911621,
         15.0,
         -34.20833206176758,
         44.91666793823242,
         -42.20833206176758,
         16.20833396911621,
         10.9375,
         -7.833333492279053,
         15.625,
         0.8333333134651184,
         1.25,
         -42.4375,
         24.109375,
         -10.75,
         -20.484375,
         1.1875,
         -8.75,
         20.53125,
         -51.75,
         23.171875,
         -3.640625,
         -11.828125,
         17.296875,
         1.203125,
         15.875,
         -20.5625,
         7.34375,
         -44.125,
         12.203125,
         26.15625,
         -4.109375,
         13.8125,
         -28.78125,
         -4.1875,
         -40.0,
         9.703125,
         15.5625,
         -25.9375,
         -4.25,
         -10.8125,
         5.71875,
         -14.34375,
         8.71875,
         46.1875,
         -23.796875,
         -14.90625,
         7.15625,
         29.59375,
         -27.609375,
         4.5625,
         34.265625,
         -34.984375,
         -42.5,
         -4.25,
         28.09375,
         -4.25,
         11.078125,
         7.0625,
         -30.0625,
         -46.96875,
         24.5625,
         10.53125,
         -9.0625,
         9.484375,
         21.03125,
         -12.046875,
         -37.125,
         39.1875,
         -14.078125,
         5.8125,
         38.796875,
         -4.59375,
         2.28125,
         -63.0,
         11.5625,
         -14.796875,
         16.515625,
         21.90625,
         -19.671875,
         -6.203125,
         -32.96875,
         18.90625,
         -5.34375,
         25.78125,
         7.21875,
         -28.09375,
         -17.234375,
         1.375,
         11.375,
         16.14583396911621,
         40.625,
         4.8125,
         -41.02083206176758,
         -13.833333015441895,
         10.541666984558105,
         -74.66666412353516,
         -25.45833396911621,
         -2.0208332538604736,
         9.0625,
         -14.125,
         26.125,
         -12.375,
         -72.9375,
         -4.078125,
         7.8125,
         -2.0,
         6.53125,
         33.046875,
         15.5625,
         -43.375,
         0.6875,
         23.4375,
         1.59375,
         17.109375,
         49.484375,
         19.40625,
         -48.40625,
         24.5,
         12.53125,
         -0.78125,
         24.953125,
         47.96875,
         33.21875,
         -29.75,
         48.21875,
         5.109375,
         30.78125,
         25.171875,
         35.15625,
         36.9375,
         -21.0,
         19.984375,
         2.765625,
         44.265625,
         21.5625,
         38.5,
         22.28125,
         -15.21875,
         25.296875,
         -0.1875,
         28.671875,
         16.609375,
         20.1875,
         -11.375,
         8.09375,
         -13.453125,
         25.921875,
         11.703125,
         25.390625,
         8.734375,
         -33.34375,
         -6.9375,
         -37.859375,
         24.421875,
         3.859375,
         36.8125,
         11.671875,
         7.21875,
         -18.84375,
         -8.328125,
         0.609375,
         -0.40625,
         30.984375,
         -14.65625,
         12.75,
         -16.4375,
         -8.65625,
         -12.625,
         14.640625,
         47.9375,
         -14.5,
         34.84375,
         -23.34375,
         6.8125,
         -6.90625,
         -1.390625,
         22.34375,
         -3.859375,
         48.71875,
         -13.958333015441895,
         17.58333396911621,
         -2.6458332538604736,
         -12.854166984558105,
         7.041666507720947,
         -15.291666984558105,
         34.66666793823242,
         45.875,
         10.75,
         -17.02083396911621,
         -6.145833492279053,
         -32.60416793823242,
         23.64583396911621,
         3.7916667461395264,
         39.6875,
         17.375,
         -21.71875,
         -23.5625,
         -12.515625,
         3.5,
         9.71875,
         14.65625,
         -8.015625,
         -32.171875,
         -18.03125,
         -12.96875,
         -9.359375,
         14.25,
         39.8125,
         -17.78125,
         -31.828125,
         -24.34375,
         5.15625,
         -1.484375,
         19.375,
         25.09375,
         -2.953125,
         -25.875,
         -31.921875,
         27.546875,
         -17.515625,
         10.625,
         6.21875,
         -30.5,
         -23.875,
         -10.03125,
         28.890625,
         -2.796875,
         -18.03125,
         16.90625,
         -14.9375,
         -22.703125,
         -21.984375,
         53.171875,
         6.484375,
         15.3125,
         2.15625,
         -14.1875,
         -3.84375,
         -6.328125,
         25.46875,
         5.75,
         23.5625,
         -12.59375,
         -30.8125,
         8.28125,
         17.75,
         36.265625,
         19.796875,
         36.25,
         17.0625,
         -24.625,
         16.0625,
         17.6875,
         15.25,
         11.453125,
         82.59375,
         7.71875,
         -32.3125,
         26.390625,
         14.59375,
         14.484375,
         -5.296875,
         65.28125,
         -12.9375,
         -22.15625,
         35.859375,
         11.59375,
         34.40625,
         -8.90625,
         76.46875,
         -3.7916667461395264,
         -24.5,
         31.39583396911621,
         -2.2291667461395264,
         21.70833396911621,
         -11.520833015441895,
         70.75,
         -68.125,
         22.0625,
         -17.95833396911621,
         -11.270833015441895,
         -36.33333206176758,
         -46.97916793823242,
         -34.54166793823242,
         -38.6875,
         27.0,
         -14.03125,
         2.78125,
         -35.6875,
         -43.265625,
         -29.8125,
         -6.59375,
         24.78125,
         -25.75,
         -14.34375,
         -42.78125,
         -36.171875,
         -7.65625,
         22.0625,
         31.21875,
         -17.609375,
         7.109375,
         -1.734375,
         -14.96875,
         -0.90625,
         53.75,
         37.515625,
         -38.8125,
         7.265625,
         8.015625,
         -2.09375,
         2.15625,
         46.25,
         23.671875,
         -33.75,
         11.59375,
         24.4375,
         20.125,
         23.96875,
         11.75,
         13.859375,
         -36.84375,
         8.953125,
         63.65625,
         33.984375,
         22.6875,
         -17.375,
         21.375,
         -29.328125,
         20.1875,
         33.609375,
         6.921875,
         59.9375,
         -44.1875,
         27.75,
         -5.640625,
         43.1875,
         0.203125,
         14.578125,
         47.125,
         -65.625,
         31.71875,
         -30.75,
         28.71875,
         -7.984375,
         -6.171875,
         23.9375,
         -42.71875,
         48.078125,
         -23.453125,
         31.59375,
         -24.859375,
         -4.59375,
         19.96875,
         -17.75,
         10.4375,
         -26.140625,
         6.046875,
         -30.9375,
         17.421875,
         -10.84375,
         -9.333333015441895,
         2.0625,
         -39.02083206176758,
         -23.70833396911621,
         -19.27083396911621,
         19.5,
         -16.5});
    op_avg_pool2d_out(
        self,
        kernel_size,
        stride,
        padding,
        ceil_mode,
        count_include_pad,
        divisor_override,
        out);
    EXPECT_TENSOR_CLOSE(out, out_expected);
  }
};

TEST_F(OpAvgPool2DOutTest, SanityCheck4D){
#define TEST_ENTRY(ctype, dtype) test_4d_dtype<ScalarType::dtype>();
    ET_FORALL_FLOATHBF16_TYPES(TEST_ENTRY)
#undef TEST_ENTRY
}

TEST_F(OpAvgPool2DOutTest, SanityCheck4DDivisorOverride) {
#define TEST_ENTRY(ctype, dtype) \
  test_4d_divisor_override_dtype<ScalarType::dtype>();
  ET_FORALL_FLOATHBF16_TYPES(TEST_ENTRY)
#undef TEST_ENTRY
}

TEST_F(OpAvgPool2DOutTest, SanityCheck4DCeilModeNoIncludePadding) {
#define TEST_ENTRY(ctype, dtype) \
  test_4d_ceil_mode_no_include_padding_dtype<ScalarType::dtype>();
  ET_FORALL_FLOATHBF16_TYPES(TEST_ENTRY)
#undef TEST_ENTRY
}
