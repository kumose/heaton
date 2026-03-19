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

#include  <heaton/sinks/async_file_sink.h>
#include <turbo/strings/string_builder.h>
#include <turbo/log/log_sink_registry.h>
#include <absl/log/log_sink_registry.h>
#include <turbo/base/internal/sysinfo.h>

namespace heaton {
    void TurboAsyncFileSink::Send(const turbo::LogEntry &entry) {
        std::string_view log_data = entry.newline()
                                        ? entry.text_message_with_prefix_and_newline()
                                        : entry.text_message_with_prefix();
        if (entry.log_severity() == turbo::LogSeverity::kFatal && !entry.stacktrace().empty()) {
            /// keep log and stacktrace together
            turbo::StringBuilder builder;
            builder << log_data << "\n" << entry.stacktrace() << "\n";
            auto msg = builder.str();
            auto entity = std::make_unique<LogEntity>(to_log_level(entry.log_severity()), entry.timestamp(),
                                                      std::move(msg));
            _sink->push_log_entity(std::move(entity));
            return;
        }
        std::string msg(log_data);
        auto entity = std::make_unique<
            LogEntity>(to_log_level(entry.log_severity()), entry.timestamp(), std::move(msg));
        _sink->push_log_entity(std::move(entity));
    }


    void AbslAsyncFileSink::Send(const absl::LogEntry &entry) {
        std::string_view log_data = entry.text_message_with_prefix_and_newline();
        if (entry.log_severity() == absl::LogSeverity::kFatal && !entry.stacktrace().empty()) {
            /// keep log and stacktrace together
            turbo::StringBuilder builder;
            builder << log_data << "\n" << entry.stacktrace() << "\n";
            auto msg = builder.str();
            auto entity = std::make_unique<LogEntity>(to_log_level(entry.log_severity()),
                                                      turbo::Time::from_nanoseconds(
                                                          absl::ToUnixNanos(entry.timestamp())), std::move(msg));
            _sink->push_log_entity(std::move(entity));
            return;
        }

        std::string msg(log_data);
        auto entity = std::make_unique<LogEntity>(to_log_level(entry.log_severity()),
                                                  turbo::Time::from_nanoseconds(absl::ToUnixNanos(entry.timestamp())),
                                                  std::move(msg));
        _sink->push_log_entity(std::move(entity));
    }


    void GlogAsyncFileSink::send(google::LogSeverity severity, const char *full_filename,
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

        auto entity = std::make_unique<LogEntity>(to_log_level(severity), turbo::Time::from_chrono(time.when()),
                                                  std::move(msg));
        _sink->push_log_entity(std::move(entity));
    }

    FileAsyncSinkBase::FileAsyncSinkBase(FileAsyncSink *sink)
        : turbo_file_sink(sink),
          absl_file_sink(sink),
          glog_file_sink(sink) {
    }

    void FileAsyncSinkBase::initialize_internal(const FileSinkOptions &option) {
        if (option.enable_turbo) {
            _turbo = option.enable_turbo;
            turbo::add_log_sink(&turbo_file_sink);
        }

        if (option.enable_absl) {
            _absl = option.enable_absl;
            absl::AddLogSink(&absl_file_sink);
        }

        if (option.enable_glog) {
            _glog = option.enable_glog;
            google::AddLogSink(&glog_file_sink);
        }
    }

    void FileAsyncSinkBase::shutdown() {
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

    turbo::Status FileAsyncSink::initialize(const FileSinkOptions &option) {
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
        initialize_internal(option);

        _thread = std::make_unique<std::thread>(thread_func, this);
        std::unique_lock<std::mutex> lk(_start_mutex);
        _start_cond.wait(lk);
        return turbo::OkStatus();
    }

    void FileAsyncSink::push_log_entity(std::unique_ptr<LogEntity> &&entry) {
        if (entry->level == LogLevel::LL_FATAL) {
            _g_target->apply_log(entry.get());
            stop_work();
            return;
        }
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_running) {
            return;
        }
        if (_log_entities.size() >= _options.max_queue_length) {
            _log_entities.clear();
        }
        _log_entities.push_back(std::move(entry));
        if (_log_entities.size() == 1) {
            _cond.notify_one();
        }
    }

    void FileAsyncSink::thread_func(FileAsyncSink *sink) {
        std::cerr<<"starting async log thread"<<std::endl;
        {
            std::unique_lock lk(sink->_start_mutex);
            sink->_start_cond.notify_one();
        }
        std::deque<std::unique_ptr<LogEntity> > log_entities;
        static const size_t kDefaultWait = 5;
        while (sink->_running.load()) {
            {
                std::unique_lock lk(sink->_mutex);
                while (sink->_log_entities.empty() && sink->_running.load()) {
                    auto ws = sink->_options.flush_interval_s > 0 ? sink->_options.flush_interval_s : kDefaultWait;
                    auto dl = turbo::Time::current_time() + turbo::Duration::seconds(ws);
                    sink->_cond.wait_until(lk, turbo::Time::to_chrono(dl));
                }
                log_entities.swap(sink->_log_entities);
            }
            for (auto &entity: log_entities) {
                sink->_g_target->apply_log(entity.get());
                auto target = sink->_file_target[static_cast<int>(entity->level)];
                target->apply_log(entity.get());
            }
            log_entities.clear();

        }

        {
            std::unique_lock lk(sink->_mutex);
            log_entities.swap(sink->_log_entities);
        }

        for (auto &entity: log_entities) {
            sink->_g_target->apply_log(entity.get());
            auto target = sink->_file_target[static_cast<int>(entity->level)];
            target->apply_log(entity.get());
        }

        std::cerr<<"exiting async log thread"<<std::endl;
    }

    void FileAsyncSink::stop_work() {
        if (!_thread) {
            return;
        }
        _running = false;
        auto entity = std::make_unique<LogEntity>(LogLevel::LL_INFO, turbo::Time::current_time(), "stop log worker");
        push_log_entity(std::move(entity));
        _cond.notify_all();
        _thread->join();
        _thread.reset();
    }
} // namespace heaton
