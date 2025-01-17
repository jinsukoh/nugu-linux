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

#ifndef __NUGU_FOCUS_H__
#define __NUGU_FOCUS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file nugu_focus.h
 * @defgroup NuguFocus Focus
 * @ingroup SDKCore
 * @brief Focus control
 *
 * Control the focus by each speaker and microphone resource type.
 * Only one focus can be owned by a resource at the same time, and focus can be
 * taken by priority when requesting focus.
 *
 * @{
 */

/**
 * @brief Supported resource types
 */
enum nugu_focus_resource {
	NUGU_FOCUS_RESOURCE_MIC = 0, /**< Microphone */
	NUGU_FOCUS_RESOURCE_SPK = 1 /**< Speaker */
};

/**
 * @see enum nugu_focus_resource
 */
typedef enum nugu_focus_resource NuguFocusResource;

/**
 * @brief Predefined focus types by priority
 */
enum nugu_focus_type {
	NUGU_FOCUS_TYPE_WAKEWORD, /**< Wakeword */
	NUGU_FOCUS_TYPE_ASR, /**< ASR */
	NUGU_FOCUS_TYPE_ALERT, /**< Alerts */
	NUGU_FOCUS_TYPE_TTS, /**< TTS */
	NUGU_FOCUS_TYPE_ASR_EXPECT, /**< ASR Expect speech */
	NUGU_FOCUS_TYPE_MEDIA, /**< Media player */
	NUGU_FOCUS_TYPE_CUSTOM /**< Custom type */
};

/**
 * @see enum nugu_focus_type
 */
typedef enum nugu_focus_type NuguFocusType;

/**
 * @brief Return type of focus/unfocus callback
 */
enum nugu_focus_result {
	NUGU_FOCUS_OK, /**< OK - got focus */
	NUGU_FOCUS_FAIL, /**< Failure */
	NUGU_FOCUS_REMOVE, /**< OK - Unfocus with remove */
	NUGU_FOCUS_PAUSE /**< OK - Unfocus with pause */
};

/**
 * @see enum nugu_focus_result
 */
typedef enum nugu_focus_result NuguFocusResult;

/**
 * @brief Return type of focus steal callback
 */
enum nugu_focus_steal_result {
	NUGU_FOCUS_STEAL_ALLOW, /**< Steal allow (focus giveup) */
	NUGU_FOCUS_STEAL_REJECT /**< Reject the steal by other focus */
};

/**
 * @see enum nugu_focus_steal_result
 */
typedef enum nugu_focus_steal_result NuguFocusStealResult;

/**
 * @brief Focus object
 */
typedef struct _nugu_focus NuguFocus;

/**
 * @brief Callback prototype for got focus
 */
typedef NuguFocusResult (*NuguFocusCallback)(NuguFocus *focus,
					     NuguFocusResource rsrc,
					     void *event, void *userdata);

/**
 * @brief Callback prototype for lost focus
 */
typedef NuguFocusResult (*NuguUnfocusCallback)(NuguFocus *focus,
					       NuguFocusResource rsrc,
					       void *event, void *userdata);
/**
 * @brief Callback prototype for steal focus
 */
typedef NuguFocusStealResult (*NuguStealfocusCallback)(NuguFocus *self,
						       NuguFocusResource rsrc,
						       void *event,
						       NuguFocus *target,
						       void *userdata);

/**
 * @brief Create new focus object
 * @param[in] name name of focus
 * @param[in] type type of focus
 * @param[in] focus_cb callback for got focus
 * @param[in] unfocus_cb callback for lost focus
 * @param[in] steal_cb callback for steal focus
 * @param[in] userdata data to pass to the user callback
 * @return focus object
 * @see nugu_focus_free()
 */
NuguFocus *nugu_focus_new(const char *name, NuguFocusType type,
			  NuguFocusCallback focus_cb,
			  NuguUnfocusCallback unfocus_cb,
			  NuguStealfocusCallback steal_cb, void *userdata);

/**
 * @brief Destroy the focus object
 * @param[in] focus focus object
 * @see nugu_focus_new()
 */
void nugu_focus_free(NuguFocus *focus);

/**
 * @brief Get the name of focus
 * @param[in] focus focus object
 * @return name. Please don't free the data manually.
 */
const char *nugu_focus_peek_name(NuguFocus *focus);

/**
 * @brief Get the type of focus object
 * @param[in] focus focus object
 * @return focus type
 */
NuguFocusType nugu_focus_get_type(NuguFocus *focus);

/**
 * @brief Get the current focused object
 * @param[in] focus focus object
 * @return focus object
 */
NuguFocus *nugu_focus_peek_top(NuguFocusResource rsrc);

/**
 * @brief Request to get focus.
 * @param[in] focus focus object
 * @param[in] rsrc resource type
 * @param[in] event custom event data to pass the callback
 * @return result
 * @retval 0 success
 * @retval -1 failure
 * @see nugu_focus_release()
 */
int nugu_focus_request(NuguFocus *focus, NuguFocusResource rsrc, void *event);

/**
 * @brief Request to release focus.
 * @param[in] focus focus object
 * @param[in] rsrc resource type
 * @return result
 * @retval 0 success
 * @retval -1 failure
 * @see nugu_focus_request()
 */
int nugu_focus_release(NuguFocus *focus, NuguFocusResource rsrc);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
