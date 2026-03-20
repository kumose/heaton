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

#include <heaton/upstream_base.h>
#include <absl/log/log_sink.h>
#include <heaton/dispatch.h>

namespace heaton {
    class AbslUpstream : public UpstreamBase {
    public:
        AbslUpstream() :UpstreamBase("absl"){}

        ~AbslUpstream() override = default;

        turbo::Status enable_upstream(turbo::Nonnull<void *> sink, std::string_view name) override;

        turbo::Status disable_upstream(turbo::Nonnull<void *> sink) override;

    };

    class AbslUpstreamer : public UpstreamerBase {
    public:
        /// target must be not nullptr.
        AbslUpstreamer() = default;

        ~AbslUpstreamer() override {
        }

        static UpstreamerBase* instance() {
            static AbslUpstreamer instance;
            return &instance;
        }


        std::string name() const {
            return "absl_upstream";
        }

    };

    class AbslLogSink : public absl::LogSink {
    public:
        /// target must be not nullptr.
        AbslLogSink() = default;

        ~AbslLogSink() override {
        }

        static AbslLogSink* instance() {
            static AbslLogSink instance;
            return &instance;
        }

        void Send(const absl::LogEntry &entry) override;

        void Flush() override {
        }

        void setup_dispatch(Dispatch *dis) {
            _dispatch = dis;
        }
    protected:
        Dispatch *_dispatch{nullptr};
    };

} // namespace heaton
