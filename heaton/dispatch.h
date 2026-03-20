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
#include <mutex>
#include <turbo/utility/status.h>
#include <turbo/times/time.h>
#include <heaton/log_entity.h>
#include <turbo/log/internal/append_file.h>
#include <heaton/utility/log_filename.h>
#include <heaton/sink_base.h>
#include <vector>

namespace heaton {

    struct DispatchOptions {
        /// non zero, if missing, default
        /// set to 4096.
        size_t flush_items{4096};
        /// non zero, if missing, default
        /// set to 60.
        size_t flush_interval_seconds{60};
    };
    class Dispatch {
    public:
        Dispatch() = default;

        ~Dispatch() = default;

        void set_options(const DispatchOptions &options) {
            std::lock_guard<std::mutex> lock(_mutex);
            _options = options;
        }

        DispatchOptions get_options() const {
            return _options;
        }
        /// dispatch no transfer log, to avoid copy.
        void apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len);

        /// dispatch reformated and allocated log, using move to avoid copy
        void apply_log(LogLevel l, turbo::Time stamp, std::string &&msg);

        /// global_sink should not duplicated in branch
        /// sinks, remove it from branches.
        void set_global_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            _global_sink = sink;
            remove_sink(_infos, sink);
            remove_sink(_warnings, sink);
            remove_sink(_errors, sink);
            remove_sink(_fatals, sink);
        }

        turbo::Status add_info_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            return add_sink(_infos, sink);
        }

        turbo::Status add_warning_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            return add_sink(_warnings, sink);
        }

        turbo::Status add_error_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            return add_sink(_errors, sink);
        }
        turbo::Status add_fatal_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            return add_sink(_fatals, sink);
        }

        void remove_info_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            remove_sink(_infos, sink);
        }
        void remove_warning_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            remove_sink(_warnings, sink);
        }
        void remove_error_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            remove_sink(_errors, sink);
        }
        void remove_fatal_sink(const SinkBasePtr &sink) {
            std::lock_guard<std::mutex> lock(_mutex);
            remove_sink(_fatals, sink);
        }
    private:

        /// helper function for dispatch, we always assume that, the branch logs
        /// are not rarely case, if need, let it copy the data.
        static void dispatch_log(turbo::Nonnull<std::vector<SinkBasePtr> *> sinks, LogLevel l,
                                 turbo::Time stamp, const char *data, size_t len);

        /// flush log by items and time trigger,
        /// must triggered up by one case,and then
        /// reset the statemachine.
        void flush_logs(turbo::Time stamp);

        /// safe, no duplicated.
        turbo::Status add_sink(std::vector<SinkBasePtr> &old, const SinkBasePtr &sink);

        void remove_sink(std::vector<SinkBasePtr> &old, const SinkBasePtr &sink);
    private:
        std::mutex _mutex;
        /// non nullptr, default will be initialize with
        /// a console sink,if disabled all output,replace by
        /// a nullsink to discard all log by other than a nullptr
        /// to avoid if branch missing
        SinkBasePtr   _global_sink;
        std::vector<SinkBasePtr> _infos;
        std::vector<SinkBasePtr> _warnings;
        std::vector<SinkBasePtr> _errors;
        std::vector<SinkBasePtr> _fatals;

        size_t                   _current_items{0};
        turbo::Time              _next_flush_time{};
        DispatchOptions          _options;
    };
} // namespace heaton
