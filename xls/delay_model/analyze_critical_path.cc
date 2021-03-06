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

#include "xls/delay_model/analyze_critical_path.h"

#include "xls/common/logging/logging.h"
#include "xls/common/status/status_macros.h"
#include "xls/delay_model/delay_estimator.h"
#include "xls/ir/node_iterator.h"

namespace xls {

absl::StatusOr<std::vector<CriticalPathEntry>> AnalyzeCriticalPath(
    Function* f, absl::optional<int64> clock_period_ps,
    const DelayEstimator& delay_estimator) {
  absl::flat_hash_map<Node*, std::pair<int64, bool>> node_to_output_delay;

  auto get_max_operands_delay = [&](Node* node) {
    int64 earliest = 0;
    for (Node* operand : node->operands()) {
      earliest = std::max(earliest, node_to_output_delay[operand].first);
    }
    return earliest;
  };

  for (Node* node : TopoSort(f)) {
    int64 earliest = get_max_operands_delay(node);
    XLS_ASSIGN_OR_RETURN(int64 node_effort,
                         delay_estimator.GetOperationDelayInPs(node));
    bool bumped = false;
    // If the dependency straddles a clock boundary we have to make our delay
    // start from the clock time.
    if (clock_period_ps.has_value() &&
        (((earliest + node_effort) / clock_period_ps.value()) >
         (earliest / clock_period_ps.value()))) {
      int64 new_earliest =
          RoundDownToNearest(earliest + node_effort, clock_period_ps.value());
      XLS_CHECK_GT(new_earliest, earliest);
      earliest = new_earliest;
      bumped = true;
    }
    node_to_output_delay[node] = {earliest + node_effort, bumped};
  }

  std::vector<CriticalPathEntry> critical_path;

  Node* n = f->return_value();
  while (true) {
    critical_path.push_back(CriticalPathEntry{
        n, delay_estimator.GetOperationDelayInPs(n).value(),
        node_to_output_delay[n].first, node_to_output_delay[n].second});
    Node* next = nullptr;
    int64 next_delay = 0;
    for (Node* operand : n->operands()) {
      int64 operand_delay = node_to_output_delay[operand].first;
      if (operand_delay > next_delay) {
        next = operand;
        next_delay = operand_delay;
      }
    }
    if (next == nullptr) {
      break;
    }
    n = next;
  }

  return critical_path;
}

}  // namespace xls
