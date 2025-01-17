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

#ifndef __NUGU_MEDIA_H__
#define __NUGU_MEDIA_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file nugu_media.h
 * @defgroup Media Media
 * @ingroup SDKCore
 * @brief media management
 *
 * @{
 */

/**
 * @brief Default value of minimum volume
 */
#define NUGU_SET_VOLUME_MIN 0

/**
 * @brief Default value of maximum volume
 */
#define NUGU_SET_VOLUME_MAX 100

/**
 * @brief Default value of volume
 */
#define NUGU_SET_VOLUME_DEFAULT 50

/**
 * @brief Default value of loading timeout
 */
#define NUGU_SET_LOADING_TIMEOUT 5

/**
 * @brief media status
 */
enum nugu_media_status {
	MEDIA_STATUS_STOPPED, /**< media stopped */
	MEDIA_STATUS_READY, /**< media ready */
	MEDIA_STATUS_PLAYING, /**< media playing */
	MEDIA_STATUS_PAUSED /**< media paused */
};

/**
 * @brief media event type
 */
enum nugu_media_event {
	MEDIA_EVENT_MEDIA_SOURCE_CHANGED, /**< media source changed */
	MEDIA_EVENT_MEDIA_INVALID, /**< media invalid */
	MEDIA_EVENT_MEDIA_LOAD_FAILED, /**< media load failed */
	MEDIA_EVENT_MEDIA_LOADED, /**< media loaded */
	MEDIA_EVENT_MEDIA_UNDERRUN, /**< media buffer underrun */
	MEDIA_EVENT_MEDIA_BUFFER_FULL, /**< media buffer rull */
	MEDIA_EVENT_END_OF_STREAM, /**< end of stream */
	MEDIA_EVENT_MAX
};

/**
 * @brief Callback type for media status
 */
typedef void (*mediaStatusCallback)(enum nugu_media_status status,
				    void *userdata);

/**
 * @brief Callback type for media event
 */
typedef void (*mediaEventCallback)(enum nugu_media_event event, void *userdata);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
