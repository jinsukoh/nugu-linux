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

#ifndef __NUGU_WAKEUP_INTERFACE_H__
#define __NUGU_WAKEUP_INTERFACE_H__

namespace NuguInterface {

/**
 * @file wakeup_interface.hh
 * @defgroup WakeupInterface
 * @ingroup SDKNuguInterface
 * @brief Wakeup interface
 *
 * Start the wakeup engine and receive the status of wakeup.
 *
 * @{
 */

/**
 * @brief WakeupDetectState
 */
enum class WakeupDetectState {
    WAKEUP_DETECTING, /**< Wakeup word detecting */
    WAKEUP_DETECTED, /**< Wakeup word is detected */
    WAKEUP_FAIL /**< Failure */
};

/**
 * @brief Wakeup listener interface
 * @see IWakeupHandler
 */
class IWakeupListener {
public:
    virtual ~IWakeupListener() = default;

    /**
     * @brief WakeupState changed
     * @param[in] state wakeup detection state
     * @see IWakeupHandler::startWakeup()
     */
    virtual void onWakeupState(WakeupDetectState state) = 0;
};

/**
 * @brief Wakeup handler interface
 * @see IWakeupListener
 */
class IWakeupHandler {
public:
    virtual ~IWakeupHandler() = default;

    /**
     * @brief Set the Listener object
     * @param[in] listener listener object
     */
    virtual void setListener(IWakeupListener* listener) = 0;

    /**
     * @brief Start the wakeup detection
     * @see IWakeupListener::onWakeupState()
     */
    virtual void startWakeup(void) = 0;
};

/**
 * @}
 */

} // NuguClientKit

#endif /* __NUGU_WAKEUP_INTERFACE_H__ */
