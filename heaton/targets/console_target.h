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

#include <mutex>
#include <heaton/target_base.h>
#include <cstdio>

namespace heaton {
    class ConsoleTargetBase : public TargetBase {
    public:
        ConsoleTargetBase(FILE *file) : _file{file} {
        }

        ~ConsoleTargetBase() override = default;

        turbo::Status initialize(const TargetOptions &base) override {
            TURBO_UNUSED(base);
            return turbo::OkStatus();
        }

        turbo::Status shutdown() override {
            return turbo::OkStatus();
        }

        void apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) override;

    private:
        FILE *_file{nullptr};
    };

    class ConsoleTargetStdout : public ConsoleTargetBase {
    public:
        ConsoleTargetStdout() : ConsoleTargetBase(stdout) {
        }

        ~ConsoleTargetStdout() override = default;

        static std::shared_ptr<TargetBase> create() {
            static std::shared_ptr<TargetBase> sink(new(std::nothrow) ConsoleTargetStdout());
            return sink;
        }

    };

    class ConsoleTargetStderr : public ConsoleTargetBase {
    public:
        ConsoleTargetStderr() : ConsoleTargetBase(stderr) {
        }

        ~ConsoleTargetStderr() override = default;
        static std::shared_ptr<TargetBase> create() {
            static std::shared_ptr<TargetBase> sink(new(std::nothrow) ConsoleTargetStderr());
            return sink;
        }
    };

    class ColorConsoleTargetBase : public TargetBase {
    public:
        ColorConsoleTargetBase(FILE *file) : _file{file} {
        }

        ~ColorConsoleTargetBase() override = default;

        turbo::Status initialize(const TargetOptions &base) override {
            TURBO_UNUSED(base);
            return turbo::OkStatus();
        }

        turbo::Status shutdown() override {
            return turbo::OkStatus();
        }

        void apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) override;

    private:
        FILE *_file{nullptr};
    };

    class ColorConsoleTargetStdout : public ColorConsoleTargetBase {
    public:
        ColorConsoleTargetStdout() : ColorConsoleTargetBase(stdout) {
        }

        ~ColorConsoleTargetStdout() override = default;

        static std::shared_ptr<TargetBase> create() {
            static std::shared_ptr<TargetBase> sink(new(std::nothrow) ColorConsoleTargetStdout());
            return sink;
        }

    };

    class ColorConsoleTargetStderr : public ColorConsoleTargetBase {
    public:
        ColorConsoleTargetStderr() : ColorConsoleTargetBase(stderr) {
        }

        ~ColorConsoleTargetStderr() override = default;
        static std::shared_ptr<TargetBase> create() {
            static std::shared_ptr<TargetBase> sink(new(std::nothrow) ColorConsoleTargetStderr());
            return sink;
        }
    };

} // namespace heaton
