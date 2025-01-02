/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#version 450 core

#define PRECISION ${PRECISION}

#define VEC4_T ${texel_type(DTYPE)}

#define TILE_SIZE ${TILE_SIZE}

#define op(X, A, B) ${OPERATOR}

#include "indexing_utils.h"

layout(std430) buffer;

${layout_declare_tensor(0, "w", "t_out", DTYPE, "texture3d")}
${layout_declare_tensor(1, "r", "t_in", DTYPE, "texture3d")}
${layout_declare_tensor(2, "r", "t_kernel", DTYPE, "texture2d")}
${layout_declare_tensor(3, "r", "t_bias", DTYPE, "texture2d")}
${layout_declare_ubo(4, "ivec3", "out_limits")}
${layout_declare_ubo(5, "ivec4", "in_sizes")}
${layout_declare_ubo(6, "ivec2", "kernel_size", "ivec2", "stride", "ivec2", "padding", "ivec2", "dilation")}
${layout_declare_ubo(7, "ivec2", "overlay_region", "int", "in_group_size")}
${layout_declare_ubo(8, "float", "out_min", "float", "out_max")}

layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2) in;

#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

/*
 * Computes a depthwise convolution. Each shader invocation calculates the
 * output at a single output location.
 */
void main() {
  const u16vec3 pos = u16vec3(
    gl_GlobalInvocationID.x % out_limits.x,
    (gl_GlobalInvocationID.x / out_limits.x) % out_limits.y,
    gl_GlobalInvocationID.x / (out_limits.x * out_limits.y));

  if (any(greaterThanEqual(pos, out_limits))) {
    return;
  }

  // Compute the index of the top-left element of the overlay region. Negative
  // indices indicate that the top-left element is in a region added by padding.
  const u16vec2 ipos = pos.xy * u16vec2(stride) - u16vec2(padding);

  // Compute the start and end of the input indices to load. Padding is assumed
  // to be constant 0 padding, so any reads from the padding region is skipped.
  const u16vec2 start = ipos;
  const u16vec2 end = ipos + u16vec2(overlay_region.xy);

  VEC4_T sum = texelFetch(t_bias, u16vec2(pos.z, 0), 0);
  uint16_t kx = uint16_t(0);
  for (uint16_t y = start.y, i = uint16_t(0); i < uint16_t(TILE_SIZE); y += uint16_t(dilation.y), i++) {
    for (uint16_t x = start.x, j = uint16_t(0); j < uint16_t(TILE_SIZE); x += uint16_t(dilation.x), j++) {
      // The weight kernel was rearranged such that every NxN filter is
      // flattened to fit in one row. Each filter was then stacked on top of
      // each other vertically.
      const vec4 in_texel = texelFetch(t_in, u16vec3(x, y, pos.z), 0);
      sum = fma(in_texel, texelFetch(t_kernel, u16vec2(kx, pos.z), 0), sum);
      kx++;
    }
  }

  imageStore(t_out, pos, op(sum, out_min, out_max));
}
