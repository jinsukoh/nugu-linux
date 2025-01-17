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

#ifndef __NUGU_CAPABILITY_INTERFACE_H__
#define __NUGU_CAPABILITY_INTERFACE_H__

#include <list>
#include <string>

#include <core/nugu_directive.h>
#include <interface/capability/capability_observer.hh>
#include <json/json.h>

namespace NuguInterface {

/**
 * @file capability_interface.hh
 * @defgroup CapabilityInterface CapabilityInterface
 * @ingroup SDKNuguInterface
 * @brief capability interface
 *
 * An abstract object that a capability agent must perform in common.
 *
 * @{
 */

/**
 * @brief CapabilityType
 */
enum class CapabilityType {
    AudioPlayer, /**< the type of AudioPlayer agent */
    Display, /**< the type of Display agent */
    System, /**< the type of System agent */
    TTS, /**< the type of TTS agent */
    ASR, /**< the type of ASR agent */
    Text, /**< the type of Text agent */
    Extension /**< the type of Extension agent */
};

/**
 * @brief capability handler interface
 * @see ICapabilityListener
 */
class ICapabilityHandler {
public:
    virtual ~ICapabilityHandler() = default;
};

/**
 * @brief capability listener interface
 * @see ICapabilityHandler
 */
class ICapabilityListener {
public:
    virtual ~ICapabilityListener() = default;

    /**
     * @brief Set the handler object
     * @param[in] capa_handler capability object
     */
    virtual void registerCapabilityHandler(ICapabilityHandler* capa_handler);

protected:
    ICapabilityHandler* handler = nullptr;
};

/**
 * @brief capability observer interface
 * @see ICapabilityObserver
 */
class ICapabilityObservable {
public:
    virtual ~ICapabilityObservable() = default;

    /**
     * @brief Add the observer
     * @param[in] observer observer
     */
    virtual void registerObserver(ICapabilityObserver* observer) = 0;

    /**
     * @brief Remove the observer
     * @param[in] observer observer
     */
    virtual void removeObserver(ICapabilityObserver* observer) = 0;

    /**
     * @brief notify signal to the observer
     * @param[in] signal signal
     * @param[in] data signal's metadata
     */
    virtual void notifyObservers(CapabilitySignal signal, void* data) = 0;
};

/**
 * @brief capability interface
 * @see NuguDirective
 * @see ICapabilityListener
 */
class ICapabilityInterface : public ICapabilityObservable {
public:
    virtual ~ICapabilityInterface() = default;

    /**
     * @brief Initialize the current object.
     */
    virtual void initialize() = 0;

    /**
     * @brief Get the capability type name of the current object.
     * @param[in] type capability type
     * @return capability type name
     */
    virtual std::string getTypeName(CapabilityType type) = 0;

    /**
     * @brief Get the capability type of the current object.
     * @return capability type of the object
     */
    virtual CapabilityType getType() = 0;

    /**
     * @brief Process directive received from Directive Sequencer.
     * @param[in] ndir directive
     */
    virtual void processDirective(NuguDirective* ndir) = 0;

    /**
     * @brief Update the current context information of the capability agent.
     * @param[in] ctx capability agent's context
     */
    virtual void updateInfoForContext(Json::Value& ctx) = 0;

    /**
     * @brief Process commands from other objects.
     * @param[in] from capability who send the command
     * @param[in] command command
     * @param[in] param command parameter
     */
    virtual void receiveCommand(CapabilityType from, std::string command, std::string param) = 0;

    /**
     * @brief Processes command received from capability manager.
     * @param[in] command command
     * @param[in] param command parameter
     */
    virtual void receiveCommandAll(std::string command, std::string param) = 0;

    /**
     * @brief It is possible to share own property value among objects.
     * @param[in] property calability property
     * @param[in] values capability property value
     */
    virtual void getProperty(std::string property, std::string& value) = 0;

    /**
     * @brief It is possible to share own property values among objects.
     * @param[in] property calability property
     * @param[in] values capability property values
     */
    virtual void getProperties(std::string property, std::list<std::string>& values) = 0;

    /**
     * @brief Set the listener object
     * @param[in] clistener listener
     */
    virtual void setCapabilityListener(ICapabilityListener* clistener) = 0;
};

/**
 * @}
 */

} // NuguInterface

#endif /* __NUGU_CAPABILITY_INTERFACE_H__ */
