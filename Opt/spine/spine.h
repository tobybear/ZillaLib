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

#include <stdlib.h>
#include <string.h>

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: spine.h
//

#ifndef SPINE_SPINE_H_
#define SPINE_SPINE_H_


//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Animation.h
//

#ifndef SPINE_ANIMATION_H_
#define SPINE_ANIMATION_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Timeline Timeline;
struct Skeleton;

typedef struct {
	const char* const name;
	float duration;

	int timelineCount;
	Timeline** timelines;
} Animation;

Animation* Animation_create (const char* name, int timelineCount);
void Animation_dispose (Animation* self);

void Animation_apply (const Animation* self, struct Skeleton* skeleton, float time, bool loop);
void Animation_mix (const Animation* self, struct Skeleton* skeleton, float time, bool loop, float alpha);

/**/

struct Timeline {
	const void* const vtable;
};

void Timeline_dispose (Timeline* self);
void Timeline_apply (const Timeline* self, struct Skeleton* skeleton, float time, float alpha);

/**/

typedef struct {
	Timeline super;
	float* curves; /* dfx, dfy, ddfx, ddfy, dddfx, dddfy, ... */
} CurveTimeline;

void CurveTimeline_setLinear (CurveTimeline* self, int frameIndex);
void CurveTimeline_setStepped (CurveTimeline* self, int frameIndex);

/* Sets the control handle positions for an interpolation bezier curve used to transition from this keyframe to the next.
 * cx1 and cx2 are from 0 to 1, representing the percent of time between the two keyframes. cy1 and cy2 are the percent of
 * the difference between the keyframe's values. */
void CurveTimeline_setCurve (CurveTimeline* self, int frameIndex, float cx1, float cy1, float cx2, float cy2);
float CurveTimeline_getCurvePercent (const CurveTimeline* self, int frameIndex, float percent);

/**/

typedef struct BaseTimeline {
	CurveTimeline super;
	int const framesLength;
	float* const frames; /* time, angle, ... for rotate. time, x, y, ... for translate and scale. */
	int boneIndex;
} RotateTimeline;

RotateTimeline* RotateTimeline_create (int frameCount);

void RotateTimeline_setFrame (RotateTimeline* self, int frameIndex, float time, float angle);

/**/

typedef struct BaseTimeline TranslateTimeline;

TranslateTimeline* TranslateTimeline_create (int frameCount);

void TranslateTimeline_setFrame (TranslateTimeline* self, int frameIndex, float time, float x, float y);

/**/

typedef struct BaseTimeline ScaleTimeline;

ScaleTimeline* ScaleTimeline_create (int frameCount);

void ScaleTimeline_setFrame (ScaleTimeline* self, int frameIndex, float time, float x, float y);

/**/

typedef struct {
	CurveTimeline super;
	int const framesLength;
	float* const frames; /* time, r, g, b, a, ... */
	int slotIndex;
} ColorTimeline;

ColorTimeline* ColorTimeline_create (int frameCount);

void ColorTimeline_setFrame (ColorTimeline* self, int frameIndex, float time, float r, float g, float b, float a);

/**/

typedef struct {
	Timeline super;
	int const framesLength;
	float* const frames; /* time, ... */
	int slotIndex;
	const char** const attachmentNames;
} AttachmentTimeline;

AttachmentTimeline* AttachmentTimeline_create (int frameCount);

/* @param attachmentName May be 0. */
void AttachmentTimeline_setFrame (AttachmentTimeline* self, int frameIndex, float time, const char* attachmentName);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ANIMATION_H_ */

//
//MERGED FILE END: Animation.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: AnimationState.h
//

#ifndef SPINE_ANIMATIONSTATE_H_
#define SPINE_ANIMATIONSTATE_H_


//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: AnimationStateData.h
//

#ifndef SPINE_ANIMATIONSTATEDATA_H_
#define SPINE_ANIMATIONSTATEDATA_H_


//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: SkeletonData.h
//

#ifndef SPINE_SKELETONDATA_H_
#define SPINE_SKELETONDATA_H_


//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: BoneData.h
//

#ifndef SPINE_BONEDATA_H_
#define SPINE_BONEDATA_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BoneData BoneData;
struct BoneData {
	const char* const name;
	BoneData* const parent;
	float length;
	float x, y;
	float rotation;
	float scaleX, scaleY;
};

BoneData* BoneData_create (const char* name, BoneData* parent);
void BoneData_dispose (BoneData* self);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_BONEDATA_H_ */

//
//MERGED FILE END: BoneData.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: SlotData.h
//

#ifndef SPINE_SLOTDATA_H_
#define SPINE_SLOTDATA_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char* const name;
	const BoneData* const boneData;
	const char* const attachmentName;
	float r, g, b, a;
} SlotData;

SlotData* SlotData_create (const char* name, BoneData* boneData);
void SlotData_dispose (SlotData* self);

/* @param attachmentName May be 0 for no setup pose attachment. */
void SlotData_setAttachmentName (SlotData* self, const char* attachmentName);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_SLOTDATA_H_ */

//
//MERGED FILE END: SlotData.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Skin.h
//

#ifndef SPINE_SKIN_H_
#define SPINE_SKIN_H_


//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Attachment.h
//

#ifndef SPINE_ATTACHMENT_H_
#define SPINE_ATTACHMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

struct Slot;

typedef enum {
	ATTACHMENT_REGION, ATTACHMENT_REGION_SEQUENCE
} AttachmentType;

typedef struct Attachment Attachment;
struct Attachment {
	const char* const name;
	AttachmentType type;

	const void* const vtable;
};

void Attachment_dispose (Attachment* self);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ATTACHMENT_H_ */

//
//MERGED FILE END: Attachment.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

struct Skeleton;

typedef struct {
	const char* const name;
} Skin;

Skin* Skin_create (const char* name);
void Skin_dispose (Skin* self);

/* The Skin owns the attachment. */
void Skin_addAttachment (Skin* self, int slotIndex, const char* name, Attachment* attachment);
/* Returns 0 if the attachment was not found. */
Attachment* Skin_getAttachment (const Skin* self, int slotIndex, const char* name);

/* Returns 0 if the slot or attachment was not found. */
const char* Skin_getAttachmentName (const Skin* self, int slotIndex, int attachmentIndex);

/** Attach each attachment in this skin if the corresponding attachment in oldSkin is currently attached. */
void Skin_attachAll (const Skin* self, struct Skeleton* skeleton, const Skin* oldSkin);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_SKIN_H_ */

//
//MERGED FILE END: Skin.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int boneCount;
	BoneData** bones;

	int slotCount;
	SlotData** slots;

	int skinCount;
	Skin** skins;
	Skin* defaultSkin;

	int animationCount;
	Animation** animations;
} SkeletonData;

SkeletonData* SkeletonData_create ();
void SkeletonData_dispose (SkeletonData* self);

BoneData* SkeletonData_findBone (const SkeletonData* self, const char* boneName);
int SkeletonData_findBoneIndex (const SkeletonData* self, const char* boneName);

SlotData* SkeletonData_findSlot (const SkeletonData* self, const char* slotName);
int SkeletonData_findSlotIndex (const SkeletonData* self, const char* slotName);

Skin* SkeletonData_findSkin (const SkeletonData* self, const char* skinName);

Animation* SkeletonData_findAnimation (const SkeletonData* self, const char* animationName);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_SKELETONDATA_H_ */

//
//MERGED FILE END: SkeletonData.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	SkeletonData* const skeletonData;
	float defaultMix;
	const void* const entries;
} AnimationStateData;

AnimationStateData* AnimationStateData_create (SkeletonData* skeletonData);
void AnimationStateData_dispose (AnimationStateData* self);

void AnimationStateData_setMixByName (AnimationStateData* self, const char* fromName, const char* toName, float duration);
void AnimationStateData_setMix (AnimationStateData* self, Animation* from, Animation* to, float duration);
/* Returns 0 if there is no mixing between the animations. */
float AnimationStateData_getMix (AnimationStateData* self, Animation* from, Animation* to);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ANIMATIONSTATEDATA_H_ */

//
//MERGED FILE END: AnimationStateData.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	AnimationStateData* const data;
	Animation* const animation;
	float time;
	bool loop;
} AnimationState;

/* @param data May be 0 for no mixing. */
AnimationState* AnimationState_create (AnimationStateData* data);
void AnimationState_dispose (AnimationState* self);

void AnimationState_update (AnimationState* self, float delta);

void AnimationState_apply (AnimationState* self, struct Skeleton* skeleton);

/* @param animationName May be 0. */
void AnimationState_setAnimationByName (AnimationState* self, const char* animationName, bool loop);
/* @param animation May be 0. */
void AnimationState_setAnimation (AnimationState* self, Animation* animation, bool loop);

/** @param animationName May be 0.
 * @param delay May be <= 0 to use duration of previous animation minus any mix duration plus the negative delay. */
void AnimationState_addAnimationByName (AnimationState* self, const char* animationName, bool loop, float delay);
/** @param animation May be 0.
 * @param delay May be <= 0 to use duration of previous animation minus any mix duration plus the negative delay. */
void AnimationState_addAnimation (AnimationState* self, Animation* animation, bool loop, float delay);

void AnimationState_clearAnimation (AnimationState* self);

bool AnimationState_isComplete (AnimationState* self);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ANIMATIONSTATE_H_ */

//
//MERGED FILE END: AnimationState.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Atlas.h
//

#ifndef SPINE_ATLAS_H_
#define SPINE_ATLAS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	ATLAS_ALPHA, ATLAS_INTENSITY, ATLAS_LUMINANCE_ALPHA, ATLAS_RGB565, ATLAS_RGBA4444, ATLAS_RGB888, ATLAS_RGBA8888
} AtlasFormat;

typedef enum {
	ATLAS_NEAREST,
	ATLAS_LINEAR,
	ATLAS_MIPMAP,
	ATLAS_MIPMAP_NEAREST_NEAREST,
	ATLAS_MIPMAP_LINEAR_NEAREST,
	ATLAS_MIPMAP_NEAREST_LINEAR,
	ATLAS_MIPMAP_LINEAR_LINEAR
} AtlasFilter;

typedef enum {
	ATLAS_MIRROREDREPEAT, ATLAS_CLAMPTOEDGE, ATLAS_REPEAT
} AtlasWrap;

typedef struct AtlasPage AtlasPage;
struct AtlasPage {
	const char* name;
	AtlasFormat format;
	AtlasFilter minFilter, magFilter;
	AtlasWrap uWrap, vWrap;

	void* rendererObject;
	int width, height;

	AtlasPage* next;
};

AtlasPage* AtlasPage_create (const char* name);
void AtlasPage_dispose (AtlasPage* self);

/**/

typedef struct AtlasRegion AtlasRegion;
struct AtlasRegion {
	const char* name;
	int x, y, width, height;
	float u, v, u2, v2;
	int offsetX, offsetY;
	int originalWidth, originalHeight;
	int index;
	bool rotate;
	bool flip;
	int* splits;
	int* pads;

	AtlasPage* page;

	AtlasRegion* next;
};

AtlasRegion* AtlasRegion_create ();
void AtlasRegion_dispose (AtlasRegion* self);

/**/

typedef struct {
	AtlasPage* pages;
	AtlasRegion* regions;
} Atlas;

/* Image files referenced in the atlas file will be prefixed with dir. */
Atlas* Atlas_readAtlas (const char* data, int length, const char* dir);
/* Image files referenced in the atlas file will be prefixed with the directory containing the atlas file. */
Atlas* Atlas_readAtlasFile (const char* path);
void Atlas_dispose (Atlas* atlas);

/* Returns 0 if the region was not found. */
AtlasRegion* Atlas_findRegion (const Atlas* self, const char* name);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ATLAS_H_ */

//
//MERGED FILE END: Atlas.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: AtlasAttachmentLoader.h
//

#ifndef SPINE_ATLASATTACHMENTLOADER_H_
#define SPINE_ATLASATTACHMENTLOADER_H_


//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: AttachmentLoader.h
//

#ifndef SPINE_ATTACHMENTLOADER_H_
#define SPINE_ATTACHMENTLOADER_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct AttachmentLoader AttachmentLoader;
struct AttachmentLoader {
	const char* error1;
	const char* error2;

	const void* const vtable;
#ifdef __cplusplus
	AttachmentLoader () : error1(0), error2(0), vtable(0) {}
#endif
};

void AttachmentLoader_dispose (AttachmentLoader* self);

/* Returns 0 to not load an attachment. If 0 is returned and AttachmentLoader.error1 is set, an error occurred. */
Attachment* AttachmentLoader_newAttachment (AttachmentLoader* self, Skin* skin, AttachmentType type, const char* name);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ATTACHMENTLOADER_H_ */

//
//MERGED FILE END: AttachmentLoader.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	AttachmentLoader super;
	Atlas* atlas;
} AtlasAttachmentLoader;

AtlasAttachmentLoader* AtlasAttachmentLoader_create (Atlas* atlas);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_ATLASATTACHMENTLOADER_H_ */

//
//MERGED FILE END: AtlasAttachmentLoader.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Bone.h
//

#ifndef SPINE_BONE_H_
#define SPINE_BONE_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct Bone Bone;
struct Bone {
	BoneData* const data;
	Bone* const parent;
	float x, y;
	float rotation;
	float scaleX, scaleY;

	float const m00, m01, worldX; /* a b x */
	float const m10, m11, worldY; /* c d y */
	float const worldRotation;
	float const worldScaleX, worldScaleY;
};

void Bone_setYDown (bool yDown);

/* @param parent May be 0. */
Bone* Bone_create (BoneData* data, Bone* parent);
void Bone_dispose (Bone* self);

void Bone_setToSetupPose (Bone* self);

void Bone_updateWorldTransform (Bone* self, bool flipX, bool flipY);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_BONE_H_ */

//
//MERGED FILE END: Bone.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: RegionAttachment.h
//

#ifndef SPINE_REGIONATTACHMENT_H_
#define SPINE_REGIONATTACHMENT_H_


//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Slot.h
//

#ifndef SPINE_SLOT_H_
#define SPINE_SLOT_H_


#ifdef __cplusplus
extern "C" {
#endif

struct Skeleton;

typedef struct Slot {
	SlotData* const data;
	struct Skeleton* const skeleton;
	Bone* const bone;
	float r, g, b, a;
	Attachment* const attachment;
} Slot;

Slot* Slot_create (SlotData* data, struct Skeleton* skeleton, Bone* bone);
void Slot_dispose (Slot* self);

/* @param attachment May be 0 to clear the attachment for the slot. */
void Slot_setAttachment (Slot* self, Attachment* attachment);

void Slot_setAttachmentTime (Slot* self, float time);
float Slot_getAttachmentTime (const Slot* self);

void Slot_setToSetupPose (Slot* self);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_SLOT_H_ */

//
//MERGED FILE END: Slot.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//typedef enum {
//	VERTEX_X1 = 0, VERTEX_Y1, VERTEX_X2, VERTEX_Y2, VERTEX_X3, VERTEX_Y3, VERTEX_X4, VERTEX_Y4
//} VertexIndex;

typedef struct RegionAttachment RegionAttachment;
struct RegionAttachment {
	Attachment super;
	float x, y, scaleX, scaleY, rotation, width, height;

	void* rendererObject;
	int regionOffsetX, regionOffsetY; /* Pixels stripped from the bottom left, unrotated. */
	int regionWidth, regionHeight; /* Unrotated, stripped pixel size. */
	int regionOriginalWidth, regionOriginalHeight; /* Unrotated, unstripped pixel size. */

	float offset[8];
	float uvs[8];
};

RegionAttachment* RegionAttachment_create (const char* name);
void RegionAttachment_setUVs (RegionAttachment* self, float u, float v, float u2, float v2, bool rotate);
void RegionAttachment_updateOffset (RegionAttachment* self);
void RegionAttachment_computeVertices (RegionAttachment* self, float x, float y, Bone* bone, float* vertices);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_REGIONATTACHMENT_H_ */

//
//MERGED FILE END: RegionAttachment.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Skeleton.h
//

#ifndef SPINE_SKELETON_H_
#define SPINE_SKELETON_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct Skeleton Skeleton;
struct Skeleton {
	SkeletonData* const data;

	int boneCount;
	Bone** bones;
	Bone* const root;

	int slotCount;
	Slot** slots;
	Slot** drawOrder;

	Skin* const skin;
	float r, g, b, a;
	float time;
	bool flipX, flipY;
	float x, y;
};

Skeleton* Skeleton_create (SkeletonData* data);
void Skeleton_dispose (Skeleton* self);

void Skeleton_updateWorldTransform (const Skeleton* self);

void Skeleton_setToSetupPose (const Skeleton* self);
void Skeleton_setBonesToSetupPose (const Skeleton* self);
void Skeleton_setSlotsToSetupPose (const Skeleton* self);

/* Returns 0 if the bone was not found. */
Bone* Skeleton_findBone (const Skeleton* self, const char* boneName);
/* Returns -1 if the bone was not found. */
int Skeleton_findBoneIndex (const Skeleton* self, const char* boneName);

/* Returns 0 if the slot was not found. */
Slot* Skeleton_findSlot (const Skeleton* self, const char* slotName);
/* Returns -1 if the slot was not found. */
int Skeleton_findSlotIndex (const Skeleton* self, const char* slotName);

/* Sets the skin used to look up attachments not found in the SkeletonData defaultSkin. Attachments from the new skin are
 * attached if the corresponding attachment from the old skin was attached.
 * @param skin May be 0.*/
void Skeleton_setSkin (Skeleton* self, Skin* skin);
/* Returns 0 if the skin was not found. See Skeleton_setSkin.
 * @param skinName May be 0. */
int Skeleton_setSkinByName (Skeleton* self, const char* skinName);

/* Returns 0 if the slot or attachment was not found. */
Attachment* Skeleton_getAttachmentForSlotName (const Skeleton* self, const char* slotName, const char* attachmentName);
/* Returns 0 if the slot or attachment was not found. */
Attachment* Skeleton_getAttachmentForSlotIndex (const Skeleton* self, int slotIndex, const char* attachmentName);
/* Returns 0 if the slot or attachment was not found. */
int Skeleton_setAttachment (Skeleton* self, const char* slotName, const char* attachmentName);

void Skeleton_update (Skeleton* self, float deltaTime);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_SKELETON_H_*/

//
//MERGED FILE END: Skeleton.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: SkeletonJson.h
//

#ifndef SPINE_SKELETONJSON_H_
#define SPINE_SKELETONJSON_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float scale;
	AttachmentLoader* attachmentLoader;
	const char* const error;
} SkeletonJson;

SkeletonJson* SkeletonJson_createWithLoader (AttachmentLoader* attachmentLoader);
SkeletonJson* SkeletonJson_create (Atlas* atlas);
void SkeletonJson_dispose (SkeletonJson* self);

SkeletonData* SkeletonJson_readSkeletonData (SkeletonJson* self, const char* json);
SkeletonData* SkeletonJson_readSkeletonDataFile (SkeletonJson* self, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_SKELETONJSON_H_ */

//
//MERGED FILE END: SkeletonJson.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* SPINE_SPINE_H_ */

//
//MERGED FILE END: spine.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: extension.h
//

/*
 Implementation notes:

 - An OOP style is used where each "class" is made up of a struct and a number of functions prefixed with the struct name.

 - struct fields that are const are readonly. Either they are set in a create function and can never be changed, or they can only
 be changed by calling a function.

 - Inheritance is done using a struct field named "super" as the first field, allowing the struct to be cast to its "super class".
 This works because a pointer to a struct is guaranteed to be a pointer to the first struct field.

 - Classes intended for inheritance provide init/deinit functions which subclasses must call in their create/dispose functions.

 - Polymorphism is done by a base class providing function pointers in its init function. The public API delegates to this
 function.

 - Subclasses do not provide a dispose function, instead the base class' dispose function should be used, which will delegate to
 a dispose function.

 - Classes not designed for inheritance cannot be extended. They may use an internal subclass to hide private data and don't
 expose function pointers.

 - The public API hides implementation details such init/deinit functions. An internal API is exposed in extension.h to allow
 classes to be extended. Internal functions begin with underscore (_).

 - OOP in C tends to lose type safety. Macros are provided in extension.h to give context for why a cast is being done.
 */

#ifndef SPINE_EXTENSION_H_
#define SPINE_EXTENSION_H_

/* All allocation uses these. */
#define MALLOC(TYPE,COUNT) ((TYPE*)malloc(sizeof(TYPE) * COUNT))
#define CALLOC(TYPE,COUNT) ((TYPE*)calloc(COUNT, sizeof(TYPE)))
#define NEW(TYPE) CALLOC(TYPE,1)

/* Gets the direct super class. Type safe. */
#define SUPER(VALUE) (&VALUE->super)

/* Cast to a super class. Not type safe, use with care. Prefer SUPER() where possible. */
#define SUPER_CAST(TYPE,VALUE) ((TYPE*)VALUE)

/* Cast to a sub class. Not type safe, use with care. */
#define SUB_CAST(TYPE,VALUE) ((TYPE*)VALUE)

/* Casts away const. Can be used as an lvalue. Not type safe, use with care. */
#define CONST_CAST(TYPE,VALUE) (*(TYPE*)&VALUE)

/* Gets the vtable for the specified type. Not type safe, use with care. */
#define VTABLE(TYPE,VALUE) ((_##TYPE##Vtable*)((TYPE*)VALUE)->vtable)

/* Frees memory. Can be used on const types. */
#define FREE(VALUE) free((void*)VALUE)

/* Allocates a new char[], assigns it to TO, and copies FROM to it. Can be used on const types. */
#define MALLOC_STR(TO,FROM) strcpy(CONST_CAST(char*, TO) = (char*)malloc(strlen(FROM) + 1), FROM)


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Functions that must be implemented:
 */

void _AtlasPage_createTexture (AtlasPage* self, const char* path);
void _AtlasPage_disposeTexture (AtlasPage* self);
char* _Util_readFile (const char* path, size_t* length);

/*
 * Override order
 */


typedef enum {
	VERTEX_X1 = 0, VERTEX_Y1, VERTEX_X4, VERTEX_Y4, VERTEX_X2, VERTEX_Y2, VERTEX_X3, VERTEX_Y3
} VertexIndex;

/*
 * Error String Setting
 */

#ifdef _NDEBUG
#define _SkeletonJson_setError(s,r,v1,v2) ((void)0)
#define _AttachmentLoader_setError(s,e1,e2) ((void)0)
#define _AttachmentLoader_setUnknownTypeError(s,t) ((void)0)
#else
extern void _AttachmentLoader_setError_(AttachmentLoader* self, const char* error1, const char* error2);
extern void _AttachmentLoader_setUnknownTypeError_(AttachmentLoader* self, AttachmentType type);
#define _SkeletonJson_setError(s,r,v1,v2) _SkeletonJson_setError_(s,r,v1,v2)
#define _AttachmentLoader_setError(s,e1,e2) _AttachmentLoader_setError_(s,e1,e2)
#define _AttachmentLoader_setUnknownTypeError(s,t) _AttachmentLoader_setUnknownTypeError_(s,t)
#endif

/*
 * Internal API available for extension:
 */

void _AttachmentLoader_init (AttachmentLoader* self, /**/
		void (*dispose) (AttachmentLoader* self), /**/
		Attachment* (*newAttachment) (AttachmentLoader* self, Skin* skin, AttachmentType type, const char* name));
void _AttachmentLoader_deinit (AttachmentLoader* self);

/**/

void _Attachment_init (Attachment* self, const char* name, AttachmentType type, /**/
		void (*dispose) (Attachment* self));
void _Attachment_deinit (Attachment* self);

/**/

void _Timeline_init (Timeline* self, /**/
		void (*dispose) (Timeline* self), /**/
		void (*apply) (const Timeline* self, Skeleton* skeleton, float time, float alpha));
void _Timeline_deinit (Timeline* self);

/**/

void _CurveTimeline_init (CurveTimeline* self, int frameCount, /**/
		void (*dispose) (Timeline* self), /**/
		void (*apply) (const Timeline* self, Skeleton* skeleton, float time, float alpha));
void _CurveTimeline_deinit (CurveTimeline* self);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_EXTENSION_H_ */

//
//MERGED FILE END: extension.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: Json.h
//

/* Esoteric Software: Removed everything except parsing, shorter method names, more get methods, double to float, formatted. */

#ifndef SPINE_JSON_H_
#define SPINE_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Json Types: */
#define Json_False 0
#define Json_True 1
#define Json_NULL 2
#define Json_Number 3
#define Json_String 4
#define Json_Array 5
#define Json_Object 6

/* The Json structure: */
typedef struct Json {
	struct Json* next;
	struct Json* prev; /* next/prev allow you to walk array/object chains. Alternatively, use getSize/getItemAt/getItem */
	struct Json* child; /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type; /* The type of the item, as above. */

	const char* valuestring; /* The item's string, if type==Json_String */
	int valueint; /* The item's number, if type==Json_Number */
	float valuefloat; /* The item's number, if type==Json_Number */

	const char* name; /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} Json;

/* Supply a block of JSON, and this returns a Json object you can interrogate. Call Json_dispose when finished. */
Json* Json_create (const char* value);

/* Delete a Json entity and all subentities. */
void Json_dispose (Json* json);

/* Returns the number of items in an array (or object). */
int Json_getSize (Json* json);

/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
Json* Json_getItemAt (Json* json, int item);

/* Get item "string" from object. Case insensitive. */
Json* Json_getItem (Json* json, const char* string);
const char* Json_getString (Json* json, const char* name, const char* defaultValue);
float Json_getFloat (Json* json, const char* name, float defaultValue);
int Json_getInt (Json* json, const char* name, int defaultValue);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when Json_create() returns 0. 0 when Json_create() succeeds. */
const char* Json_getError (void);

#ifdef __cplusplus
}
#endif

#endif /* SPINE_JSON_H_ */

//
//MERGED FILE END: Json.h
//------------------------------------------------------------------------------------------------------------------------------------------------------
