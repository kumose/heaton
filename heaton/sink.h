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

#include <heaton/sinks/color_sink.h>
#include <heaton/file_target_base.h>
#include <heaton/sinks/file_sink.h>
#include <heaton/sinks/async_file_sink.h>

namespace heaton {
    enum class SinkType {
        SINK_STDERR,
        SINK_STDOUT,
        SINK_FILE,
        SINK_ASYNC_FILE,
    };

    struct SinkOption {
        SinkOption(const std::string &a0) : argv0(a0) {
        }

        SinkType type{SinkType::SINK_STDERR};
        std::string argv0;
        FileSinkOptions options;
        /// for file sink
        bool create_if_missing = false;
    };

    class SinkProxy {
    public:
        SinkProxy() : _option("") {
        }

        static SinkProxy *get_instance() {
            static std::unique_ptr<SinkProxy> instance(new SinkProxy());
            return instance.get();
        }

        turbo::Status initialize(const SinkOption &op);

        const SinkOption &get_sink_option() const {
            return _option;
        }

    private:
        turbo::Status initialize_directories() const;

        turbo::Status initialize_directory(const FileTargetOptions &fop) const;

    private:
        bool initialized = false;
        SinkOption _option;
        StderrColorSink _stderr_color_sink;
        StdoutColorSink _stdout_color_sink;
        FileSink _file_sink;
        FileAsyncSink _file_async_sink;
    };
} // namespace heaton
