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
#include <mutex>
#include <deque>
#include <condition_variable>

namespace heaton {
    /// AsyncSink
    class AsyncSink;

    typedef void (*notify_func)(AsyncSink *sink);

    class AsyncSink : public SinkBase {
    private:
        ////////////////////////////////////////
        /// worker state
        /// unstart
        static constexpr int kInit = 0;

        /// started but not in loop
        static constexpr int kStarted = 1;

        /// waiting tasks, only this state,
        /// producer need to notify.
        /// the thread func when loop again,
        /// it first to check the log list,
        /// if empty, start kIdling, otherwise,
        /// keep working
        static constexpr int kIdling = 2;

        /// do working
        static constexpr int kWorking = 3;

        /// exit loop and flush the remain
        /// logs
        static constexpr int kExiting = 4;

        /// flush all the logs, and next
        /// exit the thread func.
        static constexpr int kStop = 5;
        static constexpr int kMax = 6;

        static std::array<notify_func, kMax> notify_funcs;

        static void notify_logs(AsyncSink *sink);
        static void notify_logs_empty(AsyncSink *sink) {
            TURBO_UNUSED(sink);
        }
    public:
        AsyncSink() = default;

        ~AsyncSink() override;

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
        static void thread_func(AsyncSink *sink);

        void apply_entity(std::unique_ptr<LogEntity> &&msg);

    private:
        std::shared_ptr<TargetBase> _target{NullTarget::create()};
        std::atomic<bool> _running = true;

        std::mutex _mutex;
        std::condition_variable _cond;
        std::deque<std::unique_ptr<LogEntity> > _log_entities;
        std::unique_ptr<std::thread> _thread{nullptr};
        std::atomic<int> _state{0};

        std::mutex _start_mutex;
        std::condition_variable _start_cond;

        size_t _max_queue_size{4096};
    };
} // namespace heaton
