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
#include <heaton/targets/console_target.h>

namespace heaton {
    class ConsoleColor {
    public:
        // Formatting codes
        static constexpr std::string_view reset = "\033[m";
        static constexpr std::string_view bold = "\033[1m";
        static constexpr std::string_view dark = "\033[2m";
        static constexpr std::string_view underline = "\033[4m";
        static constexpr std::string_view blink = "\033[5m";
        static constexpr std::string_view reverse = "\033[7m";
        static constexpr std::string_view concealed = "\033[8m";
        static constexpr std::string_view clear_line = "\033[K";

        // Foreground colors
        static constexpr std::string_view black = "\033[30m";
        static constexpr std::string_view red = "\033[31m";
        static constexpr std::string_view green = "\033[32m";
        static constexpr std::string_view yellow = "\033[33m";
        static constexpr std::string_view blue = "\033[34m";
        static constexpr std::string_view magenta = "\033[35m";
        static constexpr std::string_view cyan = "\033[36m";
        static constexpr std::string_view white = "\033[37m";

        /// Background colors
        static constexpr std::string_view on_black = "\033[40m";
        static constexpr std::string_view on_red = "\033[41m";
        static constexpr std::string_view on_green = "\033[42m";
        static constexpr std::string_view on_yellow = "\033[43m";
        static constexpr std::string_view on_blue = "\033[44m";
        static constexpr std::string_view on_magenta = "\033[45m";
        static constexpr std::string_view on_cyan = "\033[46m";
        static constexpr std::string_view on_white = "\033[47m";

        /// Bold colors
        static constexpr std::string_view yellow_bold = "\033[33m\033[1m";
        static constexpr std::string_view red_bold = "\033[31m\033[1m";
        static constexpr std::string_view bold_on_red = "\033[1m\033[41m";

        static void set_level_color(uint32_t severity, const std::string_view color) {
            KCHECK(severity < 10);
            colors_map[severity] = color;
        }

        static std::string_view get_level_color(uint32_t severity) {
            if (TURBO_LIKELY(severity < 10)) {
                return colors_map[severity];
            }
            return "";
        }

        static std::array<std::string_view, 10> colors_map;
    };


    class TurboColorSink : public turbo::LogSink {
    public:
        /// target must be not nullptr.
        TurboColorSink(FileTargetBase *target) : _file_target(target) {
            KCHECK(_file_target);
        }

        ~TurboColorSink() override {
        }

        void Send(const turbo::LogEntry &entry) override;

        void Flush() override {
        }

    private:
        FileTargetBase *_file_target{nullptr};
    };

    class AbslColorSink : public absl::LogSink {
    public:
        /// target must be not nullptr.
        AbslColorSink(FileTargetBase *target) : _file_target(target) {
            KCHECK(_file_target);
        }

        ~AbslColorSink() override {
        }

        void Send(const absl::LogEntry &entry) override;

        void Flush() override {
        }

    private:
        FileTargetBase *_file_target{nullptr};
    };

    class GlogColorSink : public google::LogSink {
    public:
        /// target must be not nullptr.
        GlogColorSink(FileTargetBase *target) : _file_target(target) {
            KCHECK(_file_target);
        }

        ~GlogColorSink() override {
        }

        void send(google::LogSeverity severity, const char *full_filename,
                  const char *base_filename, int line, const google::LogMessageTime &time,
                  const char *message, size_t message_len) override;

    private:
        FileTargetBase *_file_target{nullptr};
    };

    class ColorSinkBase {
    public:
        ColorSinkBase(FileTargetBase *target);

        virtual ~ColorSinkBase() = default;

        void initialize(bool turbo = true, bool absl = true, bool glog = true);

        void shutdown();

    private:
        TurboColorSink turbo_color_sink;
        AbslColorSink absl_color_sink;
        GlogColorSink glog_color_sink;

        bool _turbo{false};
        bool _absl{false};
        bool _glog{false};
    };

    class StderrColorSink : public ColorSinkBase {
    public:
        StderrColorSink() : ColorSinkBase(&console_sink) {
        }

        ~StderrColorSink() override {
        }

    private:
        ConsoleSinkStderr console_sink;
    };

    class StdoutColorSink : public ColorSinkBase {
    public:
        StdoutColorSink() : ColorSinkBase(&console_sink) {
        }

        ~StdoutColorSink() override {
        }

    private:
        ConsoleSinkStdout console_sink;
    };
} // namespace heaton
