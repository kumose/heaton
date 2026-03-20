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

#include <heaton/targets/daily_file_target.h>
#include <turbo/log/internal/fs_helper.h>
#include <turbo/log/internal/globals.h>
#include <turbo/strings/match.h>
#include <turbo/files/filesystem.h>

namespace heaton {
    DailyFileTarget::DailyFileTarget() : TargetBase() {
    }

    DailyFileTarget::~DailyFileTarget() {
        if (_file_writer != nullptr) {
            _file_writer->close();
        }
    }

    turbo::Status DailyFileTarget::initialize(const TargetOptions &base) {
        _options = base;
        if (base.filename.empty()) {
            return turbo::invalid_argument_error("base file name is required");
        }
        BaseFilename b(base.filename);
        _base_filename = b;

        auto now = turbo::Time::current_time();

        _next_check_time = next_check_time(now);
        _next_rotation_time = next_rotation_time(now);

        TURBO_RETURN_NOT_OK(load_timezone(_options.timezone, _timezone));
        auto filename = DailyLogFilename::make_path(now, _timezone, _base_filename);
        if (_options.truncate) {
            ::remove(filename.c_str());
        }

        init_file_queue(now);

        _file_writer = std::make_unique<turbo::log_internal::AppendFile>();

        if (_file_writer->initialize(filename) != 0) {
            _file_writer.reset();
            return turbo::invalid_argument_error("open writer failed");
        }
        return turbo::OkStatus();
    }

    void DailyFileTarget::apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) {
        if (shutting_down) {
            return;
        }
        rotate_file(stamp);
        if (_file_writer == nullptr) {
            return;
        }
        _file_writer->write(std::string_view(data, len));
        ++_current_items;
    }

    void DailyFileTarget::init_file_queue(turbo::Time now) {
        std::vector<std::string> filenames;
        _files = turbo::circular_queue<std::string>(static_cast<size_t>(_options.max_files));
        while (filenames.size() < _options.max_files) {
            auto filename = DailyLogFilename::make_path(now, _timezone, _base_filename);
            if (!turbo::log_internal::path_exists(filename)) {
                break;
            }
            filenames.emplace_back(filename);
            now -= turbo::Duration::hours(24);
        }
        for (auto iter = filenames.rbegin(); iter != filenames.rend(); ++iter) {
            _files.push_back(std::move(*iter));
        }
    }


    void DailyFileTarget::rotate_file(turbo::Time stamp) {
        check_file(stamp);
        if (stamp < _next_rotation_time) {
            return;
        }
        _next_rotation_time = next_rotation_time(stamp);
        auto filename = DailyLogFilename::make_path(stamp, _timezone, _base_filename);
        _file_writer->close();
        _file_writer.reset();
        reopen_writer(stamp);
        if (!_file_writer) {
            return;
        }
        if (_options.max_files == 0) {
            return;
        }

        auto current_file = _file_writer->file_path();
        if (_files.full()) {
            auto old_filename = std::move(_files.front());
            _files.pop_front();
            bool ok = turbo::log_internal::remove_if_exists(old_filename) == 0;
            if (!ok) {
                std::cerr << "Failed removing daily file " + old_filename << std::endl;
            }
        }
        _files.push_back(std::move(current_file));
    }

    turbo::Time DailyFileTarget::next_rotation_time(turbo::Time stamp) const {
        auto tm = turbo::Time::to_tm(stamp, _timezone);
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        auto rotation_time = turbo::Time::from_tm(tm, _timezone);
        if (rotation_time > stamp) {
            return rotation_time;
        }
        return rotation_time + turbo::Duration::hours(24);
    }

    void DailyFileTarget::flush() {
        if (_file_writer != nullptr) {
            _file_writer->flush();
        }
    }
} // namespace heaton
