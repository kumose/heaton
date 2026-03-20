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

#include <heaton/upstream/glog.h>
#include <turbo/log/log_sink.h>
#include <turbo/log/log_sink_registry.h>
#include <heaton/glog.h>
#include <turbo/base/internal/sysinfo.h>
#include <heaton/dispatch.h>

namespace heaton {

    turbo::Status GlogUpstream::enable_upstream(turbo::Nonnull<void *> sink, std::string_view name) {
        google::AddLogSink(static_cast<google::LogSink *>(sink));
        return turbo::OkStatus();
    }

    turbo::Status GlogUpstream::disable_upstream(turbo::Nonnull<void *> sink) {
        google::RemoveLogSink(static_cast<google::LogSink *>(sink));
        return turbo::OkStatus();
    }


    void GlogLogSink::send(google::LogSeverity severity, const char *full_filename,
                             const char *base_filename, int line, const google::LogMessageTime &time,
                             const char *message, size_t message_len) {
        static std::array<char, 4> gl = {'I', 'W', 'E', 'F'};
        auto msg = turbo::str_format("%c%02d%02d %02d:%02d:%02d.%06d  %5d %s:%d] %s\n",
                                     gl[static_cast<int>(severity)],
                                     time.month(),
                                     time.day(),
                                     time.hour(),
                                     time.min(),
                                     time.sec(),
                                     time.usec(),
                                     turbo::base_internal::GetCachedTID(),
                                     base_filename,
                                     line,
                                     std::string_view(message, message_len)
        );
        _dispatch->apply_log(to_log_level(severity), turbo::Time::from_chrono(time.when()), std::move(msg));
    }


}  // namespace heaton
