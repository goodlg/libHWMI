/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
//#define LOG_NDEBUG 0
#define LOG_TAG "Cam-HAL"

#include <ctype.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <linux/msm_mdp.h>
#include <linux/msm_ion.h>
#include <sys/mman.h>

#include <utils/Log.h>

#include "mm_camera_interface.h"

#include "mm_hwmi.h"

mm_camera_vtbl_t *mCameraHandle;
int32_t mCameraOpened = 0;

void camEvtHandle(uint32_t camera_handle,
                mm_camera_event_t *evt,
                void *user_data)
{
    ALOGI("%s: ", __func__);
    if (evt) {
        switch(evt->server_event_type) {
            case CAM_EVENT_TYPE_DAEMON_DIED:
                ALOGE("%s: Fatal, camera daemon died", __func__);
                break;
            case CAM_EVENT_TYPE_DAEMON_PULL_REQ:
                ALOGI("%s: HAL got request pull from Daemon", __func__);
                break;
            default:
                ALOGI("%s: Warning: Unhandled event %d", __func__,
                        evt->server_event_type);
                break;
        }
    } else {
        ALOGE("%s: NULL user_data/evt", __func__);
    }
}

int32_t mm_camera_open(int32_t cam_id)
{
    int rc = 0;

    if (mCameraHandle) {
        ALOGE("Failure: Camera already opened");
        return -1;
    }

    ALOGI("%s: open (id %d)", __func__, cam_id);

    mCameraHandle = camera_open((uint8_t)cam_id);
    if (NULL == mCameraHandle) {
        ALOGE("%s: dev open error, reason: %s(%d)\n", __func__, strerror(errno), errno);
        return errno;
    }

    mCameraOpened = 1;

    rc = mCameraHandle->ops->register_event_notify(mCameraHandle->camera_handle,
            camEvtHandle, (void *)&mCameraOpened);
    if (rc < 0) {
        ALOGE("%s: Error, failed to register event callback", __func__);
    }

    return rc;
}

int32_t mm_camera_close()
{
    int rc = 0;

    if (mCameraOpened) {
    	rc = mCameraHandle->ops->close_camera(mCameraHandle->camera_handle);
    	mCameraHandle = NULL;
    	mCameraOpened = 0;
    }

    return rc;
}

int32_t mm_camera_get_num()
{
    return get_num_of_cameras();
}

