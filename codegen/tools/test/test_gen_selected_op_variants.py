#!/usr/bin/env fbpython
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import os
import tempfile

import executorch.codegen.tools.gen_selected_op_variants as gen_selected_op_variants
import expecttest


class TestGenSelectedMobileOpsHeader(expecttest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp_dir.cleanup)
        self.selected_ops_yaml = os.path.join(
            self.temp_dir.name, "selected_operators.yaml"
        )
        with open(self.selected_ops_yaml, "w") as f:
            f.write(
                """
include_all_non_op_selectives: False
include_all_operators: False
debug_info:
  - model1@v100
  - model2@v50
operators:
  aten::add:
    is_root_operator: Yes
    is_used_for_training: Yes
    include_all_overloads: No
  aten::add.int:
    is_root_operator: No
    is_used_for_training: No
    include_all_overloads: Yes
kernel_metadata: {}
et_kernel_metadata:
  aten::add.out:
    # A list of different kernel keys (tensors with dtype-enum/dim-order) combinations used in model
      - v1/6;0,1|6;0,1|6;0,1|6;0,1  # Float, 0, 1
      - v1/3;0,1|3;0,1|3;0,1|3;0,1  # Int, 0, 1
  aten::mul.out:
      - v1/6;0,1|6;0,1|6;0,1|6;0,1  # Float, 0, 1
  aten::sub.out:
      - default
build_features: []
custom_classes: []
            """
            )

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_generates_correct_header(self) -> None:
        gen_selected_op_variants.write_selected_op_variants(
            os.path.join(self.temp_dir.name, "selected_operators.yaml"),
            self.temp_dir.name,
        )
        with open(
            os.path.join(self.temp_dir.name, "selected_op_variants.h"), "r"
        ) as result:
            self.assertExpectedInline(
                result.read(),
                """#pragma once
/**
 * Generated by executorch/codegen/tools/gen_selected_op_variants.py
 */

inline constexpr bool should_include_kernel_dtype(
  const char *operator_name,
  executorch::aten::ScalarType scalar_type
) {
  return ((executorch::aten::string_view(operator_name).compare("add.out") == 0)
        && (scalar_type == executorch::aten::ScalarType::Float || scalar_type == executorch::aten::ScalarType::Int))
 || ((executorch::aten::string_view(operator_name).compare("mul.out") == 0)
        && (scalar_type == executorch::aten::ScalarType::Float))
 || ((executorch::aten::string_view(operator_name).compare("sub.out") == 0)
        && (true));
}
""",
            )


class TestGenSelectedMobileOpsHeader_Empty(expecttest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp_dir.cleanup)
        self.selected_ops_yaml = os.path.join(
            self.temp_dir.name, "selected_operators.yaml"
        )
        with open(self.selected_ops_yaml, "w") as f:
            f.write(
                """
build_features: []
custom_classes: []
et_kernel_metadata: {}
include_all_non_op_selectives: false
include_all_operators: true
kernel_metadata: {}
operators: {}
                """
            )

    def tearDown(self):
        self.temp_dir.cleanup()

    def test_generates_correct_header(self) -> None:
        gen_selected_op_variants.write_selected_op_variants(
            os.path.join(self.temp_dir.name, "selected_operators.yaml"),
            self.temp_dir.name,
        )
        with open(
            os.path.join(self.temp_dir.name, "selected_op_variants.h"), "r"
        ) as result:
            self.assertExpectedInline(
                result.read(),
                """#pragma once
/**
 * Generated by executorch/codegen/tools/gen_selected_op_variants.py
 */

inline constexpr bool should_include_kernel_dtype(
  const char *operator_name,
  executorch::aten::ScalarType scalar_type
) {
  return true;
}
""",
            )
