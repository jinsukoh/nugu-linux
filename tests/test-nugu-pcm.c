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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "nugu_pcm.h"

static enum nugu_media_status _status;
static enum nugu_media_event _event;

static enum nugu_media_status _status2;
static enum nugu_media_event _event2;

#define CHECK_STATUS(s)                                                        \
	{                                                                      \
		_status = s;                                                   \
	}
#define CHECK_EVENT(e)                                                         \
	{                                                                      \
		_event = e;                                                    \
	}
#define CHECK_STATUS2(s)                                                       \
	{                                                                      \
		_status2 = s;                                                  \
	}
#define CHECK_EVENT2(e)                                                        \
	{                                                                      \
		_event2 = e;                                                   \
	}

static int dummy_start(NuguPcmDriver *driver, NuguPcm *pcm,
		       NuguAudioProperty prop)
{
	if (pcm == nugu_pcm_find("pcm2")) {
		nugu_pcm_emit_status(pcm, MEDIA_STATUS_READY);
		nugu_pcm_emit_event(pcm, MEDIA_EVENT_MEDIA_LOAD_FAILED);
	} else {
		nugu_pcm_emit_event(pcm, MEDIA_EVENT_MEDIA_LOADED);
		nugu_pcm_emit_status(pcm, MEDIA_STATUS_PLAYING);
	}
	return 0;
}

static int dummy_stop(NuguPcmDriver *driver, NuguPcm *pcm)
{
	nugu_pcm_emit_status(pcm, MEDIA_STATUS_STOPPED);
	return 0;
}

static struct nugu_pcm_driver_ops dummy_driver_ops = { .start = dummy_start,
						       .stop = dummy_stop };

static void pcm_status_callback(enum nugu_media_status status, void *userdata)
{
	(void)userdata;
	g_assert(status == _status);
}

static void pcm_event_callback(enum nugu_media_event event, void *userdata)
{
	(void)userdata;
	g_assert(event == _event);
}

static void pcm_status_callback2(enum nugu_media_status status, void *userdata)
{
	(void)userdata;
	g_assert(status == _status2);
}

static void pcm_event_callback2(enum nugu_media_event event, void *userdata)
{
	(void)userdata;
	g_assert(event == _event2);
}

static void test_pcm_default(void)
{
	NuguPcmDriver *driver;
	NuguPcm *pcm;
	NuguAudioProperty prop;
	char tmp[20] = {
		0,
	};

	driver = nugu_pcm_driver_new("dummy", &dummy_driver_ops);
	g_assert(driver != NULL);
	g_assert(nugu_pcm_driver_register(driver) == 0);

	pcm = nugu_pcm_new("tts", driver);
	g_assert(nugu_pcm_add(pcm) == 0);

	nugu_pcm_set_status_callback(pcm, pcm_status_callback, NULL);
	nugu_pcm_set_event_callback(pcm, pcm_event_callback, NULL);

	prop.samplerate = AUDIO_SAMPLE_RATE_22K;
	prop.format = AUDIO_FORMAT_S16_LE;
	prop.channel = 1;
	g_assert(nugu_pcm_set_property(pcm, prop) == 0);

	CHECK_EVENT(MEDIA_EVENT_MEDIA_LOADED);
	CHECK_STATUS(MEDIA_STATUS_PLAYING);
	g_assert(nugu_pcm_start(pcm) == 0);

	/* push data 10 bytes */
	g_assert(nugu_pcm_push_data(pcm, "1234567890", 10, 0) == 10);
	g_assert(nugu_pcm_get_data_size(pcm) == 10);

	/* get data 15 bytes */
	g_assert(nugu_pcm_get_data(pcm, tmp, 15) == 10);
	g_assert(nugu_pcm_get_data_size(pcm) == 0);

	/* push data 5 bytes */
	g_assert(nugu_pcm_push_data(pcm, "abcde", 5, 0) == 5);
	g_assert(nugu_pcm_get_data_size(pcm) == 5);

	memset(tmp, 0, sizeof(tmp));

	/* get data 3 bytes */
	g_assert(nugu_pcm_get_data(pcm, tmp, 3) == 3);
	g_assert_cmpstr(tmp, ==, "abc");
	g_assert(nugu_pcm_get_data_size(pcm) == 2);

	CHECK_STATUS(MEDIA_STATUS_STOPPED);
	g_assert(nugu_pcm_stop(pcm) == 0);

	nugu_pcm_remove(pcm);
	nugu_pcm_free(pcm);

	nugu_pcm_driver_remove(driver);
}

static void test_pcm_multiple(void)
{
	NuguPcmDriver *driver;
	NuguPcm *pcm1, *pcm2;
	NuguAudioProperty prop;

	driver = nugu_pcm_driver_new("dummy", &dummy_driver_ops);
	g_assert(driver != NULL);
	g_assert(nugu_pcm_driver_register(driver) == 0);

	pcm1 = nugu_pcm_new("pcm1", driver);
	pcm2 = nugu_pcm_new("pcm2", driver);

	g_assert(nugu_pcm_add(pcm1) == 0);
	g_assert(nugu_pcm_add(pcm2) == 0);

	nugu_pcm_set_status_callback(pcm1, pcm_status_callback, NULL);
	nugu_pcm_set_event_callback(pcm1, pcm_event_callback, NULL);

	nugu_pcm_set_status_callback(pcm2, pcm_status_callback2, NULL);
	nugu_pcm_set_event_callback(pcm2, pcm_event_callback2, NULL);

	prop.samplerate = AUDIO_SAMPLE_RATE_22K;
	prop.format = AUDIO_FORMAT_S16_LE;
	prop.channel = 1;
	g_assert(nugu_pcm_set_property(pcm1, prop) == 0);

	CHECK_STATUS(MEDIA_STATUS_PLAYING);
	g_assert(nugu_pcm_start(pcm1) == 0);

	CHECK_STATUS(MEDIA_STATUS_STOPPED);
	g_assert(nugu_pcm_stop(pcm1) == 0);

	CHECK_EVENT2(MEDIA_EVENT_MEDIA_LOAD_FAILED);
	CHECK_STATUS2(MEDIA_STATUS_READY);
	g_assert(nugu_pcm_start(pcm2) == 0);

	CHECK_STATUS2(MEDIA_STATUS_STOPPED);
	g_assert(nugu_pcm_stop(pcm2) == 0);

	nugu_pcm_remove(pcm1);
	nugu_pcm_remove(pcm2);

	nugu_pcm_free(pcm1);
	nugu_pcm_free(pcm2);

	nugu_pcm_driver_remove(driver);
}

int main(int argc, char *argv[])
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init();
#endif

	g_test_init(&argc, &argv, NULL);
	g_log_set_always_fatal((GLogLevelFlags)G_LOG_FATAL_MASK);

	g_test_add_func("/pcm/default", test_pcm_default);
	g_test_add_func("/pcm/driver", test_pcm_multiple);

	return g_test_run();
}
