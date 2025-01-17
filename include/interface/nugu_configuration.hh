/*
 * Copyright (c) 2019 SK Telecom Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __NUGU_CONFIGURATION_H__
#define __NUGU_CONFIGURATION_H__

#include <map>
#include <string>

#include <core/nugu_config.h>

namespace NuguClientKit {

/**
 * @file nugu_configuration.hh
 */

namespace NuguConfig {
    using NuguConfigType = std::map<std::string, std::string>;

    namespace Key {
        const std::string WAKEUP_WITH_LISTENING = "wakeup_with_listening";
        const std::string WAKEUP_WORD = "wakeup_word";
        const std::string ASR_EPD_TYPE = "asr_epd_type";
        const std::string ASR_ENCODING = "asr_encoding";
        const std::string MODEL_PATH = "model_path";
        const std::string SERVER_TYPE = "server_type";
        const std::string USER_AGENT = NUGU_CONFIG_KEY_USER_AGENT;
        const std::string GATEWAY_REGISTRY_DNS = NUGU_CONFIG_KEY_GATEWAY_REGISTRY_DNS;
        const std::string UUID_PHASE = NUGU_CONFIG_KEY_UUID_PHASE;
    }

    const NuguConfigType getDefaultValues();
    const NuguConfigType getDefaultValues(NuguConfigType& user_map);
}

} // NuguClientKit

#endif /* __NUGU_CONFIGURATION__ */
