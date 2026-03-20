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

#include <heaton/heaton.h>
#include <heaton/upstream/turbo.h>
#include <heaton/upstream/absl.h>
#include <heaton/upstream/glog.h>
#include <turbo/files/filesystem.h>
#include <absl/log/initialize.h>

namespace heaton {

    turbo::Status Heaton::enable_upstream(std::unique_ptr<UpstreamBase> &&up, std::string_view sink_name, turbo::Nonnull<void*> sink) {
        if (!up) {
            return turbo::invalid_argument_error("UpstreamBase ptr is nullptr");
        }
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _upstreams.find(up->name());
        if (it == _upstreams.end()) {
            TURBO_ABORT_NOT_OK(up->enable(sink, sink_name));
            _upstreams[up->name()] = std::move(up);
            return turbo::OkStatus();
        }
        return it->second->enable(sink, sink_name);
    }

    turbo::Status Heaton::enable_upstream(std::string_view up, std::string_view sink_name, turbo::Nonnull<void*> sink) {
        if (!up.empty()) {
            return turbo::invalid_argument_error("UpstreamBase name is empty");
        }
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _upstreams.find(up);
        if (it != _upstreams.end()) {
            return it->second->enable(sink, sink_name);
        }

        auto ptr = make_builtin_upstream(up);
        if (!ptr) {
            return turbo::invalid_argument_error("unknown upstream name:", up, " not registry or builtin");
        }
        TURBO_ABORT_NOT_OK(ptr->enable(sink, sink_name));
        _upstreams[ptr->name()] = std::move(ptr);
        return turbo::OkStatus();

    }

    turbo::Status Heaton::disable_upstream(std::string_view up,turbo::Nonnull<void *> sink) {
        if (!up.empty()) {
            return turbo::invalid_argument_error("UpstreamBase name is empty");
        }
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _upstreams.find(up);
        if (it == _upstreams.end()) {
            return turbo::invalid_argument_error("upstream not found:", up);
        }
        return it->second->disable(sink);
    }

    std::unique_ptr<UpstreamBase> Heaton::make_builtin_upstream(std::string_view name) {
        if (name.empty()) {
            return nullptr;
        }
        if (turbo::equals_ignore_case(name, "turbo")) {
            return std::make_unique<TurboUpstream>();
        }
        if (turbo::equals_ignore_case(name, "absl")) {
            return std::make_unique<AbslUpstream>();
        }
        if (turbo::equals_ignore_case(name, "glog")) {
            return std::make_unique<GlogUpstream>();
        }
        return nullptr;
    }

    void* Heaton::make_builtin_upstreamer(std::string_view name, Dispatch *dis) {
        if (name.empty()) {
            return nullptr;
        }
        if (turbo::equals_ignore_case(name, "turbo")) {
            auto ptr = TurboLogSink::instance();
            ptr->setup_dispatch(dis);
            return ptr;
        }
        if (turbo::equals_ignore_case(name, "absl")) {
            auto ptr = AbslLogSink::instance();
            ptr->setup_dispatch(dis);
            return ptr;
        }
        if (turbo::equals_ignore_case(name, "glog")) {
            auto ptr = GlogLogSink::instance();
            ptr->setup_dispatch(dis);
            return ptr;
        }
        return nullptr;
    }

    turbo::Status Heaton::initialize(const HeatonOption &op) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_initialized) {
                return turbo::already_exists_error("already inited");
            }
            _initialized = true;
        }

        _option = op;
        TURBO_RETURN_NOT_OK(initialize_upstream());
        TURBO_RETURN_NOT_OK(initialize_directories());
        TURBO_MOVE_OR_RAISE(auto g, SinkBase::create_sink(_option.global));
        _dispatch.set_global_sink(g);

        for (auto &t : _option.infos) {
            TURBO_MOVE_OR_RAISE(auto tptr, SinkBase::create_sink(t));
            TURBO_RETURN_NOT_OK(_dispatch.add_info_sink(tptr));
        }

        for (auto &t : _option.warnings) {
            TURBO_MOVE_OR_RAISE(auto tptr, SinkBase::create_sink(t));
            TURBO_RETURN_NOT_OK(_dispatch.add_info_sink(tptr));
        }
        for (auto &t : _option.errors) {
            TURBO_MOVE_OR_RAISE(auto tptr, SinkBase::create_sink(t));
            TURBO_RETURN_NOT_OK(_dispatch.add_info_sink(tptr));
        }

        for (auto &t : _option.fatals) {
            TURBO_MOVE_OR_RAISE(auto tptr, SinkBase::create_sink(t));
            TURBO_RETURN_NOT_OK(_dispatch.add_info_sink(tptr));
        }

        return turbo::OkStatus();
    }

    turbo::Status Heaton::initialize_upstream() {
        if (_option.upstream.enable_turbo) {
            auto m = make_builtin_upstream("turbo");
            auto p = make_builtin_upstreamer("turbo", &_dispatch);
            if (!m) {
                return turbo::unavailable_error("unknown upstream turbo");
            }
            if (!p) {
                return turbo::unavailable_error("unknown upstreamer turbo");
            }
            TURBO_RETURN_NOT_OK(enable_upstream(std::move(m), "turbo", p));
            turbo::initialize_log();
        }

        if (_option.upstream.enable_absl) {
            auto m = make_builtin_upstream("absl");
            auto p = make_builtin_upstreamer("absl", &_dispatch);
            if (!m) {
                return turbo::unavailable_error("unknown upstream absl");
            }
            if (!p) {
                return turbo::unavailable_error("unknown upstreamer absl");
            }
            TURBO_RETURN_NOT_OK(enable_upstream(std::move(m), "absl", p));
            absl::InitializeLog();
        }

        if (_option.upstream.enable_glog) {
            auto m = make_builtin_upstream("glog");
            auto p = make_builtin_upstreamer("glog", &_dispatch);
            if (!m) {
                return turbo::unavailable_error("unknown upstream glog");
            }
            if (!p) {
                return turbo::unavailable_error("unknown upstreamer glog");
            }
            TURBO_RETURN_NOT_OK(enable_upstream(std::move(m), "glog", p));
            google::InitGoogleLogging(_option.argv0.c_str());
        }
        return turbo::OkStatus();
    }

    turbo::Status Heaton::initialize_directories() const {
        TURBO_RETURN_NOT_OK(initialize_directory(_option.global.target));
        for (auto &t : _option.infos) {
            TURBO_RETURN_NOT_OK(initialize_directory(t.target));
        }
        for (auto &t : _option.warnings) {
            TURBO_RETURN_NOT_OK(initialize_directory(t.target));
        }
        for (auto &t : _option.errors) {
            TURBO_RETURN_NOT_OK(initialize_directory(t.target));
        }
        for (auto &t : _option.fatals) {
            TURBO_RETURN_NOT_OK(initialize_directory(t.target));
        }
        return turbo::OkStatus();
    }

    turbo::Status Heaton::initialize_directory(const TargetOptions &fop) const {
        if (fop.target_type == TargetType::TARGET_NULL) {
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
