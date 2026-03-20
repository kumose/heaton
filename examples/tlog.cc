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
#include <heaton/glog.h>
#include <turbo/log/logging.h>

int main(int argc, char **argv) {
    heaton::HeatonOption option(argv[0]);
    option.upstream.enable_absl = true;
    option.upstream.enable_turbo = true;
    option.upstream.enable_glog = true;
    option.create_if_missing = true;
    option.global.type = heaton::SinkType::SINK_ASYNC_FILE;
    option.global.target.target_type = heaton::TargetType::TARGET_DAILY;
    option.global.target.filename = "logs/tlog.txt";
    auto rs = heaton::Heaton::get_instance()->initialize(option);
    std::cerr<<rs.to_string()<<std::endl;
    if (!rs.ok()) {
        return 1;
    }
    ABSL_LOG(INFO)<<"absl log";
    KLOG(INFO)<<"turbo log";
    LOG(INFO)<<"glog log";
    return 0;
}