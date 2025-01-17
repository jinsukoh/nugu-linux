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

#include "capability_creator.hh"

#include "asr_agent.hh"
#include "audio_player_agent.hh"
#include "display_agent.hh"
#include "extension_agent.hh"
#include "network_manager.hh"
#include "speech_recognizer.hh"
#include "system_agent.hh"
#include "text_agent.hh"
#include "tts_agent.hh"
#include "wakeup_handler.hh"

namespace NuguCore {

const std::list<CapabilityType> CapabilityCreator::CAPABILITY_LIST {
    CapabilityType::ASR,
    CapabilityType::TTS,
    CapabilityType::AudioPlayer,
    CapabilityType::System,
    CapabilityType::Display,
    CapabilityType::Extension,
    CapabilityType::Text
};

ICapabilityInterface* CapabilityCreator::createCapability(CapabilityType ctype)
{
    switch (ctype) {
    case CapabilityType::ASR:
        return new ASRAgent();
    case CapabilityType::TTS:
        return new TTSAgent();
    case CapabilityType::AudioPlayer:
        return new AudioPlayerAgent();
    case CapabilityType::System:
        return new SystemAgent();
    case CapabilityType::Display:
        return new DisplayAgent();
    case CapabilityType::Extension:
        return new ExtensionAgent();
    case CapabilityType::Text:
        return new TextAgent();
    default:
        return nullptr;
    }
}

IWakeupHandler* CapabilityCreator::createWakeupHandler()
{
    return new WakeupHandler();
}

INetworkManager* CapabilityCreator::createNetworkManager()
{
    return new NetworkManager();
}

} // NuguCore
