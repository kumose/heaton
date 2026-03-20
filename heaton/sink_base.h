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

#include <string_view>
#include <cstddef>
#include <turbo/utility/status.h>
#include <turbo/times/time.h>
#include <heaton/log_entity.h>
#include <turbo/log/internal/append_file.h>
#include <heaton/utility/log_filename.h>
#include <memory>
#include <heaton/target_base.h>

namespace heaton {
    enum class SinkType {
        SINK_FILE,
        SINK_ASYNC_FILE,
    };

    struct SinkOption {
        SinkType type{SinkType::SINK_FILE};
        size_t max_queue_length{4096};
        TargetOptions target;
    };

    class SinkBase {
    public:
        virtual ~SinkBase() = default;

        virtual turbo::Status initialize(const SinkOption &op, std::shared_ptr<TargetBase> &target) = 0;

        /// dispatch no transfer log, to avoid copy.
        virtual void apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) = 0;

        /// dispatch reformated and allocated log, using move to avoid copy
        virtual void apply_log(LogLevel l, turbo::Time stamp, std::string &&msg) = 0;

        virtual void flush() = 0;

        virtual void shutdown() = 0;

        /// factory for sink
        static turbo::Result<std::shared_ptr<SinkBase>> create_sink(const SinkOption &op, std::shared_ptr<TargetBase> &target);

        static turbo::Result<std::shared_ptr<SinkBase>> create_sink(const SinkOption &op);
    };

    using SinkBasePtr = std::shared_ptr<SinkBase>;
} // namespace heaton
