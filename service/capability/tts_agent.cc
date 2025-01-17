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

#include "nugu_log.h"
#include "nugu_uuid.h"
#include "tts_agent.hh"

namespace NuguCore {

static const std::string capability_version = "1.0";

TTSAgent::TTSAgent()
    : Capability(CapabilityType::TTS, capability_version)
    , cur_token("")
    , speak_status(-1)
    , finish(false)
    , speak_dir(nullptr)
    , pcm(nullptr)
    , decoder(nullptr)
    , ps_id("")
    , tts_listener(nullptr)
{
}

TTSAgent::~TTSAgent()
{
    if (speak_dir) {
        nugu_directive_set_data_callback(speak_dir, NULL, NULL);
        destoryDirective(speak_dir);
        speak_dir = NULL;
    }

    if (decoder) {
        nugu_decoder_free(decoder);
        decoder = NULL;
    }

    if (pcm) {
        nugu_pcm_stop(pcm);
        nugu_pcm_set_event_callback(pcm, NULL, NULL);
        nugu_pcm_set_status_callback(pcm, NULL, NULL);
        nugu_pcm_remove(pcm);
        nugu_pcm_free(pcm);
        pcm = NULL;
    }

    initialized = false;
    CapabilityManager::getInstance()->removeFocus("cap_tts");
}

void TTSAgent::initialize()
{
    if (initialized) {
        nugu_info("It's already initialized.");
        return;
    }

    pcm = nugu_pcm_new("ttsplayer", nugu_pcm_driver_get_default());
    if (nugu_pcm_add(pcm) == -1) {
        nugu_error("ttsplayer is not unique");
        nugu_pcm_free(pcm);
        return;
    }
    decoder = nugu_decoder_new(nugu_decoder_driver_find("opus"), pcm);

    nugu_pcm_set_status_callback(pcm, pcmStatusCallback, this);
    nugu_pcm_set_event_callback(pcm, pcmEventCallback, this);

    nugu_pcm_set_property(pcm, (NuguAudioProperty){ AUDIO_SAMPLE_RATE_22K, AUDIO_FORMAT_S16_LE, 1 });

    CapabilityManager::getInstance()->addFocus("cap_tts", NUGU_FOCUS_TYPE_TTS, this);

    initialized = true;
}

void TTSAgent::pcmStatusCallback(enum nugu_media_status status, void* userdata)
{
    TTSAgent* tts = static_cast<TTSAgent*>(userdata);

    nugu_dbg("pcm state changed(%d -> %d)", tts->speak_status, status);

    switch (status) {
    case MEDIA_STATUS_PLAYING:
        tts->sendEventSpeechStarted(tts->cur_token);
        break;
    case MEDIA_STATUS_STOPPED:
        tts->sendEventSpeechStopped(tts->cur_token);
        break;
    default:
        status = MEDIA_STATUS_STOPPED;
        break;
    }

    tts->speak_status = status;
}

void TTSAgent::pcmEventCallback(enum nugu_media_event event, void* userdata)
{
    TTSAgent* tts = static_cast<TTSAgent*>(userdata);

    switch (event) {
    case MEDIA_EVENT_END_OF_STREAM:
        tts->sendEventSpeechFinished(tts->cur_token);
        tts->finish = true;
        tts->speak_status = MEDIA_STATUS_STOPPED;
        CapabilityManager::getInstance()->releaseFocus("cap_tts", NUGU_FOCUS_RESOURCE_SPK);
        break;
    default:
        break;
    }
}

void TTSAgent::directiveDataCallback(NuguDirective* ndir, void* userdata)
{
    getAttachmentData(ndir, userdata);
}

void TTSAgent::getAttachmentData(NuguDirective* ndir, void* userdata)
{
    TTSAgent* tts = static_cast<TTSAgent*>(userdata);
    unsigned char* buf;
    size_t length = 0;

    buf = nugu_directive_get_data(ndir, &length);
    if (buf) {
        unsigned char* dbuf;
        size_t size;

        dbuf = (unsigned char*)nugu_decoder_decode(tts->decoder, (const void*)buf, length, &size);

        nugu_pcm_push_data(tts->pcm, (const char*)dbuf, size, 0);

        free(dbuf);
        free(buf);
    }

    if (nugu_directive_is_data_end(ndir)) {
        nugu_pcm_push_data_done(tts->pcm);
        tts->destoryDirective(ndir);
        tts->speak_dir = NULL;
    }
}

NuguFocusResult TTSAgent::onFocus(NuguFocusResource rsrc, void* event)
{
    nugu_info("speak_status: %d", speak_status);

    switch (speak_status) {
    case -1:
    case MEDIA_STATUS_STOPPED:
    case MEDIA_STATUS_READY:
        nugu_info("nugu_pcm_start(speak->pcm)");
        if (nugu_directive_get_data_size(speak_dir) > 0)
            getAttachmentData(speak_dir, this);
        break;
    case MEDIA_STATUS_PLAYING:
        nugu_info("already playing");
        break;
    }

    return NUGU_FOCUS_OK;
}

NuguFocusResult TTSAgent::onUnfocus(NuguFocusResource rsrc, void* event)
{
    nugu_pcm_stop(pcm);

    playsync_manager->removeContext(ps_id, getType(), !finish);

    return NUGU_FOCUS_REMOVE;
}

NuguFocusStealResult TTSAgent::onStealRequest(NuguFocusResource rsrc, void* event, NuguFocusType target_type)
{
    if (target_type == NUGU_FOCUS_TYPE_ASR)
        return NUGU_FOCUS_STEAL_ALLOW;
    else
        return NUGU_FOCUS_STEAL_REJECT;
}

void TTSAgent::stopTTS()
{
    if (speak_dir) {
        nugu_directive_set_data_callback(speak_dir, NULL, NULL);
        destoryDirective(speak_dir);
        speak_dir = NULL;
    }
    if (pcm) {
        nugu_pcm_stop(pcm);
    }
}

void TTSAgent::startTTS(NuguDirective* ndir)
{
    finish = false;

    if (pcm)
        nugu_pcm_start(pcm);

    if (ndir) {
        speak_dir = ndir;
        nugu_directive_set_data_callback(speak_dir, directiveDataCallback, this);
    }
}

void TTSAgent::requestTTS(std::string text, std::string play_service_id)
{
    std::string token;
    char* uuid;

    uuid = nugu_uuid_generate_short();
    token = uuid;
    free(uuid);

    sendEventSpeechPlay(token, text, play_service_id);
}

void TTSAgent::processDirective(NuguDirective* ndir)
{
    const char* dname;
    const char* message;

    message = nugu_directive_peek_json(ndir);
    dname = nugu_directive_peek_name(ndir);

    if (!message || !dname) {
        nugu_error("directive message is not correct");
        destoryDirective(ndir);
        return;
    }

    nugu_dbg("message: %s", message);

    // directive name check
    if (!strcmp(dname, "Speak")) {
        parsingSpeak(message, ndir);
    } else if (!strcmp(dname, "Stop")) {
        parsingStop(message);
        destoryDirective(ndir);
    } else {
        nugu_warn("%s[%s] is not support %s directive", getName().c_str(), getVersion().c_str(), dname);
        destoryDirective(ndir);
    }
}

void TTSAgent::updateInfoForContext(Json::Value& ctx)
{
    Json::Value tts;

    tts["version"] = getVersion();
    switch (speak_status) {
    case -1:
        tts["ttsActivity"] = "IDLE";
        break;
    case MEDIA_STATUS_STOPPED:
        tts["ttsActivity"] = "STOPPED";
        break;
    case MEDIA_STATUS_READY:
    case MEDIA_STATUS_PLAYING:
        tts["ttsActivity"] = "PLAYING";
        break;
    default:
        tts["ttsActivity"] = "STOPPED";
        break;
    }

    if (finish)
        tts["ttsActivity"] = "FINISHED";

    ctx[getName()] = tts;
}

std::string TTSAgent::getContextInfo()
{
    Json::Value ctx;
    CapabilityManager* cmanager = CapabilityManager::getInstance();

    updateInfoForContext(ctx);
    return cmanager->makeContextInfo(ctx);
}

void TTSAgent::receiveCommand(CapabilityType from, std::string command, std::string param)
{
}

void TTSAgent::receiveCommandAll(std::string command, std::string param)
{
}

void TTSAgent::sendEventSpeechStarted(std::string token)
{
    sendEventCommon("SpeechStarted", token);

    if (tts_listener)
        tts_listener->onTTSState(TTSState::TTS_SPEECH_START);
}

void TTSAgent::sendEventSpeechFinished(std::string token)
{
    sendEventCommon("SpeechFinished", token);

    if (tts_listener)
        tts_listener->onTTSState(TTSState::TTS_SPEECH_FINISH);
}

void TTSAgent::sendEventSpeechStopped(std::string token)
{
    sendEventCommon("SpeechStopped", token);
}

void TTSAgent::sendEventSpeechPlay(std::string token, std::string text, std::string play_service_id)
{
    Json::StyledWriter writer;
    Json::Value root;
    NuguEvent* event;
    std::string event_json;
    std::string skml = text;

    if (text.size() <= 0)
        return;

    if (text.find("<skml", 0) == std::string::npos)
        skml = "<skml domain=\"general\">" + text + "</skml>";

    event = nugu_event_new(getName().c_str(), "SpeechPlay",
        getVersion().c_str());

    nugu_event_set_context(event, getContextInfo().c_str());

    root["format"] = "SKML";
    root["text"] = skml;
    root["token"] = token;
    if (play_service_id.size())
        root["playServiceId"] = play_service_id;
    else
        root["playServiceId"] = ps_id;
    event_json = writer.write(root);

    nugu_event_set_json(event, event_json.c_str());

    sendEvent(event);

    nugu_event_free(event);
}

void TTSAgent::sendEventCommon(std::string ename, std::string token)
{
    Json::StyledWriter writer;
    Json::Value root;
    NuguEvent* event;
    std::string event_json;

    event = nugu_event_new(getName().c_str(), ename.c_str(),
        getVersion().c_str());

    nugu_event_set_context(event, getContextInfo().c_str());

    root["token"] = token;
    root["playServiceId"] = ps_id;

    event_json = writer.write(root);

    nugu_event_set_json(event, event_json.c_str());

    sendEvent(event);

    nugu_event_free(event);
}

void TTSAgent::parsingSpeak(const char* message, NuguDirective* ndir)
{
    Json::Value root;
    Json::Reader reader;
    std::string format;
    std::string text;
    std::string token;
    std::string charge;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    format = root["format"].asString();
    text = root["text"].asString();
    token = root["token"].asString();
    charge = root["charge"].asString();
    ps_id = root["playServiceId"].asString();

    if (format.size() == 0 || text.size() == 0 || token.size() == 0) {
        nugu_error("There is no mandatory data in directive message");
        return;
    }

    (void)charge;
    cur_token = token;

    stopTTS();

    playsync_manager->addContext(ps_id, getType());

    startTTS(ndir);

    CapabilityManager::getInstance()->requestFocus("cap_tts", NUGU_FOCUS_RESOURCE_SPK, NULL);

    if (tts_listener)
        tts_listener->onTTSText(text);
}

void TTSAgent::parsingStop(const char* message)
{
    Json::Value root;
    Json::Reader reader;
    std::string token;

    if (!reader.parse(message, root)) {
        nugu_error("parsing error");
        return;
    }

    token = root["token"].asString();
    ps_id = root["playServiceId"].asString();

    if (token.size() == 0 || ps_id.size() == 0) {
        nugu_error("There is no mandatory data in directive message");
        return;
    }

    if (cur_token.compare(token)) {
        nugu_error("the token(%s) is not valid", token.c_str());
        return;
    }

    stopTTS();
}

void TTSAgent::setCapabilityListener(ICapabilityListener* clistener)
{
    if (clistener)
        tts_listener = dynamic_cast<ITTSListener*>(clistener);
}

} // NuguCore
