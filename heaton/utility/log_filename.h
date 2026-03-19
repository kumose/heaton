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
#pragma once

#include <string>
#include <turbo/times/time.h>
#include <turbo/utility/status.h>

namespace heaton {
    struct BaseFilename {
        BaseFilename() = default;

        BaseFilename(std::string_view filename);

        ~BaseFilename() = default;

        std::string basename;
        std::string extension;
        std::string directory;
    };

    turbo::Status load_timezone(const std::string &zeone, turbo::TimeZone &zone);

    class LogFilename {
    public:
        virtual ~LogFilename() = default;

        [[nodiscard]] constexpr bool valid() const { return valid_; }

    protected:
        bool valid_{false};
    };

    class SequentialLogFilename : public LogFilename {
    public:
        SequentialLogFilename(std::string_view filename, const BaseFilename &base);

        ~SequentialLogFilename() override = default;

        [[nodiscard]] constexpr int compare(const SequentialLogFilename &other) const {
            return sequence_ - other.sequence_;
        }

        [[nodiscard]] constexpr int64_t sequence() const {
            return sequence_;
        }

        static std::string make_path(int64_t seq, const BaseFilename &base);

    private:
        int64_t sequence_{-1};
    };

    inline bool operator==(const SequentialLogFilename &lhs, const SequentialLogFilename &rhs) {
        return lhs.compare(rhs) == 0;
    }

    inline bool operator!=(const SequentialLogFilename &lhs, const SequentialLogFilename &rhs) {
        return lhs.compare(rhs) != 0;
    }

    inline bool operator<(const SequentialLogFilename &lhs, const SequentialLogFilename &rhs) {
        return lhs.compare(rhs) < 0;
    }

    inline bool operator>(const SequentialLogFilename &lhs, const SequentialLogFilename &rhs) {
        return lhs.compare(rhs) > 0;
    }

    class DailyLogFilename : public LogFilename {
    public:
        DailyLogFilename(std::string_view filename, const BaseFilename &base);

        ~DailyLogFilename() override = default;

        const turbo::CivilDay &datetime() const {
            return datetime_;
        }

        static std::string make_path(const turbo::Time &stamp, const turbo::TimeZone &tz, const BaseFilename &base);

        static std::string make_path(const turbo::CivilDay &civil, const BaseFilename &base);

        [[nodiscard]] constexpr int compare(const DailyLogFilename &other) const {
            if (datetime_ < other.datetime_) {
                return -1;
            }
            if (datetime_ > other.datetime_) {
                return 1;
            }
            return 0;
        }

    private:
        turbo::CivilDay datetime_;
    };

    inline bool operator==(const DailyLogFilename &lhs, const DailyLogFilename &rhs) {
        return lhs.compare(rhs) == 0;
    }

    inline bool operator!=(const DailyLogFilename &lhs, const DailyLogFilename &rhs) {
        return lhs.compare(rhs) != 0;
    }

    inline bool operator<(const DailyLogFilename &lhs, const DailyLogFilename &rhs) {
        return lhs.compare(rhs) < 0;
    }

    inline bool operator>(const DailyLogFilename &lhs, const DailyLogFilename &rhs) {
        return lhs.compare(rhs) > 0;
    }

    class HourlyLogFilename : public LogFilename {
    public:
        HourlyLogFilename(std::string_view filename, const BaseFilename &base);

        ~HourlyLogFilename() override = default;

        const turbo::CivilHour &datetime() const {
            return datetime_;
        }

        [[nodiscard]] constexpr int compare(const HourlyLogFilename &other) const {
            if (datetime_ < other.datetime_) {
                return -1;
            }
            if (datetime_ > other.datetime_) {
                return 1;
            }
            return 0;
        }

        static std::string make_path(const turbo::Time &stamp, const turbo::TimeZone &tz, const BaseFilename &base);

        static std::string make_path(const turbo::CivilHour &civil, const BaseFilename &base);

    private:
        turbo::CivilHour datetime_;
    };
} // namespace heaton
