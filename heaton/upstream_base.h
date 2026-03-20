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
#include <turbo/utility/status.h>
#include <turbo/base/nullability.h>
#include <mutex>
#include <turbo/container/flat_hash_map.h>

namespace heaton {
    struct UpstreamOption {
        bool enable_turbo{true};
        bool enable_absl{true};
        bool enable_glog{true};
        std::vector<std::string> enabled_upstreams;
    };

    /// UpstreamBase
    /// upstream sink for logs, lock by global
    /// to avoid multi upstream conflict,
    /// this is low race only at beginning
    /// or re-config phase need to operate.
    class UpstreamBase {
    public:
        static std::mutex global_mutex;

        UpstreamBase(std::string_view name) : _name(name) {
        }

        virtual ~UpstreamBase() = default;

        const std::string &name() const {
            return _name;
        }

        turbo::Status enable(turbo::Nonnull<void *> sink, std::string_view name);

        turbo::Status disable(turbo::Nonnull<void *> sink);

        turbo::Status disable(std::string_view name);

        turbo::flat_hash_map<void *, std::string> sinks() const {
            std::lock_guard<std::mutex> lock(global_mutex);
            return _sinks;
        }

        turbo::flat_hash_map<std::string, void *> sinks_by_name() const {
            std::lock_guard<std::mutex> lock(global_mutex);
            return _names;
        }

    protected:
        virtual turbo::Status enable_upstream(turbo::Nonnull<void *> sink, std::string_view name) = 0;

        virtual turbo::Status disable_upstream(turbo::Nonnull<void *> sink) = 0;

    protected:
        turbo::flat_hash_map<void *, std::string> _sinks;
        turbo::flat_hash_map<std::string, void *> _names;
        std::string _name;
    };

    class Dispatch;

    class UpstreamerBase {
    public:
        virtual ~UpstreamerBase() = default;

        virtual std::string name() const = 0;

    };
} // namespace heaton
