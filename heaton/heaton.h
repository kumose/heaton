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

#include <mutex>
#include <memory>
#include <turbo/container/flat_hash_map.h>
#include <heaton/upstream_base.h>
#include <heaton/dispatch.h>

namespace heaton {
    struct HeatonOption {
        HeatonOption() = default;

        HeatonOption(std::string_view a0) : argv0(a0) {
        }

        UpstreamOption upstream;
        SinkOption global;
        std::vector<SinkOption> infos;
        std::vector<SinkOption> warnings;
        std::vector<SinkOption> errors;
        std::vector<SinkOption> fatals;
        std::string argv0;
        bool create_if_missing{true};
    };

    class Heaton {
    public:
        ~Heaton() = default;

        static Heaton *get_instance() {
            static std::unique_ptr<Heaton> instance(new Heaton());
            return instance.get();
        }


        Dispatch *get_dispatch() {
            return &_dispatch;
        }

        turbo::Status initialize(const HeatonOption &op);

        /////////////////////////////////////////////////////////////////////////////////
        /// dispatch
        void set_options(const DispatchOptions &options) {
            _dispatch.set_options(options);
        }

        DispatchOptions get_options() {
            return _dispatch.get_options();
        }

        void set_global_sink(const SinkBasePtr &sink) {
            _dispatch.set_global_sink(sink);
        }

        turbo::Status add_info_sink(const SinkBasePtr &sink) {
            return _dispatch.add_info_sink(sink);
        }

        turbo::Status add_warning_sink(const SinkBasePtr &sink) {
            return _dispatch.add_warning_sink(sink);
        }

        turbo::Status add_error_sink(const SinkBasePtr &sink) {
            return _dispatch.add_error_sink(sink);
        }

        turbo::Status add_fatal_sink(const SinkBasePtr &sink) {
            return _dispatch.add_fatal_sink(sink);
        }

        void remove_info_sink(const SinkBasePtr &sink) {
            _dispatch.remove_info_sink(sink);
        }

        void remove_warning_sink(const SinkBasePtr &sink) {
            _dispatch.remove_warning_sink(sink);
        }

        void remove_error_sink(const SinkBasePtr &sink) {
            _dispatch.remove_error_sink(sink);
        }

        void remove_fatal_sink(const SinkBasePtr &sink) {
            _dispatch.remove_fatal_sink(sink);
        }

        /////////////////////////////////////////////////////////////////////////////////
        /// for user defined upstream
        turbo::Status enable_upstream(std::unique_ptr<UpstreamBase> &&up, std::string_view sink_name,
                                      turbo::Nonnull<void *> sink);

        turbo::Status enable_upstream(std::string_view up, std::string_view sink_name, turbo::Nonnull<void *> sink);

        turbo::Status disable_upstream(std::string_view up, turbo::Nonnull<void *> sink);

        /// turbo, absl,glog
        static std::unique_ptr<UpstreamBase> make_builtin_upstream(std::string_view name);

        /// turbo, absl,glog
        static void *make_builtin_upstreamer(std::string_view name, Dispatch *dis);

    private:
        Heaton() = default;

        turbo::Status initialize_upstream();

        turbo::Status initialize_directories() const;

        turbo::Status initialize_directory(const TargetOptions &fop) const;

    private:
        turbo::flat_hash_map<std::string, std::unique_ptr<UpstreamBase> > _upstreams;
        Dispatch _dispatch;
        std::mutex _mutex;
        HeatonOption _option;
        bool _initialized{false};
    };
} // namespace heaton
