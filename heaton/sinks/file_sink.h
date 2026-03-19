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
#include <mutex>

namespace heaton {
    class TurboFileSink : public turbo::LogSink {
    public:
        /// target must be not nullptr.
        TurboFileSink(FileTargetBase *g, std::mutex *mut,
                      std::array<FileTargetBase *, 4> target) : _g(g), _file_target(target), _mutex(mut) {
        }

        ~TurboFileSink() override {
        }

        void Send(const turbo::LogEntry &entry) override;

        void Flush() override {
        }

        void set_target(std::array<FileTargetBase *, 4> target);

        void set_g_target(FileTargetBase *target);

    private:
        FileTargetBase *_g{nullptr};
        std::array<FileTargetBase *, 4> _file_target{};
        std::mutex *_mutex;
    };

    class AbslFileSink : public absl::LogSink {
    public:
        /// target must be not nullptr.
        AbslFileSink(FileTargetBase *g, std::mutex *mut,
                     std::array<FileTargetBase *, 4> target) : _g(g), _file_target(target), _mutex(mut) {
        }

        ~AbslFileSink() override {
        }

        void Send(const absl::LogEntry &entry) override;

        void Flush() override {
        }

        void set_target(std::array<FileTargetBase *, 4> target);

        void set_g_target(FileTargetBase *target);

    private:
        FileTargetBase *_g{nullptr};
        std::array<FileTargetBase *, 4> _file_target{};
        std::mutex *_mutex;
    };

    class GlogFileSink : public google::LogSink {
    public:
        /// target must be not nullptr.
        GlogFileSink(FileTargetBase *g, std::mutex *mut,
                     std::array<FileTargetBase *, 4> target) : _g(g), _file_target(target), _mutex(mut) {
        }

        ~GlogFileSink() override {
        }

        void send(google::LogSeverity severity, const char *full_filename,
                  const char *base_filename, int line, const google::LogMessageTime &time,
                  const char *message, size_t message_len) override;

        void set_target(std::array<FileTargetBase *, 4> target);

        void set_g_target(FileTargetBase *target);

    private:
        FileTargetBase *_g{nullptr};
        std::array<FileTargetBase *, 4> _file_target{};
        std::mutex *_mutex;
    };

    class FileSinkBase {
    public:
        FileSinkBase(FileTargetBase *g, std::array<FileTargetBase *, 4> target);

        virtual ~FileSinkBase() = default;

        void initialize_internal(const FileSinkOptions &option, FileTargetBase *g, std::array<FileTargetBase *, 4> target);

        void shutdown();

        void set_target(std::array<FileTargetBase *, 4> target);

        void set_g_target(FileTargetBase *target);

    private:
        TurboFileSink turbo_file_sink;
        AbslFileSink absl_file_sink;
        GlogFileSink glog_file_sink;

        bool _turbo{false};
        bool _absl{false};
        bool _glog{false};
        std::mutex _mutex;
    };

    class FileSink : public FileSinkBase {
    public:
        FileSink() : FileSinkBase(NullTarget::get_instance(), get_null_target_array<4>()) {
        }

        ~FileSink() override {
        }

        turbo::Status initialize(const FileSinkOptions &option);

    private:
        std::array<FileTargetBase *, 4> _file_target{};
        FileTargetBase* _g_target{nullptr};
        turbo::flat_hash_map<std::string, std::unique_ptr<FileTargetBase> > _real_targets;
        std::unique_ptr<FileTargetBase> _real_g_target{nullptr};
    };
} // namespace heaton
