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

#include <heaton/sink.h>
#include <heaton/glog.h>
#include <turbo/log/globals.h>
#include <absl/log/initialize.h>
#include <turbo/files/filesystem.h>

namespace heaton {

    turbo::Status SinkProxy::initialize(const SinkOption &op) {
        _option = op;
        turbo::initialize_log();
        absl::InitializeLog();
        google::InitGoogleLogging(op.argv0.c_str());
        if (op.type == SinkType::SINK_STDERR) {
            _stderr_color_sink.initialize(true, true, true);
            return turbo::OkStatus();
        }
        if (op.type == SinkType::SINK_STDOUT) {
            _stdout_color_sink.initialize(true, true, true);
            return turbo::OkStatus();
        }

        /// init log dir
        TURBO_RETURN_NOT_OK(initialize_directories());
        if (op.type == SinkType::SINK_FILE) {
            return _file_sink.initialize(_option.options);
        }
        if (op.type == SinkType::SINK_ASYNC_FILE) {
            return _file_async_sink.initialize(_option.options);
        }
        return turbo::invalid_argument_error("unknown sink type");
    }

    turbo::Status SinkProxy::initialize_directories() const {
        TURBO_RETURN_NOT_OK(initialize_directory(_option.options.global_option));
        for (auto &t : _option.options.targets) {
            TURBO_RETURN_NOT_OK(initialize_directory(t));
        }
        return turbo::OkStatus();
    }

    turbo::Status SinkProxy::initialize_directory(const FileTargetOptions &fop) const {
        if (fop.log_type == TargetType::TARGET_NULL) {
            return turbo::OkStatus();
        }
        if (fop.filename.empty()) {
            return turbo::invalid_argument_error("log dir:", fop.filename, " required");
        }
        BaseFilename fn(fop.filename);
        TURBO_MOVE_OR_RAISE(auto e, turbo::is_directory(fn.directory));
        if (e) {
            return turbo::OkStatus();
        }
        if (!_option.create_if_missing) {
            return turbo::unavailable_error("log dir:", fn.directory, " not exists and not assign create");
        }
        TURBO_MOVE_OR_RAISE(auto c, turbo::create_directories(fn.directory));
        if (!c) {
            return turbo::unavailable_error("log dir:", fn.directory, " not create");
        }
        return turbo::OkStatus();
    }
}  // namespace heaton
