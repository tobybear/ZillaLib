//------------------------------------------------------------------------------
/* Copyright (c) 2013, Esoteric Software
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
//------------------------------------------------------------------------------
/*
 json.c Copyright (c) 2009 Dave Gamble
 
 Permission is hereby granted, dispose of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */
//------------------------------------------------------------------------------

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "spine.h"

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Animation.c
//

Animation* Animation_create (const char* name, int timelineCount) {
	Animation* self = NEW(Animation);
	MALLOC_STR(self->name, name);
	self->timelineCount = timelineCount;
	self->timelines = MALLOC(Timeline*, timelineCount);
	return self;
}

void Animation_dispose (Animation* self) {
	int i;
	for (i = 0; i < self->timelineCount; ++i)
		Timeline_dispose(self->timelines[i]);
	FREE(self->timelines);
	FREE(self->name);
	FREE(self);
}

void Animation_apply (const Animation* self, Skeleton* skeleton, float time, bool loop) {
	int i, n = self->timelineCount;

#ifdef __STDC_VERSION__
	if (loop && self->duration) time = fmodf(time, self->duration);
#else
	if (loop && self->duration) time = (float)fmod(time, self->duration);
#endif

	for (i = 0; i < n; ++i)
		Timeline_apply(self->timelines[i], skeleton, time, 1);
}

void Animation_mix (const Animation* self, Skeleton* skeleton, float time, bool loop, float alpha) {
	int i, n = self->timelineCount;

#ifdef __STDC_VERSION__
	if (loop && self->duration) time = fmodf(time, self->duration);
#else
	if (loop && self->duration) time = (float)fmod(time, self->duration);
#endif

	for (i = 0; i < n; ++i)
		Timeline_apply(self->timelines[i], skeleton, time, alpha);
}



typedef struct _TimelineVtable {
	void (*apply) (const Timeline* self, Skeleton* skeleton, float time, float alpha);
	void (*dispose) (Timeline* self);
} _TimelineVtable;

void _Timeline_init (Timeline* self, 
		void (*dispose) (Timeline* self), 
		void (*apply) (const Timeline* self, Skeleton* skeleton, float time, float alpha)) {
	CONST_CAST(void*, self->vtable) = NEW(_TimelineVtable);
	VTABLE(Timeline, self) ->dispose = dispose;
	VTABLE(Timeline, self) ->apply = apply;
}

void _Timeline_deinit (Timeline* self) {
	FREE(self->vtable);
}

void Timeline_dispose (Timeline* self) {
	VTABLE(Timeline, self) ->dispose(self);
}

void Timeline_apply (const Timeline* self, Skeleton* skeleton, float time, float alpha) {
	VTABLE(Timeline, self) ->apply(self, skeleton, time, alpha);
}



static const float CURVE_LINEAR = 0;
static const float CURVE_STEPPED = -1;
static const int CURVE_SEGMENTS = 10;

void _CurveTimeline_init (CurveTimeline* self, int frameCount, 
		void (*dispose) (Timeline* self), 
		void (*apply) (const Timeline* self, Skeleton* skeleton, float time, float alpha)) {
	_Timeline_init(SUPER(self), dispose, apply);
	self->curves = CALLOC(float, (frameCount - 1) * 6);
}

void _CurveTimeline_deinit (CurveTimeline* self) {
	_Timeline_deinit(SUPER(self));
	FREE(self->curves);
}

void CurveTimeline_setLinear (CurveTimeline* self, int frameIndex) {
	self->curves[frameIndex * 6] = CURVE_LINEAR;
}

void CurveTimeline_setStepped (CurveTimeline* self, int frameIndex) {
	self->curves[frameIndex * 6] = CURVE_STEPPED;
}

void CurveTimeline_setCurve (CurveTimeline* self, int frameIndex, float cx1, float cy1, float cx2, float cy2) {
	float subdiv_step = 1.0f / CURVE_SEGMENTS;
	float subdiv_step2 = subdiv_step * subdiv_step;
	float subdiv_step3 = subdiv_step2 * subdiv_step;
	float pre1 = 3 * subdiv_step;
	float pre2 = 3 * subdiv_step2;
	float pre4 = 6 * subdiv_step2;
	float pre5 = 6 * subdiv_step3;
	float tmp1x = -cx1 * 2 + cx2;
	float tmp1y = -cy1 * 2 + cy2;
	float tmp2x = (cx1 - cx2) * 3 + 1;
	float tmp2y = (cy1 - cy2) * 3 + 1;
	int i = frameIndex * 6;
	self->curves[i] = cx1 * pre1 + tmp1x * pre2 + tmp2x * subdiv_step3;
	self->curves[i + 1] = cy1 * pre1 + tmp1y * pre2 + tmp2y * subdiv_step3;
	self->curves[i + 2] = tmp1x * pre4 + tmp2x * pre5;
	self->curves[i + 3] = tmp1y * pre4 + tmp2y * pre5;
	self->curves[i + 4] = tmp2x * pre5;
	self->curves[i + 5] = tmp2y * pre5;
}

float CurveTimeline_getCurvePercent (const CurveTimeline* self, int frameIndex, float percent) {
	float dfy;
	float ddfx;
	float ddfy;
	float dddfx;
	float dddfy;
	float x, y;
	int i;
	int curveIndex = frameIndex * 6;
	float dfx = self->curves[curveIndex];
	if (dfx == CURVE_LINEAR) return percent;
	if (dfx == CURVE_STEPPED) return 0;
	dfy = self->curves[curveIndex + 1];
	ddfx = self->curves[curveIndex + 2];
	ddfy = self->curves[curveIndex + 3];
	dddfx = self->curves[curveIndex + 4];
	dddfy = self->curves[curveIndex + 5];
	x = dfx, y = dfy;
	i = CURVE_SEGMENTS - 2;
	while (1) {
		if (x >= percent) {
			float lastX = x - dfx;
			float lastY = y - dfy;
			return lastY + (y - lastY) * (percent - lastX) / (x - lastX);
		}
		if (i == 0) break;
		i--;
		dfx += ddfx;
		dfy += ddfy;
		ddfx += dddfx;
		ddfy += dddfy;
		x += dfx;
		y += dfy;
	}
	return y + (1 - y) * (percent - x) / (1 - x); 
}


static int binarySearch (float *values, int valuesLength, float target, int step) {
	int low = 0, current;
	int high = valuesLength / step - 2;
	if (high == 0) return step;
	current = high >> 1;
	while (1) {
		if (values[(current + 1) * step] <= target)
			low = current + 1;
		else
			high = current;
		if (low == high) return (low + 1) * step;
		current = (low + high) >> 1;
	}
	return 0;
}





void _BaseTimeline_dispose (Timeline* timeline) {
	struct BaseTimeline* self = SUB_CAST(struct BaseTimeline, timeline);
	_CurveTimeline_deinit(SUPER(self));
	FREE(self->frames);
	FREE(self);
}


struct BaseTimeline* _BaseTimeline_create (int frameCount, int frameSize, 
		void (*apply) (const Timeline* self, Skeleton* skeleton, float time, float alpha)) {

	struct BaseTimeline* self = NEW(struct BaseTimeline);
	_CurveTimeline_init(SUPER(self), frameCount, _BaseTimeline_dispose, apply);

	CONST_CAST(int, self->framesLength) = frameCount * frameSize;
	CONST_CAST(float*, self->frames) = CALLOC(float, self->framesLength);

	return self;
}



static const int ROTATE_LAST_FRAME_TIME = -2;
static const int ROTATE_FRAME_VALUE = 1;

void _RotateTimeline_apply (const Timeline* timeline, Skeleton* skeleton, float time, float alpha) {
	Bone *bone;
	int frameIndex;
	float lastFrameValue, frameTime, percent, amount;

	RotateTimeline* self = SUB_CAST(RotateTimeline, timeline);

	if (time < self->frames[0]) return; 

	bone = skeleton->bones[self->boneIndex];

	if (time >= self->frames[self->framesLength - 2]) { 
		float amount = bone->data->rotation + self->frames[self->framesLength - 1] - bone->rotation;
		while (amount > 180)
			amount -= 360;
		while (amount < -180)
			amount += 360;
		bone->rotation += amount * alpha;
		return;
	}

	
	frameIndex = binarySearch(self->frames, self->framesLength, time, 2);
	lastFrameValue = self->frames[frameIndex - 1];
	frameTime = self->frames[frameIndex];
	percent = 1 - (time - frameTime) / (self->frames[frameIndex + ROTATE_LAST_FRAME_TIME] - frameTime);
	percent = CurveTimeline_getCurvePercent(SUPER(self), frameIndex / 2 - 1, percent < 0 ? 0 : (percent > 1 ? 1 : percent));

	amount = self->frames[frameIndex + ROTATE_FRAME_VALUE] - lastFrameValue;
	while (amount > 180)
		amount -= 360;
	while (amount < -180)
		amount += 360;
	amount = bone->data->rotation + (lastFrameValue + amount * percent) - bone->rotation;
	while (amount > 180)
		amount -= 360;
	while (amount < -180)
		amount += 360;
	bone->rotation += amount * alpha;
}

RotateTimeline* RotateTimeline_create (int frameCount) {
	return _BaseTimeline_create(frameCount, 2, _RotateTimeline_apply);
}

void RotateTimeline_setFrame (RotateTimeline* self, int frameIndex, float time, float angle) {
	frameIndex *= 2;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + 1] = angle;
}



static const int TRANSLATE_LAST_FRAME_TIME = -3;
static const int TRANSLATE_FRAME_X = 1;
static const int TRANSLATE_FRAME_Y = 2;

void _TranslateTimeline_apply (const Timeline* timeline, Skeleton* skeleton, float time, float alpha) {
	Bone *bone;
	int frameIndex;
	float lastFrameX, lastFrameY, frameTime, percent;

	TranslateTimeline* self = SUB_CAST(TranslateTimeline, timeline);

	if (time < self->frames[0]) return; 

	bone = skeleton->bones[self->boneIndex];

	if (time >= self->frames[self->framesLength - 3]) { 
		bone->x += (bone->data->x + self->frames[self->framesLength - 2] - bone->x) * alpha;
		bone->y += (bone->data->y + self->frames[self->framesLength - 1] - bone->y) * alpha;
		return;
	}

	
	frameIndex = binarySearch(self->frames, self->framesLength, time, 3);
	lastFrameX = self->frames[frameIndex - 2];
	lastFrameY = self->frames[frameIndex - 1];
	frameTime = self->frames[frameIndex];
	percent = 1 - (time - frameTime) / (self->frames[frameIndex + TRANSLATE_LAST_FRAME_TIME] - frameTime);
	percent = CurveTimeline_getCurvePercent(SUPER(self), frameIndex / 3 - 1, percent < 0 ? 0 : (percent > 1 ? 1 : percent));

	bone->x += (bone->data->x + lastFrameX + (self->frames[frameIndex + TRANSLATE_FRAME_X] - lastFrameX) * percent - bone->x)
			* alpha;
	bone->y += (bone->data->y + lastFrameY + (self->frames[frameIndex + TRANSLATE_FRAME_Y] - lastFrameY) * percent - bone->y)
			* alpha;
}

TranslateTimeline* TranslateTimeline_create (int frameCount) {
	return _BaseTimeline_create(frameCount, 3, _TranslateTimeline_apply);
}

void TranslateTimeline_setFrame (TranslateTimeline* self, int frameIndex, float time, float x, float y) {
	frameIndex *= 3;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + 1] = x;
	self->frames[frameIndex + 2] = y;
}



void _ScaleTimeline_apply (const Timeline* timeline, Skeleton* skeleton, float time, float alpha) {
	Bone *bone;
	int frameIndex;
	float lastFrameX, lastFrameY, frameTime, percent;

	ScaleTimeline* self = SUB_CAST(ScaleTimeline, timeline);
	
	if (time < self->frames[0]) return; 

	bone = skeleton->bones[self->boneIndex];
	if (time >= self->frames[self->framesLength - 3]) { 
		bone->scaleX += (bone->data->scaleX - 1 + self->frames[self->framesLength - 2] - bone->scaleX) * alpha;
		bone->scaleY += (bone->data->scaleY - 1 + self->frames[self->framesLength - 1] - bone->scaleY) * alpha;
		return;
	}

	
	frameIndex = binarySearch(self->frames, self->framesLength, time, 3);
	lastFrameX = self->frames[frameIndex - 2];
	lastFrameY = self->frames[frameIndex - 1];
	frameTime = self->frames[frameIndex];
	percent = 1 - (time - frameTime) / (self->frames[frameIndex + TRANSLATE_LAST_FRAME_TIME] - frameTime);
	percent = CurveTimeline_getCurvePercent(SUPER(self), frameIndex / 3 - 1, percent < 0 ? 0 : (percent > 1 ? 1 : percent));

	bone->scaleX += (bone->data->scaleX - 1 + lastFrameX + (self->frames[frameIndex + TRANSLATE_FRAME_X] - lastFrameX) * percent
			- bone->scaleX) * alpha;
	bone->scaleY += (bone->data->scaleY - 1 + lastFrameY + (self->frames[frameIndex + TRANSLATE_FRAME_Y] - lastFrameY) * percent
			- bone->scaleY) * alpha;
}

ScaleTimeline* ScaleTimeline_create (int frameCount) {
	return _BaseTimeline_create(frameCount, 3, _ScaleTimeline_apply);
}

void ScaleTimeline_setFrame (ScaleTimeline* self, int frameIndex, float time, float x, float y) {
	TranslateTimeline_setFrame(self, frameIndex, time, x, y);
}



static const int COLOR_LAST_FRAME_TIME = -5;
static const int COLOR_FRAME_R = 1;
static const int COLOR_FRAME_G = 2;
static const int COLOR_FRAME_B = 3;
static const int COLOR_FRAME_A = 4;

void _ColorTimeline_apply (const Timeline* timeline, Skeleton* skeleton, float time, float alpha) {
	Slot *slot;
	int frameIndex;
	float lastFrameR, lastFrameG, lastFrameB, lastFrameA, percent, frameTime;
	float r, g, b, a;
	ColorTimeline* self = (ColorTimeline*)timeline;

	if (time < self->frames[0]) return; 

	slot = skeleton->slots[self->slotIndex];

	if (time >= self->frames[self->framesLength - 5]) { 
		int i = self->framesLength - 1;
		slot->r = self->frames[i - 3];
		slot->g = self->frames[i - 2];
		slot->b = self->frames[i - 1];
		slot->a = self->frames[i];
		return;
	}

	
	frameIndex = binarySearch(self->frames, self->framesLength, time, 5);
	lastFrameR = self->frames[frameIndex - 4];
	lastFrameG = self->frames[frameIndex - 3];
	lastFrameB = self->frames[frameIndex - 2];
	lastFrameA = self->frames[frameIndex - 1];
	frameTime = self->frames[frameIndex];
	percent = 1 - (time - frameTime) / (self->frames[frameIndex + COLOR_LAST_FRAME_TIME] - frameTime);
	percent = CurveTimeline_getCurvePercent(SUPER(self), frameIndex / 5 - 1, percent < 0 ? 0 : (percent > 1 ? 1 : percent));

	r = lastFrameR + (self->frames[frameIndex + COLOR_FRAME_R] - lastFrameR) * percent;
	g = lastFrameG + (self->frames[frameIndex + COLOR_FRAME_G] - lastFrameG) * percent;
	b = lastFrameB + (self->frames[frameIndex + COLOR_FRAME_B] - lastFrameB) * percent;
	a = lastFrameA + (self->frames[frameIndex + COLOR_FRAME_A] - lastFrameA) * percent;
	if (alpha < 1) {
		slot->r += (r - slot->r) * alpha;
		slot->g += (g - slot->g) * alpha;
		slot->b += (b - slot->b) * alpha;
		slot->a += (a - slot->a) * alpha;
	} else {
		slot->r = r;
		slot->g = g;
		slot->b = b;
		slot->a = a;
	}
}

ColorTimeline* ColorTimeline_create (int frameCount) {
	return (ColorTimeline*)_BaseTimeline_create(frameCount, 5, _ColorTimeline_apply);
}

void ColorTimeline_setFrame (ColorTimeline* self, int frameIndex, float time, float r, float g, float b, float a) {
	frameIndex *= 5;
	self->frames[frameIndex] = time;
	self->frames[frameIndex + 1] = r;
	self->frames[frameIndex + 2] = g;
	self->frames[frameIndex + 3] = b;
	self->frames[frameIndex + 4] = a;
}



void _AttachmentTimeline_apply (const Timeline* timeline, Skeleton* skeleton, float time, float alpha) {
	int frameIndex;
	const char* attachmentName;
	AttachmentTimeline* self = (AttachmentTimeline*)timeline;

	if (time < self->frames[0]) return; 

	if (time >= self->frames[self->framesLength - 1]) 
		frameIndex = self->framesLength - 1;
	else
		frameIndex = binarySearch(self->frames, self->framesLength, time, 1) - 1;

	attachmentName = self->attachmentNames[frameIndex];
	Slot_setAttachment(skeleton->slots[self->slotIndex],
			attachmentName ? Skeleton_getAttachmentForSlotIndex(skeleton, self->slotIndex, attachmentName) : 0);
}

void _AttachmentTimeline_dispose (Timeline* timeline) {
	AttachmentTimeline* self;
	int i;

	_Timeline_deinit(timeline);
	self = (AttachmentTimeline*)timeline;

	for (i = 0; i < self->framesLength; ++i)
		FREE(self->attachmentNames[i]);
	FREE(self->attachmentNames);
	FREE(self->frames);
	FREE(self);
}

AttachmentTimeline* AttachmentTimeline_create (int frameCount) {
	AttachmentTimeline* self = NEW(AttachmentTimeline);
	_Timeline_init(SUPER(self), _AttachmentTimeline_dispose, _AttachmentTimeline_apply);

	CONST_CAST(char**, self->attachmentNames) = CALLOC(char*, frameCount);
	CONST_CAST(int, self->framesLength) = frameCount;
	CONST_CAST(float*, self->frames) = CALLOC(float, frameCount);

	return self;
}

void AttachmentTimeline_setFrame (AttachmentTimeline* self, int frameIndex, float time, const char* attachmentName) {
	self->frames[frameIndex] = time;
	FREE(self->attachmentNames[frameIndex]);
	if (attachmentName)
		MALLOC_STR(self->attachmentNames[frameIndex], attachmentName);
	else
		self->attachmentNames[frameIndex] = 0;
}

//
//MERGED FILE END: Animation.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: AnimationState.c
//

typedef struct _EntryAnimationState _EntryAnimationState;
struct _EntryAnimationState {
	Animation* animation;
	bool loop;
	float delay;
	_EntryAnimationState* next;
};

typedef struct {
	AnimationState super;
	Animation *previous;
	float previousTime;
	bool previousLoop;
	float mixTime;
	float mixDuration;
	_EntryAnimationState* queue;
} _InternalAnimationState;

AnimationState* AnimationState_create (AnimationStateData* data) {
	AnimationState* self = SUPER(NEW(_InternalAnimationState));
	CONST_CAST(AnimationStateData*, self->data) = data;
	return self;
}

void _AnimationState_clearQueue (AnimationState* self) {
	_InternalAnimationState* internal = SUB_CAST(_InternalAnimationState, self);
	_EntryAnimationState* entry = internal->queue;
	while (entry) {
		_EntryAnimationState* nextEntry = entry->next;
		FREE(entry);
		entry = nextEntry;
	}
	internal->queue = 0;
}

void AnimationState_dispose (AnimationState* self) {
	_AnimationState_clearQueue(self);
	FREE(self);
}


void AnimationState_addAnimation (AnimationState* self, Animation* animation, bool loop, float delay) {
	_EntryAnimationState* existingEntry;
	Animation* previousAnimation;

	_InternalAnimationState* internal = SUB_CAST(_InternalAnimationState, self);
	_EntryAnimationState* entry = NEW(_EntryAnimationState);
	entry->animation = animation;
	entry->loop = loop;

	existingEntry = internal->queue;
	if (existingEntry) {
		while (existingEntry->next)
			existingEntry = existingEntry->next;
		existingEntry->next = entry;
		previousAnimation = existingEntry->animation;
	} else {
		internal->queue = entry;
		previousAnimation = self->animation;
	}

	if (delay <= 0) {
		if (previousAnimation)
			delay = previousAnimation->duration - AnimationStateData_getMix(self->data, previousAnimation, animation) + delay;
		else
			delay = 0;
	}
	entry->delay = delay;
}

void AnimationState_addAnimationByName (AnimationState* self, const char* animationName, bool loop, float delay) {
	Animation* animation = animationName ? SkeletonData_findAnimation(self->data->skeletonData, animationName) : 0;
	AnimationState_addAnimation(self, animation, loop, delay);
}

void _AnimationState_setAnimation (AnimationState* self, Animation* newAnimation, bool loop) {
	_InternalAnimationState* internal = SUB_CAST(_InternalAnimationState, self);
	internal->previous = 0;
	if (newAnimation && self->animation && self->data) {
		internal->mixDuration = AnimationStateData_getMix(self->data, self->animation, newAnimation);
		if (internal->mixDuration > 0) {
			internal->mixTime = 0;
			internal->previous = self->animation;
			internal->previousTime = self->time;
			internal->previousLoop = self->loop;
		}
	}
	CONST_CAST(Animation*, self->animation) = newAnimation;
	self->loop = loop;
	self->time = 0;
}

void AnimationState_setAnimation (AnimationState* self, Animation* newAnimation, bool loop) {
	_AnimationState_clearQueue(self);
	_AnimationState_setAnimation(self, newAnimation, loop);
}

void AnimationState_setAnimationByName (AnimationState* self, const char* animationName, bool loop) {
	Animation* animation = animationName ? SkeletonData_findAnimation(self->data->skeletonData, animationName) : 0;
	AnimationState_setAnimation(self, animation, loop);
}

void AnimationState_clearAnimation (AnimationState* self) {
	_InternalAnimationState* internal = SUB_CAST(_InternalAnimationState, self);
	internal->previous = 0;
	CONST_CAST(Animation*, self->animation) = 0;
	_AnimationState_clearQueue(self);
}

void AnimationState_update (AnimationState* self, float delta) {
	_EntryAnimationState* next;
	_InternalAnimationState* internal = SUB_CAST(_InternalAnimationState, self);

	self->time += delta;
	internal->previousTime += delta;
	internal->mixTime += delta;

	if (internal->queue && self->time >= internal->queue->delay) {
		_AnimationState_setAnimation(self, internal->queue->animation, internal->queue->loop);
		next = internal->queue->next;
		FREE(internal->queue);
		internal->queue = next;
	}
}

void AnimationState_apply (AnimationState* self, Skeleton* skeleton) {
	_InternalAnimationState* internal;
	float alpha;

	if (!self->animation) return;
	internal = SUB_CAST(_InternalAnimationState, self);
	if (internal->previous) {
		Animation_apply(internal->previous, skeleton, internal->previousTime, internal->previousLoop);
		alpha = internal->mixTime / internal->mixDuration;
		if (alpha >= 1) {
			alpha = 1;
			internal->previous = 0;
		}
		Animation_mix(self->animation, skeleton, self->time, self->loop, alpha);
	} else
		Animation_apply(self->animation, skeleton, self->time, self->loop);
}

bool AnimationState_isComplete (AnimationState* self) {
	return !self->animation || self->time >= self->animation->duration;
}

//
//MERGED FILE END: AnimationState.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: AnimationStateData.c
//

typedef struct _ToEntry _ToEntry;
struct _ToEntry {
	Animation* animation;
	float duration;
	_ToEntry* next;
};

_ToEntry* _ToEntry_create (Animation* to, float duration) {
	_ToEntry* self = NEW(_ToEntry);
	self->animation = to;
	self->duration = duration;
	return self;
}

void _ToEntry_dispose (_ToEntry* self) {
	FREE(self);
}



typedef struct _FromEntry _FromEntry;
struct _FromEntry {
	Animation* animation;
	_ToEntry* toEntries;
	_FromEntry* next;
};

_FromEntry* _FromEntry_create (Animation* from) {
	_FromEntry* self = NEW(_FromEntry);
	self->animation = from;
	return self;
}

void _FromEntry_dispose (_FromEntry* self) {
	FREE(self);
}



AnimationStateData* AnimationStateData_create (SkeletonData* skeletonData) {
	AnimationStateData* self = NEW(AnimationStateData);
	CONST_CAST(SkeletonData*, self->skeletonData) = skeletonData;
	return self;
}

void AnimationStateData_dispose (AnimationStateData* self) {
	_ToEntry* toEntry;
	_ToEntry* nextToEntry;
	_FromEntry* nextFromEntry;

	_FromEntry* fromEntry = (_FromEntry*)self->entries;
	while (fromEntry) {
		toEntry = fromEntry->toEntries;
		while (toEntry) {
			nextToEntry = toEntry->next;
			_ToEntry_dispose(toEntry);
			toEntry = nextToEntry;
		}
		nextFromEntry = fromEntry->next;
		_FromEntry_dispose(fromEntry);
		fromEntry = nextFromEntry;
	}

	FREE(self);
}

void AnimationStateData_setMixByName (AnimationStateData* self, const char* fromName, const char* toName, float duration) {
	Animation* to;
	Animation* from = SkeletonData_findAnimation(self->skeletonData, fromName);
	if (!from) return;
	to = SkeletonData_findAnimation(self->skeletonData, toName);
	if (!to) return;
	AnimationStateData_setMix(self, from, to, duration);
}

void AnimationStateData_setMix (AnimationStateData* self, Animation* from, Animation* to, float duration) {
	
	_ToEntry* toEntry;
	_FromEntry* fromEntry = (_FromEntry*)self->entries;
	while (fromEntry) {
		if (fromEntry->animation == from) {
			
			toEntry = fromEntry->toEntries;
			while (toEntry) {
				if (toEntry->animation == to) {
					toEntry->duration = duration;
					return;
				}
				toEntry = toEntry->next;
			}
			break; 
		}
		fromEntry = fromEntry->next;
	}
	if (!fromEntry) {
		fromEntry = _FromEntry_create(from);
		fromEntry->next = (_FromEntry*)self->entries;
		CONST_CAST(void*, self->entries) = fromEntry;
	}
	toEntry = _ToEntry_create(to, duration);
	toEntry->next = fromEntry->toEntries;
	fromEntry->toEntries = toEntry;
}

float AnimationStateData_getMix (AnimationStateData* self, Animation* from, Animation* to) {
	_FromEntry* fromEntry = (_FromEntry*)self->entries;
	while (fromEntry) {
		if (fromEntry->animation == from) {
			_ToEntry* toEntry = fromEntry->toEntries;
			while (toEntry) {
				if (toEntry->animation == to) return toEntry->duration;
				toEntry = toEntry->next;
			}
		}
		fromEntry = fromEntry->next;
	}
	return self->defaultMix;
}

//
//MERGED FILE END: AnimationStateData.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Atlas.c
//

AtlasPage* AtlasPage_create (const char* name) {
	AtlasPage* self = NEW(AtlasPage);
	MALLOC_STR(self->name, name);
	return self;
}

void AtlasPage_dispose (AtlasPage* self) {
	_AtlasPage_disposeTexture(self);
	FREE(self->name);
	FREE(self);
}



AtlasRegion* AtlasRegion_create () {
	return NEW(AtlasRegion) ;
}

void AtlasRegion_dispose (AtlasRegion* self) {
	FREE(self->name);
	FREE(self->splits);
	FREE(self->pads);
	FREE(self);
}



typedef struct {
	const char* begin;
	const char* end;
} Str;

static void trim (Str* str) {
	while (isspace((int)*str->begin) && str->begin < str->end)
		(str->begin)++;
	if (str->begin == str->end) return;
	str->end--;
	while (isspace((int)*str->end) && str->end >= str->begin)
		str->end--;
	str->end++;
}


static int readLine (const char* begin, const char* end, Str* str) {
	static const char* nextStart;
	if (begin) {
		nextStart = begin;
		return 1;
	}
	if (nextStart == end) return 0;
	str->begin = nextStart;

	
	while (nextStart != end && *nextStart != '\n')
		nextStart++;

	str->end = nextStart;
	trim(str);

	if (nextStart != end) nextStart++;
	return 1;
}


static int beginPast (Str* str, char c) {
	const char* begin = str->begin;
	while (1) {
		char lastSkippedChar = *begin;
		if (begin == str->end) return 0;
		begin++;
		if (lastSkippedChar == c) break;
	}
	str->begin = begin;
	return 1;
}


static int readValue (const char* end, Str* str) {
	readLine(0, end, str);
	if (!beginPast(str, ':')) return 0;
	trim(str);
	return 1;
}


static int readTuple (const char* end, Str tuple[]) {
	int i;
	Str str;
	readLine(0, end, &str);
	if (!beginPast(&str, ':')) return 0;

	for (i = 0; i < 3; ++i) {
		tuple[i].begin = str.begin;
		if (!beginPast(&str, ',')) {
			if (i == 0) return 0;
			break;
		}
		tuple[i].end = str.begin - 2;
		trim(&tuple[i]);
	}
	tuple[i].begin = str.begin;
	tuple[i].end = str.end;
	trim(&tuple[i]);
	return i + 1;
}

static char* mallocString (Str* str) {
	size_t length = str->end - str->begin;
	char* string = MALLOC(char, length + 1);
	memcpy(string, str->begin, length);
	string[length] = '\0';
	return string;
}

static int indexOf (const char** array, int count, Str* str) {
	size_t length = str->end - str->begin;
	int i;
	for (i = count - 1; i >= 0; i--)
		if (strncmp(array[i], str->begin, length) == 0) return i;
	return -1;
}

static bool equals (Str* str, const char* other) {
	return strncmp(other, str->begin, str->end - str->begin) == 0;
}

static int toInt (Str* str) {
	return strtol(str->begin, (char**)&str->end, 10);
}

static Atlas* abortAtlas (Atlas* self) {
	Atlas_dispose(self);
	return 0;
}

static const char* formatNames[] = {"Alpha", "Intensity", "LuminanceAlpha", "RGB565", "RGBA4444", "RGB888", "RGBA8888"};
static const char* textureFilterNames[] = {"Nearest", "Linear", "MipMap", "MipMapNearestNearest", "MipMapLinearNearest",
		"MipMapNearestLinear", "MipMapLinearLinear"};

Atlas* Atlas_readAtlas (const char* begin, size_t length, const char* dir) {
	int count;
	const char* end = begin + length;
	size_t dirLength = strlen(dir);
	int needsSlash = dirLength > 0 && dir[dirLength - 1] != '/' && dir[dirLength - 1] != '\\';

	Atlas* self = NEW(Atlas);

	AtlasPage *page = 0;
	AtlasPage *lastPage = 0;
	AtlasRegion *lastRegion = 0;
	Str str;
	Str tuple[4];
	readLine(begin, 0, 0);
	while (readLine(0, end, &str)) {
		if (str.end - str.begin == 0) {
			page = 0;
		} else if (!page) {
			char* name = mallocString(&str);
			char* path = MALLOC(char, dirLength + needsSlash + strlen(name) + 1);
			memcpy(path, dir, dirLength);
			if (needsSlash) path[dirLength] = '/';
			strcpy(path + dirLength + needsSlash, name);

			page = AtlasPage_create(name);
			FREE(name);
			if (lastPage)
				lastPage->next = page;
			else
				self->pages = page;
			lastPage = page;

			if (!readValue(end, &str)) return abortAtlas(self);
			page->format = (AtlasFormat)indexOf(formatNames, 7, &str);

			if (!readTuple(end, tuple)) return abortAtlas(self);
			page->minFilter = (AtlasFilter)indexOf(textureFilterNames, 7, tuple);
			page->magFilter = (AtlasFilter)indexOf(textureFilterNames, 7, tuple + 1);

			if (!readValue(end, &str)) return abortAtlas(self);
			if (!equals(&str, "none")) {
				page->uWrap = *str.begin == 'x' ? ATLAS_REPEAT : (*str.begin == 'y' ? ATLAS_CLAMPTOEDGE : ATLAS_REPEAT);
				page->vWrap = *str.begin == 'x' ? ATLAS_CLAMPTOEDGE : (*str.begin == 'y' ? ATLAS_REPEAT : ATLAS_REPEAT);
			}

			_AtlasPage_createTexture(page, path);
			FREE(path);
		} else {
			AtlasRegion *region = AtlasRegion_create();
			if (lastRegion)
				lastRegion->next = region;
			else
				self->regions = region;
			lastRegion = region;

			region->page = page;
			region->name = mallocString(&str);

			if (!readValue(end, &str)) return abortAtlas(self);
			region->rotate = equals(&str, "true");

			if (readTuple(end, tuple) != 2) return abortAtlas(self);
			region->x = toInt(tuple);
			region->y = toInt(tuple + 1);

			if (readTuple(end, tuple) != 2) return abortAtlas(self);
			region->width = toInt(tuple);
			region->height = toInt(tuple + 1);

			region->u = region->x / (float)page->width;
			region->v = region->y / (float)page->height;
			if (region->rotate) {
				region->u2 = (region->x + region->height) / (float)page->width;
				region->v2 = (region->y + region->width) / (float)page->height;
			} else {
				region->u2 = (region->x + region->width) / (float)page->width;
				region->v2 = (region->y + region->height) / (float)page->height;
			}

			if (!(count = readTuple(end, tuple))) return abortAtlas(self);
			if (count == 4) { 
				region->splits = MALLOC(int, 4);
				region->splits[0] = toInt(tuple);
				region->splits[1] = toInt(tuple + 1);
				region->splits[2] = toInt(tuple + 2);
				region->splits[3] = toInt(tuple + 3);

				if (!(count = readTuple(end, tuple))) return abortAtlas(self);
				if (count == 4) { 
					region->pads = MALLOC(int, 4);
					region->pads[0] = toInt(tuple);
					region->pads[1] = toInt(tuple + 1);
					region->pads[2] = toInt(tuple + 2);
					region->pads[3] = toInt(tuple + 3);

					if (!readTuple(end, tuple)) return abortAtlas(self);
				}
			}

			region->originalWidth = toInt(tuple);
			region->originalHeight = toInt(tuple + 1);

			readTuple(end, tuple);
			region->offsetX = toInt(tuple);
			region->offsetY = toInt(tuple + 1);

			if (!readValue(end, &str)) return abortAtlas(self);
			region->index = toInt(&str);
		}
	}

	return self;
}

Atlas* Atlas_readAtlasFile (const char* path) {
	size_t dirLength;
	char *dir;
	size_t length;
	const char* data;

	Atlas* atlas = 0;

	
	const char* lastForwardSlash = strrchr(path, '/');
	const char* lastBackwardSlash = strrchr(path, '\\');
	const char* lastSlash = lastForwardSlash > lastBackwardSlash ? lastForwardSlash : lastBackwardSlash;
	if (lastSlash == path) lastSlash++; 
	dirLength = lastSlash ? lastSlash - path : 0;
	dir = MALLOC(char, dirLength + 1);
	memcpy(dir, path, dirLength);
	dir[dirLength] = '\0';

	data = _Util_readFile(path, &length);
	if (data) atlas = Atlas_readAtlas(data, length, dir);

	FREE(data);
	FREE(dir);
	return atlas;
}

void Atlas_dispose (Atlas* self) {
	AtlasRegion* region, *nextRegion;
	AtlasPage* page = self->pages;
	while (page) {
		AtlasPage* nextPage = page->next;
		AtlasPage_dispose(page);
		page = nextPage;
	}

	region = self->regions;
	while (region) {
		nextRegion = region->next;
		AtlasRegion_dispose(region);
		region = nextRegion;
	}

	FREE(self);
}

AtlasRegion* Atlas_findRegion (const Atlas* self, const char* name) {
	AtlasRegion* region = self->regions;
	while (region) {
		if (strcmp(region->name, name) == 0) return region;
		region = region->next;
	}
	return 0;
}

//
//MERGED FILE END: Atlas.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: AtlasAttachmentLoader.c
//

Attachment* _AtlasAttachmentLoader_newAttachment (AttachmentLoader* loader, Skin* skin, AttachmentType type, const char* name) {
	AtlasAttachmentLoader* self = SUB_CAST(AtlasAttachmentLoader, loader);
	switch (type) {
	case ATTACHMENT_REGION: {
		RegionAttachment* attachment;
		AtlasRegion* region = Atlas_findRegion(self->atlas, name);
		if (!region) {
			_AttachmentLoader_setError(loader, "Region not found: ", name);
			return 0;
		}
		attachment = RegionAttachment_create(name);
		attachment->rendererObject = region;
		RegionAttachment_setUVs(attachment, region->u, region->v, region->u2, region->v2, region->rotate);
		attachment->regionOffsetX = region->offsetX;
		attachment->regionOffsetY = region->offsetY;
		attachment->regionWidth = region->width;
		attachment->regionHeight = region->height;
		attachment->regionOriginalWidth = region->originalWidth;
		attachment->regionOriginalHeight = region->originalHeight;
		return SUPER(attachment);
	}
	default:
		_AttachmentLoader_setUnknownTypeError(loader, type);
		return 0;
	}
}

AtlasAttachmentLoader* AtlasAttachmentLoader_create (Atlas* atlas) {
	AtlasAttachmentLoader* self = NEW(AtlasAttachmentLoader);
	_AttachmentLoader_init(SUPER(self), _AttachmentLoader_deinit, _AtlasAttachmentLoader_newAttachment);
	self->atlas = atlas;
	return self;
}

//
//MERGED FILE END: AtlasAttachmentLoader.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Attachment.c
//

typedef struct _AttachmentVtable {
	void (*dispose) (Attachment* self);
} _AttachmentVtable;

void _Attachment_init (Attachment* self, const char* name, AttachmentType type, 
		void (*dispose) (Attachment* self)) {

	CONST_CAST(void*, self->vtable) = NEW(_AttachmentVtable);
	VTABLE(Attachment, self) ->dispose = dispose;

	MALLOC_STR(self->name, name);
	self->type = type;
}

void _Attachment_deinit (Attachment* self) {
	FREE(self->vtable);
	FREE(self->name);
}

void Attachment_dispose (Attachment* self) {
	VTABLE(Attachment, self) ->dispose(self);
	FREE(self);
}

//
//MERGED FILE END: Attachment.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: AttachmentLoader.c
//

typedef struct _AttachmentLoaderVtable {
	Attachment* (*newAttachment) (AttachmentLoader* self, Skin* skin, AttachmentType type, const char* name);
	void (*dispose) (AttachmentLoader* self);
} _AttachmentLoaderVtable;

void _AttachmentLoader_init (AttachmentLoader* self, 
		void (*dispose) (AttachmentLoader* self), 
		Attachment* (*newAttachment) (AttachmentLoader* self, Skin* skin, AttachmentType type, const char* name)) {
	CONST_CAST(void*, self->vtable) = NEW(_AttachmentLoaderVtable);
	VTABLE(AttachmentLoader, self) ->dispose = dispose;
	VTABLE(AttachmentLoader, self) ->newAttachment = newAttachment;
}

void _AttachmentLoader_deinit (AttachmentLoader* self) {
	FREE(self->vtable);
	FREE(self->error1);
	FREE(self->error2);
}

void AttachmentLoader_dispose (AttachmentLoader* self) {
	VTABLE(AttachmentLoader, self) ->dispose(self);
	FREE(self);
}

Attachment* AttachmentLoader_newAttachment (AttachmentLoader* self, Skin* skin, AttachmentType type, const char* name) {
	FREE(self->error1);
	FREE(self->error2);
	self->error1 = 0;
	self->error2 = 0;
	return VTABLE(AttachmentLoader, self) ->newAttachment(self, skin, type, name);
}

void _AttachmentLoader_setError_ (AttachmentLoader* self, const char* error1, const char* error2) {
	FREE(self->error1);
	FREE(self->error2);
	MALLOC_STR(self->error1, error1);
	MALLOC_STR(self->error2, error2);
}

void _AttachmentLoader_setUnknownTypeError_ (AttachmentLoader* self, AttachmentType type) {
	char buffer[16];
	sprintf(buffer, "%d", type);
	_AttachmentLoader_setError(self, "Unknown attachment type: ", buffer);
}

//
//MERGED FILE END: AttachmentLoader.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Bone.c
//

static int yDown;

void Bone_setYDown (int value) {
	yDown = value;
}

Bone* Bone_create (BoneData* data, Bone* parent) {
	Bone* self = NEW(Bone);
	CONST_CAST(BoneData*, self->data) = data;
	CONST_CAST(Bone*, self->parent) = parent;
	Bone_setToSetupPose(self);
	return self;
}

void Bone_dispose (Bone* self) {
	FREE(self);
}

void Bone_setToSetupPose (Bone* self) {
	self->x = self->data->x;
	self->y = self->data->y;
	self->rotation = self->data->rotation;
	self->scaleX = self->data->scaleX;
	self->scaleY = self->data->scaleY;
}

void Bone_updateWorldTransform (Bone* self, bool flipX, bool flipY) {
	float radians, cosine, sine;
	if (self->parent) {
		CONST_CAST(float, self->worldX) = self->x * self->parent->m00 + self->y * self->parent->m01 + self->parent->worldX;
		CONST_CAST(float, self->worldY) = self->x * self->parent->m10 + self->y * self->parent->m11 + self->parent->worldY;
		CONST_CAST(float, self->worldScaleX) = self->parent->worldScaleX * self->scaleX;
		CONST_CAST(float, self->worldScaleY) = self->parent->worldScaleY * self->scaleY;
		CONST_CAST(float, self->worldRotation) = self->parent->worldRotation + self->rotation;
	} else {
		CONST_CAST(float, self->worldX) = flipX ? -self->x : self->x;
		CONST_CAST(float, self->worldY) = flipX ? -self->y : self->y;
		CONST_CAST(float, self->worldScaleX) = self->scaleX;
		CONST_CAST(float, self->worldScaleY) = self->scaleY;
		CONST_CAST(float, self->worldRotation) = self->rotation;
	}
	radians = (float)(self->worldRotation * 3.1415926535897932385 / 180);
#ifdef __STDC_VERSION__
	cosine = cosf(radians);
	sine = sinf(radians);
#else
	cosine = (float)cos(radians);
	sine = (float)sin(radians);
#endif
	CONST_CAST(float, self->m00) = cosine * self->worldScaleX;
	CONST_CAST(float, self->m10) = sine * self->worldScaleX;
	CONST_CAST(float, self->m01) = -sine * self->worldScaleY;
	CONST_CAST(float, self->m11) = cosine * self->worldScaleY;
	if (flipX) {
		CONST_CAST(float, self->m00) = -self->m00;
		CONST_CAST(float, self->m01) = -self->m01;
	}
	if (flipY) {
		CONST_CAST(float, self->m10) = -self->m10;
		CONST_CAST(float, self->m11) = -self->m11;
	}
	if (yDown) {
		CONST_CAST(float, self->m10) = -self->m10;
		CONST_CAST(float, self->m11) = -self->m11;
	}
}

//
//MERGED FILE END: Bone.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: BoneData.c
//

BoneData* BoneData_create (const char* name, BoneData* parent) {
	BoneData* self = NEW(BoneData);
	MALLOC_STR(self->name, name);
	CONST_CAST(BoneData*, self->parent) = parent;
	self->scaleX = 1;
	self->scaleY = 1;
	return self;
}

void BoneData_dispose (BoneData* self) {
	FREE(self->name);
	FREE(self);
}

//
//MERGED FILE END: BoneData.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: extension.c
//



//
//MERGED FILE END: extension.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Json.c
//

static const char* ep;

const char* Json_getError (void) {
	return ep;
}

static int Json_strcasecmp (const char* s1, const char* s2) {
	if (!s1) return (s1 == s2) ? 0 : 1;
	if (!s2) return 1;
	for (; tolower((int)*s1) == tolower((int)*s2); ++s1, ++s2)
		if (*s1 == 0) return 0;
	return tolower((int)*(const unsigned char*)s1) - tolower((int)*(const unsigned char*)s2);
}


static Json *Json_new (void) {
	return (Json*)CALLOC(Json, 1);
}


void Json_dispose (Json *c) {
	Json *next;
	while (c) {
		next = c->next;
		if (c->child) Json_dispose(c->child);
		if (c->valuestring) FREE(c->valuestring);
		if (c->name) FREE(c->name);
		FREE(c);
		c = next;
	}
}


static const char* parse_number (Json *item, const char* num) {
	float n = 0, sign = 1, scale = 0;
	int subscale = 0, signsubscale = 1;

	
	if (*num == '-') sign = -1, num++; 
	if (*num == '0') num++; 
	if (*num >= '1' && *num <= '9') do
		n = (n * 10.0f) + (*num++ - '0');
	while (*num >= '0' && *num <= '9'); 
	if (*num == '.' && num[1] >= '0' && num[1] <= '9') {
		num++;
		do
			n = (n * 10.0f) + (*num++ - '0'), scale--;
		while (*num >= '0' && *num <= '9');
	} 
	if (*num == 'e' || *num == 'E') 
	{
		num++;
		if (*num == '+')
			num++;
		else if (*num == '-') signsubscale = -1, num++; 
		while (*num >= '0' && *num <= '9')
			subscale = (subscale * 10) + (*num++ - '0'); 
	}

	n = sign * n * (float)pow(10.0f, (scale + subscale * signsubscale)); 

	item->valuefloat = n;
	item->valueint = (int)n;
	item->type = Json_Number;
	return num;
}


static const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
static const char* parse_string (Json *item, const char* str) {
	const char* ptr = str + 1;
	char* ptr2;
	char* out;
	int len = 0;
	unsigned uc, uc2;
	if (*str != '\"') {
		ep = str;
		return 0;
	} 

	while (*ptr != '\"' && *ptr && ++len)
		if (*ptr++ == '\\') ptr++; 

	out = (char*)malloc(len + 1); 
	if (!out) return 0;

	ptr = str + 1;
	ptr2 = out;
	while (*ptr != '\"' && *ptr) {
		if (*ptr != '\\')
			*ptr2++ = *ptr++;
		else {
			ptr++;
			switch (*ptr) {
			case 'b':
				*ptr2++ = '\b';
				break;
			case 'f':
				*ptr2++ = '\f';
				break;
			case 'n':
				*ptr2++ = '\n';
				break;
			case 'r':
				*ptr2++ = '\r';
				break;
			case 't':
				*ptr2++ = '\t';
				break;
			case 'u': 
				sscanf(ptr + 1, "%4x", &uc);
				ptr += 4; 

				if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0) break; 

				if (uc >= 0xD800 && uc <= 0xDBFF) 
				{
					if (ptr[1] != '\\' || ptr[2] != 'u') break; 
					sscanf(ptr + 3, "%4x", &uc2);
					ptr += 6;
					if (uc2 < 0xDC00 || uc2 > 0xDFFF) break; 
					uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
				}

				len = 4;
				if (uc < 0x80)
					len = 1;
				else if (uc < 0x800)
					len = 2;
				else if (uc < 0x10000) len = 3;
				ptr2 += len;

				switch (len) {
				case 4:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 3:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 2:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 1:
					*--ptr2 = (uc | firstByteMark[len]);
				}
				ptr2 += len;
				break;
			default:
				*ptr2++ = *ptr;
				break;
			}
			ptr++;
		}
	}
	*ptr2 = 0;
	if (*ptr == '\"') ptr++;
	item->valuestring = out;
	item->type = Json_String;
	return ptr;
}


static const char* parse_value (Json *item, const char* value);
static const char* parse_array (Json *item, const char* value);
static const char* parse_object (Json *item, const char* value);


static const char* skip (const char* in) {
	while (in && *in && (unsigned char)*in <= 32)
		in++;
	return in;
}


Json *Json_create (const char* value) {
	const char* end = 0;
	Json *c = Json_new();
	ep = 0;
	if (!c) return 0; 

	end = parse_value(c, skip(value));
	if (!end) {
		Json_dispose(c);
		return 0;
	} 

	return c;
}


static const char* parse_value (Json *item, const char* value) {
	if (!value) return 0; 
	if (!strncmp(value, "null", 4)) {
		item->type = Json_NULL;
		return value + 4;
	}
	if (!strncmp(value, "false", 5)) {
		item->type = Json_False;
		return value + 5;
	}
	if (!strncmp(value, "true", 4)) {
		item->type = Json_True;
		item->valueint = 1;
		return value + 4;
	}
	if (*value == '\"') {
		return parse_string(item, value);
	}
	if (*value == '-' || (*value >= '0' && *value <= '9')) {
		return parse_number(item, value);
	}
	if (*value == '[') {
		return parse_array(item, value);
	}
	if (*value == '{') {
		return parse_object(item, value);
	}

	ep = value;
	return 0; 
}


static const char* parse_array (Json *item, const char* value) {
	Json *child;
	if (*value != '[') {
		ep = value;
		return 0;
	} 

	item->type = Json_Array;
	value = skip(value + 1);
	if (*value == ']') return value + 1; 

	item->child = child = Json_new();
	if (!item->child) return 0; 
	value = skip(parse_value(child, skip(value))); 
	if (!value) return 0;

	while (*value == ',') {
		Json *new_item;
		if (!(new_item = Json_new())) return 0; 
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_value(child, skip(value + 1)));
		if (!value) return 0; 
	}

	if (*value == ']') return value + 1; 
	ep = value;
	return 0; 
}


static const char* parse_object (Json *item, const char* value) {
	Json *child;
	if (*value != '{') {
		ep = value;
		return 0;
	} 

	item->type = Json_Object;
	value = skip(value + 1);
	if (*value == '}') return value + 1; 

	item->child = child = Json_new();
	if (!item->child) return 0;
	value = skip(parse_string(child, skip(value)));
	if (!value) return 0;
	child->name = child->valuestring;
	child->valuestring = 0;
	if (*value != ':') {
		ep = value;
		return 0;
	} 
	value = skip(parse_value(child, skip(value + 1))); 
	if (!value) return 0;

	while (*value == ',') {
		Json *new_item;
		if (!(new_item = Json_new())) return 0; 
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_string(child, skip(value + 1)));
		if (!value) return 0;
		child->name = child->valuestring;
		child->valuestring = 0;
		if (*value != ':') {
			ep = value;
			return 0;
		} 
		value = skip(parse_value(child, skip(value + 1))); 
		if (!value) return 0;
	}

	if (*value == '}') return value + 1; 
	ep = value;
	return 0; 
}


int Json_getSize (Json *array) {
	Json *c = array->child;
	int i = 0;
	while (c)
		i++, c = c->next;
	return i;
}

Json *Json_getItemAt (Json *array, int item) {
	Json *c = array->child;
	while (c && item > 0)
		item--, c = c->next;
	return c;
}

Json *Json_getItem (Json *object, const char* string) {
	Json *c = object->child;
	while (c && Json_strcasecmp(c->name, string))
		c = c->next;
	return c;
}

const char* Json_getString (Json* object, const char* name, const char* defaultValue) {
	object = Json_getItem(object, name);
	if (object) return object->valuestring;
	return defaultValue;
}

float Json_getFloat (Json* value, const char* name, float defaultValue) {
	value = Json_getItem(value, name);
	return value ? value->valuefloat : defaultValue;
}

int Json_getInt (Json* value, const char* name, int defaultValue) {
	value = Json_getItem(value, name);
	return value ? (int)value->valuefloat : defaultValue;
}

//
//MERGED FILE END: Json.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: RegionAttachment.c
//

RegionAttachment* RegionAttachment_create (const char* name) {
	RegionAttachment* self = NEW(RegionAttachment);
	self->scaleX = 1;
	self->scaleY = 1;
	_Attachment_init(SUPER(self), name, ATTACHMENT_REGION, _Attachment_deinit);
	return self;
}

void RegionAttachment_setUVs (RegionAttachment* self, float u, float v, float u2, float v2, bool rotate) {
	if (rotate) {
		self->uvs[VERTEX_X2] = u;
		self->uvs[VERTEX_Y2] = v2;
		self->uvs[VERTEX_X3] = u;
		self->uvs[VERTEX_Y3] = v;
		self->uvs[VERTEX_X4] = u2;
		self->uvs[VERTEX_Y4] = v;
		self->uvs[VERTEX_X1] = u2;
		self->uvs[VERTEX_Y1] = v2;
	} else {
		self->uvs[VERTEX_X1] = u;
		self->uvs[VERTEX_Y1] = v2;
		self->uvs[VERTEX_X2] = u;
		self->uvs[VERTEX_Y2] = v;
		self->uvs[VERTEX_X3] = u2;
		self->uvs[VERTEX_Y3] = v;
		self->uvs[VERTEX_X4] = u2;
		self->uvs[VERTEX_Y4] = v2;
	}
}

void RegionAttachment_updateOffset (RegionAttachment* self) {
	float regionScaleX = self->width / self->regionOriginalWidth * self->scaleX;
	float regionScaleY = self->height / self->regionOriginalHeight * self->scaleY;
	float localX = -self->width / 2 * self->scaleX + self->regionOffsetX * regionScaleX;
	float localY = -self->height / 2 * self->scaleY + self->regionOffsetY * regionScaleY;
	float localX2 = localX + self->regionWidth * regionScaleX;
	float localY2 = localY + self->regionHeight * regionScaleY;
	float radians = (float)(self->rotation * 3.1415926535897932385 / 180);
#ifdef __STDC_VERSION__
	float cosine = cosf(radians);
	float sine = sinf(radians);
#else
	float cosine = (float)cos(radians);
	float sine = (float)sin(radians);
#endif
	float localXCos = localX * cosine + self->x;
	float localXSin = localX * sine;
	float localYCos = localY * cosine + self->y;
	float localYSin = localY * sine;
	float localX2Cos = localX2 * cosine + self->x;
	float localX2Sin = localX2 * sine;
	float localY2Cos = localY2 * cosine + self->y;
	float localY2Sin = localY2 * sine;
	self->offset[VERTEX_X1] = localXCos - localYSin;
	self->offset[VERTEX_Y1] = localYCos + localXSin;
	self->offset[VERTEX_X2] = localXCos - localY2Sin;
	self->offset[VERTEX_Y2] = localY2Cos + localXSin;
	self->offset[VERTEX_X3] = localX2Cos - localY2Sin;
	self->offset[VERTEX_Y3] = localY2Cos + localX2Sin;
	self->offset[VERTEX_X4] = localX2Cos - localYSin;
	self->offset[VERTEX_Y4] = localYCos + localX2Sin;
}

void RegionAttachment_computeVertices (RegionAttachment* self, float x, float y, Bone* bone, float* vertices) {
	float* offset = self->offset;
	x += bone->worldX;
	y += bone->worldY;
	vertices[VERTEX_X1] = offset[VERTEX_X1] * bone->m00 + offset[VERTEX_Y1] * bone->m01 + x;
	vertices[VERTEX_Y1] = offset[VERTEX_X1] * bone->m10 + offset[VERTEX_Y1] * bone->m11 + y;
	vertices[VERTEX_X2] = offset[VERTEX_X2] * bone->m00 + offset[VERTEX_Y2] * bone->m01 + x;
	vertices[VERTEX_Y2] = offset[VERTEX_X2] * bone->m10 + offset[VERTEX_Y2] * bone->m11 + y;
	vertices[VERTEX_X3] = offset[VERTEX_X3] * bone->m00 + offset[VERTEX_Y3] * bone->m01 + x;
	vertices[VERTEX_Y3] = offset[VERTEX_X3] * bone->m10 + offset[VERTEX_Y3] * bone->m11 + y;
	vertices[VERTEX_X4] = offset[VERTEX_X4] * bone->m00 + offset[VERTEX_Y4] * bone->m01 + x;
	vertices[VERTEX_Y4] = offset[VERTEX_X4] * bone->m10 + offset[VERTEX_Y4] * bone->m11 + y;
}

//
//MERGED FILE END: RegionAttachment.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Skeleton.c
//

Skeleton* Skeleton_create (SkeletonData* data) {
	int i, ii;

	Skeleton* self = NEW(Skeleton);
	CONST_CAST(SkeletonData*, self->data) = data;

	self->boneCount = self->data->boneCount;
	self->bones = MALLOC(Bone*, self->boneCount);

	for (i = 0; i < self->boneCount; ++i) {
		BoneData* boneData = self->data->bones[i];
		Bone* parent = 0;
		if (boneData->parent) {
			
			for (ii = 0; ii < self->boneCount; ++ii) {
				if (data->bones[ii] == boneData->parent) {
					parent = self->bones[ii];
					break;
				}
			}
		}
		self->bones[i] = Bone_create(boneData, parent);
	}
	CONST_CAST(Bone*, self->root) = self->bones[0];

	self->slotCount = data->slotCount;
	self->slots = MALLOC(Slot*, self->slotCount);
	for (i = 0; i < self->slotCount; ++i) {
		SlotData *slotData = data->slots[i];

		
		Bone* bone = 0;
		for (ii = 0; ii < self->boneCount; ++ii) {
			if (data->bones[ii] == slotData->boneData) {
				bone = self->bones[ii];
				break;
			}
		}
		self->slots[i] = Slot_create(slotData, self, bone);
	}

	self->drawOrder = MALLOC(Slot*, self->slotCount);
	memcpy(self->drawOrder, self->slots, sizeof(Slot*) * self->slotCount);

	self->r = 1;
	self->g = 1;
	self->b = 1;
	self->a = 1;

	return self;
}

void Skeleton_dispose (Skeleton* self) {
	int i;
	for (i = 0; i < self->boneCount; ++i)
		Bone_dispose(self->bones[i]);
	FREE(self->bones);

	for (i = 0; i < self->slotCount; ++i)
		Slot_dispose(self->slots[i]);
	FREE(self->slots);

	FREE(self->drawOrder);
	FREE(self);
}

void Skeleton_updateWorldTransform (const Skeleton* self) {
	int i;
	for (i = 0; i < self->boneCount; ++i)
		Bone_updateWorldTransform(self->bones[i], self->flipX, self->flipY);
}

void Skeleton_setToSetupPose (const Skeleton* self) {
	Skeleton_setBonesToSetupPose(self);
	Skeleton_setSlotsToSetupPose(self);
}

void Skeleton_setBonesToSetupPose (const Skeleton* self) {
	int i;
	for (i = 0; i < self->boneCount; ++i)
		Bone_setToSetupPose(self->bones[i]);
}

void Skeleton_setSlotsToSetupPose (const Skeleton* self) {
	int i;
	for (i = 0; i < self->slotCount; ++i)
		Slot_setToSetupPose(self->slots[i]);
}

Bone* Skeleton_findBone (const Skeleton* self, const char* boneName) {
	int i;
	for (i = 0; i < self->boneCount; ++i)
		if (strcmp(self->data->bones[i]->name, boneName) == 0) return self->bones[i];
	return 0;
}

int Skeleton_findBoneIndex (const Skeleton* self, const char* boneName) {
	int i;
	for (i = 0; i < self->boneCount; ++i)
		if (strcmp(self->data->bones[i]->name, boneName) == 0) return i;
	return -1;
}

Slot* Skeleton_findSlot (const Skeleton* self, const char* slotName) {
	int i;
	for (i = 0; i < self->slotCount; ++i)
		if (strcmp(self->data->slots[i]->name, slotName) == 0) return self->slots[i];
	return 0;
}

int Skeleton_findSlotIndex (const Skeleton* self, const char* slotName) {
	int i;
	for (i = 0; i < self->slotCount; ++i)
		if (strcmp(self->data->slots[i]->name, slotName) == 0) return i;
	return -1;
}

int Skeleton_setSkinByName (Skeleton* self, const char* skinName) {
	Skin *skin;
	if (!skinName) {
		Skeleton_setSkin(self, 0);
		return 1;
	}
	skin = SkeletonData_findSkin(self->data, skinName);
	if (!skin) return 0;
	Skeleton_setSkin(self, skin);
	return 1;
}

void Skeleton_setSkin (Skeleton* self, Skin* newSkin) {
	if (self->skin && newSkin) Skin_attachAll(newSkin, self, self->skin);
	CONST_CAST(Skin*, self->skin) = newSkin;
}

Attachment* Skeleton_getAttachmentForSlotName (const Skeleton* self, const char* slotName, const char* attachmentName) {
	int slotIndex = SkeletonData_findSlotIndex(self->data, slotName);
	return Skeleton_getAttachmentForSlotIndex(self, slotIndex, attachmentName);
}

Attachment* Skeleton_getAttachmentForSlotIndex (const Skeleton* self, int slotIndex, const char* attachmentName) {
	if (slotIndex == -1) return 0;
	if (self->skin) {
		Attachment *attachment = Skin_getAttachment(self->skin, slotIndex, attachmentName);
		if (attachment) return attachment;
	}
	if (self->data->defaultSkin) {
		Attachment *attachment = Skin_getAttachment(self->data->defaultSkin, slotIndex, attachmentName);
		if (attachment) return attachment;
	}
	return 0;
}

int Skeleton_setAttachment (Skeleton* self, const char* slotName, const char* attachmentName) {
	int i;
	for (i = 0; i < self->slotCount; ++i) {
		Slot *slot = self->slots[i];
		if (strcmp(slot->data->name, slotName) == 0) {
			if (!attachmentName)
				Slot_setAttachment(slot, 0);
			else {
				Attachment* attachment = Skeleton_getAttachmentForSlotIndex(self, i, attachmentName);
				if (!attachment) return 0;
				Slot_setAttachment(slot, attachment);
			}
			return 1;
		}
	}
	return 0;
}

void Skeleton_update (Skeleton* self, float deltaTime) {
	self->time += deltaTime;
}

//
//MERGED FILE END: Skeleton.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: SkeletonData.c
//

SkeletonData* SkeletonData_create () {
	return NEW(SkeletonData);
}

void SkeletonData_dispose (SkeletonData* self) {
	int i;
	for (i = 0; i < self->boneCount; ++i)
		BoneData_dispose(self->bones[i]);
	FREE(self->bones);

	for (i = 0; i < self->slotCount; ++i)
		SlotData_dispose(self->slots[i]);
	FREE(self->slots);

	for (i = 0; i < self->skinCount; ++i)
		Skin_dispose(self->skins[i]);
	FREE(self->skins);

	for (i = 0; i < self->animationCount; ++i)
		Animation_dispose(self->animations[i]);
	FREE(self->animations);

	FREE(self);
}

BoneData* SkeletonData_findBone (const SkeletonData* self, const char* boneName) {
	int i;
	for (i = 0; i < self->boneCount; ++i)
		if (strcmp(self->bones[i]->name, boneName) == 0) return self->bones[i];
	return 0;
}

int SkeletonData_findBoneIndex (const SkeletonData* self, const char* boneName) {
	int i;
	for (i = 0; i < self->boneCount; ++i)
		if (strcmp(self->bones[i]->name, boneName) == 0) return i;
	return 0;
}

SlotData* SkeletonData_findSlot (const SkeletonData* self, const char* slotName) {
	int i;
	for (i = 0; i < self->slotCount; ++i)
		if (strcmp(self->slots[i]->name, slotName) == 0) return self->slots[i];
	return 0;
}

int SkeletonData_findSlotIndex (const SkeletonData* self, const char* slotName) {
	int i;
	for (i = 0; i < self->slotCount; ++i)
		if (strcmp(self->slots[i]->name, slotName) == 0) return i;
	return 0;
}

Skin* SkeletonData_findSkin (const SkeletonData* self, const char* skinName) {
	int i;
	for (i = 0; i < self->skinCount; ++i)
		if (strcmp(self->skins[i]->name, skinName) == 0) return self->skins[i];
	return 0;
}

Animation* SkeletonData_findAnimation (const SkeletonData* self, const char* animationName) {
	int i;
	for (i = 0; i < self->animationCount; ++i)
		if (strcmp(self->animations[i]->name, animationName) == 0) return self->animations[i];
	return 0;
}

//
//MERGED FILE END: SkeletonData.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: SkeletonJson.c
//

typedef struct {
	SkeletonJson super;
	int ownsLoader;
} _InternalSkeletonJson;

SkeletonJson* SkeletonJson_createWithLoader (AttachmentLoader* attachmentLoader) {
	SkeletonJson* self = SUPER(NEW(_InternalSkeletonJson));
	self->scale = 1;
	self->attachmentLoader = attachmentLoader;
	return self;
}

SkeletonJson* SkeletonJson_create (Atlas* atlas) {
	AtlasAttachmentLoader* attachmentLoader = AtlasAttachmentLoader_create(atlas);
	SkeletonJson* self = SkeletonJson_createWithLoader(SUPER(attachmentLoader));
	SUB_CAST(_InternalSkeletonJson, self) ->ownsLoader = 1;
	return self;
}

void SkeletonJson_dispose (SkeletonJson* self) {
	if (SUB_CAST(_InternalSkeletonJson, self) ->ownsLoader) AttachmentLoader_dispose(self->attachmentLoader);
	FREE(self->error);
	FREE(self);
}

void _SkeletonJson_setError_ (SkeletonJson* self, Json* root, const char* value1, const char* value2) {
	char message[256];
	size_t length;
	FREE(self->error);
	strcpy(message, value1);
	length = strlen(value1);
	if (value2) strncat(message + length, value2, 256 - 1 - length);
	MALLOC_STR(self->error, message);
	if (root) Json_dispose(root);
}

static float toColor (const char* value, int index) {
	char digits[3];
	char *error;
	int color;

	if (strlen(value) != 8) return -1;
	value += index * 2;
	
	digits[0] = *value;
	digits[1] = *(value + 1);
	digits[2] = '\0';
	color = strtoul(digits, &error, 16);
	if (*error != 0) return -1;
	return color / (float)255;
}

static void readCurve (CurveTimeline* timeline, int frameIndex, Json* frame) {
	Json* curve = Json_getItem(frame, "curve");
	if (!curve) return;
	if (curve->type == Json_String && strcmp(curve->valuestring, "stepped") == 0)
		CurveTimeline_setStepped(timeline, frameIndex);
	else if (curve->type == Json_Array) {
		CurveTimeline_setCurve(timeline, frameIndex, Json_getItemAt(curve, 0)->valuefloat, Json_getItemAt(curve, 1)->valuefloat,
				Json_getItemAt(curve, 2)->valuefloat, Json_getItemAt(curve, 3)->valuefloat);
	}
}

static Animation* _SkeletonJson_readAnimation (SkeletonJson* self, Json* root, SkeletonData *skeletonData) {
	Animation* animation;

	Json* bones = Json_getItem(root, "bones");
	int boneCount = bones ? Json_getSize(bones) : 0;

	Json* slots = Json_getItem(root, "slots");
	int slotCount = slots ? Json_getSize(slots) : 0;

	int timelineCount = 0;
	int i, ii, iii;
	for (i = 0; i < boneCount; ++i)
		timelineCount += Json_getSize(Json_getItemAt(bones, i));
	for (i = 0; i < slotCount; ++i)
		timelineCount += Json_getSize(Json_getItemAt(slots, i));
	animation = Animation_create(root->name, timelineCount);
	animation->timelineCount = 0;
	skeletonData->animations[skeletonData->animationCount] = animation;
	skeletonData->animationCount++;

	for (i = 0; i < boneCount; ++i) {
		int timelineCount;
		Json* boneMap = Json_getItemAt(bones, i);

		const char* boneName = boneMap->name;

		int boneIndex = SkeletonData_findBoneIndex(skeletonData, boneName);
		if (boneIndex == -1) {
			Animation_dispose(animation);
			_SkeletonJson_setError(self, root, "Bone not found: ", boneName);
			return 0;
		}

		timelineCount = Json_getSize(boneMap);
		for (ii = 0; ii < timelineCount; ++ii) {
			float duration;
			Json* timelineArray = Json_getItemAt(boneMap, ii);
			int frameCount = Json_getSize(timelineArray);
			const char* timelineType = timelineArray->name;

			if (strcmp(timelineType, "rotate") == 0) {
				
				RotateTimeline *timeline = RotateTimeline_create(frameCount);
				timeline->boneIndex = boneIndex;
				for (iii = 0; iii < frameCount; ++iii) {
					Json* frame = Json_getItemAt(timelineArray, iii);
					RotateTimeline_setFrame(timeline, iii, Json_getFloat(frame, "time", 0), Json_getFloat(frame, "angle", 0));
					readCurve(SUPER(timeline), iii, frame);
				}
				animation->timelines[animation->timelineCount++] = (Timeline*)timeline;
				duration = timeline->frames[frameCount * 2 - 2];
				if (duration > animation->duration) animation->duration = duration;

			} else {
				int isScale = strcmp(timelineType, "scale") == 0;
				if (isScale || strcmp(timelineType, "translate") == 0) {
					float scale = isScale ? 1 : self->scale;
					TranslateTimeline *timeline = isScale ? ScaleTimeline_create(frameCount) : TranslateTimeline_create(frameCount);
					timeline->boneIndex = boneIndex;
					for (iii = 0; iii < frameCount; ++iii) {
						Json* frame = Json_getItemAt(timelineArray, iii);
						TranslateTimeline_setFrame(timeline, iii, Json_getFloat(frame, "time", 0), Json_getFloat(frame, "x", 0) * scale,
								Json_getFloat(frame, "y", 0) * scale);
						readCurve(SUPER(timeline), iii, frame);
					}
					animation->timelines[animation->timelineCount++] = (Timeline*)timeline;
					duration = timeline->frames[frameCount * 3 - 3];
					if (duration > animation->duration) animation->duration = duration;
				} else {
					Animation_dispose(animation);
					_SkeletonJson_setError(self, 0, "Invalid timeline type for a bone: ", timelineType);
					return 0;
				}
			}
		}
	}

	for (i = 0; i < slotCount; ++i) {
		int timelineCount;
		Json* slotMap = Json_getItemAt(slots, i);
		const char* slotName = slotMap->name;

		int slotIndex = SkeletonData_findSlotIndex(skeletonData, slotName);
		if (slotIndex == -1) {
			Animation_dispose(animation);
			_SkeletonJson_setError(self, root, "Slot not found: ", slotName);
			return 0;
		}

		timelineCount = Json_getSize(slotMap);
		for (ii = 0; ii < timelineCount; ++ii) {
			float duration;
			Json* timelineArray = Json_getItemAt(slotMap, ii);
			int frameCount = Json_getSize(timelineArray);
			const char* timelineType = timelineArray->name;

			if (strcmp(timelineType, "color") == 0) {
				ColorTimeline *timeline = ColorTimeline_create(frameCount);
				timeline->slotIndex = slotIndex;
				for (iii = 0; iii < frameCount; ++iii) {
					Json* frame = Json_getItemAt(timelineArray, iii);
					const char* s = Json_getString(frame, "color", 0);
					ColorTimeline_setFrame(timeline, iii, Json_getFloat(frame, "time", 0), toColor(s, 0), toColor(s, 1), toColor(s, 2),
							toColor(s, 3));
					readCurve(SUPER(timeline), iii, frame);
				}
				animation->timelines[animation->timelineCount++] = (Timeline*)timeline;
				duration = timeline->frames[frameCount * 5 - 5];
				if (duration > animation->duration) animation->duration = duration;

			} else if (strcmp(timelineType, "attachment") == 0) {
				AttachmentTimeline *timeline = AttachmentTimeline_create(frameCount);
				timeline->slotIndex = slotIndex;
				for (iii = 0; iii < frameCount; ++iii) {
					Json* frame = Json_getItemAt(timelineArray, iii);
					Json* name = Json_getItem(frame, "name");
					AttachmentTimeline_setFrame(timeline, iii, Json_getFloat(frame, "time", 0),
							name->type == Json_NULL ? 0 : name->valuestring);
				}
				animation->timelines[animation->timelineCount++] = (Timeline*)timeline;
				duration = timeline->frames[frameCount - 1];
				if (duration > animation->duration) animation->duration = duration;

			} else {
				Animation_dispose(animation);
				_SkeletonJson_setError(self, 0, "Invalid timeline type for a slot: ", timelineType);
				return 0;
			}
		}
	}

	return animation;
}

SkeletonData* SkeletonJson_readSkeletonDataFile (SkeletonJson* self, const char* path) {
	size_t length;
	SkeletonData* skeletonData;
	const char* json = _Util_readFile(path, &length);
	if (!json) {
		_SkeletonJson_setError(self, 0, "Unable to read skeleton file: ", path);
		return 0;
	}
	skeletonData = SkeletonJson_readSkeletonData(self, json);
	FREE(json);
	return skeletonData;
}

SkeletonData* SkeletonJson_readSkeletonData (SkeletonJson* self, const char* json) {
	SkeletonData* skeletonData;
	Json *root, *bones;
	int i, ii, iii, boneCount;
	Json* slots;
	Json* skinsMap;
	Json* animations;

	FREE(self->error);
	CONST_CAST(char*, self->error) = 0;

	root = Json_create(json);
	if (!root) {
		_SkeletonJson_setError(self, 0, "Invalid skeleton JSON: ", Json_getError());
		return 0;
	}

	skeletonData = SkeletonData_create();

	bones = Json_getItem(root, "bones");
	boneCount = Json_getSize(bones);
	skeletonData->bones = MALLOC(BoneData*, boneCount);
	for (i = 0; i < boneCount; ++i) {
		Json* boneMap = Json_getItemAt(bones, i);
		BoneData* boneData;

		const char* boneName = Json_getString(boneMap, "name", 0);

		BoneData* parent = 0;
		const char* parentName = Json_getString(boneMap, "parent", 0);
		if (parentName) {
			parent = SkeletonData_findBone(skeletonData, parentName);
			if (!parent) {
				SkeletonData_dispose(skeletonData);
				_SkeletonJson_setError(self, root, "Parent bone not found: ", parentName);
				return 0;
			}
		}

		boneData = BoneData_create(boneName, parent);
		boneData->length = Json_getFloat(boneMap, "length", 0) * self->scale;
		boneData->x = Json_getFloat(boneMap, "x", 0) * self->scale;
		boneData->y = Json_getFloat(boneMap, "y", 0) * self->scale;
		boneData->rotation = Json_getFloat(boneMap, "rotation", 0);
		boneData->scaleX = Json_getFloat(boneMap, "scaleX", 1);
		boneData->scaleY = Json_getFloat(boneMap, "scaleY", 1);

		skeletonData->bones[i] = boneData;
		skeletonData->boneCount++;
	}

	slots = Json_getItem(root, "slots");
	if (slots) {
		int slotCount = Json_getSize(slots);
		skeletonData->slots = MALLOC(SlotData*, slotCount);
		for (i = 0; i < slotCount; ++i) {
			SlotData* slotData;
			const char* color;
			Json *attachmentItem;
			Json* slotMap = Json_getItemAt(slots, i);

			const char* slotName = Json_getString(slotMap, "name", 0);

			const char* boneName = Json_getString(slotMap, "bone", 0);
			BoneData* boneData = SkeletonData_findBone(skeletonData, boneName);
			if (!boneData) {
				SkeletonData_dispose(skeletonData);
				_SkeletonJson_setError(self, root, "Slot bone not found: ", boneName);
				return 0;
			}

			slotData = SlotData_create(slotName, boneData);

			color = Json_getString(slotMap, "color", 0);
			if (color) {
				slotData->r = toColor(color, 0);
				slotData->g = toColor(color, 1);
				slotData->b = toColor(color, 2);
				slotData->a = toColor(color, 3);
			}

			attachmentItem = Json_getItem(slotMap, "attachment");
			if (attachmentItem) SlotData_setAttachmentName(slotData, attachmentItem->valuestring);

			skeletonData->slots[i] = slotData;
			skeletonData->slotCount++;
		}
	}

	skinsMap = Json_getItem(root, "skins");
	if (skinsMap) {
		int skinCount = Json_getSize(skinsMap);
		skeletonData->skins = MALLOC(Skin*, skinCount);
		for (i = 0; i < skinCount; ++i) {
			Json* slotMap = Json_getItemAt(skinsMap, i);
			const char* skinName = slotMap->name;
			Skin *skin = Skin_create(skinName);
			int slotNameCount;

			skeletonData->skins[i] = skin;
			skeletonData->skinCount++;
			if (strcmp(skinName, "default") == 0) skeletonData->defaultSkin = skin;

			slotNameCount = Json_getSize(slotMap);
			for (ii = 0; ii < slotNameCount; ++ii) {
				Json* attachmentsMap = Json_getItemAt(slotMap, ii);
				const char* slotName = attachmentsMap->name;
				int slotIndex = SkeletonData_findSlotIndex(skeletonData, slotName);

				int attachmentCount = Json_getSize(attachmentsMap);
				for (iii = 0; iii < attachmentCount; ++iii) {
					Attachment* attachment;
					Json* attachmentMap = Json_getItemAt(attachmentsMap, iii);
					const char* skinAttachmentName = attachmentMap->name;
					const char* attachmentName = Json_getString(attachmentMap, "name", skinAttachmentName);

					const char* typeString = Json_getString(attachmentMap, "type", "region");
					AttachmentType type;
					if (strcmp(typeString, "region") == 0)
						type = ATTACHMENT_REGION;
					else if (strcmp(typeString, "regionSequence") == 0)
						type = ATTACHMENT_REGION_SEQUENCE;
					else {
						SkeletonData_dispose(skeletonData);
						_SkeletonJson_setError(self, root, "Unknown attachment type: ", typeString);
						return 0;
					}

					attachment = AttachmentLoader_newAttachment(self->attachmentLoader, skin, type, attachmentName);
					if (!attachment) {
						if (self->attachmentLoader->error1) {
							SkeletonData_dispose(skeletonData);
							_SkeletonJson_setError(self, root, self->attachmentLoader->error1, self->attachmentLoader->error2);
							return 0;
						}
						continue;
					}

					if (attachment->type == ATTACHMENT_REGION || attachment->type == ATTACHMENT_REGION_SEQUENCE) {
						RegionAttachment* regionAttachment = (RegionAttachment*)attachment;
						regionAttachment->x = Json_getFloat(attachmentMap, "x", 0) * self->scale;
						regionAttachment->y = Json_getFloat(attachmentMap, "y", 0) * self->scale;
						regionAttachment->scaleX = Json_getFloat(attachmentMap, "scaleX", 1);
						regionAttachment->scaleY = Json_getFloat(attachmentMap, "scaleY", 1);
						regionAttachment->rotation = Json_getFloat(attachmentMap, "rotation", 0);
						regionAttachment->width = Json_getFloat(attachmentMap, "width", 32) * self->scale;
						regionAttachment->height = Json_getFloat(attachmentMap, "height", 32) * self->scale;
						RegionAttachment_updateOffset(regionAttachment);
					}

					Skin_addAttachment(skin, slotIndex, skinAttachmentName, attachment);
				}
			}
		}
	}

	animations = Json_getItem(root, "animations");
	if (animations) {
		int animationCount = Json_getSize(animations);
		skeletonData->animations = MALLOC(Animation*, animationCount);
		for (i = 0; i < animationCount; ++i) {
			Json* animationMap = Json_getItemAt(animations, i);
			_SkeletonJson_readAnimation(self, animationMap, skeletonData);
		}
	}

	Json_dispose(root);
	return skeletonData;
}

//
//MERGED FILE END: SkeletonJson.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Skin.c
//

typedef struct _EntrySkin _EntrySkin;
struct _EntrySkin {
	int slotIndex;
	const char* name;
	Attachment* attachment;
	_EntrySkin* next;
};

_EntrySkin* _EntrySkin_create (int slotIndex, const char* name, Attachment* attachment) {
	_EntrySkin* self = NEW(_EntrySkin);
	self->slotIndex = slotIndex;
	MALLOC_STR(self->name, name);
	self->attachment = attachment;
	return self;
}

void _EntrySkin_dispose (_EntrySkin* self) {
	Attachment_dispose(self->attachment);
	FREE(self->name);
	FREE(self);
}



typedef struct {
	Skin super;
	_EntrySkin* entries;
} _InternalSkin;

Skin* Skin_create (const char* name) {
	Skin* self = SUPER(NEW(_InternalSkin));
	MALLOC_STR(self->name, name);
	return self;
}

void Skin_dispose (Skin* self) {
	_EntrySkin* entry = SUB_CAST(_InternalSkin, self) ->entries;
	while (entry) {
		_EntrySkin* nextEntry = entry->next;
		_EntrySkin_dispose(entry);
		entry = nextEntry;
	}

	FREE(self->name);
	FREE(self);
}

void Skin_addAttachment (Skin* self, int slotIndex, const char* name, Attachment* attachment) {
	_EntrySkin* newEntry = _EntrySkin_create(slotIndex, name, attachment);
	newEntry->next = SUB_CAST(_InternalSkin, self) ->entries;
	SUB_CAST(_InternalSkin, self) ->entries = newEntry;
}

Attachment* Skin_getAttachment (const Skin* self, int slotIndex, const char* name) {
	const _EntrySkin* entry = SUB_CAST(_InternalSkin, self) ->entries;
	while (entry) {
		if (entry->slotIndex == slotIndex && strcmp(entry->name, name) == 0) return entry->attachment;
		entry = entry->next;
	}
	return 0;
}

const char* Skin_getAttachmentName (const Skin* self, int slotIndex, int attachmentIndex) {
	const _EntrySkin* entry = SUB_CAST(_InternalSkin, self) ->entries;
	int i = 0;
	while (entry) {
		if (entry->slotIndex == slotIndex) {
			if (i == attachmentIndex) return entry->name;
			i++;
		}
		entry = entry->next;
	}
	return 0;
}

void Skin_attachAll (const Skin* self, Skeleton* skeleton, const Skin* oldSkin) {
	const _EntrySkin *entry = SUB_CAST(_InternalSkin, oldSkin) ->entries;
	while (entry) {
		Slot *slot = skeleton->slots[entry->slotIndex];
		if (slot->attachment == entry->attachment) {
			Attachment *attachment = Skin_getAttachment(self, entry->slotIndex, entry->name);
			if (attachment) Slot_setAttachment(slot, attachment);
		}
		entry = entry->next;
	}
}

//
//MERGED FILE END: Skin.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Slot.c
//

typedef struct {
	Slot super;
	float attachmentTime;
} _InternalSlot;

Slot* Slot_create (SlotData* data, Skeleton* skeleton, Bone* bone) {
	Slot* self = SUPER(NEW(_InternalSlot));
	CONST_CAST(SlotData*, self->data) = data;
	CONST_CAST(Skeleton*, self->skeleton) = skeleton;
	CONST_CAST(Bone*, self->bone) = bone;
	Slot_setToSetupPose(self);
	return self;
}

void Slot_dispose (Slot* self) {
	FREE(self);
}

void Slot_setAttachment (Slot* self, Attachment* attachment) {
	CONST_CAST(Attachment*, self->attachment) = attachment;
 	SUB_CAST(_InternalSlot, self) ->attachmentTime = self->skeleton->time;
}

void Slot_setAttachmentTime (Slot* self, float time) {
	SUB_CAST(_InternalSlot, self) ->attachmentTime = self->skeleton->time - time;
}

float Slot_getAttachmentTime (const Slot* self) {
	return self->skeleton->time - SUB_CAST(_InternalSlot, self) ->attachmentTime;
}

void Slot_setToSetupPose (Slot* self) {
	Attachment* attachment = 0;
	self->r = self->data->r;
	self->g = self->data->g;
	self->b = self->data->b;
	self->a = self->data->a;

	if (self->data->attachmentName) {
		
		int i;
		for (i = 0; i < self->skeleton->data->slotCount; ++i) {
			if (self->data == self->skeleton->data->slots[i]) {
				attachment = Skeleton_getAttachmentForSlotIndex(self->skeleton, i, self->data->attachmentName);
				break;
			}
		}
	}
	Slot_setAttachment(self, attachment);
}

//
//MERGED FILE END: Slot.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: SlotData.c
//

SlotData* SlotData_create (const char* name, BoneData* boneData) {
	SlotData* self = NEW(SlotData);
	MALLOC_STR(self->name, name);
	CONST_CAST(BoneData*, self->boneData) = boneData;
	self->r = 1;
	self->g = 1;
	self->b = 1;
	self->a = 1;
	return self;
}

void SlotData_dispose (SlotData* self) {
	FREE(self->name);
	FREE(self->attachmentName);
	FREE(self);
}

void SlotData_setAttachmentName (SlotData* self, const char* attachmentName) {
	FREE(self->attachmentName);
	if (attachmentName)
		MALLOC_STR(self->attachmentName, attachmentName);
	else
		CONST_CAST(char*, self->attachmentName) = 0;
}

//
//MERGED FILE END: SlotData.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//ZILLALIB INTEGRATION
//

#include <ZL_Surface.h>
#include <ZL_File.h>

void _AtlasPage_createTexture (AtlasPage* self, const char* path) {
	ZL_Surface* pSurface = new ZL_Surface(path);
	self->rendererObject = pSurface;
	self->width = pSurface->GetWidth();
	self->height = pSurface->GetHeight();
}

void _AtlasPage_disposeTexture (AtlasPage* self) {
	delete (ZL_Surface*)self->rendererObject;
}

char* _Util_readFile (const char* path, size_t* length) {
	return ZL_File(path).GetContentsMalloc(length);
}

//
//------------------------------------------------------------------------------------------------------------------------------------------------------
