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

#include <heaton/dispatch.h>

namespace heaton {
    void Dispatch::apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) {
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        std::vector<SinkBasePtr> *sink{nullptr};
        switch (l) {
            case LogLevel::LL_INFO:
                sink = &_infos;
                break;
            case LogLevel::LL_WARNING:
                sink = &_warnings;
                break;
            case LogLevel::LL_ERROR:
                sink = &_errors;
                break;
            default:
                sink = &_fatals;
                break;
        }
        _global_sink->apply_log(l, stamp, data, len);
        dispatch_log(sink, l, stamp, data, len);
        flush_logs(stamp);
    }

    void Dispatch::apply_log(LogLevel l, turbo::Time stamp, std::string &&msg) {
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        std::vector<SinkBasePtr> *sink{nullptr};
        switch (l) {
            case LogLevel::LL_INFO:
                sink = &_infos;
                break;
            case LogLevel::LL_WARNING:
                sink = &_warnings;
                break;
            case LogLevel::LL_ERROR:
                sink = &_errors;
                break;
            default:
                sink = &_fatals;
                break;
        }
        dispatch_log(sink, l, stamp, msg.data(), msg.size());
        _global_sink->apply_log(l, stamp, std::move(msg));
        flush_logs(stamp);
    }

    void Dispatch::dispatch_log(turbo::Nonnull<std::vector<SinkBasePtr> *> sinks, LogLevel l, turbo::Time stamp,
                                const char *data, size_t len) {
        for (auto &sink: *sinks) {
            sink->apply_log(l, stamp, data, len);
        }
    }

    void Dispatch::flush_logs(turbo::Time stamp) {
        if (TURBO_LIKELY(stamp < _next_flush_time && _current_items < _options.flush_items)) {
            return;
        }
        _global_sink->flush();
        for (auto &it : _infos) {
            it->flush();
        }
        for (auto &it : _warnings) {
            it->flush();
        }
        for (auto &it : _errors) {
            it->flush();
        }
        for (auto &it : _fatals) {
            it->flush();
        }
        _next_flush_time = stamp + turbo::Duration::seconds(_options.flush_interval_seconds);
        _current_items = 0;
    }

    turbo::Status Dispatch::add_sink(std::vector<SinkBasePtr> &old, const SinkBasePtr &sink) {
        if (!sink) {
            return turbo::invalid_argument_error("sink is nullptr");
        }
        if (sink == _global_sink) {
            return turbo::invalid_argument_error("sink is same to global sink, global sink can not be duplicated");
        }
        std::vector<SinkBasePtr> sinks;
        for (auto &it : old) {
            if (it == sink) {
                continue;
            }
            sinks.push_back(it);
        }
        sinks.push_back(sink);
        old.swap(sinks);
        return turbo::Status();
    }

    void Dispatch::remove_sink(std::vector<SinkBasePtr> &old, const SinkBasePtr &sink) {
        std::vector<SinkBasePtr> sinks;
        for (auto &it : old) {
            if (it == sink) {
                continue;
            }
            sinks.push_back(it);
        }
        old.swap(sinks);
    }
} // namespace heaton
