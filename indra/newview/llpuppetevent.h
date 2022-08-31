/** 
 * @file llpuppetevent.h
 * @brief Implementation of LLPuppetEvent class.
 *
 * $LicenseInfo:firstyear=2001&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#pragma once
#ifndef LL_LLPUPPETEVENT_H
#define LL_LLPUPPETEVENT_H

//-----------------------------------------------------------------------------
// Header files
//-----------------------------------------------------------------------------
#include <deque>
#include <vector>
#include "lldatapacker.h"
#include "llframetimer.h"
#include "llquaternion.h"
#include "v3math.h"
#include "llframetimer.h"

class LLPuppetJointEvent
{
    //Information about an expression event that we want to broadcast
public:
    enum E_EVENT_FLAG
    {
        EF_POSITION = 1 << 0,
        EF_POSITION_IN_PARENT_FRAME = 1 << 1, // unset-->ROOT_FRAME, set-->PARENT_FRAME
        EF_ROTATION = 1 << 2,
        EF_ROTATION_IN_PARENT_FRAME = 1 << 3, // unset-->ROOT_FRAME, set-->PARENT_FRAME
        EF_SCALE = 1 << 4,
        EF_DISABLE_CONSTRAINT = 1 << 7
    };

    enum E_REFERENCE_FRAME
    {
        ROOT_FRAME = 0,
        PARENT_FRAME = 1
    };

public:
    LLPuppetJointEvent() {}

    void setRotation(const LLQuaternion& rotation, E_REFERENCE_FRAME frame=ROOT_FRAME);
    void setPosition(const LLVector3& position, E_REFERENCE_FRAME frame=ROOT_FRAME);
    void setScale(const LLVector3& scale);
    void setJointID(S32 id);
    void disableConstraint() { mMask |= EF_DISABLE_CONSTRAINT; }

    S16 getJointID() const { return mJointID; }
    LLQuaternion getRotation() const { return mRotation; }
    LLVector3 getPosition() const { return mPosition; }
    LLVector3 getScale() const { return mScale; }

    size_t getSize() const;
    size_t pack(U8* wptr) const;
    size_t unpack(U8* wptr);

    void interpolate(F32 del, const LLPuppetJointEvent& A, const LLPuppetJointEvent& B);

    bool isEmpty() const { return (mMask & (EF_ROTATION | EF_POSITION | EF_SCALE | EF_DISABLE_CONSTRAINT)) == 0; }
    bool hasRotation() const { return (mMask & EF_ROTATION) > 0; }
    bool hasPosition() const { return (mMask & EF_POSITION) > 0; }
    bool hasScale() const { return (mMask & EF_SCALE) > 0; }
    bool hasDisabledConstraint() const { return (mMask & EF_DISABLE_CONSTRAINT) > 0; }

    bool rotationIsParentLocal() const { return (mMask & EF_ROTATION_IN_PARENT_FRAME) > 0; }

    // parent-local position not yet supported
    //bool positionIsParentLocal() const { return (mMask & EF_POSITION_IN_PARENT_FRAME) > 0; }

private:
    LLQuaternion mRotation;
    LLVector3 mPosition;
    LLVector3 mScale;
    U16 mJointID = -1;
    U8 mMask = 0x0;
};

class LLPuppetEvent
{
    //An event is snapshot at mTimestamp (msec from start)
    //with 1 or more joints that have moved or rotated
    //These snapshots along with the time delta are used to
    //reconstruct the animation on the receiving clients.
public:
    typedef std::deque<LLPuppetJointEvent> joint_deq_t;

public:
    LLPuppetEvent() {}
    void addJointEvent(const LLPuppetJointEvent& joint_event);
    bool pack(LLDataPackerBinaryBuffer& buffer);
    bool unpack(LLDataPackerBinaryBuffer& mesgsys);

    // for outbound LLPuppetEvents
    void updateTimestamp() { mTimestamp = (S32)(LLFrameTimer::getElapsedSeconds() * MSEC_PER_SEC); }

    // for inbound LLPuppetEvents we compute a localized timestamp and slam it
    void setTimestamp(S32 timestamp) { mTimestamp = timestamp; }

    S32 getTimestamp() const { return mTimestamp; }
    U32 getNumJoints() const { return mJointEvents.size(); }
    S32 getMinEventSize() const;

public:
    joint_deq_t mJointEvents;

private:
    S32 mTimestamp = 0; // msec
};

#endif
