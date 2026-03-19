# Copyright (C) 2026 Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
###########################################################################
# below are examples, after initialization, modify it to your own.
###########################################################################
###kmcmake_example

set(HEATON_SRC
        log_entity.cc
        file_target_base.cc
        heaton.cc
        sink.cc
        sinks/color_sink.cc
        sinks/file_sink.cc
        sinks/async_file_sink.cc
        targets/console_target.cc
        targets/daily_file_target.cc
        targets/hourly_file_target.cc
        targets/rotating_file_target.cc
        utility/log_filename.cc
)

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME heaton
        SOURCES
        ${HEATON_SRC}
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        ${KMCMAKE_DEPS_LINK}
        PUBLIC
)

###kmcmake_example