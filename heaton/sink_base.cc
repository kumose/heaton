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

#include <heaton/sink_base.h>
#include <heaton/sinks/sync_sink.h>
#include <heaton/sinks/async_sink.h>

namespace heaton {
    turbo::Result<std::shared_ptr<SinkBase> >
    SinkBase::create_sink(const SinkOption &op, std::shared_ptr<TargetBase> &target) {
        std::shared_ptr<SinkBase> ptr;
        switch (op.type) {
            case SinkType::SINK_FILE:
                ptr = std::make_shared<SyncSink>();
                break;
            case SinkType::SINK_ASYNC_FILE:
                ptr = std::make_shared<AsyncSink>();
                break;
            default:
                return turbo::invalid_argument_error("unknown sink type");
        }
        TURBO_ABORT_NOT_OK(ptr->initialize(op, target));
        return ptr;
    }

    turbo::Result<std::shared_ptr<SinkBase>> SinkBase::create_sink(const SinkOption &op) {
        TURBO_MOVE_OR_RAISE(auto t, TargetBase::create_target(op.target));
        return create_sink(op, t);
    }
} // namespace heaton
