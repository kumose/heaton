// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <heaton/sink_base.h>
#include <heaton/target_base.h>
#include <heaton/targets/null_target.h>

namespace heaton {

    /// Sync Sinks
    class SyncSink : public SinkBase {
    public:
        SyncSink() = default;

        ~SyncSink() override = default;

        turbo::Status initialize(const SinkOption &op, std::shared_ptr<TargetBase> &target) override;

        /// dispatch no transfer log, to avoid copy.
        void apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) override;

        /// dispatch reformated and allocated log, using move to avoid copy
        void apply_log(LogLevel l, turbo::Time stamp, std::string &&msg) override;

        void flush() override;

        /// mark shutdown and
        /// reset target to null_target, so, it
        /// avoid if branch.
        void shutdown() override;

    private:
        std::shared_ptr<TargetBase> _target{NullTarget::create()};
        bool _shutting_down = false;
    };
}  // namespace heaton
