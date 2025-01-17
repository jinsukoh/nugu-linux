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

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "capability_manager.hh"
#include "display_agent.hh"
#include "nugu_log.h"

namespace NuguCore {

static const std::string capability_version = "1.0";

DisplayAgent::DisplayAgent()
    : Capability(CapabilityType::Display, capability_version)
    , display_listener(NULL)
    , ps_id("")
    , cur_token("")
{
}

DisplayAgent::~DisplayAgent()
{
}

void DisplayAgent::initialize()
{
}

void DisplayAgent::processDirective(NuguDirective* ndir)
{
    Json::Value root;
    Json::Reader reader;
    const char* dname;
    const char* message;
    std::string duration;

    message = nugu_directive_peek_json(ndir);
    dname = nugu_directive_peek_name(ndir);

    if (!message || !dname) {
        nugu_error("directive message is not correct");
        destoryDirective(ndir);
        return;
    }

    nugu_dbg("message: %s", message);

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        destoryDirective(ndir);
        return;
    }
    ps_id = root["playServiceId"].asString();

    // sync display rendering with context
    PlaySyncManager::DisplayRenderer renderer;
    renderer.cap_type = getType();
    renderer.listener = this;
    renderer.only_rendering = true;
    renderer.render_info = std::make_pair<std::string, std::string>(dname, message);

    playsync_manager->addContext(ps_id, getType(), renderer);

    destoryDirective(ndir);
}

void DisplayAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value display;

    display["version"] = getVersion();
    if (cur_token.size()) {
        display["playServiceId"] = ps_id;
        display["token"] = cur_token;
    }

    ctx[getName()] = display;
}

std::string DisplayAgent::getContextInfo()
{
    Json::Value ctx;
    CapabilityManager* cmanager = CapabilityManager::getInstance();

    updateInfoForContext(ctx);
    return cmanager->makeContextInfo(ctx);
}

void DisplayAgent::receiveCommand(CapabilityType from, std::string command, std::string param)
{
}

void DisplayAgent::receiveCommandAll(std::string command, std::string param)
{
}

void DisplayAgent::setCapabilityListener(ICapabilityListener* listener)
{
    if (listener)
        display_listener = dynamic_cast<IDisplayListener*>(listener);
}

void DisplayAgent::displayRendered(std::string token)
{
    cur_token = token;
}

void DisplayAgent::displayCleared()
{
    cur_token = "";
}

void DisplayAgent::elementSelected(std::string item_token)
{
    sendEventElementSelected(item_token);
}

void DisplayAgent::sendEventElementSelected(std::string item_token)
{
    Json::StyledWriter writer;
    Json::Value root;
    NuguEvent* event;
    std::string event_json;

    event = nugu_event_new(getName().c_str(), "ElementSelected",
        getVersion().c_str());

    nugu_event_set_context(event, getContextInfo().c_str());

    root["playServiceId"] = ps_id;
    root["token"] = item_token;
    event_json = writer.write(root);

    nugu_event_set_json(event, event_json.c_str());

    sendEvent(event);

    nugu_event_free(event);
}

void DisplayAgent::onSyncContext(std::string ps_id, std::pair<std::string, std::string> render_info)
{
    nugu_dbg("Display sync context");

    if (display_listener)
        display_listener->renderDisplay(render_info.first, render_info.second);
}
void DisplayAgent::onReleaseContext(std::string ps_id)
{
    nugu_dbg("Display release context");

    if (display_listener)
        display_listener->clearDisplay();
}

} // NuguCore
