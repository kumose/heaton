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

#include <heaton/sinks/async_sink.h>

namespace heaton {

    std::array<notify_func, AsyncSink::kMax> AsyncSink::notify_funcs = {
        AsyncSink::notify_logs_empty,
        AsyncSink::notify_logs_empty,
        AsyncSink::notify_logs,
        AsyncSink::notify_logs_empty,
        AsyncSink::notify_logs_empty,
        AsyncSink::notify_logs_empty
    };

    void AsyncSink::notify_logs(AsyncSink *sink) {
        sink->_cond.notify_one();
    }

    AsyncSink::~AsyncSink()  {
        /// stop thread
        _running = false;
        while (_thread) {
            _cond.notify_one();
            _thread->join();
            _thread.reset();
        }
        /// _state should be kStop
        if (_target) {
            TURBO_UNUSED(_target->shutdown());
            _target = NullTarget::create();
        }
    }
    turbo::Status AsyncSink::initialize(const SinkOption &op, std::shared_ptr<TargetBase> &target) {
        if (target == nullptr) {
            return turbo::invalid_argument_error("target is nullptr");
        }
        _target = target;
        _max_queue_size = op.max_queue_length;
        if (_max_queue_size < 4096) {
            _max_queue_size = 4096;
            std::cerr<<"max_queue_size="<<_max_queue_size<<" too small, set to 4096"<<std::endl;
        }

        /// start thread
        _thread = std::make_unique<std::thread>(thread_func, this);
        std::unique_lock<std::mutex> lk(_start_mutex);
        _start_cond.wait(lk);
        return turbo::OkStatus();
    }

    /// dispatch no transfer log, to avoid copy.
    void AsyncSink::apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) {
        if (TURBO_UNLIKELY(_running)) {
            return;
        }
        auto entity = std::make_unique<LogEntity>(l, stamp,
                                                 std::string(data, len));
        std::unique_lock<std::mutex> lock(_mutex);
        apply_entity(std::move(entity));
        notify_funcs[_state.load()](this);
    }

    /// dispatch reformated and allocated log, using move to avoid copy
    void AsyncSink::apply_log(LogLevel l, turbo::Time stamp, std::string &&msg) {
        if (TURBO_UNLIKELY(_running)) {
            return;
        }
        auto entity = std::make_unique<LogEntity>(l, stamp,
                                                std::move(msg));
        std::unique_lock<std::mutex> lock(_mutex);
        apply_entity(std::move(entity));
        notify_funcs[_state.load()](this);
    }

    void AsyncSink::apply_entity(std::unique_ptr<LogEntity> &&entity) {
        if (TURBO_UNLIKELY(_max_queue_size <= _log_entities.size())) {
            _log_entities.clear();
        }
        _log_entities.push_back(std::move(entity));
    }

    void AsyncSink::flush() {
        if (TURBO_UNLIKELY(_running)) {
            return;
        }
        _target->flush();
    }

    void AsyncSink::shutdown() {
        /// stop thread
        _running = false;
        while (_thread) {
            _cond.notify_one();
            _thread->join();
            _thread.reset();
        }
        /// _state should be kStop
        if (_target) {
            TURBO_UNUSED(_target->shutdown());
            _target = NullTarget::create();
        }
    }


    void AsyncSink::thread_func(AsyncSink *sink) {
        std::cerr<<"starting async log thread"<<std::endl;
        sink->_state.store(kStarted);
        {
            std::unique_lock lk(sink->_start_mutex);
            sink->_start_cond.notify_one();
        }
        std::deque<std::unique_ptr<LogEntity> > log_entities;
        while (sink->_running.load()) {
            {
                std::unique_lock lk(sink->_mutex);
                while (sink->_log_entities.empty() && sink->_running.load()) {
                    sink->_state.store(kStarted);
                    sink->_cond.wait(lk);
                }
                log_entities.swap(sink->_log_entities);
                sink->_state.store(kWorking);
            }
            for (auto &entity: log_entities) {
                sink->_target->apply_log(entity.get());
            }
            log_entities.clear();
        }
        sink->_state.store(kExiting);
        {
            std::unique_lock lk(sink->_mutex);
            log_entities.swap(sink->_log_entities);
        }

        for (auto &entity: log_entities) {
            sink->_target->apply_log(entity.get());
        }
        log_entities.clear();
        sink->_state.store(kStop);
        std::cerr<<"exiting async log thread"<<std::endl;
    }
}  // namespace heaton
