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

#ifndef __NUGU_ASR_AGENT_H__
#define __NUGU_ASR_AGENT_H__

#include <glib.h>
#include <memory>
#include <vector>

#include <core/nugu_focus.h>
#include <core/nugu_timer.h>
#include <interface/capability/asr_interface.hh>

#include "capability.hh"
#include "capability_manager.hh"
#include "speech_recognizer.hh"

namespace NuguCore {

using namespace NuguInterface;

#define DEFAULT_RESPONSE_TIMEOUT (10 * 1000) // 10ms

typedef struct expect_speech_attr {
    bool is_handle;
    std::string timeout;
    std::string session_id;
    std::string play_service_id;
    std::string property;
    Json::Value domain_types;
} ExpectSpeechAttr;

class ASRAgent : public Capability,
                 public IASRHandler,
                 public ISpeechRecognizerListener {
public:
    ASRAgent();
    virtual ~ASRAgent();
    void initialize() override;

    void startRecognition(void) override;
    void stopRecognition(void) override;

    void processDirective(NuguDirective* ndir) override;
    void updateInfoForContext(Json::Value& ctx) override;
    std::string getContextInfo();
    void saveAllContextInfo();

    void receiveCommand(CapabilityType from, std::string command, std::string param) override;
    void receiveCommandAll(std::string command, std::string param) override;
    void getProperty(std::string property, std::string& value) override;
    void getProperties(std::string property, std::list<std::string>& values) override;

    void checkResponseTimeout();
    void clearResponseTimeout();

    void sendEventRecognize(unsigned char* data, size_t length, bool is_end);
    void sendEventResponseTimeout();
    void sendEventListenTimeout();
    void sendEventListenFailed();

    void setCapabilityListener(ICapabilityListener* clistener) override;
    void addListener(IASRListener* listener);
    void removeListener(IASRListener* listener);
    std::vector<IASRListener*> getListener();

    void resetExpectSpeechState();
    bool isExpectSpeechState();

private:
    // implements ISpeechRecognizerListener
    void onListeningState(ListeningState state) override;
    void onRecordData(unsigned char* buf, int length) override;

    // parsing directive
    void parsingExpectSpeech(const char* message);
    void parsingNotifyResult(const char* message);
    void parsingStopCapture(const char* message);

    void releaseASRSpeakFocus(bool is_cancel, ASRError error);

    ExpectSpeechAttr es_attr;
    NuguEvent* rec_event;
    NuguTimer* timer;
    bool mic_off;
    std::unique_ptr<SpeechRecognizer> speech_recognizer;
    std::string epd_type;
    std::string asr_encoding;
    std::string all_context_info;
    std::string dialog_id;
    IFocusListener* asr_focus_listener;
    IFocusListener* expect_focus_listener;
    std::vector<IASRListener*> asr_listeners;
    ListeningState prev_listening_state = ListeningState::DONE;
};

} // NuguCore

#endif
