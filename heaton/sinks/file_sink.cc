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

#include  <heaton/sinks/file_sink.h>
#include <array>
#include <turbo/strings/string_builder.h>
#include <turbo/log/log_sink_registry.h>
#include <absl/log/log_sink_registry.h>
#include <turbo/base/internal/sysinfo.h>

namespace heaton {
    void TurboFileSink::Send(const turbo::LogEntry &entry) {
        std::unique_lock<std::mutex> lk(*_mutex);
        std::string_view log_data = entry.newline()
                                        ? entry.text_message_with_prefix_and_newline()
                                        : entry.text_message_with_prefix();
        auto target = _file_target[static_cast<size_t>(entry.log_severity())];
        if (entry.log_severity() == turbo::LogSeverity::kFatal && !entry.stacktrace().empty()) {
            /// keep log and stacktrace together
            turbo::StringBuilder builder;
            builder << log_data << "\n" << entry.stacktrace() << "\n";
            auto msg = builder.str();
            target->apply_log(to_log_level(entry.log_severity()), entry.timestamp(), msg.data(), msg.size());
            _g->apply_log(to_log_level(entry.log_severity()), entry.timestamp(), msg.data(), msg.size());
            return;
        }

        target->apply_log(to_log_level(entry.log_severity()), entry.timestamp(), log_data.data(),
                          log_data.size());
        _g->apply_log(to_log_level(entry.log_severity()), entry.timestamp(), log_data.data(),
                      log_data.size());
    }

    void TurboFileSink::set_target(std::array<FileTargetBase *, 4> target) {
        KCHECK(target[0] != nullptr || target[1] != nullptr || target[2] != nullptr || target[3] != nullptr);
        std::unique_lock<std::mutex> lk(*_mutex);
        _file_target = target;
    }

    void TurboFileSink::set_g_target(FileTargetBase *target) {
        std::unique_lock<std::mutex> lk(*_mutex);
        _g = target;
        KCHECK(_g);
    }

    void AbslFileSink::Send(const absl::LogEntry &entry) {
        std::unique_lock<std::mutex> lk(*_mutex);
        std::string_view log_data = entry.text_message_with_prefix_and_newline();
        auto target = _file_target[static_cast<size_t>(entry.log_severity())];
        if (entry.log_severity() == absl::LogSeverity::kFatal && !entry.stacktrace().empty()) {
            /// keep log and stacktrace together
            turbo::StringBuilder builder;
            builder << log_data << "\n" << entry.stacktrace() << "\n";
            auto msg = builder.str();
            target->apply_log(to_log_level(entry.log_severity()),
                              turbo::Time::from_nanoseconds(absl::ToUnixNanos(entry.timestamp())), msg.data(),
                              msg.size());
            _g->apply_log(to_log_level(entry.log_severity()),
                          turbo::Time::from_nanoseconds(absl::ToUnixNanos(entry.timestamp())), msg.data(),
                          msg.size());
            return;
        }

        target->apply_log(to_log_level(entry.log_severity()),
                          turbo::Time::from_nanoseconds(absl::ToUnixNanos(entry.timestamp())), log_data.data(),
                          log_data.size());
        _g->apply_log(to_log_level(entry.log_severity()),
                      turbo::Time::from_nanoseconds(absl::ToUnixNanos(entry.timestamp())), log_data.data(),
                      log_data.size());
    }

    void AbslFileSink::set_target(std::array<FileTargetBase *, 4> target) {
        if (target[0] == nullptr || target[1] == nullptr || target[2] == nullptr || target[3] == nullptr) {
            return;
        }
        std::unique_lock<std::mutex> lk(*_mutex);
        _file_target = target;
    }

    void AbslFileSink::set_g_target(FileTargetBase *target) {
        if (target == nullptr) {
            return;
        }
        std::unique_lock<std::mutex> lk(*_mutex);
        _g = target;
    }


    void GlogFileSink::send(google::LogSeverity severity, const char *full_filename,
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
        std::unique_lock<std::mutex> lk(*_mutex);
        auto target = _file_target[static_cast<size_t>(severity)];
        target->apply_log(to_log_level(severity), turbo::Time::from_chrono(time.when()), msg.data(), msg.size());
        _g->apply_log(to_log_level(severity), turbo::Time::from_chrono(time.when()), msg.data(), msg.size());
    }

    void GlogFileSink::set_target(std::array<FileTargetBase *, 4> target) {
        if (target[0] == nullptr || target[1] == nullptr || target[2] == nullptr || target[3] == nullptr) {
            return;
        }
        std::unique_lock<std::mutex> lk(*_mutex);
        _file_target = target;
    }

    void GlogFileSink::set_g_target(FileTargetBase *target) {
        if (target == nullptr) {
            return;
        }
        std::unique_lock<std::mutex> lk(*_mutex);
        _g = target;
    }

    FileSinkBase::FileSinkBase(FileTargetBase *g, std::array<FileTargetBase *, 4> target)
        : turbo_file_sink(g, &_mutex, target),
          absl_file_sink(g, &_mutex, target),
          glog_file_sink(g, &_mutex, target) {
    }

    void FileSinkBase::initialize_internal(const FileSinkOptions &option, FileTargetBase *g,
                                           std::array<FileTargetBase *, 4> target) {
        if (option.enable_turbo) {
            _turbo = option.enable_turbo;
            turbo_file_sink.set_g_target(g);
            turbo_file_sink.set_target(target);
            turbo::add_log_sink(&turbo_file_sink);
        }

        if (option.enable_absl) {
            _absl = option.enable_absl;
            absl_file_sink.set_g_target(g);
            absl_file_sink.set_target(target);
            absl::AddLogSink(&absl_file_sink);
        }

        if (option.enable_glog) {
            _glog = option.enable_glog;
            glog_file_sink.set_g_target(g);
            glog_file_sink.set_target(target);
            google::AddLogSink(&glog_file_sink);
        }
    }

    void FileSinkBase::shutdown() {
        if (_turbo) {
            turbo::remove_log_sink(&turbo_file_sink);
        }
        if (_absl) {
            absl::RemoveLogSink(&absl_file_sink);
        }
        if (_glog) {
            google::RemoveLogSink(&glog_file_sink);
        }
    }

    void FileSinkBase::set_target(std::array<FileTargetBase *, 4> target) {
        turbo_file_sink.set_target(target);
        absl_file_sink.set_target(target);
        glog_file_sink.set_target(target);
    }

    void FileSinkBase::set_g_target(FileTargetBase *target) {
        turbo_file_sink.set_g_target(target);
        absl_file_sink.set_g_target(target);
        glog_file_sink.set_g_target(target);
    }

    turbo::Status FileSink::initialize(const FileSinkOptions &option) {
        auto creat_fn = [](const FileTargetOptions &option) -> std::unique_ptr<FileTargetBase> {
            switch (option.log_type) {
                case TargetType::TARGET_DAILY:
                    return std::make_unique<DailyFileTarget>();
                case TargetType::TARGET_HOURLY:
                    return std::make_unique<HourlyFileTarget>();
                case TargetType::TARGET_ROTATING:
                    return std::make_unique<RotatingFileTarget>();
                default:
                    return nullptr;
            }
        };

        if (!option.targets.empty() && option.targets.size() != 4) {
            return turbo::invalid_argument_error("bad target number, must be 4 or 0");
        }
        _file_target = get_null_target_array<4>();
        if (option.global_option.log_type != TargetType::TARGET_NULL) {
            _real_g_target = creat_fn(option.global_option);
            if (!_real_g_target) {
                return turbo::invalid_argument_error("create file sink error");
            }
            TURBO_RETURN_NOT_OK(_real_g_target->initialize(option.global_option));
            _g_target = _real_g_target.get();
        }


        _real_targets.clear();

        for (size_t i = 0; i < option.targets.size(); i++) {
            if (option.targets[i].log_type == TargetType::TARGET_NULL) {
                continue;
            }
            auto it = _real_targets.find(option.targets[i].filename);
            if (it == _real_targets.end()) {
                _real_targets[option.targets[i].filename] = creat_fn(option.targets[i]);
                it = _real_targets.find(option.targets[i].filename);
                if (!it->second) {
                    return turbo::invalid_argument_error("create file sink error");
                }

                TURBO_RETURN_NOT_OK(it->second->initialize(option.targets[i]));
            }
            _file_target[i] = it->second.get();
        }
        initialize_internal(option, _g_target, _file_target);
        return turbo::OkStatus();
    }
} // namespace heaton
