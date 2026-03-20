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

#include <heaton/upstream/absl.h>
#include <absl/log/log_sink.h>
#include <absl/log/log_sink_registry.h>
#include <heaton/dispatch.h>

namespace heaton {

    turbo::Status AbslUpstream::enable_upstream(turbo::Nonnull<void *> sink, std::string_view name) {
        absl::AddLogSink(static_cast<absl::LogSink *>(sink));
        return turbo::OkStatus();
    }

    turbo::Status AbslUpstream::disable_upstream(turbo::Nonnull<void *> sink) {
        absl::RemoveLogSink(static_cast<absl::LogSink *>(sink));
        return turbo::OkStatus();
    }


    void AbslLogSink::Send(const absl::LogEntry &entry) {
        std::string_view log_data = entry.text_message_with_prefix_and_newline();
        if (entry.log_severity() == absl::LogSeverity::kFatal && !entry.stacktrace().empty()) {
            turbo::StringBuilder builder;
            builder <<log_data<<"\n"<<entry.stacktrace() <<"\n";
            auto msg = builder.str();
            _dispatch->apply_log(to_log_level(entry.log_severity()),
                          turbo::Time::from_nanoseconds(absl::ToUnixNanos(entry.timestamp())), std::move(msg));
        }

        _dispatch->apply_log(to_log_level(entry.log_severity()),
                                turbo::Time::from_nanoseconds(absl::ToUnixNanos(entry.timestamp())), log_data.data(),
                                log_data.size());
    }

}  // namespace heaton
