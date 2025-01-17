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

#include <algorithm>
#include <iostream>

#include "nugu_log.h"
#include "playsync_manager.hh"

#define HOLD_TIME_SHORT (7 * 1000) // 7 sec
#define HOLD_TIME_MID (15 * 1000) // 15 sec
#define HOLD_TIME_LONG (30 * 1000) // 30 sec
#define HOLD_TIME_LONGEST (10 * 60 * 1000) // 10 min
#define DEFAULT_HOLD_TIME HOLD_TIME_SHORT

namespace NuguCore {

/*******************************************************************************
 * >>> Temp
 ******************************************************************************/
namespace Test {
    const std::string C_WHITE = "\033[1;97m";
    const std::string C_RESET = "\033[0m";

    void showAllContents(std::map<std::string, std::vector<CapabilityType>> context_map)
    {
        std::string cap_name;

        std::cout << C_WHITE
                  << "\n>>>>> (ContextStack) >>>>>\n";

        for (auto context : context_map) {
            std::cout << "[" << context.first << "] ";

            for (auto element : context.second) {
                if (element == CapabilityType::TTS)
                    cap_name = "TTS";
                else if (element == CapabilityType::AudioPlayer)
                    cap_name = "AudioPlayer";
                else if (element == CapabilityType::Display)
                    cap_name = "Display";

                std::cout << cap_name
                          << ", ";
            }
            std::cout << std::endl;
        }

        std::cout << "<<<<< (ContextStack) <<<<<\n"
                  << C_RESET;
    }
}
/*******************************************************************************
 * <<< Temp
 ******************************************************************************/

PlaySyncManager::PlaySyncManager()
{
    timer = nugu_timer_new(DEFAULT_HOLD_TIME, 1);
    timer_cb_param.instance = this;
}

PlaySyncManager::~PlaySyncManager()
{
    if (timer) {
        nugu_timer_delete(timer);
        timer = nullptr;
    }

    renderer_map.clear();
    context_map.clear();
    context_stack.clear();
}

void PlaySyncManager::addContext(std::string ps_id, CapabilityType cap_type)
{
    addContext(ps_id, cap_type, {});
}

void PlaySyncManager::addContext(std::string ps_id, CapabilityType cap_type, DisplayRenderer renderer)
{
    if (ps_id.empty()) {
        nugu_error("Invalid PlayServiceId.");
        return;
    }

    // add renderer if exist
    if (renderer.listener) {
        if (renderer_map.find(ps_id) == renderer_map.end() || renderer_map[ps_id].cap_type == cap_type)
            addRenderer(ps_id, renderer);

        if (renderer.only_rendering)
            return;
    }

    // remove previous context
    auto play_stack = getAllPlayStackItems();

    for (auto play_item : play_stack) {
        if (ps_id != play_item)
            removeContext(play_item, cap_type, true);
    }

    if (context_map.find(ps_id) != context_map.end()) {
        addStackElement(ps_id, cap_type);
    } else {
        nugu_dbg("[context] add context");

        std::vector<CapabilityType> stack_elements;
        stack_elements.push_back(cap_type);
        context_map[ps_id] = stack_elements;
        context_stack.push_back(ps_id);
    }

    // temp: Just debugging & test. Please, maintain in some period.
    Test::showAllContents(context_map);
}

void PlaySyncManager::removeContext(std::string ps_id, CapabilityType cap_type, bool immediately)
{
    if (ps_id.empty()) {
        nugu_error("Invalid PlayServiceId.");
        return;
    }

    if (removeStackElement(ps_id, cap_type) && !is_expect_speech) {
        auto timerCallback = [](void* userdata) {
            nugu_dbg("[context] remove context");

            TimerCallbackParam* param = static_cast<TimerCallbackParam*>(userdata);
            param->instance->context_map.erase(param->ps_id);
            param->instance->context_stack.erase(remove(param->instance->context_stack.begin(), param->instance->context_stack.end(), param->ps_id),
                param->instance->context_stack.end());
            param->instance->removeRenderer(param->ps_id);
            param->callback();
        };

        timer_cb_param.ps_id = ps_id;
        timer_cb_param.callback = [&]() {
            // temp: Just debugging & test. Please, maintain in some period.
            Test::showAllContents(context_map);
        };

        if (immediately || (renderer_map.find(ps_id) == renderer_map.end() && context_stack.size() >= 2)) {
            nugu_dbg("[context] try to remove context immediately.");

            timerCallback(&timer_cb_param);
        } else {
            nugu_dbg("[context] try to remove context by timer.");

            nugu_timer_set_callback(timer, timerCallback, &timer_cb_param);
            nugu_timer_start(timer);
        }
    }

    is_expect_speech = false;
}

std::vector<std::string> PlaySyncManager::getAllPlayStackItems()
{
    std::vector<std::string> play_stack(context_stack);
    std::reverse(play_stack.begin(), play_stack.end());
    return play_stack;
}

std::string PlaySyncManager::getPlayStackItem(CapabilityType cap_type)
{
    for (auto context : context_map) {
        for (auto element : context.second) {
            if (element == cap_type)
                return context.first;
        }
    }

    return "";
}

void PlaySyncManager::addStackElement(std::string ps_id, CapabilityType cap_type)
{
    auto& stack_elements = context_map[ps_id];

    if (std::find(stack_elements.begin(), stack_elements.end(), cap_type) == stack_elements.end())
        stack_elements.push_back(cap_type);
}

bool PlaySyncManager::removeStackElement(std::string ps_id, CapabilityType cap_type)
{
    auto& stack_elements = context_map[ps_id];
    stack_elements.erase(remove(stack_elements.begin(), stack_elements.end(), cap_type), stack_elements.end());

    // temp: Just debugging & test. Please, maintain in some period.
    Test::showAllContents(context_map);

    return stack_elements.empty();
}

void PlaySyncManager::addRenderer(std::string ps_id, DisplayRenderer& renderer)
{
    // remove previous renderers firstly
    auto renderer_keys = getKeyOfMap(renderer_map);

    for (auto renderer_key : renderer_keys) {
        removeRenderer(renderer_key);
    }

    renderer_map[ps_id] = renderer;
    renderer.listener->onSyncContext(ps_id, renderer.render_info);
}

void PlaySyncManager::removeRenderer(std::string ps_id)
{
    if (renderer_map.find(ps_id) != renderer_map.end()) {
        renderer_map[ps_id].listener->onReleaseContext(ps_id);
        renderer_map.erase(ps_id);
    }
}

template <typename T, typename V>
std::vector<std::string> PlaySyncManager::getKeyOfMap(std::map<T, V>& map)
{
    std::vector<std::string> keys;
    keys.reserve(map.size());

    std::transform(map.begin(), map.end(), std::back_inserter(keys),
        [](std::pair<T, V> const& pair) {
            return pair.first;
        });

    return keys;
}

void PlaySyncManager::setExpectSpeech(bool expect_speech)
{
    is_expect_speech = expect_speech;
}

void PlaySyncManager::onMicOn()
{
    nugu_timer_stop(timer);
}

void PlaySyncManager::onASRError()
{
    is_expect_speech = false;

    if (!context_stack.empty())
        removeContext(context_stack.back(), CapabilityType::TTS, true);
}
} // NuguCore
