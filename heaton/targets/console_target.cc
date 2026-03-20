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

#include <heaton/targets/console_target.h>
#include <heaton/color.h>

namespace heaton {

    void ConsoleTargetBase::apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) {
        TURBO_UNUSED(::fwrite(data, len, 1, _file));
    }

    void ColorConsoleTargetBase::apply_log(LogLevel l, turbo::Time stamp, const char *data, size_t len) {
        auto color = ConsoleColor::colors_map[static_cast<int>(l)];
        turbo::StringBuilder builder;
        builder << color << std::string_view(data, len) << ConsoleColor::reset;
        auto msg = builder.str();
        TURBO_UNUSED(::fwrite(msg.data(), msg.size(), 1, _file));
    }

}  // namespace heaton

