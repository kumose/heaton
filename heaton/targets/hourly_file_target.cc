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

#include <heaton/targets/hourly_file_target.h>
#include <turbo/log/internal/fs_helper.h>
#include <turbo/log/internal/globals.h>
#include <turbo/strings/match.h>

namespace heaton {
    HourlyFileTarget::HourlyFileTarget() : FileTargetBase() {
    }

    HourlyFileTarget::~HourlyFileTarget() {
        if (_file_writer != nullptr) {
            _file_writer->close();
        }
    }

    turbo::Status HourlyFileTarget::initialize(const FileTargetOptions &base) {
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
        auto filename = HourlyLogFilename::make_path(now, _timezone, _base_filename);
        if (_options.truncate) {
            ::remove(filename.c_str());
        }

        init_file_queue(now);

        _file_writer = std::make_unique<turbo::log_internal::AppendFile>();

        _file_writer->initialize(filename);
        return turbo::OkStatus();
    }

    void HourlyFileTarget::apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) {
        if (_file_writer == nullptr|| shutting_down) {
            return;
        }
        rotate_file(stamp);
        _file_writer->write(std::string_view(data, len));
        ++_current_items;
    }

    void HourlyFileTarget::init_file_queue(turbo::Time now) {
        std::vector<std::string> filenames;
        _files = turbo::circular_queue<std::string>(static_cast<size_t>(_options.max_files));
        while (filenames.size() < _options.max_files) {
            auto filename = HourlyLogFilename::make_path(now, _timezone, _base_filename);
            if (!turbo::log_internal::path_exists(filename)) {
                break;
            }
            filenames.emplace_back(filename);
            now -= turbo::Duration::hours(1);
        }
        for (auto iter = filenames.rbegin(); iter != filenames.rend(); ++iter) {
            _files.push_back(std::move(*iter));
        }
    }

    void HourlyFileTarget::check_file(turbo::Time stamp) {
        if (_file_writer == nullptr) {
            return;
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
        _file_writer->reopen();
    }

    void HourlyFileTarget::rotate_file(turbo::Time stamp) {
        check_file(stamp);
        if (stamp < _next_rotation_time) {
            return;
        }
        _next_rotation_time = next_rotation_time(stamp);
        auto filename = HourlyLogFilename::make_path(stamp, _timezone, _base_filename);
        _file_writer->close();
        _file_writer.reset();
        _file_writer = std::make_unique<turbo::log_internal::AppendFile>();
        _file_writer->initialize(filename);
        if (_options.max_files == 0) {
            return;
        }

        auto current_file = _file_writer->file_path();
        if (_files.full()) {
            auto old_filename = std::move(_files.front());
            _files.pop_front();
            bool ok = turbo::log_internal::remove_if_exists(old_filename) == 0;
            if (!ok) {
                std::cerr << "Failed removing hourly file " + old_filename << std::endl;
            }
        }
        _files.push_back(std::move(current_file));
    }

    turbo::Time HourlyFileTarget::next_rotation_time(turbo::Time stamp) const {
        auto tm = turbo::Time::to_tm(stamp, _timezone);
        tm.tm_min = 0;
        tm.tm_sec = 0;
        auto rotation_time = turbo::Time::from_tm(tm, _timezone);
        if (rotation_time > stamp) {
            return rotation_time;
        }
        return rotation_time + turbo::Duration::hours(1);
    }

    turbo::Time HourlyFileTarget::next_check_time(turbo::Time stamp) const {
        if (_options.check_interval_s == 0) {
            return turbo::Time::from_time_t(0);
        }
        return stamp + turbo::Duration::seconds(_options.check_interval_s);
    }

    void HourlyFileTarget::flush() {
        if (_file_writer != nullptr) {
            _file_writer->flush();
        }
    }
} // namespace heaton
