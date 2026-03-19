// Copyright (C) Kumo inc. and its affiliates.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <heaton/utility/log_filename.h>
#include <turbo/files/filesystem.h>
#include <turbo/strings/match.h>
#include <turbo/strings/numbers.h>
#include <turbo/strings/str_format.h>
#include <turbo/strings/str_split.h>

namespace heaton {
    BaseFilename::BaseFilename(std::string_view filename) {
        const turbo::FilePath path(filename);
        directory = path.parent_path();
        basename = path.filename().filename().stem();
        extension = path.extension();
    }

    turbo::Status load_timezone(const std::string &name, turbo::TimeZone &zone) {
        if (name.empty() || turbo::equals_ignore_case(name, "local")) {
            zone = turbo::TimeZone::local();
        } else if (turbo::equals_ignore_case(name, "utc")) {
            zone = turbo::TimeZone::utc();
        } else {
            if (!turbo::TimeZone::load(name, &zone)) {
                return turbo::invalid_argument_error("invalid timezone name:", name);
            }
        }
        return turbo::OkStatus();
    }

    SequentialLogFilename::SequentialLogFilename(std::string_view filename, const BaseFilename &base) {
        const turbo::FilePath path(filename);
        const std::string basename = path.filename().filename().stem();
        const std::string extension = path.extension();
        const std::string directory = path.parent_path().string();
        if (extension != base.extension) {
            LogFilename::valid_ = false;
            return;
        }
        if (!turbo::starts_with(basename, base.basename)) {
            LogFilename::valid_ = false;
            return;
        }
        std::string_view seq = basename;
        seq = seq.substr(base.basename.size());
        if (seq.empty() || seq.front() != '_') {
            LogFilename::valid_ = false;
            return;
        }
        seq = seq.substr(1);
        if (seq.empty()) {
            LogFilename::valid_ = false;
        }
        if (!turbo::simple_atoi(seq, &sequence_)) {
            LogFilename::valid_ = false;
            return;
        }
    }

    std::string SequentialLogFilename::make_path(int64_t seq, const BaseFilename &base) {
        if (base.directory.empty()) {
            return turbo::str_format("%s_%04d%s", base.basename, seq, base.extension);
        } else {
            return turbo::str_format("%s/%s_%04d%s", base.directory, base.basename, seq, base.extension);
        }
    }

    DailyLogFilename::DailyLogFilename(std::string_view filename, const BaseFilename &base) {
        const turbo::FilePath path(filename);
        const std::string basename = path.filename().filename().stem();
        const std::string extension = path.extension();
        const std::string directory = path.parent_path().string();
        if (extension != base.extension) {
            LogFilename::valid_ = false;
            return;
        }
        if (!turbo::starts_with(basename, base.basename)) {
            LogFilename::valid_ = false;
            return;
        }
        std::string_view seq = basename;
        seq = seq.substr(base.basename.size());
        if (seq.empty() || seq.front() != '_') {
            LogFilename::valid_ = false;
            return;
        }
        seq = seq.substr(1);
        if (seq.empty()) {
            LogFilename::valid_ = false;
        }
        if (!turbo::parse_civil_time(seq, &datetime_)) {
            LogFilename::valid_ = false;
            return;
        }
    }

    std::string DailyLogFilename::make_path(const turbo::Time &stamp, const turbo::TimeZone &tz, const BaseFilename &base) {
        const turbo::CivilDay civil_day = turbo::Time::to_civil_day(stamp, tz);
        return make_path(civil_day, base);
    }
    std::string DailyLogFilename::make_path(const turbo::CivilDay &civil, const BaseFilename &base) {
        if (base.directory.empty()) {
            return turbo::str_format("%s_%v%s", base.basename, civil, base.extension);
        } else {
            return turbo::str_format("%s/%s_%v%s", base.directory, base.basename, civil, base.extension);
        }
    }

    HourlyLogFilename::HourlyLogFilename(std::string_view filename, const BaseFilename &base) {
        const turbo::FilePath path(filename);
        const std::string basename = path.filename().filename().stem();
        const std::string extension = path.extension();
        const std::string directory = path.parent_path().string();
        if (extension != base.extension) {
            LogFilename::valid_ = false;
            return;
        }
        if (!turbo::starts_with(basename, base.basename)) {
            LogFilename::valid_ = false;
            return;
        }
        std::string_view seq = basename;
        seq = seq.substr(base.basename.size());
        if (seq.empty() || seq.front() != '_') {
            LogFilename::valid_ = false;
            return;
        }
        seq = seq.substr(1);
        if (seq.empty()) {
            LogFilename::valid_ = false;
        }
        if (!turbo::parse_civil_time(seq, &datetime_)) {
            LogFilename::valid_ = false;
            return;
        }
    }

    std::string HourlyLogFilename::make_path(const turbo::Time &stamp, const turbo::TimeZone &tz, const BaseFilename &base) {
        const turbo::CivilHour civil_hour = turbo::Time::to_civil_hour(stamp, tz);
        return make_path(civil_hour, base);
    }

    std::string HourlyLogFilename::make_path(const turbo::CivilHour &civil, const BaseFilename &base) {
        if (base.directory.empty()) {
            return turbo::str_format("%s_%v%s", base.basename, civil, base.extension);
        } else {
            return turbo::str_format("%s/%s_%v%s", base.directory, base.basename, civil, base.extension);
        }
    }
} // namespace heaton
