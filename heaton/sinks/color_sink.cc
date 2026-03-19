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

#include  <heaton/sinks/color_sink.h>
#include <array>
#include <turbo/strings/string_builder.h>
#include <turbo/log/log_sink_registry.h>
#include <absl/log/log_sink_registry.h>
#include <turbo/base/internal/sysinfo.h>

namespace heaton {
    std::array<std::string_view, 10> ConsoleColor::colors_map = {
        green, // kInfo
        yellow_bold, // kWarning
        red_bold, // kError
        bold_on_red, // kFatal
        "",
        "",
        "",
        "",
        "",
        ""
    };

    void TurboColorSink::Send(const turbo::LogEntry &entry) {
        std::string_view log_data = entry.newline()
                                        ? entry.text_message_with_prefix_and_newline()
                                        : entry.text_message_with_prefix();
        auto color = ConsoleColor::colors_map[static_cast<int>(entry.log_severity())];
        turbo::StringBuilder builder;
        builder << color << log_data << ConsoleColor::reset;

        if (entry.log_severity() == turbo::LogSeverity::kFatal && !entry.stacktrace().empty()) {
            builder << color << entry.stacktrace() << ConsoleColor::reset << "\n";
        }
        auto msg = builder.str();
        _file_target->apply_log(to_log_level(entry.log_severity()), entry.timestamp(), msg.data(), msg.size());
    }

    void AbslColorSink::Send(const absl::LogEntry &entry) {
        std::string_view log_data = entry.text_message_with_prefix_and_newline();
        auto color = ConsoleColor::colors_map[static_cast<int>(entry.log_severity())];
        turbo::StringBuilder builder;
        builder << color << log_data << ConsoleColor::reset;

        if (entry.log_severity() == absl::LogSeverity::kFatal && !entry.stacktrace().empty()) {
            builder << color << entry.stacktrace() << ConsoleColor::reset << "\n";
        }
        auto msg = builder.str();
        _file_target->apply_log(to_log_level(entry.log_severity()),
                                turbo::Time::from_nanoseconds(absl::ToUnixNanos(entry.timestamp())), msg.data(),
                                msg.size());
    }

    void GlogColorSink::send(google::LogSeverity severity, const char *full_filename,
                             const char *base_filename, int line, const google::LogMessageTime &time,
                             const char *message, size_t message_len) {
        auto color = ConsoleColor::colors_map[static_cast<int>(severity)];

        static std::array<char, 4> gl = {'I', 'W', 'E', 'F'};
        auto msg = turbo::str_format("%s%c%02d%02d %02d:%02d:%02d.%06d  %5d %s:%d] %s%s\n",
                                     color,
                                     gl[static_cast<int>(severity)],
                                     time.month(),
                                     time.day(),
                                     time.hour(),
                                     time.min(),
                                     time.sec(),
                                     time.usec(),
                                     turbo::base_internal::GetCachedTID(),
                                     base_filename,
                                     line,
                                     std::string_view(message, message_len),
                                     ConsoleColor::reset
        );
        _file_target->apply_log(to_log_level(severity), turbo::Time::from_chrono(time.when()), msg.data(), msg.size());
    }

    ColorSinkBase::ColorSinkBase(FileTargetBase *target)
        : turbo_color_sink(target),
          absl_color_sink(target),
          glog_color_sink(target) {
    }

    void ColorSinkBase::initialize(bool turbo, bool absl, bool glog) {
        if (turbo) {
            _turbo = turbo;
            turbo::add_log_sink(&turbo_color_sink);
        }

        if (absl) {
            _absl = absl;
            absl::AddLogSink(&absl_color_sink);
        }

        if (glog) {
            _glog = glog;
            google::AddLogSink(&glog_color_sink);
        }
    }

    void ColorSinkBase::shutdown() {
        if (_turbo) {
            turbo::remove_log_sink(&turbo_color_sink);
        }
        if (_absl) {
            absl::RemoveLogSink(&absl_color_sink);
        }
        if (_glog) {
            google::RemoveLogSink(&glog_color_sink);
        }
    }
} // namespace heaton
