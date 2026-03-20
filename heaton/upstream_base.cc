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

#include <heaton/upstream_base.h>

namespace heaton {

    std::mutex UpstreamBase::global_mutex;

    turbo::Status UpstreamBase::enable(turbo::Nonnull<void*> sink, std::string_view name) {
        std::lock_guard<std::mutex> lock(global_mutex);
        auto it = _sinks.find(sink);
        if (it != _sinks.end()) {
            return turbo::already_exists_error("sink already exists, ptr:", sink, " by name:", name);
        }
        auto nit = _names.find(name);
        if (nit != _names.end()) {
            return turbo::already_exists_error("name already exists, name:", name, " by ptr:", sink);
        }
        TURBO_RETURN_NOT_OK(enable_upstream(sink,name));
        _sinks[sink] = name;
        _names[name] = sink;
        return turbo::OkStatus();
    }

    turbo::Status UpstreamBase::disable(turbo::Nonnull<void*> sink) {
        std::lock_guard<std::mutex> lock(global_mutex);
        auto it = _sinks.find(sink);
        if (it == _sinks.end()) {
            return turbo::not_found_error("not found sink by ptr:", sink);
        }
        TURBO_RETURN_NOT_OK(disable_upstream(sink));
        _names.erase(it->second);
        _sinks.erase(sink);
        return turbo::OkStatus();
    }

    turbo::Status UpstreamBase::disable(std::string_view name) {
        std::lock_guard<std::mutex> lock(global_mutex);
        auto it = _names.find(name);
        if (it == _names.end()) {
            return turbo::not_found_error("not found sink by name:", name);
        }
        TURBO_RETURN_NOT_OK(disable_upstream(it->second));
        _sinks.erase(it->second);
        _names.erase(name);
        return turbo::OkStatus();
    }
}  // namespace heaton
