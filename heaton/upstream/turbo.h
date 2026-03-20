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
#include <heaton/dispatch.h>
#include <turbo/log/log_sink.h>

namespace heaton {
    class TurboUpstream : public UpstreamBase {
    public:
        TurboUpstream() :UpstreamBase("turbo"){}

        ~TurboUpstream() override = default;

        turbo::Status enable_upstream(turbo::Nonnull<void *> sink, std::string_view name) override;

        turbo::Status disable_upstream(turbo::Nonnull<void *> sink) override;

    };

    class TurboUpStreamer : public UpstreamerBase {
    public:
        /// target must be not nullptr.
        TurboUpStreamer() {
        }

        ~TurboUpStreamer() override {
        }

        static UpstreamerBase* instance() {
            static TurboUpStreamer instance;
            return &instance;
        }

        std::string name() const {
            return "turbo_upstream";
        }
    };

    class TurboLogSink : public turbo::LogSink {
    public:
        /// target must be not nullptr.
        TurboLogSink() = default;

        ~TurboLogSink() override = default;

        static TurboLogSink* instance() {
            static TurboLogSink instance;
            return &instance;
        }

        void Send(const turbo::LogEntry &entry) override;

        void Flush() override {
        }

        void setup_dispatch(Dispatch *dis) {
            _dispatch = dis;
        }
    protected:
        Dispatch *_dispatch{nullptr};
    };

} // namespace heaton
