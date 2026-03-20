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

#include <heaton/target_base.h>
#include <turbo/files/filesystem.h>
#include <heaton/targets/null_target.h>
#include <heaton/targets/console_target.h>
#include <heaton/targets/daily_file_target.h>
#include <heaton/targets/hourly_file_target.h>
#include <heaton/targets/rotating_file_target.h>

namespace heaton {

    const turbo::Time TargetBase::kZero;

    void TargetBase::reopen_writer(turbo::Time stamp) {
        auto now = turbo::Time::current_time();
        if (_next_reopen_time != kZero && now < _next_reopen_time) {
            return;
        }

        if (_next_reopen_time != kZero) {
            BaseFilename fn(_options.filename);
            TURBO_UNUSED(turbo::create_directories(fn.directory));
            auto e  = turbo::is_directory(fn.directory);
            if (!e.ok() || !e.value_or_die()) {
                _next_reopen_time = now + turbo::Duration::seconds(_options.reopen_interval_s);
                return;
            }
        }
        auto filename = HourlyLogFilename::make_path(stamp, _timezone, _base_filename);
        _file_writer = std::make_unique<turbo::log_internal::AppendFile>();
        if (_file_writer->initialize(filename) != 0) {
            _file_writer.reset();
            _next_reopen_time = now + turbo::Duration::seconds(_options.reopen_interval_s);
            return;
        }
        _next_reopen_time = turbo::Time();
    }

    void TargetBase::check_file(turbo::Time stamp) {
        bool need_reopen = true;
        if (_file_writer == nullptr) {
            reopen_writer(stamp);
            if (!_file_writer) {
                return;
            }
            need_reopen = false;
        }

        if (_options.check_interval_s == 0 &&
            _options.check_items == 0) {
            return;
            }

        if (stamp < _next_check_time && _current_items < _options.check_items) {
            return;
        }
        _next_check_time = next_check_time(stamp);
        _current_items = 0;
        if (need_reopen) {
            _file_writer->reopen();
        }
    }

    turbo::Time TargetBase::next_check_time(turbo::Time stamp) const {
        if (_options.check_interval_s == 0) {
            return turbo::Time::from_time_t(0);
        }
        return stamp + turbo::Duration::seconds(_options.check_interval_s);
    }

    turbo::Result<std::shared_ptr<TargetBase>> TargetBase::create_target(TargetOptions option) {
        std::shared_ptr<TargetBase> target;
        switch (option.target_type) {
            case TargetType::TARGET_NULL:
                target = NullTarget::create();
                break;
            case TargetType::TARGET_STDERR:
                target = ConsoleTargetStderr::create();
                break;
            case TargetType::TARGET_STDOUT:
                target = ConsoleTargetStdout::create();
                break;
            case TargetType::TARGET_COLOR_STDERR:
                target = ColorConsoleTargetStderr::create();
                break;
            case TargetType::TARGET_COLOR_STDOUT:
                target = ColorConsoleTargetStdout::create();
                break;
            case TargetType::TARGET_DAILY:
                target = std::make_shared<DailyFileTarget>();
                TURBO_RETURN_NOT_OK(target->initialize(option));
                break;
            case TargetType::TARGET_HOURLY:
                target = std::make_shared<HourlyFileTarget>();
                TURBO_RETURN_NOT_OK(target->initialize(option));
                break;
            case TargetType::TARGET_ROTATING:
                target = std::make_shared<RotatingFileTarget>();
                TURBO_RETURN_NOT_OK(target->initialize(option));
                break;

        }
        return target;
    }
}  // namespace heaton
