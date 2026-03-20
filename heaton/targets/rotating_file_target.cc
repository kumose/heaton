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

#include <heaton/targets/rotating_file_target.h>
#include <turbo/log/internal/fs_helper.h>
#include <turbo/log/internal/globals.h>
#include <turbo/strings/match.h>

namespace heaton {
    RotatingFileTarget::RotatingFileTarget() : TargetBase() {
    }

    RotatingFileTarget::~RotatingFileTarget() {
        if (_file_writer != nullptr) {
            _file_writer->close();
        }
    }

    turbo::Status RotatingFileTarget::initialize(const TargetOptions &base) {
        _options = base;
        if (base.filename.empty()) {
            return turbo::invalid_argument_error("base file name is required");
        }
        BaseFilename b(base.filename);
        _base_filename = b;

        auto now = turbo::Time::current_time();

        _next_check_time = next_check_time(now);

        TURBO_RETURN_NOT_OK(load_timezone(_options.timezone, _timezone));
        auto filename = HourlyLogFilename::make_path(now, _timezone, _base_filename);
        if (_options.truncate) {
            ::remove(filename.c_str());
        }

        _file_writer = std::make_unique<turbo::log_internal::AppendFile>();

        if (_file_writer->initialize(filename) != 0) {
            _file_writer.reset();
            return turbo::invalid_argument_error("failed to initialize AppendFile");
        }
        return turbo::OkStatus();
    }

    void RotatingFileTarget::apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) {
        if (shutting_down) {
            return;
        }
        rotate_file(stamp, len);
        if (_file_writer == nullptr) {
            return;
        }
        _file_writer->write(std::string_view(data, len));
        ++_current_items;
    }


    void RotatingFileTarget::rotate_file(turbo::Time stamp, size_t delta) {
        check_file(stamp);

        if (_options.max_file_size == 0 || _file_writer == nullptr) {
            return;
        }

        if (_file_writer->file_size() + delta < _options.max_file_size) {
            return;
        }
        _file_writer->close();
        for (auto i = _options.max_files; i > 0; --i) {
            std::string src = SequentialLogFilename::make_path(i - 1, _base_filename);
            if (!turbo::log_internal::path_exists(src)) {
                continue;
            }
            std::string target = SequentialLogFilename::make_path(i, _base_filename);

            rename_file(src, target);
        }
        _file_writer.reset();
        reopen_writer(stamp);
    }

    void RotatingFileTarget::flush() {
        if (_file_writer != nullptr) {
            _file_writer->flush();
        }
    }

    bool RotatingFileTarget::rename_file(const std::string &src_filename, const std::string &target_filename) {
        (void) turbo::log_internal::remove(target_filename);
        return turbo::log_internal::rename(src_filename, target_filename) == 0;
    }
} // namespace heaton
