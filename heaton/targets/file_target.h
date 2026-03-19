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
#include <heaton/file_target_base.h>
#include <cstdio>
#include <turbo/times/time.h>
#include <turbo/log/internal/append_file.h>
#include <turbo/container/circular_queue.h>

namespace heaton {
    struct BaseFilename {
        BaseFilename() = default;

        BaseFilename(std::string_view filename);

        ~BaseFilename() = default;

        std::string basename;
        std::string extension;
        std::string directory;
    };

    class FileTarget : public FileTargetBase {
    public:
        FileTarget() : FileTargetBase() {
        }

        ~FileTarget() override = default;

        void apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) override;

    protected:
        virtual void rotate_file(turbo::Time stamp) = 0;

    protected:
        std::unique_ptr<turbo::FileWriter> _file_writer;
    };


} // namespace heaton
