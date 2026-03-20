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
#include <heaton/glog.h>
#include <heaton/dispatch.h>

namespace heaton {
    class GlogUpstream : public UpstreamBase {
    public:
        GlogUpstream() :UpstreamBase("glog"){}

        ~GlogUpstream() override = default;

        turbo::Status enable_upstream(turbo::Nonnull<void *> sink, std::string_view name) override;

        turbo::Status disable_upstream(turbo::Nonnull<void *> sink) override;

    };

    class GlogUpStreamer : public UpstreamerBase {
    public:
        /// target must be not nullptr.
        GlogUpStreamer() {
        }

        ~GlogUpStreamer() override {
        }

        static UpstreamerBase* instance() {
            static GlogUpStreamer instance;
            return &instance;
        }

        std::string name() const {
            return "glog_upstream";
        }
    };

    class GlogLogSink : public google::LogSink {
    public:
        /// target must be not nullptr.
        GlogLogSink() {
        }

        ~GlogLogSink() override {
        }

        static GlogLogSink* instance() {
            static GlogLogSink instance;
            return &instance;
        }

        void send(google::LogSeverity severity, const char *full_filename,
                  const char *base_filename, int line, const google::LogMessageTime &time,
                  const char *message, size_t message_len) override;

        void setup_dispatch(Dispatch *dis) {
            _dispatch = dis;
        }
    protected:
        Dispatch *_dispatch{nullptr};

    };
} // namespace heaton
