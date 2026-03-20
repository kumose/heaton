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

namespace heaton {

    enum class TargetType {
        /// null sink, do nothing
        TARGET_NULL = 0,
        TARGET_STDERR = 1,
        TARGET_STDOUT = 2,
        TARGET_COLOR_STDERR = 3,
        TARGET_COLOR_STDOUT = 4,
        TARGET_DAILY = 5,
        TARGET_HOURLY = 6,
        TARGET_ROTATING = 7
    };

    struct TargetOptions {
        TargetType target_type{TargetType::TARGET_NULL};
        /// log base name
        std::string filename;

        /// 0 means no limited.
        size_t max_files{1};
        /// check file by close and reopen when reach
        /// the time duration, 0 means no check.
        size_t check_interval_s{60};
        /// check file by close and reopen when reach
        /// the items,  0 means no check.
        size_t check_items{1024};
        /// truncate old log?
        bool truncate{false};
        /// max file size for single file, 0 means no check.
        size_t max_file_size{0};
        /// max file size for single file, 0 means no check.
        /// no use now.
        size_t max_total_size{0};
        /// timezone for filename, default as local, eg "local", "utc"
        std::string timezone{"local"};
        /// try reopen file when fail on open file
        size_t reopen_interval_s{3};
    };

    class TargetBase {
    public:

        static const turbo::Time kZero;
        virtual ~TargetBase() = default;

        virtual turbo::Status initialize(const TargetOptions &base) = 0;

        virtual turbo::Status shutdown() = 0;

        virtual void apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) = 0;

        void apply_log(LogLevel l, turbo::Time stamp, std::string_view text) {
            apply_log(l, stamp, text.data(), text.size());
        }

        void apply_log(const LogEntity *entity) {
            apply_log(entity->level, entity->stamp, entity->message.data(), entity->message.size());
        }

        virtual void flush() {}

        static turbo::Result<std::shared_ptr<TargetBase>> create_target(TargetOptions option);

    protected:
        void reopen_writer(turbo::Time stamp);
        void check_file(turbo::Time stamp);

        turbo::Time next_check_time(turbo::Time stamp) const;

    protected:
        TargetOptions _options;
        turbo::Time _next_reopen_time;
        turbo::TimeZone _timezone;
        std::unique_ptr<turbo::FileWriter> _file_writer;
        BaseFilename _base_filename;

        turbo::Time _next_check_time;
        size_t      _current_items{0};
        bool shutting_down{false};
    };
} // namespace heaton
