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

#include <heaton/sinks/sync_sink.h>


namespace heaton {

    turbo::Status SyncSink::initialize(const SinkOption &op, std::shared_ptr<TargetBase> &target) {
        if (target == nullptr) {
            return turbo::invalid_argument_error("target is nullptr");
        }
        _target = target;
        return turbo::OkStatus();
    }
    /// dispatch no transfer log, to avoid copy.
    void SyncSink::apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) {
        if (TURBO_UNLIKELY(_shutting_down)) {
            return;
        }
        _target->apply_log(l, stamp, data, len);
    }

    /// dispatch reformated and allocated log, using move to avoid copy
    void SyncSink::apply_log(LogLevel l, turbo::Time stamp, std::string &&msg) {
        if (TURBO_UNLIKELY(_shutting_down)) {
            return;
        }
        _target->apply_log(l, stamp, msg.data(), msg.size());
    }

    void SyncSink::flush() {
        if (TURBO_UNLIKELY(_shutting_down)) {
            return;
        }
        _target->flush();
    }

    void SyncSink::shutdown() {
        _shutting_down = true;
        if (_target) {
            TURBO_UNUSED(_target->shutdown());
            _target = NullTarget::create();
        }
    }


}  // namespace heaton
