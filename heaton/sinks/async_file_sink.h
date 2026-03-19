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
#include <array>
#include <turbo/log/logging.h>
#include <turbo/log/log_sink.h>
#include <absl/log/log_sink.h>
#include <heaton/file_target_base.h>
#include <heaton/glog.h>
#include <heaton/targets/daily_file_target.h>
#include <heaton/targets/hourly_file_target.h>
#include <heaton/targets/rotating_file_target.h>
#include <heaton/targets/null_target.h>
#include <turbo/container/flat_hash_map.h>
#include <array>
#include <condition_variable>
#include <mutex>
#include <deque>

namespace heaton {

    class FileAsyncSink;

    class TurboAsyncFileSink : public turbo::LogSink {
    public:
        /// target must be not nullptr.
        TurboAsyncFileSink(FileAsyncSink *sink) : _sink(sink) {
        }

        ~TurboAsyncFileSink() override {
        }

        void Send(const turbo::LogEntry &entry) override;

        void Flush() override {
        }

    private:
        FileAsyncSink *_sink{nullptr};
    };

    class AbslAsyncFileSink : public absl::LogSink {
    public:
        /// target must be not nullptr.
        AbslAsyncFileSink(FileAsyncSink *sink) :_sink(sink) {
        }

        ~AbslAsyncFileSink() override {
        }

        void Send(const absl::LogEntry &entry) override;

        void Flush() override {
        }


    private:
        FileAsyncSink *_sink{nullptr};
    };

    class GlogAsyncFileSink : public google::LogSink {
    public:
        /// target must be not nullptr.
        GlogAsyncFileSink(FileAsyncSink *sink) : _sink(sink) {
        }

        ~GlogAsyncFileSink() override {
        }

        void send(google::LogSeverity severity, const char *full_filename,
                  const char *base_filename, int line, const google::LogMessageTime &time,
                  const char *message, size_t message_len) override;

    private:
        FileAsyncSink *_sink{nullptr};
    };

    class FileAsyncSinkBase {
    public:
        FileAsyncSinkBase(FileAsyncSink *sink);

        virtual ~FileAsyncSinkBase() = default;

        void initialize_internal(const FileSinkOptions &option);

        void shutdown();

    private:
        TurboAsyncFileSink turbo_file_sink;
        AbslAsyncFileSink absl_file_sink;
        GlogAsyncFileSink glog_file_sink;

        bool _turbo{false};
        bool _absl{false};
        bool _glog{false};
        std::mutex _mutex;
    };

    class FileAsyncSink : public FileAsyncSinkBase {
    public:
        FileAsyncSink() : FileAsyncSinkBase(this) {
        }

        ~FileAsyncSink() override {
            stop_work();
        }

        turbo::Status initialize(const FileSinkOptions &option);

        void push_log_entity(std::unique_ptr<LogEntity> &&entry);

        void stop_work();

        static void thread_func(FileAsyncSink *sink);

    private:
        std::array<FileTargetBase *, 4> _file_target{};
        FileTargetBase* _g_target{nullptr};
        turbo::flat_hash_map<std::string, std::unique_ptr<FileTargetBase> > _real_targets;
        std::unique_ptr<FileTargetBase> _real_g_target{nullptr};

        std::mutex _mutex;
        std::condition_variable _cond;
        std::deque<std::unique_ptr<LogEntity>> _log_entities;
        FileSinkOptions _options;
        std::unique_ptr<std::thread> _thread{nullptr};
        std::atomic<bool> _running{true};

        std::mutex _start_mutex;
        std::condition_variable _start_cond;
    };
} // namespace heaton
