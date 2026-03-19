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

#include <string>
#include <turbo/log/logging.h>
#include <absl/log/absl_log.h>
#include <heaton/glog.h>

namespace heaton {
    enum class LogLevel {
        LL_INFO,
        LL_WARNING,
        LL_ERROR,
        LL_FATAL
    };

    constexpr LogLevel to_log_level(turbo::LogSeverity s) {
        return static_cast<LogLevel>(s);
    }

    constexpr LogLevel to_log_level(absl::LogSeverity s) {
        return static_cast<LogLevel>(s);
    }

    constexpr LogLevel to_log_level(google::LogSeverity s) {
        return static_cast<LogLevel>(s);
    }

    struct LogEntity {
        LogLevel level;
        turbo::Time stamp;
        std::string message;

        LogEntity() = default;
        ~LogEntity() = default;

        LogEntity(LogLevel l, turbo::Time t, std::string &&m);

        LogEntity(const LogEntity &entry) = delete;

        LogEntity &operator=(const LogEntity &entry) = delete;
    };
} // namespace heaton
