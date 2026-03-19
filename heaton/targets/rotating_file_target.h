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

#include <heaton/file_target_base.h>
#include <turbo/times/time.h>
#include <turbo/log/internal/append_file.h>
#include <turbo/container/circular_queue.h>
#include <heaton/utility/log_filename.h>

namespace heaton {


    /// leave the multi thread gard for caller sinks proxy eg turbo,glog, absl.
    class RotatingFileTarget : public FileTargetBase {
    public:
        RotatingFileTarget();

        ~RotatingFileTarget() override;

        turbo::Status initialize(const FileTargetOptions &base) override;

        void apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) override;

        void flush() override;

        turbo::Status shutdown() override {
            shutting_down = true;
            return turbo::Status();
        }


    private:
        void rotate_file(turbo::Time stamp, size_t delta);

        static bool rename_file(const std::string &src_filename, const std::string &target_filename);
    };
}  // namespace heaton
