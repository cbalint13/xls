// Copyright 2020 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef XLS_PASSES_BIT_SLICE_SIMPLIFICATION_PASS_H_
#define XLS_PASSES_BIT_SLICE_SIMPLIFICATION_PASS_H_

#include "absl/status/statusor.h"
#include "xls/ir/function.h"
#include "xls/passes/passes.h"

namespace xls {

// Pass which simplifies bit-slices. This includes collapsing sequential
// bit-slices, eliminating degenerate full-width slices, and others.
class BitSliceSimplificationPass : public FunctionPass {
 public:
  BitSliceSimplificationPass()
      : FunctionPass("bitslice_simp", "Bit-slice simplification") {}
  ~BitSliceSimplificationPass() override {}

  absl::StatusOr<bool> RunOnFunction(Function* f, const PassOptions& options,
                                     PassResults* results) const override;
};

}  // namespace xls

#endif  // XLS_PASSES_BIT_SLICE_SIMPLIFICATION_PASS_H_
