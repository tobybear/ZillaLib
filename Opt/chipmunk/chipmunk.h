//------------------------------------------------------------------------------
/* Copyright (c) 2013 Scott Lembcke and Howling Moon Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//------------------------------------------------------------------------------
/* Copyright cpDecompose (c) 2007 Eric Jordan
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * ---
 *
 * Adapted and converted to C++ by Bernhard Schelling based on the decompose code by Eric Jordan
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: chipmunk_private.h
//

#ifndef CHIPMUNK_PRIVATE_H
#define CHIPMUNK_PRIVATE_H

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: chipmunk.h
//

#ifndef CHIPMUNK_H
#define CHIPMUNK_H

#ifdef _MSC_VER
	#define _USE_MATH_DEFINES
#endif

#include <stdlib.h>
#include <math.h>
#include <ZL_Math.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
	// For alloca().
	#include <malloc.h>
#else
	#include <alloca.h>
#endif
#define CP_EXPORT

#ifdef __cplusplus
extern "C" {
#endif

CP_EXPORT void cpMessage(const char *condition, const char *file, int line, int isError, int isHardError, const char *message, ...);
#ifdef NDEBUG
	#define	cpAssertWarn(__condition__, m)
	#define	cpAssertWarn2(__condition__, m, n)
	#define	cpAssertSoft(__condition__, m)
	#define	cpAssertSoft2(__condition__, m, n)
	#define	cpAssertSoft3(__condition__, m, n, o)
#else
	#define cpAssertSoft(__condition__, m) if(!(__condition__)){cpMessage(#__condition__, __FILE__, __LINE__, 1, 0, m), abort();}
	#define cpAssertSoft2(__condition__, m, n) if(!(__condition__)){cpMessage(#__condition__, __FILE__, __LINE__, 1, 0, m, n), abort();}
	#define cpAssertSoft3(__condition__, m, n, o) if(!(__condition__)){cpMessage(#__condition__, __FILE__, __LINE__, 1, 0, m, n, o), abort();}
	#define cpAssertWarn(__condition__, m) if(!(__condition__)) cpMessage(#__condition__, __FILE__, __LINE__, 0, 0, m)
	#define cpAssertWarn2(__condition__, m, n) if(!(__condition__)) cpMessage(#__condition__, __FILE__, __LINE__, 0, 0, m, n)
#endif

// Hard assertions are used in situations where the program definitely will crash anyway, and the reason is inexpensive to detect.
#define cpAssertHard(__condition__, m) if(!(__condition__)){cpMessage(#__condition__, __FILE__, __LINE__, 1, 1, m); abort();}

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: chipmunk_types.h
//

#ifndef CHIPMUNK_TYPES_H
#define CHIPMUNK_TYPES_H

#define CP_BOOL_TYPE bool
#define cpTrue true
#define cpFalse false
#define CP_COLLISION_TYPE_TYPE unsigned short
#define CP_GROUP_TYPE unsigned short
#define CP_LAYERS_TYPE unsigned short
#define uintptr_t size_t
#define uint32_t unsigned int
#ifdef ZL_DOUBLE_PRECISCION
#define CP_USE_DOUBLES 1
#endif

/*
#include <stdint.h>
#include <float.h>
#include <math.h>

#ifdef __APPLE__
   #include "TargetConditionals.h"
#endif

// Use CGTypes by default on iOS and Mac.
// Also enables usage of doubles on 64 bit.
// Performance is usually very comparable when the CPU cache is well utilised.
#if (TARGET_OS_IPHONE || TARGET_OS_MAC) && (!defined CP_USE_CGTYPES)
	#define CP_USE_CGTYPES 1
#endif

#if CP_USE_CGTYPES
	#if TARGET_OS_IPHONE
		#import <CoreGraphics/CGGeometry.h>
		#import <CoreGraphics/CGAffineTransform.h>
	#elif TARGET_OS_MAC
		#include <ApplicationServices/ApplicationServices.h>
	#endif

	#if defined(__LP64__) && __LP64__
		#define CP_USE_DOUBLES 1
	#else
		#define CP_USE_DOUBLES 0
	#endif
#endif

#ifndef CP_USE_DOUBLES
	// Use doubles by default for higher precision.
	#define CP_USE_DOUBLES 1
#endif
*/

/// @defgroup basicTypes Basic Types
/// Most of these types can be configured at compile time.
/// @{

#if CP_USE_DOUBLES
/// Chipmunk's floating point type.
/// Can be reconfigured at compile time.
	typedef double cpFloat;
	#define cpfsqrt sqrt
	#define cpfsin sin
	#define cpfcos cos
	#define cpfacos acos
	#define cpfatan2 atan2
	#define cpfmod fmod
	#define cpfexp exp
	#define cpfpow pow
	#define cpffloor floor
	#define cpfceil ceil
	#define CPFLOAT_MIN DBL_MIN
#else
	typedef float cpFloat;
	#define cpfsqrt sqrtf
	#define cpfsin sinf
	#define cpfcos cosf
	#define cpfacos acosf
	#define cpfatan2 atan2f
	#define cpfmod fmodf
	#define cpfexp expf
	#define cpfpow powf
	#define cpffloor floorf
	#define cpfceil ceilf
	#define CPFLOAT_MIN FLT_MIN
#endif

#ifndef INFINITY
	#ifdef _MSC_VER
		union MSVC_EVIL_FLOAT_HACK
		{
			unsigned __int8 Bytes[4];
			float Value;
		};
		static union MSVC_EVIL_FLOAT_HACK INFINITY_HACK = {{0x00, 0x00, 0x80, 0x7F}};
		#define INFINITY ((cpFloat)(INFINITY_HACK.Value))
	#endif

	#ifdef __GNUC__
		#define INFINITY ((cpFloat)(__builtin_inf()))
	#endif

	#ifndef INFINITY
		#define INFINITY ((cpFloat)(1e1000))
	#endif
#elif defined(_MSC_VER)
#pragma warning(disable:4056) //overflow in floating-point constant arithmetic (by design of INIFINITY macro)
#pragma warning(disable:4756) //overflow in constant arithmetic (by design of INIFINITY macro)
#endif

#define CP_PI ((cpFloat)3.14159265358979323846264338327950288)

/// Return the max of two cpFloats.
static inline cpFloat cpfmax(cpFloat a, cpFloat b)
{
	return (a > b) ? a : b;
}

/// Return the min of two cpFloats.
static inline cpFloat cpfmin(cpFloat a, cpFloat b)
{
	return (a < b) ? a : b;
}

/// Return the absolute value of a cpFloat.
static inline cpFloat cpfabs(cpFloat f)
{
	return (f < 0) ? -f : f;
}

/// Clamp @c f to be between @c min and @c max.
static inline cpFloat cpfclamp(cpFloat f, cpFloat min, cpFloat max)
{
	return cpfmin(cpfmax(f, min), max);
}

/// Clamp @c f to be between 0 and 1.
static inline cpFloat cpfclamp01(cpFloat f)
{
	return cpfmax(0.0f, cpfmin(f, 1.0f));
}

/// Linearly interpolate (or extrapolate) between @c f1 and @c f2 by @c t percent.
static inline cpFloat cpflerp(cpFloat f1, cpFloat f2, cpFloat t)
{
	return f1*(1.0f - t) + f2*t;
}

/// Linearly interpolate from @c f1 to @c f2 by no more than @c d.
static inline cpFloat cpflerpconst(cpFloat f1, cpFloat f2, cpFloat d)
{
	return f1 + cpfclamp(f2 - f1, -d, d);
}

/// Hash value type.
#ifdef CP_HASH_VALUE_TYPE
	typedef CP_HASH_VALUE_TYPE cpHashValue;
#else
	typedef uintptr_t cpHashValue;
#endif

/// Type used internally to cache colliding object info for cpCollideShapes().
/// Should be at least 32 bits.
typedef uint32_t cpCollisionID;

// Oh C, how we love to define our own boolean types to get compiler compatibility
/// Chipmunk's boolean type.
#ifdef CP_BOOL_TYPE
	typedef CP_BOOL_TYPE cpBool;
#else
	typedef unsigned char cpBool;
#endif

#ifndef cpTrue
/// true value.
	#define cpTrue 1
#endif

#ifndef cpFalse
/// false value.
	#define cpFalse 0
#endif

#ifdef CP_DATA_POINTER_TYPE
	typedef CP_DATA_POINTER_TYPE cpDataPointer;
#else
/// Type used for user data pointers.
	typedef void * cpDataPointer;
#endif

#ifdef CP_COLLISION_TYPE_TYPE
	typedef CP_COLLISION_TYPE_TYPE cpCollisionType;
#else
/// Type used for cpSpace.collision_type.
	typedef uintptr_t cpCollisionType;
#endif

#ifdef CP_GROUP_TYPE
	typedef CP_GROUP_TYPE cpGroup;
#else
/// Type used for cpShape.group.
	typedef uintptr_t cpGroup;
#endif

#ifdef CP_BITMASK_TYPE
	typedef CP_BITMASK_TYPE cpBitmask;
#else
/// Type used for cpShapeFilter category and mask.
	typedef unsigned int cpBitmask;
#endif

#ifdef CP_TIMESTAMP_TYPE
	typedef CP_TIMESTAMP_TYPE cpTimestamp;
#else
/// Type used for various timestamps in Chipmunk.
	typedef unsigned int cpTimestamp;
#endif

#ifndef CP_NO_GROUP
/// Value for cpShape.group signifying that a shape is in no group.
	#define CP_NO_GROUP ((cpGroup)0)
#endif

#ifndef CP_ALL_CATEGORIES
/// Value for cpShape.layers signifying that a shape is in every layer.
	#define CP_ALL_CATEGORIES ((cpBitmask)~(cpBitmask)0)
#endif

#ifndef CP_WILDCARD_COLLISION_TYPE
/// cpCollisionType value internally reserved for hashing wildcard handlers.
	#define CP_WILDCARD_COLLISION_TYPE ((cpCollisionType)~(cpCollisionType)0)
#endif

/// @}

// CGPoints are structurally the same, and allow
// easy interoperability with other Cocoa libraries
#if CP_USE_CGTYPES
	typedef CGPoint cpVect;
#elif defined(__ZL_MATH__)
	struct cpVect{cpFloat x,y;
	inline cpVect& operator=(const ZL_Vector& v){x=v.x,y=v.y;return*this;}
	inline operator ZL_Vector*(){return(ZL_Vector*)this;}
	inline operator ZL_Vector&(){return*(ZL_Vector*)this;}};
	#define ZLV2CPV(v) (*(const cpVect*)&(const ZL_Vector&)(v))
#else
/// Chipmunk's 2D vector type.
/// @addtogroup cpVect
	typedef struct cpVect{cpFloat x,y;} cpVect;
#endif

#if CP_USE_CGTYPES
	typedef CGAffineTransform cpTransform;
#else
	/// Column major affine transform.
	typedef struct cpTransform {
		cpFloat a, b, c, d, tx, ty;
	} cpTransform;
#endif

// NUKE
typedef struct cpMat2x2 {
	// Row major [[a, b][c d]]
	cpFloat a, b, c, d;
} cpMat2x2;

#endif

//
//MERGED FILE END: chipmunk_types.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

/// @defgroup misc Misc
/// @{

/// Allocated size for various Chipmunk buffers
#ifndef CP_BUFFER_BYTES
	#define CP_BUFFER_BYTES (32*1024)
#endif

#ifndef cpcalloc
	/// Chipmunk calloc() alias.
	#define cpcalloc calloc
#endif

#ifndef cprealloc
	/// Chipmunk realloc() alias.
	#define cprealloc realloc
#endif

#ifndef cpfree
	/// Chipmunk free() alias.
	#define cpfree free
#endif

typedef struct cpArray cpArray;
typedef struct cpHashSet cpHashSet;

typedef struct cpBody cpBody;

typedef struct cpShape cpShape;
typedef struct cpCircleShape cpCircleShape;
typedef struct cpSegmentShape cpSegmentShape;
typedef struct cpPolyShape cpPolyShape;

typedef struct cpConstraint cpConstraint;
typedef struct cpPinJoint cpPinJoint;
typedef struct cpSlideJoint cpSlideJoint;
typedef struct cpPivotJoint cpPivotJoint;
typedef struct cpGrooveJoint cpGrooveJoint;
typedef struct cpDampedSpring cpDampedSpring;
typedef struct cpDampedRotarySpring cpDampedRotarySpring;
typedef struct cpRotaryLimitJoint cpRotaryLimitJoint;
typedef struct cpRatchetJoint cpRatchetJoint;
typedef struct cpGearJoint cpGearJoint;
typedef struct cpSimpleMotorJoint cpSimpleMotorJoint;

typedef struct cpCollisionHandler cpCollisionHandler;
typedef struct cpContactPointSet cpContactPointSet;
typedef struct cpArbiter cpArbiter;

typedef struct cpSpace cpSpace;

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpVect.h
//

#ifndef CHIPMUNK_VECT_H
#define CHIPMUNK_VECT_H

/// @defgroup cpVect cpVect
/// Chipmunk's 2D vector type along with a handy 2D vector math lib.
/// @{

/// Constant for the zero vector.
static const cpVect cpvzero = {0.0f,0.0f};

/// Convenience constructor for cpVect structs.
static inline cpVect cpv(const cpFloat x, const cpFloat y)
{
	cpVect v = {x, y};
	return v;
}

/// Check if two vectors are equal. (Be careful when comparing floating point numbers!)
static inline cpBool cpveql(const cpVect v1, const cpVect v2)
{
	return (v1.x == v2.x && v1.y == v2.y);
}

/// Add two vectors
static inline cpVect cpvadd(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x + v2.x, v1.y + v2.y);
}

/// Subtract two vectors.
static inline cpVect cpvsub(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x - v2.x, v1.y - v2.y);
}

/// Negate a vector.
static inline cpVect cpvneg(const cpVect v)
{
	return cpv(-v.x, -v.y);
}

/// Scalar multiplication.
static inline cpVect cpvmult(const cpVect v, const cpFloat s)
{
	return cpv(v.x*s, v.y*s);
}

/// Vector dot product.
static inline cpFloat cpvdot(const cpVect v1, const cpVect v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}

/// 2D vector cross product analog.
/// The cross product of 2D vectors results in a 3D vector with only a z component.
/// This function returns the magnitude of the z value.
static inline cpFloat cpvcross(const cpVect v1, const cpVect v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}

/// Returns a perpendicular vector. (90 degree rotation)
static inline cpVect cpvperp(const cpVect v)
{
	return cpv(-v.y, v.x);
}

/// Returns a perpendicular vector. (-90 degree rotation)
static inline cpVect cpvrperp(const cpVect v)
{
	return cpv(v.y, -v.x);
}

/// Returns the vector projection of v1 onto v2.
static inline cpVect cpvproject(const cpVect v1, const cpVect v2)
{
	return cpvmult(v2, cpvdot(v1, v2)/cpvdot(v2, v2));
}

/// Returns the unit length vector for the given angle (in radians).
static inline cpVect cpvforangle(const cpFloat a)
{
	return cpv(cpfcos(a), cpfsin(a));
}

/// Returns the angular direction v is pointing in (in radians).
static inline cpFloat cpvtoangle(const cpVect v)
{
	return cpfatan2(v.y, v.x);
}

/// Uses complex number multiplication to rotate v1 by v2. Scaling will occur if v1 is not a unit vector.
static inline cpVect cpvrotate(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x*v2.x - v1.y*v2.y, v1.x*v2.y + v1.y*v2.x);
}

/// Inverse of cpvrotate().
static inline cpVect cpvunrotate(const cpVect v1, const cpVect v2)
{
	return cpv(v1.x*v2.x + v1.y*v2.y, v1.y*v2.x - v1.x*v2.y);
}

/// Returns the squared length of v. Faster than cpvlength() when you only need to compare lengths.
static inline cpFloat cpvlengthsq(const cpVect v)
{
	return cpvdot(v, v);
}

/// Returns the length of v.
static inline cpFloat cpvlength(const cpVect v)
{
	return cpfsqrt(cpvdot(v, v));
}

/// Linearly interpolate between v1 and v2.
static inline cpVect cpvlerp(const cpVect v1, const cpVect v2, const cpFloat t)
{
	return cpvadd(cpvmult(v1, 1.0f - t), cpvmult(v2, t));
}

/// Returns a normalized copy of v.
static inline cpVect cpvnormalize(const cpVect v)
{
	// Neat trick I saw somewhere to avoid div/0.
	return cpvmult(v, 1.0f/(cpvlength(v) + CPFLOAT_MIN));
}

/// Spherical linearly interpolate between v1 and v2.
static inline cpVect
cpvslerp(const cpVect v1, const cpVect v2, const cpFloat t)
{
	cpFloat dot = cpvdot(cpvnormalize(v1), cpvnormalize(v2));
	cpFloat omega = cpfacos(cpfclamp(dot, -1.0f, 1.0f));

	if(omega < 1e-3){
		// If the angle between two vectors is very small, lerp instead to avoid precision issues.
		return cpvlerp(v1, v2, t);
	} else {
		cpFloat denom = 1.0f/cpfsin(omega);
		return cpvadd(cpvmult(v1, cpfsin((1.0f - t)*omega)*denom), cpvmult(v2, cpfsin(t*omega)*denom));
	}
}

/// Spherical linearly interpolate between v1 towards v2 by no more than angle a radians
static inline cpVect
cpvslerpconst(const cpVect v1, const cpVect v2, const cpFloat a)
{
	cpFloat dot = cpvdot(cpvnormalize(v1), cpvnormalize(v2));
	cpFloat omega = cpfacos(cpfclamp(dot, -1.0f, 1.0f));

	return cpvslerp(v1, v2, cpfmin(a, omega)/omega);
}

/// Clamp v to length len.
static inline cpVect cpvclamp(const cpVect v, const cpFloat len)
{
	return (cpvdot(v,v) > len*len) ? cpvmult(cpvnormalize(v), len) : v;
}

/// Linearly interpolate between v1 towards v2 by distance d.
static inline cpVect cpvlerpconst(cpVect v1, cpVect v2, cpFloat d)
{
	return cpvadd(v1, cpvclamp(cpvsub(v2, v1), d));
}

/// Returns the distance between v1 and v2.
static inline cpFloat cpvdist(const cpVect v1, const cpVect v2)
{
	return cpvlength(cpvsub(v1, v2));
}

/// Returns the squared distance between v1 and v2. Faster than cpvdist() when you only need to compare distances.
static inline cpFloat cpvdistsq(const cpVect v1, const cpVect v2)
{
	return cpvlengthsq(cpvsub(v1, v2));
}

/// Returns true if the distance between v1 and v2 is less than dist.
static inline cpBool cpvnear(const cpVect v1, const cpVect v2, const cpFloat dist)
{
	return cpvdistsq(v1, v2) < dist*dist;
}

/// @}

/// @defgroup cpMat2x2 cpMat2x2
/// 2x2 matrix type used for tensors and such.
/// @{

// NUKE
static inline cpMat2x2
cpMat2x2New(cpFloat a, cpFloat b, cpFloat c, cpFloat d)
{
	cpMat2x2 m = {a, b, c, d};
	return m;
}

static inline cpVect
cpMat2x2Transform(cpMat2x2 m, cpVect v)
{
	return cpv(v.x*m.a + v.y*m.b, v.x*m.c + v.y*m.d);
}

///@}

#endif

//
//MERGED FILE END: cpVect.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpBB.h
//

#ifndef CHIPMUNK_BB_H
#define CHIPMUNK_BB_H

/// @defgroup cpBBB cpBB
/// Chipmunk's axis-aligned 2D bounding box type along with a few handy routines.
/// @{

/// Chipmunk's axis-aligned 2D bounding box type. (left, bottom, right, top)
typedef struct cpBB{
	cpFloat l, b, r ,t;
} cpBB;

/// Convenience constructor for cpBB structs.
static inline cpBB cpBBNew(const cpFloat l, const cpFloat b, const cpFloat r, const cpFloat t)
{
	cpBB bb = {l, b, r, t};
	return bb;
}

/// Constructs a cpBB centered on a point with the given extents (half sizes).
static inline cpBB
cpBBNewForExtents(const cpVect c, const cpFloat hw, const cpFloat hh)
{
	return cpBBNew(c.x - hw, c.y - hh, c.x + hw, c.y + hh);
}

/// Constructs a cpBB for a circle with the given position and radius.
static inline cpBB cpBBNewForCircle(const cpVect p, const cpFloat r)
{
	return cpBBNewForExtents(p, r, r);
}

/// Returns true if @c a and @c b intersect.
static inline cpBool cpBBIntersects(const cpBB a, const cpBB b)
{
	return (a.l <= b.r && b.l <= a.r && a.b <= b.t && b.b <= a.t);
}

/// Returns true if @c other lies completely within @c bb.
static inline cpBool cpBBContainsBB(const cpBB bb, const cpBB other)
{
	return (bb.l <= other.l && bb.r >= other.r && bb.b <= other.b && bb.t >= other.t);
}

/// Returns true if @c bb contains @c v.
static inline cpBool cpBBContainsVect(const cpBB bb, const cpVect v)
{
	return (bb.l <= v.x && bb.r >= v.x && bb.b <= v.y && bb.t >= v.y);
}

/// Returns a bounding box that holds both bounding boxes.
static inline cpBB cpBBMerge(const cpBB a, const cpBB b){
	return cpBBNew(
		cpfmin(a.l, b.l),
		cpfmin(a.b, b.b),
		cpfmax(a.r, b.r),
		cpfmax(a.t, b.t)
	);
}

/// Returns a bounding box that holds both @c bb and @c v.
static inline cpBB cpBBExpand(const cpBB bb, const cpVect v){
	return cpBBNew(
		cpfmin(bb.l, v.x),
		cpfmin(bb.b, v.y),
		cpfmax(bb.r, v.x),
		cpfmax(bb.t, v.y)
	);
}

/// Returns the center of a bounding box.
static inline cpVect
cpBBCenter(cpBB bb)
{
	return cpvlerp(cpv(bb.l, bb.b), cpv(bb.r, bb.t), 0.5f);
}

/// Returns the area of the bounding box.
static inline cpFloat cpBBArea(cpBB bb)
{
	return (bb.r - bb.l)*(bb.t - bb.b);
}

/// Merges @c a and @c b and returns the area of the merged bounding box.
static inline cpFloat cpBBMergedArea(cpBB a, cpBB b)
{
	return (cpfmax(a.r, b.r) - cpfmin(a.l, b.l))*(cpfmax(a.t, b.t) - cpfmin(a.b, b.b));
}

/// Returns the fraction along the segment query the cpBB is hit. Returns INFINITY if it doesn't hit.
static inline cpFloat cpBBSegmentQuery(cpBB bb, cpVect a, cpVect b)
{
	cpFloat idx = 1.0f/(b.x - a.x);
	cpFloat tx1 = (bb.l == a.x ? -INFINITY : (bb.l - a.x)*idx);
	cpFloat tx2 = (bb.r == a.x ?  INFINITY : (bb.r - a.x)*idx);
	cpFloat txmin = cpfmin(tx1, tx2);
	cpFloat txmax = cpfmax(tx1, tx2);

	cpFloat idy = 1.0f/(b.y - a.y);
	cpFloat ty1 = (bb.b == a.y ? -INFINITY : (bb.b - a.y)*idy);
	cpFloat ty2 = (bb.t == a.y ?  INFINITY : (bb.t - a.y)*idy);
	cpFloat tymin = cpfmin(ty1, ty2);
	cpFloat tymax = cpfmax(ty1, ty2);

	if(tymin <= txmax && txmin <= tymax){
		cpFloat min = cpfmax(txmin, tymin);
		cpFloat max = cpfmin(txmax, tymax);

		if(0.0 <= max && min <= 1.0) return cpfmax(min, 0.0);
	}

	return INFINITY;
}

/// Return true if the bounding box intersects the line segment with ends @c a and @c b.
static inline cpBool cpBBIntersectsSegment(cpBB bb, cpVect a, cpVect b)
{
	return (cpBBSegmentQuery(bb, a, b) != INFINITY);
}

/// Clamp a vector to a bounding box.
static inline cpVect
cpBBClampVect(const cpBB bb, const cpVect v)
{
	return cpv(cpfclamp(v.x, bb.l, bb.r), cpfclamp(v.y, bb.b, bb.t));
}

/// Wrap a vector to a bounding box.
static inline cpVect
cpBBWrapVect(const cpBB bb, const cpVect v)
{
	cpFloat dx = cpfabs(bb.r - bb.l);
	cpFloat modx = cpfmod(v.x - bb.l, dx);
	cpFloat x = (modx > 0.0f) ? modx : modx + dx;

	cpFloat dy = cpfabs(bb.t - bb.b);
	cpFloat mody = cpfmod(v.y - bb.b, dy);
	cpFloat y = (mody > 0.0f) ? mody : mody + dy;

	return cpv(x + bb.l, y + bb.b);
}

/// Returns a bounding box offseted by @c v.
static inline cpBB
cpBBOffset(const cpBB bb, const cpVect v)
{
	return cpBBNew(
		bb.l + v.x,
		bb.b + v.y,
		bb.r + v.x,
		bb.t + v.y
	);
}

///@}

#endif

//
//MERGED FILE END: cpBB.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpTransform.h
//

#ifndef CHIPMUNK_TRANSFORM_H
#define CHIPMUNK_TRANSFORM_H

/// Identity transform matrix.
static const cpTransform cpTransformIdentity = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};

/// Construct a new transform matrix.
/// (a, b) is the x basis vector.
/// (c, d) is the y basis vector.
/// (tx, ty) is the translation.
static inline cpTransform
cpTransformNew(cpFloat a, cpFloat b, cpFloat c, cpFloat d, cpFloat tx, cpFloat ty)
{
	cpTransform t = {a, b, c, d, tx, ty};
	return t;
}

/// Construct a new transform matrix in transposed order.
static inline cpTransform
cpTransformNewTranspose(cpFloat a, cpFloat c, cpFloat tx, cpFloat b, cpFloat d, cpFloat ty)
{
	cpTransform t = {a, b, c, d, tx, ty};
	return t;
}

/// Get the inverse of a transform matrix.
static inline cpTransform
cpTransformInverse(cpTransform t)
{
  cpFloat inv_det = (cpFloat)1.0/(t.a*t.d - t.c*t.b);
  return cpTransformNewTranspose(
     t.d*inv_det, -t.c*inv_det, (t.c*t.ty - t.tx*t.d)*inv_det,
    -t.b*inv_det,  t.a*inv_det, (t.tx*t.b - t.a*t.ty)*inv_det
  );
}

/// Multiply two transformation matrices.
static inline cpTransform
cpTransformMult(cpTransform t1, cpTransform t2)
{
  return cpTransformNewTranspose(
    t1.a*t2.a + t1.c*t2.b, t1.a*t2.c + t1.c*t2.d, t1.a*t2.tx + t1.c*t2.ty + t1.tx,
    t1.b*t2.a + t1.d*t2.b, t1.b*t2.c + t1.d*t2.d, t1.b*t2.tx + t1.d*t2.ty + t1.ty
  );
}

/// Transform an absolute point. (i.e. a vertex)
static inline cpVect
cpTransformPoint(cpTransform t, cpVect p)
{
  return cpv(t.a*p.x + t.c*p.y + t.tx, t.b*p.x + t.d*p.y + t.ty);
}

/// Transform a vector (i.e. a normal)
static inline cpVect
cpTransformVect(cpTransform t, cpVect v)
{
  return cpv(t.a*v.x + t.c*v.y, t.b*v.x + t.d*v.y);
}

/// Transform a cpBB.
static inline cpBB
cpTransformbBB(cpTransform t, cpBB bb)
{
	cpVect center = cpBBCenter(bb);
	cpFloat hw = (bb.r - bb.l)*(cpFloat)0.5;
	cpFloat hh = (bb.t - bb.b)*(cpFloat)0.5;

	cpFloat a = t.a*hw, b = t.c*hh, d = t.b*hw, e = t.d*hh;
	cpFloat hw_max = cpfmax(cpfabs(a + b), cpfabs(a - b));
	cpFloat hh_max = cpfmax(cpfabs(d + e), cpfabs(d - e));
	return cpBBNewForExtents(cpTransformPoint(t, center), hw_max, hh_max);
}

/// Create a transation matrix.
static inline cpTransform
cpTransformTranslate(cpVect translate)
{
  return cpTransformNewTranspose(
    1.0, 0.0, translate.x,
    0.0, 1.0, translate.y
  );
}

/// Create a scale matrix.
static inline cpTransform
cpTransformScale(cpFloat scaleX, cpFloat scaleY)
{
	return cpTransformNewTranspose(
		scaleX,    0.0, 0.0,
		   0.0, scaleY, 0.0
	);
}

/// Create a rotation matrix.
static inline cpTransform
cpTransformRotate(cpFloat radians)
{
	cpVect rot = cpvforangle(radians);
	return cpTransformNewTranspose(
		rot.x, -rot.y, 0.0,
		rot.y,  rot.x, 0.0
	);
}

/// Create a rigid transformation matrix. (transation + rotation)
static inline cpTransform
cpTransformRigid(cpVect translate, cpFloat radians)
{
	cpVect rot = cpvforangle(radians);
	return cpTransformNewTranspose(
		rot.x, -rot.y, translate.x,
		rot.y,  rot.x, translate.y
	);
}

/// Fast inverse of a rigid transformation matrix.
static inline cpTransform
cpTransformRigidInverse(cpTransform t)
{
  return cpTransformNewTranspose(
     t.d, -t.c, (t.c*t.ty - t.tx*t.d),
    -t.b,  t.a, (t.tx*t.b - t.a*t.ty)
  );
}

//MARK: Miscellaneous (but useful) transformation matrices.
// See source for documentation...

static inline cpTransform
cpTransformWrap(cpTransform outer, cpTransform inner)
{
  return cpTransformMult(cpTransformInverse(outer), cpTransformMult(inner, outer));
}

static inline cpTransform
cpTransformWrapInverse(cpTransform outer, cpTransform inner)
{
  return cpTransformMult(outer, cpTransformMult(inner, cpTransformInverse(outer)));
}

static inline cpTransform
cpTransformOrtho(cpBB bb)
{
  return cpTransformNewTranspose(
    (cpFloat)2.0/(bb.r - bb.l), (cpFloat)0.0, -(bb.r + bb.l)/(bb.r - bb.l),
    (cpFloat)0.0, (cpFloat)2.0/(bb.t - bb.b), -(bb.t + bb.b)/(bb.t - bb.b)
  );
}

static inline cpTransform
cpTransformBoneScale(cpVect v0, cpVect v1)
{
  cpVect d = cpvsub(v1, v0);
  return cpTransformNewTranspose(
    d.x, -d.y, v0.x,
    d.y,  d.x, v0.y
  );
}

static inline cpTransform
cpTransformAxialScale(cpVect axis, cpVect pivot, cpFloat scale)
{
  cpFloat A = axis.x*axis.y*(scale - (cpFloat)1.0);
  cpFloat B = cpvdot(axis, pivot)*((cpFloat)1.0 - scale);

  return cpTransformNewTranspose(
    scale*axis.x*axis.x + axis.y*axis.y, A, axis.x*B,
    A, axis.x*axis.x + scale*axis.y*axis.y, axis.y*B
  );
}

#endif

//
//MERGED FILE END: cpTransform.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSpatialIndex.h
//

/**
	@defgroup cpSpatialIndex cpSpatialIndex

	Spatial indexes are data structures that are used to accelerate collision detection
	and spatial queries. Chipmunk provides a number of spatial index algorithms to pick from
	and they are programmed in a generic way so that you can use them for holding more than
	just cpShape structs.

	It works by using @c void pointers to the objects you add and using a callback to ask your code
	for bounding boxes when it needs them. Several types of queries can be performed an index as well
	as reindexing and full collision information. All communication to the spatial indexes is performed
	through callback functions.

	Spatial indexes should be treated as opaque structs.
	This meanns you shouldn't be reading any of the struct fields.
	@{
*/

//MARK: Spatial Index

/// Spatial index bounding box callback function type.
/// The spatial index calls this function and passes you a pointer to an object you added
/// when it needs to get the bounding box associated with that object.
typedef cpBB (*cpSpatialIndexBBFunc)(void *obj);
/// Spatial index/object iterator callback function type.
typedef void (*cpSpatialIndexIteratorFunc)(void *obj, void *data);
/// Spatial query callback function type.
typedef cpCollisionID (*cpSpatialIndexQueryFunc)(void *obj1, void *obj2, cpCollisionID id, void *data);
/// Spatial segment query callback function type.
typedef cpFloat (*cpSpatialIndexSegmentQueryFunc)(void *obj1, void *obj2, void *data);

typedef struct cpSpatialIndexClass cpSpatialIndexClass;
typedef struct cpSpatialIndex cpSpatialIndex;

/// @private
struct cpSpatialIndex {
	cpSpatialIndexClass *klass;

	cpSpatialIndexBBFunc bbfunc;

	cpSpatialIndex *staticIndex, *dynamicIndex;
};

//MARK: Spatial Hash

typedef struct cpSpaceHash cpSpaceHash;

/// Allocate a spatial hash.
CP_EXPORT cpSpaceHash* cpSpaceHashAlloc(void);
/// Initialize a spatial hash.
CP_EXPORT cpSpatialIndex* cpSpaceHashInit(cpSpaceHash *hash, cpFloat celldim, int numcells, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);
/// Allocate and initialize a spatial hash.
CP_EXPORT cpSpatialIndex* cpSpaceHashNew(cpFloat celldim, int cells, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);

/// Change the cell dimensions and table size of the spatial hash to tune it.
/// The cell dimensions should roughly match the average size of your objects
/// and the table size should be ~10 larger than the number of objects inserted.
/// Some trial and error is required to find the optimum numbers for efficiency.
CP_EXPORT void cpSpaceHashResize(cpSpaceHash *hash, cpFloat celldim, int numcells);

//MARK: AABB Tree

typedef struct cpBBTree cpBBTree;

/// Allocate a bounding box tree.
CP_EXPORT cpBBTree* cpBBTreeAlloc(void);
/// Initialize a bounding box tree.
CP_EXPORT cpSpatialIndex* cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);
/// Allocate and initialize a bounding box tree.
CP_EXPORT cpSpatialIndex* cpBBTreeNew(cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);

/// Perform a static top down optimization of the tree.
CP_EXPORT void cpBBTreeOptimize(cpSpatialIndex *index);

/// Bounding box tree velocity callback function.
/// This function should return an estimate for the object's velocity.
typedef cpVect (*cpBBTreeVelocityFunc)(void *obj);
/// Set the velocity function for the bounding box tree to enable temporal coherence.
CP_EXPORT void cpBBTreeSetVelocityFunc(cpSpatialIndex *index, cpBBTreeVelocityFunc func);

//MARK: Single Axis Sweep

typedef struct cpSweep1D cpSweep1D;

/// Allocate a 1D sort and sweep broadphase.
CP_EXPORT cpSweep1D* cpSweep1DAlloc(void);
/// Initialize a 1D sort and sweep broadphase.
CP_EXPORT cpSpatialIndex* cpSweep1DInit(cpSweep1D *sweep, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);
/// Allocate and initialize a 1D sort and sweep broadphase.
CP_EXPORT cpSpatialIndex* cpSweep1DNew(cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);

//MARK: Spatial Index Implementation

typedef void (*cpSpatialIndexDestroyImpl)(cpSpatialIndex *index);

typedef int (*cpSpatialIndexCountImpl)(cpSpatialIndex *index);
typedef void (*cpSpatialIndexEachImpl)(cpSpatialIndex *index, cpSpatialIndexIteratorFunc func, void *data);

typedef cpBool (*cpSpatialIndexContainsImpl)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexInsertImpl)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexRemoveImpl)(cpSpatialIndex *index, void *obj, cpHashValue hashid);

typedef void (*cpSpatialIndexReindexImpl)(cpSpatialIndex *index);
typedef void (*cpSpatialIndexReindexObjectImpl)(cpSpatialIndex *index, void *obj, cpHashValue hashid);
typedef void (*cpSpatialIndexReindexQueryImpl)(cpSpatialIndex *index, cpSpatialIndexQueryFunc func, void *data);

typedef void (*cpSpatialIndexQueryImpl)(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data);
typedef void (*cpSpatialIndexSegmentQueryImpl)(cpSpatialIndex *index, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data);

struct cpSpatialIndexClass {
	cpSpatialIndexDestroyImpl destroy;

	cpSpatialIndexCountImpl count;
	cpSpatialIndexEachImpl each;

	cpSpatialIndexContainsImpl contains;
	cpSpatialIndexInsertImpl insert;
	cpSpatialIndexRemoveImpl remove;

	cpSpatialIndexReindexImpl reindex;
	cpSpatialIndexReindexObjectImpl reindexObject;
	cpSpatialIndexReindexQueryImpl reindexQuery;

	cpSpatialIndexQueryImpl query;
	cpSpatialIndexSegmentQueryImpl segmentQuery;
};

/// Destroy and free a spatial index.
CP_EXPORT void cpSpatialIndexFree(cpSpatialIndex *index);
/// Collide the objects in @c dynamicIndex against the objects in @c staticIndex using the query callback function.
CP_EXPORT void cpSpatialIndexCollideStatic(cpSpatialIndex *dynamicIndex, cpSpatialIndex *staticIndex, cpSpatialIndexQueryFunc func, void *data);

/// Destroy a spatial index.
static inline void cpSpatialIndexDestroy(cpSpatialIndex *index)
{
	if(index->klass) index->klass->destroy(index);
}

/// Get the number of objects in the spatial index.
static inline int cpSpatialIndexCount(cpSpatialIndex *index)
{
	return index->klass->count(index);
}

/// Iterate the objects in the spatial index. @c func will be called once for each object.
static inline void cpSpatialIndexEach(cpSpatialIndex *index, cpSpatialIndexIteratorFunc func, void *data)
{
	index->klass->each(index, func, data);
}

/// Returns true if the spatial index contains the given object.
/// Most spatial indexes use hashed storage, so you must provide a hash value too.
static inline cpBool cpSpatialIndexContains(cpSpatialIndex *index, void *obj, cpHashValue hashid)
{
	return index->klass->contains(index, obj, hashid);
}

/// Add an object to a spatial index.
/// Most spatial indexes use hashed storage, so you must provide a hash value too.
static inline void cpSpatialIndexInsert(cpSpatialIndex *index, void *obj, cpHashValue hashid)
{
	index->klass->insert(index, obj, hashid);
}

/// Remove an object from a spatial index.
/// Most spatial indexes use hashed storage, so you must provide a hash value too.
static inline void cpSpatialIndexRemove(cpSpatialIndex *index, void *obj, cpHashValue hashid)
{
	index->klass->remove(index, obj, hashid);
}

/// Perform a full reindex of a spatial index.
static inline void cpSpatialIndexReindex(cpSpatialIndex *index)
{
	index->klass->reindex(index);
}

/// Reindex a single object in the spatial index.
static inline void cpSpatialIndexReindexObject(cpSpatialIndex *index, void *obj, cpHashValue hashid)
{
	index->klass->reindexObject(index, obj, hashid);
}

/// Perform a rectangle query against the spatial index, calling @c func for each potential match.
static inline void cpSpatialIndexQuery(cpSpatialIndex *index, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data)
{
	index->klass->query(index, obj, bb, func, data);
}

/// Perform a segment query against the spatial index, calling @c func for each potential match.
static inline void cpSpatialIndexSegmentQuery(cpSpatialIndex *index, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data)
{
	index->klass->segmentQuery(index, obj, a, b, t_exit, func, data);
}

/// Simultaneously reindex and find all colliding objects.
/// @c func will be called once for each potentially overlapping pair of objects found.
/// If the spatial index was initialized with a static index, it will collide it's objects against that as well.
static inline void cpSpatialIndexReindexQuery(cpSpatialIndex *index, cpSpatialIndexQueryFunc func, void *data)
{
	index->klass->reindexQuery(index, func, data);
}

///@}

//
//MERGED FILE END: cpSpatialIndex.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpArbiter.h
//

/// @defgroup cpArbiter cpArbiter
/// The cpArbiter struct tracks pairs of colliding shapes.
/// They are also used in conjuction with collision handler callbacks
/// allowing you to retrieve information on the collision or change it.
/// A unique arbiter value is used for each pair of colliding objects. It persists until the shapes separate.
/// @{

#define CP_MAX_CONTACTS_PER_ARBITER 2

/// Get the restitution (elasticity) that will be applied to the pair of colliding objects.
CP_EXPORT cpFloat cpArbiterGetRestitution(const cpArbiter *arb);
/// Override the restitution (elasticity) that will be applied to the pair of colliding objects.
CP_EXPORT void cpArbiterSetRestitution(cpArbiter *arb, cpFloat restitution);
/// Get the friction coefficient that will be applied to the pair of colliding objects.
CP_EXPORT cpFloat cpArbiterGetFriction(const cpArbiter *arb);
/// Override the friction coefficient that will be applied to the pair of colliding objects.
CP_EXPORT void cpArbiterSetFriction(cpArbiter *arb, cpFloat friction);

// Get the relative surface velocity of the two shapes in contact.
CP_EXPORT cpVect cpArbiterGetSurfaceVelocity(cpArbiter *arb);

// Override the relative surface velocity of the two shapes in contact.
// By default this is calculated to be the difference of the two surface velocities clamped to the tangent plane.
CP_EXPORT void cpArbiterSetSurfaceVelocity(cpArbiter *arb, cpVect vr);

/// Get the user data pointer associated with this pair of colliding objects.
CP_EXPORT cpDataPointer cpArbiterGetUserData(const cpArbiter *arb);
/// Set a user data point associated with this pair of colliding objects.
/// If you need to perform any cleanup for this pointer, you must do it yourself, in the separate callback for instance.
CP_EXPORT void cpArbiterSetUserData(cpArbiter *arb, cpDataPointer userData);

/// Calculate the total impulse including the friction that was applied by this arbiter.
/// This function should only be called from a post-solve, post-step or cpBodyEachArbiter callback.
CP_EXPORT cpVect cpArbiterTotalImpulse(const cpArbiter *arb);
/// Calculate the amount of energy lost in a collision including static, but not dynamic friction.
/// This function should only be called from a post-solve, post-step or cpBodyEachArbiter callback.
CP_EXPORT cpFloat cpArbiterTotalKE(const cpArbiter *arb);

/// Mark a collision pair to be ignored until the two objects separate.
/// Pre-solve and post-solve callbacks will not be called, but the separate callback will be called.
CP_EXPORT cpBool cpArbiterIgnore(cpArbiter *arb);

/// Return the colliding shapes involved for this arbiter.
/// The order of their cpSpace.collision_type values will match
/// the order set when the collision handler was registered.
CP_EXPORT void cpArbiterGetShapes(const cpArbiter *arb, cpShape **a, cpShape **b);

/// A macro shortcut for defining and retrieving the shapes from an arbiter.
#define CP_ARBITER_GET_SHAPES(__arb__, __a__, __b__) cpShape *__a__, *__b__; cpArbiterGetShapes(__arb__, &__a__, &__b__);

/// Return the colliding bodies involved for this arbiter.
/// The order of the cpSpace.collision_type the bodies are associated with values will match
/// the order set when the collision handler was registered.
CP_EXPORT void cpArbiterGetBodies(const cpArbiter *arb, cpBody **a, cpBody **b);

/// A macro shortcut for defining and retrieving the bodies from an arbiter.
#define CP_ARBITER_GET_BODIES(__arb__, __a__, __b__) cpBody *__a__, *__b__; cpArbiterGetBodies(__arb__, &__a__, &__b__);

/// A struct that wraps up the important collision data for an arbiter.
struct cpContactPointSet {
	/// The number of contact points in the set.
	int count;

	/// The normal of the collision.
	cpVect normal;

	/// The array of contact points.
	struct {
		/// The position of the contact on the surface of each shape.
		cpVect pointA, pointB;
		/// Penetration distance of the two shapes. Overlapping means it will be negative.
		/// This value is calculated as cpvdot(cpvsub(point2, point1), normal) and is ignored by cpArbiterSetContactPointSet().
		cpFloat distance;
	} points[CP_MAX_CONTACTS_PER_ARBITER];
};

/// Return a contact set from an arbiter.
CP_EXPORT cpContactPointSet cpArbiterGetContactPointSet(const cpArbiter *arb);

/// Replace the contact point set for an arbiter.
/// This can be a very powerful feature, but use it with caution!
CP_EXPORT void cpArbiterSetContactPointSet(cpArbiter *arb, cpContactPointSet *set);

/// Returns true if this is the first step a pair of objects started colliding.
CP_EXPORT cpBool cpArbiterIsFirstContact(const cpArbiter *arb);
/// Returns true if the separate callback is due to a shape being removed from the space.
CP_EXPORT cpBool cpArbiterIsRemoval(const cpArbiter *arb);

/// Get the number of contact points for this arbiter.
CP_EXPORT int cpArbiterGetCount(const cpArbiter *arb);
/// Get the normal of the collision.
CP_EXPORT cpVect cpArbiterGetNormal(const cpArbiter *arb);
/// Get the position of the @c ith contact point on the surface of the first shape.
CP_EXPORT cpVect cpArbiterGetPointA(const cpArbiter *arb, int i);
/// Get the position of the @c ith contact point on the surface of the second shape.
CP_EXPORT cpVect cpArbiterGetPointB(const cpArbiter *arb, int i);
/// Get the depth of the @c ith contact point.
CP_EXPORT cpFloat cpArbiterGetDepth(const cpArbiter *arb, int i);

/// If you want a custom callback to invoke the wildcard callback for the first collision type, you must call this function explicitly.
/// You must decide how to handle the wildcard's return value since it may disagree with the other wildcard handler's return value or your own.
CP_EXPORT cpBool cpArbiterCallWildcardBeginA(cpArbiter *arb, cpSpace *space);
/// If you want a custom callback to invoke the wildcard callback for the second collision type, you must call this function explicitly.
/// You must decide how to handle the wildcard's return value since it may disagree with the other wildcard handler's return value or your own.
CP_EXPORT cpBool cpArbiterCallWildcardBeginB(cpArbiter *arb, cpSpace *space);

/// If you want a custom callback to invoke the wildcard callback for the first collision type, you must call this function explicitly.
/// You must decide how to handle the wildcard's return value since it may disagree with the other wildcard handler's return value or your own.
CP_EXPORT cpBool cpArbiterCallWildcardPreSolveA(cpArbiter *arb, cpSpace *space);
/// If you want a custom callback to invoke the wildcard callback for the second collision type, you must call this function explicitly.
/// You must decide how to handle the wildcard's return value since it may disagree with the other wildcard handler's return value or your own.
CP_EXPORT cpBool cpArbiterCallWildcardPreSolveB(cpArbiter *arb, cpSpace *space);

/// If you want a custom callback to invoke the wildcard callback for the first collision type, you must call this function explicitly.
CP_EXPORT void cpArbiterCallWildcardPostSolveA(cpArbiter *arb, cpSpace *space);
/// If you want a custom callback to invoke the wildcard callback for the second collision type, you must call this function explicitly.
CP_EXPORT void cpArbiterCallWildcardPostSolveB(cpArbiter *arb, cpSpace *space);

/// If you want a custom callback to invoke the wildcard callback for the first collision type, you must call this function explicitly.
CP_EXPORT void cpArbiterCallWildcardSeparateA(cpArbiter *arb, cpSpace *space);
/// If you want a custom callback to invoke the wildcard callback for the second collision type, you must call this function explicitly.
CP_EXPORT void cpArbiterCallWildcardSeparateB(cpArbiter *arb, cpSpace *space);

/// @}

//
//MERGED FILE END: cpArbiter.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpBody.h
//

/// @defgroup cpBody cpBody
/// Chipmunk's rigid body type. Rigid bodies hold the physical properties of an object like
/// it's mass, and position and velocity of it's center of gravity. They don't have an shape on their own.
/// They are given a shape by creating collision shapes (cpShape) that point to the body.
/// @{

typedef enum cpBodyType {
	/// A dynamic body is one that is affected by gravity, forces, and collisions.
	/// This is the default body type.
	CP_BODY_TYPE_DYNAMIC,
	/// A kinematic body is an infinite mass, user controlled body that is not affected by gravity, forces or collisions.
	/// Instead the body only moves based on it's velocity.
	/// Dynamic bodies collide normally with kinematic bodies, though the kinematic body will be unaffected.
	/// Collisions between two kinematic bodies, or a kinematic body and a static body produce collision callbacks, but no collision response.
	CP_BODY_TYPE_KINEMATIC,
	/// A static body is a body that never (or rarely) moves. If you move a static body, you must call one of the cpSpaceReindex*() functions.
	/// Chipmunk uses this information to optimize the collision detection.
	/// Static bodies do not produce collision callbacks when colliding with other static bodies.
	CP_BODY_TYPE_STATIC,
} cpBodyType;

/// Rigid body velocity update function type.
typedef void (*cpBodyVelocityFunc)(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);
/// Rigid body position update function type.
typedef void (*cpBodyPositionFunc)(cpBody *body, cpFloat dt);

/// Allocate a cpBody.
CP_EXPORT cpBody* cpBodyAlloc(void);
/// Initialize a cpBody.
CP_EXPORT cpBody* cpBodyInit(cpBody *body, cpFloat mass, cpFloat moment);
/// Allocate and initialize a cpBody.
CP_EXPORT cpBody* cpBodyNew(cpFloat mass, cpFloat moment);

/// Allocate and initialize a cpBody, and set it as a kinematic body.
CP_EXPORT cpBody* cpBodyNewKinematic(void);
/// Allocate and initialize a cpBody, and set it as a static body.
CP_EXPORT cpBody* cpBodyNewStatic(void);

/// Destroy a cpBody.
CP_EXPORT void cpBodyDestroy(cpBody *body);
/// Destroy and free a cpBody.
CP_EXPORT void cpBodyFree(cpBody *body);

// Defined in cpSpace.c
/// Wake up a sleeping or idle body.
CP_EXPORT void cpBodyActivate(cpBody *body);
/// Wake up any sleeping or idle bodies touching a static body.
CP_EXPORT void cpBodyActivateStatic(cpBody *body, cpShape *filter);

/// Force a body to fall asleep immediately.
CP_EXPORT void cpBodySleep(cpBody *body);
/// Force a body to fall asleep immediately along with other bodies in a group.
CP_EXPORT void cpBodySleepWithGroup(cpBody *body, cpBody *group);

/// Returns true if the body is sleeping.
CP_EXPORT cpBool cpBodyIsSleeping(const cpBody *body);

/// Get the type of the body.
CP_EXPORT cpBodyType cpBodyGetType(cpBody *body);
/// Set the type of the body.
CP_EXPORT void cpBodySetType(cpBody *body, cpBodyType type);

/// Get the space this body is added to.
CP_EXPORT cpSpace* cpBodyGetSpace(const cpBody *body);

/// Get the mass of the body.
CP_EXPORT cpFloat cpBodyGetMass(const cpBody *body);
/// Set the mass of the body.
CP_EXPORT void cpBodySetMass(cpBody *body, cpFloat m);

/// Get the moment of inertia of the body.
CP_EXPORT cpFloat cpBodyGetMoment(const cpBody *body);
/// Set the moment of inertia of the body.
CP_EXPORT void cpBodySetMoment(cpBody *body, cpFloat i);

/// Set the position of a body.
CP_EXPORT cpVect cpBodyGetPosition(const cpBody *body);
/// Set the position of the body.
CP_EXPORT void cpBodySetPosition(cpBody *body, cpVect pos);

/// Get the offset of the center of gravity in body local coordinates.
CP_EXPORT cpVect cpBodyGetCenterOfGravity(const cpBody *body);
/// Set the offset of the center of gravity in body local coordinates.
CP_EXPORT void cpBodySetCenterOfGravity(cpBody *body, cpVect cog);

/// Get the velocity of the body.
CP_EXPORT cpVect cpBodyGetVelocity(const cpBody *body);
/// Set the velocity of the body.
CP_EXPORT void cpBodySetVelocity(cpBody *body, cpVect velocity);

/// Get the force applied to the body for the next time step.
CP_EXPORT cpVect cpBodyGetForce(const cpBody *body);
/// Set the force applied to the body for the next time step.
CP_EXPORT void cpBodySetForce(cpBody *body, cpVect force);

/// Get the angle of the body.
CP_EXPORT cpFloat cpBodyGetAngle(const cpBody *body);
/// Set the angle of a body.
CP_EXPORT void cpBodySetAngle(cpBody *body, cpFloat a);

/// Get the angular velocity of the body.
CP_EXPORT cpFloat cpBodyGetAngularVelocity(const cpBody *body);
/// Set the angular velocity of the body.
CP_EXPORT void cpBodySetAngularVelocity(cpBody *body, cpFloat angularVelocity);

/// Get the torque applied to the body for the next time step.
CP_EXPORT cpFloat cpBodyGetTorque(const cpBody *body);
/// Set the torque applied to the body for the next time step.
CP_EXPORT void cpBodySetTorque(cpBody *body, cpFloat torque);

/// Get the rotation vector of the body. (The x basis vector of it's transform.)
CP_EXPORT cpVect cpBodyGetRotation(const cpBody *body);

/// Get the user data pointer assigned to the body.
CP_EXPORT cpDataPointer cpBodyGetUserData(const cpBody *body);
/// Set the user data pointer assigned to the body.
CP_EXPORT void cpBodySetUserData(cpBody *body, cpDataPointer userData);

/// Set the callback used to update a body's velocity.
CP_EXPORT void cpBodySetVelocityUpdateFunc(cpBody *body, cpBodyVelocityFunc velocityFunc);
/// Set the callback used to update a body's position.
/// NOTE: It's not generally recommended to override this unless you call the default position update function.
CP_EXPORT void cpBodySetPositionUpdateFunc(cpBody *body, cpBodyPositionFunc positionFunc);

/// Default velocity integration function..
CP_EXPORT void cpBodyUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);
/// Default position integration function.
CP_EXPORT void cpBodyUpdatePosition(cpBody *body, cpFloat dt);

/// Convert body relative/local coordinates to absolute/world coordinates.
CP_EXPORT cpVect cpBodyLocalToWorld(const cpBody *body, const cpVect point);
/// Convert body absolute/world coordinates to  relative/local coordinates.
CP_EXPORT cpVect cpBodyWorldToLocal(const cpBody *body, const cpVect point);

/// Apply a force to a body. Both the force and point are expressed in world coordinates.
CP_EXPORT void cpBodyApplyForceAtWorldPoint(cpBody *body, cpVect force, cpVect point);
/// Apply a force to a body. Both the force and point are expressed in body local coordinates.
CP_EXPORT void cpBodyApplyForceAtLocalPoint(cpBody *body, cpVect force, cpVect point);

/// Apply an impulse to a body. Both the impulse and point are expressed in world coordinates.
CP_EXPORT void cpBodyApplyImpulseAtWorldPoint(cpBody *body, cpVect impulse, cpVect point);
/// Apply an impulse to a body. Both the impulse and point are expressed in body local coordinates.
CP_EXPORT void cpBodyApplyImpulseAtLocalPoint(cpBody *body, cpVect impulse, cpVect point);

/// Get the velocity on a body (in world units) at a point on the body in world coordinates.
CP_EXPORT cpVect cpBodyGetVelocityAtWorldPoint(const cpBody *body, cpVect point);
/// Get the velocity on a body (in world units) at a point on the body in local coordinates.
CP_EXPORT cpVect cpBodyGetVelocityAtLocalPoint(const cpBody *body, cpVect point);

/// Get the amount of kinetic energy contained by the body.
CP_EXPORT cpFloat cpBodyKineticEnergy(const cpBody *body);

/// Body/shape iterator callback function type.
typedef void (*cpBodyShapeIteratorFunc)(cpBody *body, cpShape *shape, void *data);
/// Call @c func once for each shape attached to @c body and added to the space.
CP_EXPORT void cpBodyEachShape(cpBody *body, cpBodyShapeIteratorFunc func, void *data);

/// Body/constraint iterator callback function type.
typedef void (*cpBodyConstraintIteratorFunc)(cpBody *body, cpConstraint *constraint, void *data);
/// Call @c func once for each constraint attached to @c body and added to the space.
CP_EXPORT void cpBodyEachConstraint(cpBody *body, cpBodyConstraintIteratorFunc func, void *data);

/// Body/arbiter iterator callback function type.
typedef void (*cpBodyArbiterIteratorFunc)(cpBody *body, cpArbiter *arbiter, void *data);
/// Call @c func once for each arbiter that is currently active on the body.
CP_EXPORT void cpBodyEachArbiter(cpBody *body, cpBodyArbiterIteratorFunc func, void *data);

///@}

//
//MERGED FILE END: cpBody.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpShape.h
//

/// @defgroup cpShape cpShape
/// The cpShape struct defines the shape of a rigid body.
/// @{

/// Point query info struct.
typedef struct cpPointQueryInfo {
	/// The nearest shape, NULL if no shape was within range.
	const cpShape *shape;
	/// The closest point on the shape's surface. (in world space coordinates)
	cpVect point;
	/// The distance to the point. The distance is negative if the point is inside the shape.
	cpFloat distance;
	/// The gradient of the signed distance function.
	/// The value should be similar to info.p/info.d, but accurate even for very small values of info.d.
	cpVect gradient;
} cpPointQueryInfo;

/// Segment query info struct.
typedef struct cpSegmentQueryInfo {
	/// The shape that was hit, or NULL if no collision occured.
	const cpShape *shape;
	/// The point of impact.
	cpVect point;
	/// The normal of the surface hit.
	cpVect normal;
	/// The normalized distance along the query segment in the range [0, 1].
	cpFloat alpha;
} cpSegmentQueryInfo;

/// Fast collision filtering type that is used to determine if two objects collide before calling collision or query callbacks.
typedef struct cpShapeFilter {
	/// Two objects with the same non-zero group value do not collide.
	/// This is generally used to group objects in a composite object together to disable self collisions.
	cpGroup group;
	/// A bitmask of user definable categories that this object belongs to.
	/// The category/mask combinations of both objects in a collision must agree for a collision to occur.
	cpBitmask categories;
	/// A bitmask of user definable category types that this object object collides with.
	/// The category/mask combinations of both objects in a collision must agree for a collision to occur.
	cpBitmask mask;
} cpShapeFilter;

/// Collision filter value for a shape that will collide with anything except CP_SHAPE_FILTER_NONE.
static const cpShapeFilter CP_SHAPE_FILTER_ALL = {CP_NO_GROUP, CP_ALL_CATEGORIES, CP_ALL_CATEGORIES};
/// Collision filter value for a shape that does not collide with anything.
static const cpShapeFilter CP_SHAPE_FILTER_NONE = {CP_NO_GROUP, ~CP_ALL_CATEGORIES, ~CP_ALL_CATEGORIES};

/// Create a new collision filter.
static inline cpShapeFilter
cpShapeFilterNew(cpGroup group, cpBitmask categories, cpBitmask mask)
{
	cpShapeFilter filter = {group, categories, mask};
	return filter;
}

/// Destroy a shape.
CP_EXPORT void cpShapeDestroy(cpShape *shape);
/// Destroy and Free a shape.
CP_EXPORT void cpShapeFree(cpShape *shape);

/// Update, cache and return the bounding box of a shape based on the body it's attached to.
CP_EXPORT cpBB cpShapeCacheBB(cpShape *shape);
/// Update, cache and return the bounding box of a shape with an explicit transformation.
CP_EXPORT cpBB cpShapeUpdate(cpShape *shape, cpTransform transform);

/// Perform a nearest point query. It finds the closest point on the surface of shape to a specific point.
/// The value returned is the distance between the points. A negative distance means the point is inside the shape.
CP_EXPORT cpFloat cpShapePointQuery(const cpShape *shape, cpVect p, cpPointQueryInfo *out);

/// Perform a segment query against a shape. @c info must be a pointer to a valid cpSegmentQueryInfo structure.
CP_EXPORT cpBool cpShapeSegmentQuery(const cpShape *shape, cpVect a, cpVect b, cpFloat radius, cpSegmentQueryInfo *info);

/// Return contact information about two shapes.
CP_EXPORT cpContactPointSet cpShapesCollide(const cpShape *a, const cpShape *b);

/// The cpSpace this body is added to.
CP_EXPORT cpSpace* cpShapeGetSpace(const cpShape *shape);

/// The cpBody this shape is connected to.
CP_EXPORT cpBody* cpShapeGetBody(const cpShape *shape);
/// Set the cpBody this shape is connected to.
/// Can only be used if the shape is not currently added to a space.
CP_EXPORT void cpShapeSetBody(cpShape *shape, cpBody *body);

/// Get the mass of the shape if you are having Chipmunk calculate mass properties for you.
CP_EXPORT cpFloat cpShapeGetMass(cpShape *shape);
/// Set the mass of this shape to have Chipmunk calculate mass properties for you.
CP_EXPORT void cpShapeSetMass(cpShape *shape, cpFloat mass);

/// Get the density of the shape if you are having Chipmunk calculate mass properties for you.
CP_EXPORT cpFloat cpShapeGetDensity(cpShape *shape);
/// Set the density  of this shape to have Chipmunk calculate mass properties for you.
CP_EXPORT void cpShapeSetDensity(cpShape *shape, cpFloat density);

/// Get the calculated moment of inertia for this shape.
CP_EXPORT cpFloat cpShapeGetMoment(cpShape *shape);
/// Get the calculated area of this shape.
CP_EXPORT cpFloat cpShapeGetArea(cpShape *shape);
/// Get the centroid of this shape.
CP_EXPORT cpVect cpShapeGetCenterOfGravity(cpShape *shape);

/// Get the bounding box that contains the shape given it's current position and angle.
CP_EXPORT cpBB cpShapeGetBB(const cpShape *shape);

/// Get if the shape is set to be a sensor or not.
CP_EXPORT cpBool cpShapeGetSensor(const cpShape *shape);
/// Set if the shape is a sensor or not.
CP_EXPORT void cpShapeSetSensor(cpShape *shape, cpBool sensor);

/// Get the elasticity of this shape.
CP_EXPORT cpFloat cpShapeGetElasticity(const cpShape *shape);
/// Set the elasticity of this shape.
CP_EXPORT void cpShapeSetElasticity(cpShape *shape, cpFloat elasticity);

/// Get the friction of this shape.
CP_EXPORT cpFloat cpShapeGetFriction(const cpShape *shape);
/// Set the friction of this shape.
CP_EXPORT void cpShapeSetFriction(cpShape *shape, cpFloat friction);

/// Get the surface velocity of this shape.
CP_EXPORT cpVect cpShapeGetSurfaceVelocity(const cpShape *shape);
/// Set the surface velocity of this shape.
CP_EXPORT void cpShapeSetSurfaceVelocity(cpShape *shape, cpVect surfaceVelocity);

/// Get the user definable data pointer of this shape.
CP_EXPORT cpDataPointer cpShapeGetUserData(const cpShape *shape);
/// Set the user definable data pointer of this shape.
CP_EXPORT void cpShapeSetUserData(cpShape *shape, cpDataPointer userData);

/// Set the collision type of this shape.
CP_EXPORT cpCollisionType cpShapeGetCollisionType(const cpShape *shape);
/// Get the collision type of this shape.
CP_EXPORT void cpShapeSetCollisionType(cpShape *shape, cpCollisionType collisionType);

/// Get the collision filtering parameters of this shape.
CP_EXPORT cpShapeFilter cpShapeGetFilter(const cpShape *shape);
/// Set the collision filtering parameters of this shape.
CP_EXPORT void cpShapeSetFilter(cpShape *shape, cpShapeFilter filter);

/// @}
/// @defgroup cpCircleShape cpCircleShape

/// Allocate a circle shape.
CP_EXPORT cpCircleShape* cpCircleShapeAlloc(void);
/// Initialize a circle shape.
CP_EXPORT cpCircleShape* cpCircleShapeInit(cpCircleShape *circle, cpBody *body, cpFloat radius, cpVect offset);
/// Allocate and initialize a circle shape.
CP_EXPORT cpShape* cpCircleShapeNew(cpBody *body, cpFloat radius, cpVect offset);

/// Get the offset of a circle shape.
CP_EXPORT cpVect cpCircleShapeGetOffset(const cpShape *shape);
/// Get the radius of a circle shape.
CP_EXPORT cpFloat cpCircleShapeGetRadius(const cpShape *shape);

/// @}
/// @defgroup cpSegmentShape cpSegmentShape

/// Allocate a segment shape.
CP_EXPORT cpSegmentShape* cpSegmentShapeAlloc(void);
/// Initialize a segment shape.
CP_EXPORT cpSegmentShape* cpSegmentShapeInit(cpSegmentShape *seg, cpBody *body, cpVect a, cpVect b, cpFloat radius);
/// Allocate and initialize a segment shape.
CP_EXPORT cpShape* cpSegmentShapeNew(cpBody *body, cpVect a, cpVect b, cpFloat radius);

/// Let Chipmunk know about the geometry of adjacent segments to avoid colliding with endcaps.
CP_EXPORT void cpSegmentShapeSetNeighbors(cpShape *shape, cpVect prev, cpVect next);

/// Get the first endpoint of a segment shape.
CP_EXPORT cpVect cpSegmentShapeGetA(const cpShape *shape);
/// Get the second endpoint of a segment shape.
CP_EXPORT cpVect cpSegmentShapeGetB(const cpShape *shape);
/// Get the normal of a segment shape.
CP_EXPORT cpVect cpSegmentShapeGetNormal(const cpShape *shape);
/// Get the first endpoint of a segment shape.
CP_EXPORT cpFloat cpSegmentShapeGetRadius(const cpShape *shape);

/// @}

//
//MERGED FILE END: cpShape.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpPolyShape.h
//

/// @defgroup cpPolyShape cpPolyShape
/// @{

/// Allocate a polygon shape.
CP_EXPORT cpPolyShape* cpPolyShapeAlloc(void);
/// Initialize a polygon shape with rounded corners.
/// A convex hull will be created from the vertexes.
CP_EXPORT cpPolyShape* cpPolyShapeInit(cpPolyShape *poly, cpBody *body, int count, const cpVect *verts, cpTransform transform, cpFloat radius);
/// Initialize a polygon shape with rounded corners.
/// The vertexes must be convex with a counter-clockwise winding.
CP_EXPORT cpPolyShape* cpPolyShapeInitRaw(cpPolyShape *poly, cpBody *body, int count, const cpVect *verts, cpFloat radius);
/// Allocate and initialize a polygon shape with rounded corners.
/// A convex hull will be created from the vertexes.
CP_EXPORT cpShape* cpPolyShapeNew(cpBody *body, int count, const cpVect *verts, cpTransform transform, cpFloat radius);
/// Allocate and initialize a polygon shape with rounded corners.
/// The vertexes must be convex with a counter-clockwise winding.
CP_EXPORT cpShape* cpPolyShapeNewRaw(cpBody *body, int count, const cpVect *verts, cpFloat radius);

/// Initialize a box shaped polygon shape with rounded corners.
CP_EXPORT cpPolyShape* cpBoxShapeInit(cpPolyShape *poly, cpBody *body, cpFloat width, cpFloat height, cpFloat radius);
/// Initialize an offset box shaped polygon shape with rounded corners.
CP_EXPORT cpPolyShape* cpBoxShapeInit2(cpPolyShape *poly, cpBody *body, cpBB box, cpFloat radius);
/// Allocate and initialize a box shaped polygon shape.
CP_EXPORT cpShape* cpBoxShapeNew(cpBody *body, cpFloat width, cpFloat height, cpFloat radius);
/// Allocate and initialize an offset box shaped polygon shape.
CP_EXPORT cpShape* cpBoxShapeNew2(cpBody *body, cpBB box, cpFloat radius);

/// Get the number of verts in a polygon shape.
CP_EXPORT int cpPolyShapeGetCount(const cpShape *shape);
/// Get the @c ith vertex of a polygon shape.
CP_EXPORT cpVect cpPolyShapeGetVert(const cpShape *shape, int index);
/// Get the radius of a polygon shape.
CP_EXPORT cpFloat cpPolyShapeGetRadius(const cpShape *shape);

/// @}

//
//MERGED FILE END: cpPolyShape.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpConstraint.h
//

/// @defgroup cpConstraint cpConstraint
/// @{

/// Callback function type that gets called before solving a joint.
typedef void (*cpConstraintPreSolveFunc)(cpConstraint *constraint, cpSpace *space);
/// Callback function type that gets called after solving a joint.
typedef void (*cpConstraintPostSolveFunc)(cpConstraint *constraint, cpSpace *space);

/// Destroy a constraint.
CP_EXPORT void cpConstraintDestroy(cpConstraint *constraint);
/// Destroy and free a constraint.
CP_EXPORT void cpConstraintFree(cpConstraint *constraint);

/// Get the cpSpace this constraint is added to.
CP_EXPORT cpSpace* cpConstraintGetSpace(const cpConstraint *constraint);

/// Get the first body the constraint is attached to.
CP_EXPORT cpBody* cpConstraintGetBodyA(const cpConstraint *constraint);

/// Get the second body the constraint is attached to.
CP_EXPORT cpBody* cpConstraintGetBodyB(const cpConstraint *constraint);

/// Get the maximum force that this constraint is allowed to use.
CP_EXPORT cpFloat cpConstraintGetMaxForce(const cpConstraint *constraint);
/// Set the maximum force that this constraint is allowed to use. (defaults to INFINITY)
CP_EXPORT void cpConstraintSetMaxForce(cpConstraint *constraint, cpFloat maxForce);

/// Get rate at which joint error is corrected.
CP_EXPORT cpFloat cpConstraintGetErrorBias(const cpConstraint *constraint);
/// Set rate at which joint error is corrected.
/// Defaults to pow(1.0 - 0.1, 60.0) meaning that it will
/// correct 10% of the error every 1/60th of a second.
CP_EXPORT void cpConstraintSetErrorBias(cpConstraint *constraint, cpFloat errorBias);

/// Get the maximum rate at which joint error is corrected.
CP_EXPORT cpFloat cpConstraintGetMaxBias(const cpConstraint *constraint);
/// Set the maximum rate at which joint error is corrected. (defaults to INFINITY)
CP_EXPORT void cpConstraintSetMaxBias(cpConstraint *constraint, cpFloat maxBias);

/// Get if the two bodies connected by the constraint are allowed to collide or not.
CP_EXPORT cpBool cpConstraintGetCollideBodies(const cpConstraint *constraint);
/// Set if the two bodies connected by the constraint are allowed to collide or not. (defaults to cpFalse)
CP_EXPORT void cpConstraintSetCollideBodies(cpConstraint *constraint, cpBool collideBodies);

/// Get the pre-solve function that is called before the solver runs.
CP_EXPORT cpConstraintPreSolveFunc cpConstraintGetPreSolveFunc(const cpConstraint *constraint);
/// Set the pre-solve function that is called before the solver runs.
CP_EXPORT void cpConstraintSetPreSolveFunc(cpConstraint *constraint, cpConstraintPreSolveFunc preSolveFunc);

/// Get the post-solve function that is called before the solver runs.
CP_EXPORT cpConstraintPostSolveFunc cpConstraintGetPostSolveFunc(const cpConstraint *constraint);
/// Set the post-solve function that is called before the solver runs.
CP_EXPORT void cpConstraintSetPostSolveFunc(cpConstraint *constraint, cpConstraintPostSolveFunc postSolveFunc);

/// Get the user definable data pointer for this constraint
CP_EXPORT cpDataPointer cpConstraintGetUserData(const cpConstraint *constraint);
/// Set the user definable data pointer for this constraint
CP_EXPORT void cpConstraintSetUserData(cpConstraint *constraint, cpDataPointer userData);

/// Get the last impulse applied by this constraint.
CP_EXPORT cpFloat cpConstraintGetImpulse(cpConstraint *constraint);

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpPinJoint.h
//

/// @defgroup cpPinJoint cpPinJoint
/// @{

/// Check if a constraint is a pin joint.
CP_EXPORT cpBool cpConstraintIsPinJoint(const cpConstraint *constraint);

/// Allocate a pin joint.
CP_EXPORT cpPinJoint* cpPinJointAlloc(void);
/// Initialize a pin joint.
CP_EXPORT cpPinJoint* cpPinJointInit(cpPinJoint *joint, cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB);
/// Allocate and initialize a pin joint.
CP_EXPORT cpConstraint* cpPinJointNew(cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB);

/// Get the location of the first anchor relative to the first body.
CP_EXPORT cpVect cpPinJointGetAnchorA(const cpConstraint *constraint);
/// Set the location of the first anchor relative to the first body.
CP_EXPORT void cpPinJointSetAnchorA(cpConstraint *constraint, cpVect anchorA);

/// Get the location of the second anchor relative to the second body.
CP_EXPORT cpVect cpPinJointGetAnchorB(const cpConstraint *constraint);
/// Set the location of the second anchor relative to the second body.
CP_EXPORT void cpPinJointSetAnchorB(cpConstraint *constraint, cpVect anchorB);

/// Get the distance the joint will maintain between the two anchors.
CP_EXPORT cpFloat cpPinJointGetDist(const cpConstraint *constraint);
/// Set the distance the joint will maintain between the two anchors.
CP_EXPORT void cpPinJointSetDist(cpConstraint *constraint, cpFloat dist);

///@}

//
//MERGED FILE END: cpPinJoint.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSlideJoint.h
//

/// @defgroup cpSlideJoint cpSlideJoint
/// @{

/// Check if a constraint is a slide joint.
CP_EXPORT cpBool cpConstraintIsSlideJoint(const cpConstraint *constraint);

/// Allocate a slide joint.
CP_EXPORT cpSlideJoint* cpSlideJointAlloc(void);
/// Initialize a slide joint.
CP_EXPORT cpSlideJoint* cpSlideJointInit(cpSlideJoint *joint, cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB, cpFloat min, cpFloat max);
/// Allocate and initialize a slide joint.
CP_EXPORT cpConstraint* cpSlideJointNew(cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB, cpFloat min, cpFloat max);

/// Get the location of the first anchor relative to the first body.
CP_EXPORT cpVect cpSlideJointGetAnchorA(const cpConstraint *constraint);
/// Set the location of the first anchor relative to the first body.
CP_EXPORT void cpSlideJointSetAnchorA(cpConstraint *constraint, cpVect anchorA);

/// Get the location of the second anchor relative to the second body.
CP_EXPORT cpVect cpSlideJointGetAnchorB(const cpConstraint *constraint);
/// Set the location of the second anchor relative to the second body.
CP_EXPORT void cpSlideJointSetAnchorB(cpConstraint *constraint, cpVect anchorB);

/// Get the minimum distance the joint will maintain between the two anchors.
CP_EXPORT cpFloat cpSlideJointGetMin(const cpConstraint *constraint);
/// Set the minimum distance the joint will maintain between the two anchors.
CP_EXPORT void cpSlideJointSetMin(cpConstraint *constraint, cpFloat min);

/// Get the maximum distance the joint will maintain between the two anchors.
CP_EXPORT cpFloat cpSlideJointGetMax(const cpConstraint *constraint);
/// Set the maximum distance the joint will maintain between the two anchors.
CP_EXPORT void cpSlideJointSetMax(cpConstraint *constraint, cpFloat max);

/// @}

//
//MERGED FILE END: cpSlideJoint.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpPivotJoint.h
//

/// @defgroup cpPivotJoint cpPivotJoint
/// @{

/// Check if a constraint is a slide joint.
CP_EXPORT cpBool cpConstraintIsPivotJoint(const cpConstraint *constraint);

/// Allocate a pivot joint
CP_EXPORT cpPivotJoint* cpPivotJointAlloc(void);
/// Initialize a pivot joint.
CP_EXPORT cpPivotJoint* cpPivotJointInit(cpPivotJoint *joint, cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB);
/// Allocate and initialize a pivot joint.
CP_EXPORT cpConstraint* cpPivotJointNew(cpBody *a, cpBody *b, cpVect pivot);
/// Allocate and initialize a pivot joint with specific anchors.
CP_EXPORT cpConstraint* cpPivotJointNew2(cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB);

/// Get the location of the first anchor relative to the first body.
CP_EXPORT cpVect cpPivotJointGetAnchorA(const cpConstraint *constraint);
/// Set the location of the first anchor relative to the first body.
CP_EXPORT void cpPivotJointSetAnchorA(cpConstraint *constraint, cpVect anchorA);

/// Get the location of the second anchor relative to the second body.
CP_EXPORT cpVect cpPivotJointGetAnchorB(const cpConstraint *constraint);
/// Set the location of the second anchor relative to the second body.
CP_EXPORT void cpPivotJointSetAnchorB(cpConstraint *constraint, cpVect anchorB);

/// @}

//
//MERGED FILE END: cpPivotJoint.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpGrooveJoint.h
//

/// @defgroup cpGrooveJoint cpGrooveJoint
/// @{

/// Check if a constraint is a slide joint.
CP_EXPORT cpBool cpConstraintIsGrooveJoint(const cpConstraint *constraint);

/// Allocate a groove joint.
CP_EXPORT cpGrooveJoint* cpGrooveJointAlloc(void);
/// Initialize a groove joint.
CP_EXPORT cpGrooveJoint* cpGrooveJointInit(cpGrooveJoint *joint, cpBody *a, cpBody *b, cpVect groove_a, cpVect groove_b, cpVect anchorB);
/// Allocate and initialize a groove joint.
CP_EXPORT cpConstraint* cpGrooveJointNew(cpBody *a, cpBody *b, cpVect groove_a, cpVect groove_b, cpVect anchorB);

/// Get the first endpoint of the groove relative to the first body.
CP_EXPORT cpVect cpGrooveJointGetGrooveA(const cpConstraint *constraint);
/// Set the first endpoint of the groove relative to the first body.
CP_EXPORT void cpGrooveJointSetGrooveA(cpConstraint *constraint, cpVect grooveA);

/// Get the first endpoint of the groove relative to the first body.
CP_EXPORT cpVect cpGrooveJointGetGrooveB(const cpConstraint *constraint);
/// Set the first endpoint of the groove relative to the first body.
CP_EXPORT void cpGrooveJointSetGrooveB(cpConstraint *constraint, cpVect grooveB);

/// Get the location of the second anchor relative to the second body.
CP_EXPORT cpVect cpGrooveJointGetAnchorB(const cpConstraint *constraint);
/// Set the location of the second anchor relative to the second body.
CP_EXPORT void cpGrooveJointSetAnchorB(cpConstraint *constraint, cpVect anchorB);

/// @}

//
//MERGED FILE END: cpGrooveJoint.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpDampedSpring.h
//

/// @defgroup cpDampedSpring cpDampedSpring
/// @{

/// Check if a constraint is a slide joint.
CP_EXPORT cpBool cpConstraintIsDampedSpring(const cpConstraint *constraint);

/// Function type used for damped spring force callbacks.
typedef cpFloat (*cpDampedSpringForceFunc)(cpConstraint *spring, cpFloat dist);

/// Allocate a damped spring.
CP_EXPORT cpDampedSpring* cpDampedSpringAlloc(void);
/// Initialize a damped spring.
CP_EXPORT cpDampedSpring* cpDampedSpringInit(cpDampedSpring *joint, cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB, cpFloat restLength, cpFloat stiffness, cpFloat damping);
/// Allocate and initialize a damped spring.
CP_EXPORT cpConstraint* cpDampedSpringNew(cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB, cpFloat restLength, cpFloat stiffness, cpFloat damping);

/// Get the location of the first anchor relative to the first body.
CP_EXPORT cpVect cpDampedSpringGetAnchorA(const cpConstraint *constraint);
/// Set the location of the first anchor relative to the first body.
CP_EXPORT void cpDampedSpringSetAnchorA(cpConstraint *constraint, cpVect anchorA);

/// Get the location of the second anchor relative to the second body.
CP_EXPORT cpVect cpDampedSpringGetAnchorB(const cpConstraint *constraint);
/// Set the location of the second anchor relative to the second body.
CP_EXPORT void cpDampedSpringSetAnchorB(cpConstraint *constraint, cpVect anchorB);

/// Get the rest length of the spring.
CP_EXPORT cpFloat cpDampedSpringGetRestLength(const cpConstraint *constraint);
/// Set the rest length of the spring.
CP_EXPORT void cpDampedSpringSetRestLength(cpConstraint *constraint, cpFloat restLength);

/// Get the stiffness of the spring in force/distance.
CP_EXPORT cpFloat cpDampedSpringGetStiffness(const cpConstraint *constraint);
/// Set the stiffness of the spring in force/distance.
CP_EXPORT void cpDampedSpringSetStiffness(cpConstraint *constraint, cpFloat stiffness);

/// Get the damping of the spring.
CP_EXPORT cpFloat cpDampedSpringGetDamping(const cpConstraint *constraint);
/// Set the damping of the spring.
CP_EXPORT void cpDampedSpringSetDamping(cpConstraint *constraint, cpFloat damping);

/// Get the damping of the spring.
CP_EXPORT cpDampedSpringForceFunc cpDampedSpringGetSpringForceFunc(const cpConstraint *constraint);
/// Set the damping of the spring.
CP_EXPORT void cpDampedSpringSetSpringForceFunc(cpConstraint *constraint, cpDampedSpringForceFunc springForceFunc);

/// @}

//
//MERGED FILE END: cpDampedSpring.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpDampedRotarySpring.h
//

/// @defgroup cpDampedRotarySpring cpDampedRotarySpring
/// @{

/// Check if a constraint is a damped rotary springs.
CP_EXPORT cpBool cpConstraintIsDampedRotarySpring(const cpConstraint *constraint);

/// Function type used for damped rotary spring force callbacks.
typedef cpFloat (*cpDampedRotarySpringTorqueFunc)(struct cpConstraint *spring, cpFloat relativeAngle);

/// Allocate a damped rotary spring.
CP_EXPORT cpDampedRotarySpring* cpDampedRotarySpringAlloc(void);
/// Initialize a damped rotary spring.
CP_EXPORT cpDampedRotarySpring* cpDampedRotarySpringInit(cpDampedRotarySpring *joint, cpBody *a, cpBody *b, cpFloat restAngle, cpFloat stiffness, cpFloat damping);
/// Allocate and initialize a damped rotary spring.
CP_EXPORT cpConstraint* cpDampedRotarySpringNew(cpBody *a, cpBody *b, cpFloat restAngle, cpFloat stiffness, cpFloat damping);

/// Get the rest length of the spring.
CP_EXPORT cpFloat cpDampedRotarySpringGetRestAngle(const cpConstraint *constraint);
/// Set the rest length of the spring.
CP_EXPORT void cpDampedRotarySpringSetRestAngle(cpConstraint *constraint, cpFloat restAngle);

/// Get the stiffness of the spring in force/distance.
CP_EXPORT cpFloat cpDampedRotarySpringGetStiffness(const cpConstraint *constraint);
/// Set the stiffness of the spring in force/distance.
CP_EXPORT void cpDampedRotarySpringSetStiffness(cpConstraint *constraint, cpFloat stiffness);

/// Get the damping of the spring.
CP_EXPORT cpFloat cpDampedRotarySpringGetDamping(const cpConstraint *constraint);
/// Set the damping of the spring.
CP_EXPORT void cpDampedRotarySpringSetDamping(cpConstraint *constraint, cpFloat damping);

/// Get the damping of the spring.
CP_EXPORT cpDampedRotarySpringTorqueFunc cpDampedRotarySpringGetSpringTorqueFunc(const cpConstraint *constraint);
/// Set the damping of the spring.
CP_EXPORT void cpDampedRotarySpringSetSpringTorqueFunc(cpConstraint *constraint, cpDampedRotarySpringTorqueFunc springTorqueFunc);

/// @}

//
//MERGED FILE END: cpDampedRotarySpring.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpRotaryLimitJoint.h
//

/// @defgroup cpRotaryLimitJoint cpRotaryLimitJoint
/// @{

/// Check if a constraint is a damped rotary springs.
CP_EXPORT cpBool cpConstraintIsRotaryLimitJoint(const cpConstraint *constraint);

/// Allocate a damped rotary limit joint.
CP_EXPORT cpRotaryLimitJoint* cpRotaryLimitJointAlloc(void);
/// Initialize a damped rotary limit joint.
CP_EXPORT cpRotaryLimitJoint* cpRotaryLimitJointInit(cpRotaryLimitJoint *joint, cpBody *a, cpBody *b, cpFloat min, cpFloat max);
/// Allocate and initialize a damped rotary limit joint.
CP_EXPORT cpConstraint* cpRotaryLimitJointNew(cpBody *a, cpBody *b, cpFloat min, cpFloat max);

/// Get the minimum distance the joint will maintain between the two anchors.
CP_EXPORT cpFloat cpRotaryLimitJointGetMin(const cpConstraint *constraint);
/// Set the minimum distance the joint will maintain between the two anchors.
CP_EXPORT void cpRotaryLimitJointSetMin(cpConstraint *constraint, cpFloat min);

/// Get the maximum distance the joint will maintain between the two anchors.
CP_EXPORT cpFloat cpRotaryLimitJointGetMax(const cpConstraint *constraint);
/// Set the maximum distance the joint will maintain between the two anchors.
CP_EXPORT void cpRotaryLimitJointSetMax(cpConstraint *constraint, cpFloat max);

/// @}

//
//MERGED FILE END: cpRotaryLimitJoint.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpRatchetJoint.h
//

/// @defgroup cpRatchetJoint cpRatchetJoint
/// @{

/// Check if a constraint is a damped rotary springs.
CP_EXPORT cpBool cpConstraintIsRatchetJoint(const cpConstraint *constraint);

/// Allocate a ratchet joint.
CP_EXPORT cpRatchetJoint* cpRatchetJointAlloc(void);
/// Initialize a ratched joint.
CP_EXPORT cpRatchetJoint* cpRatchetJointInit(cpRatchetJoint *joint, cpBody *a, cpBody *b, cpFloat phase, cpFloat ratchet);
/// Allocate and initialize a ratchet joint.
CP_EXPORT cpConstraint* cpRatchetJointNew(cpBody *a, cpBody *b, cpFloat phase, cpFloat ratchet);

/// Get the angle of the current ratchet tooth.
CP_EXPORT cpFloat cpRatchetJointGetAngle(const cpConstraint *constraint);
/// Set the angle of the current ratchet tooth.
CP_EXPORT void cpRatchetJointSetAngle(cpConstraint *constraint, cpFloat angle);

/// Get the phase offset of the ratchet.
CP_EXPORT cpFloat cpRatchetJointGetPhase(const cpConstraint *constraint);
/// Get the phase offset of the ratchet.
CP_EXPORT void cpRatchetJointSetPhase(cpConstraint *constraint, cpFloat phase);

/// Get the angular distance of each ratchet.
CP_EXPORT cpFloat cpRatchetJointGetRatchet(const cpConstraint *constraint);
/// Set the angular distance of each ratchet.
CP_EXPORT void cpRatchetJointSetRatchet(cpConstraint *constraint, cpFloat ratchet);

/// @}

//
//MERGED FILE END: cpRatchetJoint.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpGearJoint.h
//

/// @defgroup cpGearJoint cpGearJoint
/// @{

/// Check if a constraint is a damped rotary springs.
CP_EXPORT cpBool cpConstraintIsGearJoint(const cpConstraint *constraint);

/// Allocate a gear joint.
CP_EXPORT cpGearJoint* cpGearJointAlloc(void);
/// Initialize a gear joint.
CP_EXPORT cpGearJoint* cpGearJointInit(cpGearJoint *joint, cpBody *a, cpBody *b, cpFloat phase, cpFloat ratio);
/// Allocate and initialize a gear joint.
CP_EXPORT cpConstraint* cpGearJointNew(cpBody *a, cpBody *b, cpFloat phase, cpFloat ratio);

/// Get the phase offset of the gears.
CP_EXPORT cpFloat cpGearJointGetPhase(const cpConstraint *constraint);
/// Set the phase offset of the gears.
CP_EXPORT void cpGearJointSetPhase(cpConstraint *constraint, cpFloat phase);

/// Get the angular distance of each ratchet.
CP_EXPORT cpFloat cpGearJointGetRatio(const cpConstraint *constraint);
/// Set the ratio of a gear joint.
CP_EXPORT void cpGearJointSetRatio(cpConstraint *constraint, cpFloat ratio);

/// @}

//
//MERGED FILE END: cpGearJoint.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSimpleMotor.h
//

/// @defgroup cpSimpleMotor cpSimpleMotor
/// @{

/// Opaque struct type for damped rotary springs.
typedef struct cpSimpleMotor cpSimpleMotor;

/// Check if a constraint is a damped rotary springs.
CP_EXPORT cpBool cpConstraintIsSimpleMotor(const cpConstraint *constraint);

/// Allocate a simple motor.
CP_EXPORT cpSimpleMotor* cpSimpleMotorAlloc(void);
/// initialize a simple motor.
CP_EXPORT cpSimpleMotor* cpSimpleMotorInit(cpSimpleMotor *joint, cpBody *a, cpBody *b, cpFloat rate);
/// Allocate and initialize a simple motor.
CP_EXPORT cpConstraint* cpSimpleMotorNew(cpBody *a, cpBody *b, cpFloat rate);

/// Get the rate of the motor.
CP_EXPORT cpFloat cpSimpleMotorGetRate(const cpConstraint *constraint);
/// Set the rate of the motor.
CP_EXPORT void cpSimpleMotorSetRate(cpConstraint *constraint, cpFloat rate);

/// @}

//
//MERGED FILE END: cpSimpleMotor.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

///@}

//
//MERGED FILE END: cpConstraint.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSpace.h
//

/// @defgroup cpSpace cpSpace
/// @{

//MARK: Definitions

/// Collision begin event function callback type.
/// Returning false from a begin callback causes the collision to be ignored until
/// the the separate callback is called when the objects stop colliding.
typedef cpBool (*cpCollisionBeginFunc)(cpArbiter *arb, cpSpace *space, cpDataPointer userData);
/// Collision pre-solve event function callback type.
/// Returning false from a pre-step callback causes the collision to be ignored until the next step.
typedef cpBool (*cpCollisionPreSolveFunc)(cpArbiter *arb, cpSpace *space, cpDataPointer userData);
/// Collision post-solve event function callback type.
typedef void (*cpCollisionPostSolveFunc)(cpArbiter *arb, cpSpace *space, cpDataPointer userData);
/// Collision separate event function callback type.
typedef void (*cpCollisionSeparateFunc)(cpArbiter *arb, cpSpace *space, cpDataPointer userData);

/// Struct that holds function callback pointers to configure custom collision handling.
/// Collision handlers have a pair of types; when a collision occurs between two shapes that have these types, the collision handler functions are triggered.
struct cpCollisionHandler {
	/// Collision type identifier of the first shape that this handler recognizes.
	/// In the collision handler callback, the shape with this type will be the first argument. Read only.
	cpCollisionType typeA;
	/// Collision type identifier of the second shape that this handler recognizes.
	/// In the collision handler callback, the shape with this type will be the second argument. Read only.
	cpCollisionType typeB;
	/// This function is called when two shapes with types that match this collision handler begin colliding.
	cpCollisionBeginFunc beginFunc;
	/// This function is called each step when two shapes with types that match this collision handler are colliding.
	/// It's called before the collision solver runs so that you can affect a collision's outcome.
	cpCollisionPreSolveFunc preSolveFunc;
	/// This function is called each step when two shapes with types that match this collision handler are colliding.
	/// It's called after the collision solver runs so that you can read back information about the collision to trigger events in your game.
	cpCollisionPostSolveFunc postSolveFunc;
	/// This function is called when two shapes with types that match this collision handler stop colliding.
	cpCollisionSeparateFunc separateFunc;
	/// This is a user definable context pointer that is passed to all of the collision handler functions.
	cpDataPointer userData;
};

// TODO: Make timestep a parameter?

//MARK: Memory and Initialization

/// Allocate a cpSpace.
CP_EXPORT cpSpace* cpSpaceAlloc(void);
/// Initialize a cpSpace.
CP_EXPORT cpSpace* cpSpaceInit(cpSpace *space);
/// Allocate and initialize a cpSpace.
CP_EXPORT cpSpace* cpSpaceNew(void);

/// Destroy a cpSpace.
CP_EXPORT void cpSpaceDestroy(cpSpace *space);
/// Destroy and free a cpSpace.
CP_EXPORT void cpSpaceFree(cpSpace *space);

//MARK: Properties

/// Number of iterations to use in the impulse solver to solve contacts and other constraints.
CP_EXPORT int cpSpaceGetIterations(const cpSpace *space);
CP_EXPORT void cpSpaceSetIterations(cpSpace *space, int iterations);

/// Gravity to pass to rigid bodies when integrating velocity.
CP_EXPORT cpVect cpSpaceGetGravity(const cpSpace *space);
CP_EXPORT void cpSpaceSetGravity(cpSpace *space, cpVect gravity);

/// Damping rate expressed as the fraction of velocity bodies retain each second.
/// A value of 0.9 would mean that each body's velocity will drop 10% per second.
/// The default value is 1.0, meaning no damping is applied.
/// @note This damping value is different than those of cpDampedSpring and cpDampedRotarySpring.
CP_EXPORT cpFloat cpSpaceGetDamping(const cpSpace *space);
CP_EXPORT void cpSpaceSetDamping(cpSpace *space, cpFloat damping);

/// Speed threshold for a body to be considered idle.
/// The default value of 0 means to let the space guess a good threshold based on gravity.
CP_EXPORT cpFloat cpSpaceGetIdleSpeedThreshold(const cpSpace *space);
CP_EXPORT void cpSpaceSetIdleSpeedThreshold(cpSpace *space, cpFloat idleSpeedThreshold);

/// Time a group of bodies must remain idle in order to fall asleep.
/// Enabling sleeping also implicitly enables the the contact graph.
/// The default value of INFINITY disables the sleeping algorithm.
CP_EXPORT cpFloat cpSpaceGetSleepTimeThreshold(const cpSpace *space);
CP_EXPORT void cpSpaceSetSleepTimeThreshold(cpSpace *space, cpFloat sleepTimeThreshold);

/// Amount of encouraged penetration between colliding shapes.
/// Used to reduce oscillating contacts and keep the collision cache warm.
/// Defaults to 0.1. If you have poor simulation quality,
/// increase this number as much as possible without allowing visible amounts of overlap.
CP_EXPORT cpFloat cpSpaceGetCollisionSlop(const cpSpace *space);
CP_EXPORT void cpSpaceSetCollisionSlop(cpSpace *space, cpFloat collisionSlop);

/// Determines how fast overlapping shapes are pushed apart.
/// Expressed as a fraction of the error remaining after each second.
/// Defaults to pow(1.0 - 0.1, 60.0) meaning that Chipmunk fixes 10% of overlap each frame at 60Hz.
CP_EXPORT cpFloat cpSpaceGetCollisionBias(const cpSpace *space);
CP_EXPORT void cpSpaceSetCollisionBias(cpSpace *space, cpFloat collisionBias);

/// Number of frames that contact information should persist.
/// Defaults to 3. There is probably never a reason to change this value.
CP_EXPORT cpTimestamp cpSpaceGetCollisionPersistence(const cpSpace *space);
CP_EXPORT void cpSpaceSetCollisionPersistence(cpSpace *space, cpTimestamp collisionPersistence);

/// User definable data pointer.
/// Generally this points to your game's controller or game state
/// class so you can access it when given a cpSpace reference in a callback.
CP_EXPORT cpDataPointer cpSpaceGetUserData(const cpSpace *space);
CP_EXPORT void cpSpaceSetUserData(cpSpace *space, cpDataPointer userData);

/// The Space provided static body for a given cpSpace.
/// This is merely provided for convenience and you are not required to use it.
CP_EXPORT cpBody* cpSpaceGetStaticBody(const cpSpace *space);

/// Returns the current (or most recent) time step used with the given space.
/// Useful from callbacks if your time step is not a compile-time global.
CP_EXPORT cpFloat cpSpaceGetCurrentTimeStep(const cpSpace *space);

/// returns true from inside a callback when objects cannot be added/removed.
CP_EXPORT cpBool cpSpaceIsLocked(cpSpace *space);

//MARK: Collision Handlers

/// Create or return the existing collision handler that is called for all collisions that are not handled by a more specific collision handler.
CP_EXPORT cpCollisionHandler *cpSpaceAddDefaultCollisionHandler(cpSpace *space);
/// Create or return the existing collision handler for the specified pair of collision types.
/// If wildcard handlers are used with either of the collision types, it's the responibility of the custom handler to invoke the wildcard handlers.
CP_EXPORT cpCollisionHandler *cpSpaceAddCollisionHandler(cpSpace *space, cpCollisionType a, cpCollisionType b);
/// Create or return the existing wildcard collision handler for the specified type.
CP_EXPORT cpCollisionHandler *cpSpaceAddWildcardHandler(cpSpace *space, cpCollisionType type);

//MARK: Add/Remove objects

/// Add a collision shape to the simulation.
/// If the shape is attached to a static body, it will be added as a static shape.
CP_EXPORT cpShape* cpSpaceAddShape(cpSpace *space, cpShape *shape);
/// Add a rigid body to the simulation.
CP_EXPORT cpBody* cpSpaceAddBody(cpSpace *space, cpBody *body);
/// Add a constraint to the simulation.
CP_EXPORT cpConstraint* cpSpaceAddConstraint(cpSpace *space, cpConstraint *constraint);

/// Remove a collision shape from the simulation.
CP_EXPORT void cpSpaceRemoveShape(cpSpace *space, cpShape *shape);
/// Remove a rigid body from the simulation.
CP_EXPORT void cpSpaceRemoveBody(cpSpace *space, cpBody *body);
/// Remove a constraint from the simulation.
CP_EXPORT void cpSpaceRemoveConstraint(cpSpace *space, cpConstraint *constraint);

/// Test if a collision shape has been added to the space.
CP_EXPORT cpBool cpSpaceContainsShape(cpSpace *space, cpShape *shape);
/// Test if a rigid body has been added to the space.
CP_EXPORT cpBool cpSpaceContainsBody(cpSpace *space, cpBody *body);
/// Test if a constraint has been added to the space.
CP_EXPORT cpBool cpSpaceContainsConstraint(cpSpace *space, cpConstraint *constraint);

//MARK: Post-Step Callbacks

/// Post Step callback function type.
typedef void (*cpPostStepFunc)(cpSpace *space, void *key, void *data);
/// Schedule a post-step callback to be called when cpSpaceStep() finishes.
/// You can only register one callback per unique value for @c key.
/// Returns true only if @c key has never been scheduled before.
/// It's possible to pass @c NULL for @c func if you only want to mark @c key as being used.
CP_EXPORT cpBool cpSpaceAddPostStepCallback(cpSpace *space, cpPostStepFunc func, void *key, void *data);

//MARK: Queries

// TODO: Queries and iterators should take a cpSpace parametery.
// TODO: They should also be abortable.

/// Nearest point query callback function type.
typedef void (*cpSpacePointQueryFunc)(cpShape *shape, cpVect point, cpFloat distance, cpVect gradient, void *data);
/// Query the space at a point and call @c func for each shape found.
CP_EXPORT void cpSpacePointQuery(cpSpace *space, cpVect point, cpFloat maxDistance, cpShapeFilter filter, cpSpacePointQueryFunc func, void *data);
/// Query the space at a point and return the nearest shape found. Returns NULL if no shapes were found.
CP_EXPORT cpShape *cpSpacePointQueryNearest(cpSpace *space, cpVect point, cpFloat maxDistance, cpShapeFilter filter, cpPointQueryInfo *out);

/// Segment query callback function type.
typedef void (*cpSpaceSegmentQueryFunc)(cpShape *shape, cpVect point, cpVect normal, cpFloat alpha, void *data);
/// Perform a directed line segment query (like a raycast) against the space calling @c func for each shape intersected.
CP_EXPORT void cpSpaceSegmentQuery(cpSpace *space, cpVect start, cpVect end, cpFloat radius, cpShapeFilter filter, cpSpaceSegmentQueryFunc func, void *data);
/// Perform a directed line segment query (like a raycast) against the space and return the first shape hit. Returns NULL if no shapes were hit.
CP_EXPORT cpShape *cpSpaceSegmentQueryFirst(cpSpace *space, cpVect start, cpVect end, cpFloat radius, cpShapeFilter filter, cpSegmentQueryInfo *out);

/// Rectangle Query callback function type.
typedef void (*cpSpaceBBQueryFunc)(cpShape *shape, void *data);
/// Perform a fast rectangle query on the space calling @c func for each shape found.
/// Only the shape's bounding boxes are checked for overlap, not their full shape.
CP_EXPORT void cpSpaceBBQuery(cpSpace *space, cpBB bb, cpShapeFilter filter, cpSpaceBBQueryFunc func, void *data);

/// Shape query callback function type.
typedef void (*cpSpaceShapeQueryFunc)(cpShape *shape, cpContactPointSet *points, void *data);
/// Query a space for any shapes overlapping the given shape and call @c func for each shape found.
CP_EXPORT cpBool cpSpaceShapeQuery(cpSpace *space, cpShape *shape, cpSpaceShapeQueryFunc func, void *data);

//MARK: Iteration

/// Space/body iterator callback function type.
typedef void (*cpSpaceBodyIteratorFunc)(cpBody *body, void *data);
/// Call @c func for each body in the space.
CP_EXPORT void cpSpaceEachBody(cpSpace *space, cpSpaceBodyIteratorFunc func, void *data);

/// Space/body iterator callback function type.
typedef void (*cpSpaceShapeIteratorFunc)(cpShape *shape, void *data);
/// Call @c func for each shape in the space.
CP_EXPORT void cpSpaceEachShape(cpSpace *space, cpSpaceShapeIteratorFunc func, void *data);

/// Space/constraint iterator callback function type.
typedef void (*cpSpaceConstraintIteratorFunc)(cpConstraint *constraint, void *data);
/// Call @c func for each shape in the space.
CP_EXPORT void cpSpaceEachConstraint(cpSpace *space, cpSpaceConstraintIteratorFunc func, void *data);

//MARK: Indexing

/// Update the collision detection info for the static shapes in the space.
CP_EXPORT void cpSpaceReindexStatic(cpSpace *space);
/// Update the collision detection data for a specific shape in the space.
CP_EXPORT void cpSpaceReindexShape(cpSpace *space, cpShape *shape);
/// Update the collision detection data for all shapes attached to a body.
CP_EXPORT void cpSpaceReindexShapesForBody(cpSpace *space, cpBody *body);

/// Switch the space to use a spatial has as it's spatial index.
CP_EXPORT void cpSpaceUseSpatialHash(cpSpace *space, cpFloat dim, int count);

//MARK: Time Stepping

/// Step the space forward in time by @c dt.
CP_EXPORT void cpSpaceStep(cpSpace *space, cpFloat dt);

//MARK: Debug API

#ifndef CP_SPACE_DISABLE_DEBUG_API

/// Color type to use with the space debug drawing API.
typedef struct cpSpaceDebugColor {
	float r, g, b, a;
} cpSpaceDebugColor;

/// Callback type for a function that draws a filled, stroked circle.
typedef void (*cpSpaceDebugDrawCircleImpl)(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor, cpDataPointer data);
/// Callback type for a function that draws a line segment.
typedef void (*cpSpaceDebugDrawSegmentImpl)(cpVect a, cpVect b, cpSpaceDebugColor color, cpDataPointer data);
/// Callback type for a function that draws a thick line segment.
typedef void (*cpSpaceDebugDrawFatSegmentImpl)(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor, cpDataPointer data);
/// Callback type for a function that draws a convex polygon.
typedef void (*cpSpaceDebugDrawPolygonImpl)(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor, cpDataPointer data);
/// Callback type for a function that draws a dot.
typedef void (*cpSpaceDebugDrawDotImpl)(cpFloat size, cpVect pos, cpSpaceDebugColor color, cpDataPointer data);
/// Callback type for a function that returns a color for a given shape. This gives you an opportunity to color shapes based on how they are used in your engine.
typedef cpSpaceDebugColor (*cpSpaceDebugDrawColorForShapeImpl)(cpShape *shape, cpDataPointer data);

typedef enum cpSpaceDebugDrawFlags {
	CP_SPACE_DEBUG_DRAW_SHAPES = 1<<0,
	CP_SPACE_DEBUG_DRAW_CONSTRAINTS = 1<<1,
	CP_SPACE_DEBUG_DRAW_COLLISION_POINTS = 1<<2,
} cpSpaceDebugDrawFlags;

/// Struct used with cpSpaceDebugDraw() containing drawing callbacks and other drawing settings.
typedef struct cpSpaceDebugDrawOptions {
	/// Function that will be invoked to draw circles.
	cpSpaceDebugDrawCircleImpl drawCircle;
	/// Function that will be invoked to draw line segments.
	cpSpaceDebugDrawSegmentImpl drawSegment;
	/// Function that will be invoked to draw thick line segments.
	cpSpaceDebugDrawFatSegmentImpl drawFatSegment;
	/// Function that will be invoked to draw convex polygons.
	cpSpaceDebugDrawPolygonImpl drawPolygon;
	/// Function that will be invoked to draw dots.
	cpSpaceDebugDrawDotImpl drawDot;

	/// Flags that request which things to draw (collision shapes, constraints, contact points).
	cpSpaceDebugDrawFlags flags;
	/// Outline color passed to the drawing function.
	cpSpaceDebugColor shapeOutlineColor;
	/// Function that decides what fill color to draw shapes using.
	cpSpaceDebugDrawColorForShapeImpl colorForShape;
	/// Color passed to drawing functions for constraints.
	cpSpaceDebugColor constraintColor;
	/// Color passed to drawing functions for collision points.
	cpSpaceDebugColor collisionPointColor;

	/// User defined context pointer passed to all of the callback functions as the 'data' argument.
	cpDataPointer data;
} cpSpaceDebugDrawOptions;

/// Debug draw the current state of the space using the supplied drawing options.
CP_EXPORT void cpSpaceDebugDraw(cpSpace *space, cpSpaceDebugDrawOptions *options);

#endif

/// @}

//
//MERGED FILE END: cpSpace.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

// Chipmunk 7.0.1
#define CP_VERSION_MAJOR 7
#define CP_VERSION_MINOR 0
#define CP_VERSION_RELEASE 1

/// Version string.
CP_EXPORT extern const char *cpVersionString;

/// Calculate the moment of inertia for a circle.
/// @c r1 and @c r2 are the inner and outer diameters. A solid circle has an inner diameter of 0.
CP_EXPORT cpFloat cpMomentForCircle(cpFloat m, cpFloat r1, cpFloat r2, cpVect offset);

/// Calculate area of a hollow circle.
/// @c r1 and @c r2 are the inner and outer diameters. A solid circle has an inner diameter of 0.
CP_EXPORT cpFloat cpAreaForCircle(cpFloat r1, cpFloat r2);

/// Calculate the moment of inertia for a line segment.
/// Beveling radius is not supported.
CP_EXPORT cpFloat cpMomentForSegment(cpFloat m, cpVect a, cpVect b, cpFloat radius);

/// Calculate the area of a fattened (capsule shaped) line segment.
CP_EXPORT cpFloat cpAreaForSegment(cpVect a, cpVect b, cpFloat radius);

/// Calculate the moment of inertia for a solid polygon shape assuming it's center of gravity is at it's centroid. The offset is added to each vertex.
CP_EXPORT cpFloat cpMomentForPoly(cpFloat m, int count, const cpVect *verts, cpVect offset, cpFloat radius);

/// Calculate the signed area of a polygon. A Clockwise winding gives positive area.
/// This is probably backwards from what you expect, but matches Chipmunk's the winding for poly shapes.
CP_EXPORT cpFloat cpAreaForPoly(const int count, const cpVect *verts, cpFloat radius);

/// Calculate the natural centroid of a polygon.
CP_EXPORT cpVect cpCentroidForPoly(const int count, const cpVect *verts);

/// Calculate the moment of inertia for a solid box.
CP_EXPORT cpFloat cpMomentForBox(cpFloat m, cpFloat width, cpFloat height);

/// Calculate the moment of inertia for a solid box.
CP_EXPORT cpFloat cpMomentForBox2(cpFloat m, cpBB box);

/// Calculate the convex hull of a given set of points. Returns the count of points in the hull.
/// @c result must be a pointer to a @c cpVect array with at least @c count elements. If @c verts == @c result, then @c verts will be reduced inplace.
/// @c first is an optional pointer to an integer to store where the first vertex in the hull came from (i.e. verts[first] == result[0])
/// @c tol is the allowed amount to shrink the hull when simplifying it. A tolerance of 0.0 creates an exact hull.
CP_EXPORT int cpConvexHull(int count, const cpVect *verts, cpVect *result, int *first, cpFloat tol);

#ifdef _MSC_VER
#include "malloc.h"
#endif

/// Convenience macro to work with cpConvexHull.
/// @c count and @c verts is the input array passed to cpConvexHull().
/// @c count_var and @c verts_var are the names of the variables the macro creates to store the result.
/// The output vertex array is allocated on the stack using alloca() so it will be freed automatically, but cannot be returned from the current scope.
#define CP_CONVEX_HULL(__count__, __verts__, __count_var__, __verts_var__) \
cpVect *__verts_var__ = (cpVect *)alloca(__count__*sizeof(cpVect)); \
int __count_var__ = cpConvexHull(__count__, __verts__, __verts_var__, NULL, 0.0); \

/// Returns the closest point on the line segment ab, to the point p.
static inline cpVect
cpClosetPointOnSegment(const cpVect p, const cpVect a, const cpVect b)
{
	cpVect delta = cpvsub(a, b);
	cpFloat t = cpfclamp01(cpvdot(delta, cpvsub(p, b))/cpvlengthsq(delta));
	return cpvadd(b, cpvmult(delta, t));
}

#if defined(__has_extension)
#if __has_extension(blocks)
// Define alternate block based alternatives for a few of the callback heavy functions.
// Collision handlers are post-step callbacks are not included to avoid memory management issues.
// If you want to use blocks for those and are aware of how to correctly manage the memory, the implementation is trivial.

void cpSpaceEachBody_b(cpSpace *space, void (^block)(cpBody *body));
void cpSpaceEachShape_b(cpSpace *space, void (^block)(cpShape *shape));
void cpSpaceEachConstraint_b(cpSpace *space, void (^block)(cpConstraint *constraint));

void cpBodyEachShape_b(cpBody *body, void (^block)(cpShape *shape));
void cpBodyEachConstraint_b(cpBody *body, void (^block)(cpConstraint *constraint));
void cpBodyEachArbiter_b(cpBody *body, void (^block)(cpArbiter *arbiter));

typedef void (^cpSpacePointQueryBlock)(cpShape *shape, cpVect point, cpFloat distance, cpVect gradient);
void cpSpacePointQuery_b(cpSpace *space, cpVect point, cpFloat maxDistance, cpShapeFilter filter, cpSpacePointQueryBlock block);

typedef void (^cpSpaceSegmentQueryBlock)(cpShape *shape, cpVect point, cpVect normal, cpFloat alpha);
void cpSpaceSegmentQuery_b(cpSpace *space, cpVect start, cpVect end, cpFloat radius, cpShapeFilter filter, cpSpaceSegmentQueryBlock block);

typedef void (^cpSpaceBBQueryBlock)(cpShape *shape);
void cpSpaceBBQuery_b(cpSpace *space, cpBB bb, cpShapeFilter filter, cpSpaceBBQueryBlock block);

typedef void (^cpSpaceShapeQueryBlock)(cpShape *shape, cpContactPointSet *points);
cpBool cpSpaceShapeQuery_b(cpSpace *space, cpShape *shape, cpSpaceShapeQueryBlock block);

#endif
#endif

//@}

#ifdef __cplusplus
}

static inline cpVect operator *(const cpVect v, const cpFloat s){return cpvmult(v, s);}
static inline cpVect operator +(const cpVect v1, const cpVect v2){return cpvadd(v1, v2);}
static inline cpVect operator -(const cpVect v1, const cpVect v2){return cpvsub(v1, v2);}
static inline cpBool operator ==(const cpVect v1, const cpVect v2){return cpveql(v1, v2);}
static inline cpVect operator -(const cpVect v){return cpvneg(v);}

#endif
#endif

//
//MERGED FILE END: chipmunk.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

#define CP_HASH_COEF ((cpHashValue)3344921057ul)
#define CP_HASH_PAIR(A, B) ((cpHashValue)(A)*CP_HASH_COEF ^ (cpHashValue)(B)*CP_HASH_COEF)

// TODO: Eww. Magic numbers.
#define MAGIC_EPSILON 1e-5

//MARK: cpArray

struct cpArray {
	int num, max;
	void **arr;
};

cpArray *cpArrayNew(int size);

void cpArrayFree(cpArray *arr);

void cpArrayPush(cpArray *arr, void *object);
void *cpArrayPop(cpArray *arr);
void cpArrayDeleteObj(cpArray *arr, void *obj);
cpBool cpArrayContains(cpArray *arr, void *ptr);

void cpArrayFreeEach(cpArray *arr, void (freeFunc)(void*));

//MARK: cpHashSet

typedef cpBool (*cpHashSetEqlFunc)(void *ptr, void *elt);
typedef void *(*cpHashSetTransFunc)(void *ptr, void *data);

cpHashSet *cpHashSetNew(int size, cpHashSetEqlFunc eqlFunc);
void cpHashSetSetDefaultValue(cpHashSet *set, void *default_value);

void cpHashSetFree(cpHashSet *set);

int cpHashSetCount(cpHashSet *set);
void *cpHashSetInsert(cpHashSet *set, cpHashValue hash, void *ptr, cpHashSetTransFunc trans, void *data);
void *cpHashSetRemove(cpHashSet *set, cpHashValue hash, void *ptr);
void *cpHashSetFind(cpHashSet *set, cpHashValue hash, void *ptr);

typedef void (*cpHashSetIteratorFunc)(void *elt, void *data);
void cpHashSetEach(cpHashSet *set, cpHashSetIteratorFunc func, void *data);

typedef cpBool (*cpHashSetFilterFunc)(void *elt, void *data);
void cpHashSetFilter(cpHashSet *set, cpHashSetFilterFunc func, void *data);

//MARK: Bodies

struct cpBody {
	// Integration functions
	cpBodyVelocityFunc velocity_func;
	cpBodyPositionFunc position_func;

	// mass and it's inverse
	cpFloat m;
	cpFloat m_inv;

	// moment of inertia and it's inverse
	cpFloat i;
	cpFloat i_inv;

	// center of gravity
	cpVect cog;

	// position, velocity, force
	cpVect p;
	cpVect v;
	cpVect f;

	// Angle, angular velocity, torque (radians)
	cpFloat a;
	cpFloat w;
	cpFloat t;

	cpTransform transform;

	cpDataPointer userData;

	// "pseudo-velocities" used for eliminating overlap.
	// Erin Catto has some papers that talk about what these are.
	cpVect v_bias;
	cpFloat w_bias;

	cpSpace *space;

	cpShape *shapeList;
	cpArbiter *arbiterList;
	cpConstraint *constraintList;

	struct {
		cpBody *root;
		cpBody *next;
		cpFloat idleTime;
	} sleeping;
};

void cpBodyAddShape(cpBody *body, cpShape *shape);
void cpBodyRemoveShape(cpBody *body, cpShape *shape);

//void cpBodyAccumulateMassForShape(cpBody *body, cpShape *shape);
void cpBodyAccumulateMassFromShapes(cpBody *body);

void cpBodyRemoveConstraint(cpBody *body, cpConstraint *constraint);

//MARK: Spatial Index Functions

cpSpatialIndex *cpSpatialIndexInit(cpSpatialIndex *index, cpSpatialIndexClass *klass, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex);

//MARK: Arbiters

enum cpArbiterState {
	// Arbiter is active and its the first collision.
	CP_ARBITER_STATE_FIRST_COLLISION,
	// Arbiter is active and its not the first collision.
	CP_ARBITER_STATE_NORMAL,
	// Collision has been explicitly ignored.
	// Either by returning false from a begin collision handler or calling cpArbiterIgnore().
	CP_ARBITER_STATE_IGNORE,
	// Collison is no longer active. A space will cache an arbiter for up to cpSpace.collisionPersistence more steps.
	CP_ARBITER_STATE_CACHED,
	// Collison arbiter is invalid because one of the shapes was removed.
	CP_ARBITER_STATE_INVALIDATED,
};

struct cpArbiterThread {
	struct cpArbiter *next, *prev;
};

struct cpContact {
	cpVect r1, r2;

	cpFloat nMass, tMass;
	cpFloat bounce; // TODO: look for an alternate bounce solution.

	cpFloat jnAcc, jtAcc, jBias;
	cpFloat bias;

	cpHashValue hash;
};

struct cpCollisionInfo {
	const cpShape *a, *b;
	cpCollisionID id;

	cpVect n;

	int count;
	// TODO Should this be a unique struct type?
	struct cpContact *arr;
};

struct cpArbiter {
	cpFloat e;
	cpFloat u;
	cpVect surface_vr;

	cpDataPointer data;

	const cpShape *a, *b;
	cpBody *body_a, *body_b;
	struct cpArbiterThread thread_a, thread_b;

	int count;
	struct cpContact *contacts;
	cpVect n;

	// Regular, wildcard A and wildcard B collision handlers.
	cpCollisionHandler *handler, *handlerA, *handlerB;
	cpBool swapped;

	cpTimestamp stamp;
	enum cpArbiterState state;
};

cpArbiter* cpArbiterInit(cpArbiter *arb, cpShape *a, cpShape *b);

static inline struct cpArbiterThread *
cpArbiterThreadForBody(cpArbiter *arb, cpBody *body)
{
	return (arb->body_a == body ? &arb->thread_a : &arb->thread_b);
}

void cpArbiterUnthread(cpArbiter *arb);

void cpArbiterUpdate(cpArbiter *arb, struct cpCollisionInfo *info, cpSpace *space);
void cpArbiterPreStep(cpArbiter *arb, cpFloat dt, cpFloat bias, cpFloat slop);
void cpArbiterApplyCachedImpulse(cpArbiter *arb, cpFloat dt_coef);
void cpArbiterApplyImpulse(cpArbiter *arb);

//MARK: Shapes/Collisions

struct cpShapeMassInfo {
	cpFloat m;
	cpFloat i;
	cpVect cog;
	cpFloat area;
};

typedef enum cpShapeType{
	CP_CIRCLE_SHAPE,
	CP_SEGMENT_SHAPE,
	CP_POLY_SHAPE,
	CP_NUM_SHAPES
} cpShapeType;

typedef cpBB (*cpShapeCacheDataImpl)(cpShape *shape, cpTransform transform);
typedef void (*cpShapeDestroyImpl)(cpShape *shape);
typedef void (*cpShapePointQueryImpl)(const cpShape *shape, cpVect p, cpPointQueryInfo *info);
typedef void (*cpShapeSegmentQueryImpl)(const cpShape *shape, cpVect a, cpVect b, cpFloat radius, cpSegmentQueryInfo *info);

typedef struct cpShapeClass cpShapeClass;

struct cpShapeClass {
	cpShapeType type;

	cpShapeCacheDataImpl cacheData;
	cpShapeDestroyImpl destroy;
	cpShapePointQueryImpl pointQuery;
	cpShapeSegmentQueryImpl segmentQuery;
};

struct cpShape {
	const cpShapeClass *klass;

	cpSpace *space;
	cpBody *body;
	struct cpShapeMassInfo massInfo;
	cpBB bb;

	cpBool sensor;

	cpFloat e;
	cpFloat u;
	cpVect surfaceV;

	cpDataPointer userData;

	cpCollisionType type;
	cpShapeFilter filter;

	cpShape *next;
	cpShape *prev;

	cpHashValue hashid;
};

struct cpCircleShape {
	cpShape shape;

	cpVect c, tc;
	cpFloat r;
};

struct cpSegmentShape {
	cpShape shape;

	cpVect a, b, n;
	cpVect ta, tb, tn;
	cpFloat r;

	cpVect a_tangent, b_tangent;
};

struct cpSplittingPlane {
	cpVect v0, n;
};

#define CP_POLY_SHAPE_INLINE_ALLOC 6

struct cpPolyShape {
	cpShape shape;

	cpFloat r;

	int count;
	// The untransformed planes are appended at the end of the transformed planes.
	struct cpSplittingPlane *planes;

	// Allocate a small number of splitting planes internally for simple poly.
	struct cpSplittingPlane _planes[2*CP_POLY_SHAPE_INLINE_ALLOC];
};

cpShape *cpShapeInit(cpShape *shape, const cpShapeClass *klass, cpBody *body, struct cpShapeMassInfo massInfo);

static inline cpBool
cpShapeActive(cpShape *shape)
{
	// checks if the shape is added to a shape list.
	// TODO could this just check the space now?
	return (shape->prev || (shape->body && shape->body->shapeList == shape));
}

// Note: This function returns contact points with r1/r2 in absolute coordinates, not body relative.
struct cpCollisionInfo cpCollide(const cpShape *a, const cpShape *b, cpCollisionID id, struct cpContact *contacts);

static inline void
CircleSegmentQuery(cpShape *shape, cpVect center, cpFloat r1, cpVect a, cpVect b, cpFloat r2, cpSegmentQueryInfo *info)
{
	cpVect da = cpvsub(a, center);
	cpVect db = cpvsub(b, center);
	cpFloat rsum = r1 + r2;

	cpFloat qa = cpvdot(da, da) - 2.0f*cpvdot(da, db) + cpvdot(db, db);
	cpFloat qb = cpvdot(da, db) - cpvdot(da, da);
	cpFloat det = qb*qb - qa*(cpvdot(da, da) - rsum*rsum);

	if(det >= 0.0f){
		cpFloat t = (-qb - cpfsqrt(det))/(qa);
		if(0.0f<= t && t <= 1.0f){
			cpVect n = cpvnormalize(cpvlerp(da, db, t));

			info->shape = shape;
			info->point = cpvsub(cpvlerp(a, b, t), cpvmult(n, r2));
			info->normal = n;
			info->alpha = t;
		}
	}
}

static inline cpBool
cpShapeFilterReject(cpShapeFilter a, cpShapeFilter b)
{
	// Reject the collision if:
	return (
		// They are in the same non-zero group.
		(a.group != 0 && a.group == b.group) ||
		// One of the category/mask combinations fails.
		(a.categories & b.mask) == 0 ||
		(b.categories & a.mask) == 0
	);
}

void cpLoopIndexes(const cpVect *verts, int count, int *start, int *end);

//MARK: Constraints
// TODO naming conventions here

typedef void (*cpConstraintPreStepImpl)(cpConstraint *constraint, cpFloat dt);
typedef void (*cpConstraintApplyCachedImpulseImpl)(cpConstraint *constraint, cpFloat dt_coef);
typedef void (*cpConstraintApplyImpulseImpl)(cpConstraint *constraint, cpFloat dt);
typedef cpFloat (*cpConstraintGetImpulseImpl)(cpConstraint *constraint);

typedef struct cpConstraintClass {
	cpConstraintPreStepImpl preStep;
	cpConstraintApplyCachedImpulseImpl applyCachedImpulse;
	cpConstraintApplyImpulseImpl applyImpulse;
	cpConstraintGetImpulseImpl getImpulse;
} cpConstraintClass;

struct cpConstraint {
	const cpConstraintClass *klass;

	cpSpace *space;

	cpBody *a, *b;
	cpConstraint *next_a, *next_b;

	cpFloat maxForce;
	cpFloat errorBias;
	cpFloat maxBias;

	cpBool collideBodies;

	cpConstraintPreSolveFunc preSolve;
	cpConstraintPostSolveFunc postSolve;

	cpDataPointer userData;
};

struct cpPinJoint {
	cpConstraint constraint;
	cpVect anchorA, anchorB;
	cpFloat dist;

	cpVect r1, r2;
	cpVect n;
	cpFloat nMass;

	cpFloat jnAcc;
	cpFloat bias;
};

struct cpSlideJoint {
	cpConstraint constraint;
	cpVect anchorA, anchorB;
	cpFloat min, max;

	cpVect r1, r2;
	cpVect n;
	cpFloat nMass;

	cpFloat jnAcc;
	cpFloat bias;
};

struct cpPivotJoint {
	cpConstraint constraint;
	cpVect anchorA, anchorB;

	cpVect r1, r2;
	cpMat2x2 k;

	cpVect jAcc;
	cpVect bias;
};

struct cpGrooveJoint {
	cpConstraint constraint;
	cpVect grv_n, grv_a, grv_b;
	cpVect  anchorB;

	cpVect grv_tn;
	cpFloat clamp;
	cpVect r1, r2;
	cpMat2x2 k;

	cpVect jAcc;
	cpVect bias;
};

struct cpDampedSpring {
	cpConstraint constraint;
	cpVect anchorA, anchorB;
	cpFloat restLength;
	cpFloat stiffness;
	cpFloat damping;
	cpDampedSpringForceFunc springForceFunc;

	cpFloat target_vrn;
	cpFloat v_coef;

	cpVect r1, r2;
	cpFloat nMass;
	cpVect n;

	cpFloat jAcc;
};

struct cpDampedRotarySpring {
	cpConstraint constraint;
	cpFloat restAngle;
	cpFloat stiffness;
	cpFloat damping;
	cpDampedRotarySpringTorqueFunc springTorqueFunc;

	cpFloat target_wrn;
	cpFloat w_coef;

	cpFloat iSum;
	cpFloat jAcc;
};

struct cpRotaryLimitJoint {
	cpConstraint constraint;
	cpFloat min, max;

	cpFloat iSum;

	cpFloat bias;
	cpFloat jAcc;
};

struct cpRatchetJoint {
	cpConstraint constraint;
	cpFloat angle, phase, ratchet;

	cpFloat iSum;

	cpFloat bias;
	cpFloat jAcc;
};

struct cpGearJoint {
	cpConstraint constraint;
	cpFloat phase, ratio;
	cpFloat ratio_inv;

	cpFloat iSum;

	cpFloat bias;
	cpFloat jAcc;
};

struct cpSimpleMotor {
	cpConstraint constraint;
	cpFloat rate;

	cpFloat iSum;

	cpFloat jAcc;
};

void cpConstraintInit(cpConstraint *constraint, const struct cpConstraintClass *klass, cpBody *a, cpBody *b);

static inline void
cpConstraintActivateBodies(cpConstraint *constraint)
{
	cpBody *a = constraint->a; cpBodyActivate(a);
	cpBody *b = constraint->b; cpBodyActivate(b);
}

static inline cpVect
relative_velocity(cpBody *a, cpBody *b, cpVect r1, cpVect r2){
	cpVect v1_sum = cpvadd(a->v, cpvmult(cpvperp(r1), a->w));
	cpVect v2_sum = cpvadd(b->v, cpvmult(cpvperp(r2), b->w));

	return cpvsub(v2_sum, v1_sum);
}

static inline cpFloat
normal_relative_velocity(cpBody *a, cpBody *b, cpVect r1, cpVect r2, cpVect n){
	return cpvdot(relative_velocity(a, b, r1, r2), n);
}

static inline void
apply_impulse(cpBody *body, cpVect j, cpVect r){
	body->v = cpvadd(body->v, cpvmult(j, body->m_inv));
	body->w += body->i_inv*cpvcross(r, j);
}

static inline void
apply_impulses(cpBody *a , cpBody *b, cpVect r1, cpVect r2, cpVect j)
{
	apply_impulse(a, cpvneg(j), r1);
	apply_impulse(b, j, r2);
}

static inline void
apply_bias_impulse(cpBody *body, cpVect j, cpVect r)
{
	body->v_bias = cpvadd(body->v_bias, cpvmult(j, body->m_inv));
	body->w_bias += body->i_inv*cpvcross(r, j);
}

static inline void
apply_bias_impulses(cpBody *a , cpBody *b, cpVect r1, cpVect r2, cpVect j)
{
	apply_bias_impulse(a, cpvneg(j), r1);
	apply_bias_impulse(b, j, r2);
}

static inline cpFloat
k_scalar_body(cpBody *body, cpVect r, cpVect n)
{
	cpFloat rcn = cpvcross(r, n);
	return body->m_inv + body->i_inv*rcn*rcn;
}

static inline cpFloat
k_scalar(cpBody *a, cpBody *b, cpVect r1, cpVect r2, cpVect n)
{
	cpFloat value = k_scalar_body(a, r1, n) + k_scalar_body(b, r2, n);
	cpAssertSoft(value != 0.0, "Unsolvable collision or constraint.");

	return value;
}

static inline cpMat2x2
k_tensor(cpBody *a, cpBody *b, cpVect r1, cpVect r2)
{
	cpFloat m_sum = a->m_inv + b->m_inv;

	// start with Identity*m_sum
	cpFloat k11 = m_sum, k12 = 0.0f;
	cpFloat k21 = 0.0f,  k22 = m_sum;

	// add the influence from r1
	cpFloat a_i_inv = a->i_inv;
	cpFloat r1xsq =  r1.x * r1.x * a_i_inv;
	cpFloat r1ysq =  r1.y * r1.y * a_i_inv;
	cpFloat r1nxy = -r1.x * r1.y * a_i_inv;
	k11 += r1ysq; k12 += r1nxy;
	k21 += r1nxy; k22 += r1xsq;

	// add the influnce from r2
	cpFloat b_i_inv = b->i_inv;
	cpFloat r2xsq =  r2.x * r2.x * b_i_inv;
	cpFloat r2ysq =  r2.y * r2.y * b_i_inv;
	cpFloat r2nxy = -r2.x * r2.y * b_i_inv;
	k11 += r2ysq; k12 += r2nxy;
	k21 += r2nxy; k22 += r2xsq;

	// invert
	cpFloat det = k11*k22 - k12*k21;
	cpAssertSoft(det != 0.0, "Unsolvable constraint.");

	cpFloat det_inv = 1.0f/det;
	return cpMat2x2New(
		 k22*det_inv, -k12*det_inv,
		-k21*det_inv,  k11*det_inv
 	);
}

static inline cpFloat
bias_coef(cpFloat errorBias, cpFloat dt)
{
	return 1.0f - cpfpow(errorBias, dt);
}

//MARK: Spaces

typedef struct cpContactBufferHeader cpContactBufferHeader;
typedef void (*cpSpaceArbiterApplyImpulseFunc)(cpArbiter *arb);

struct cpSpace {
	int iterations;

	cpVect gravity;
	cpFloat damping;

	cpFloat idleSpeedThreshold;
	cpFloat sleepTimeThreshold;

	cpFloat collisionSlop;
	cpFloat collisionBias;
	cpTimestamp collisionPersistence;

	cpDataPointer userData;

	cpTimestamp stamp;
	cpFloat curr_dt;

	cpArray *dynamicBodies;
	cpArray *staticBodies;
	cpArray *rousedBodies;
	cpArray *sleepingComponents;

	cpHashValue shapeIDCounter;
	cpSpatialIndex *staticShapes;
	cpSpatialIndex *dynamicShapes;

	cpArray *constraints;

	cpArray *arbiters;
	cpContactBufferHeader *contactBuffersHead;
	cpHashSet *cachedArbiters;
	cpArray *pooledArbiters;

	cpArray *allocatedBuffers;
	unsigned int locked;

	cpBool usesWildcards;
	cpHashSet *collisionHandlers;
	cpCollisionHandler defaultHandler;

	cpBool skipPostStep;
	cpArray *postStepCallbacks;

	cpBody *staticBody;
	cpBody _staticBody;
};

#define cpAssertSpaceUnlocked(space) \
	cpAssertHard(!space->locked, \
		"This operation cannot be done safely during a call to cpSpaceStep() or during a query. " \
		"Put these calls into a post-step callback." \
	);

void cpSpaceSetStaticBody(cpSpace *space, cpBody *body);

extern cpCollisionHandler cpCollisionHandlerDoNothing;

void cpSpaceProcessComponents(cpSpace *space, cpFloat dt);

void cpSpacePushFreshContactBuffer(cpSpace *space);
struct cpContact *cpContactBufferGetArray(cpSpace *space);
void cpSpacePushContacts(cpSpace *space, int count);

typedef struct cpPostStepCallback {
	cpPostStepFunc func;
	void *key;
	void *data;
} cpPostStepCallback;

cpPostStepCallback *cpSpaceGetPostStepCallback(cpSpace *space, void *key);

cpBool cpSpaceArbiterSetFilter(cpArbiter *arb, cpSpace *space);
void cpSpaceFilterArbiters(cpSpace *space, cpBody *body, cpShape *filter);

void cpSpaceActivateBody(cpSpace *space, cpBody *body);
void cpSpaceLock(cpSpace *space);
void cpSpaceUnlock(cpSpace *space, cpBool runPostStep);

static inline void
cpSpaceUncacheArbiter(cpSpace *space, cpArbiter *arb)
{
	const cpShape *a = arb->a, *b = arb->b;
	const cpShape *shape_pair[] = {a, b};
	cpHashValue arbHashID = CP_HASH_PAIR((cpHashValue)(size_t)a, (cpHashValue)(size_t)b);
	cpHashSetRemove(space->cachedArbiters, arbHashID, shape_pair);
	cpArrayDeleteObj(space->arbiters, arb);
}

static inline cpArray *
cpSpaceArrayForBodyType(cpSpace *space, cpBodyType type)
{
	return (type == CP_BODY_TYPE_STATIC ? space->staticBodies : space->dynamicBodies);
}

void cpShapeUpdateFunc(cpShape *shape, void *unused);
cpCollisionID cpSpaceCollideShapes(cpShape *a, cpShape *b, cpCollisionID id, cpSpace *space);

//MARK: Foreach loops

static inline cpConstraint *
cpConstraintNext(cpConstraint *node, cpBody *body)
{
	return (node->a == body ? node->next_a : node->next_b);
}

#define CP_BODY_FOREACH_CONSTRAINT(bdy, var)\
	for(cpConstraint *var = bdy->constraintList; var; var = cpConstraintNext(var, bdy))

static inline cpArbiter *
cpArbiterNext(cpArbiter *node, cpBody *body)
{
	return (node->body_a == body ? node->thread_a.next : node->thread_b.next);
}

#define CP_BODY_FOREACH_ARBITER(bdy, var)\
	for(cpArbiter *var = bdy->arbiterList; var; var = cpArbiterNext(var, bdy))

#define CP_BODY_FOREACH_SHAPE(body, var)\
	for(cpShape *var = body->shapeList; var; var = var->next)

#define CP_BODY_FOREACH_COMPONENT(root, var)\
	for(cpBody *var = root; var; var = var->sleeping.next)

#endif

//
//MERGED FILE END: chipmunk_private.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: chipmunk_unsafe.h
//

/* This header defines a number of "unsafe" operations on Chipmunk objects.
 * In this case "unsafe" is referring to operations which may reduce the
 * physical accuracy or numerical stability of the simulation, but will not
 * cause crashes.
 *
 * The prime example is mutating collision shapes. Chipmunk does not support
 * this directly. Mutating shapes using this API will caused objects in contact
 * to be pushed apart using Chipmunk's overlap solver, but not using real
 * persistent velocities. Probably not what you meant, but perhaps close enough.
 */

/// @defgroup unsafe Chipmunk Unsafe Shape Operations
/// These functions are used for mutating collision shapes.
/// Chipmunk does not have any way to get velocity information on changing shapes,
/// so the results will be unrealistic. You must explicity include the chipmunk_unsafe.h header to use them.
/// @{

#ifndef CHIPMUNK_UNSAFE_H
#define CHIPMUNK_UNSAFE_H

#ifdef __cplusplus
extern "C" {
#endif

/// Set the radius of a circle shape.
CP_EXPORT void cpCircleShapeSetRadius(cpShape *shape, cpFloat radius);
/// Set the offset of a circle shape.
CP_EXPORT void cpCircleShapeSetOffset(cpShape *shape, cpVect offset);

/// Set the endpoints of a segment shape.
CP_EXPORT void cpSegmentShapeSetEndpoints(cpShape *shape, cpVect a, cpVect b);
/// Set the radius of a segment shape.
CP_EXPORT void cpSegmentShapeSetRadius(cpShape *shape, cpFloat radius);

/// Set the vertexes of a poly shape.
CP_EXPORT void cpPolyShapeSetVerts(cpShape *shape, int count, cpVect *verts, cpTransform transform);
CP_EXPORT void cpPolyShapeSetVertsRaw(cpShape *shape, int count, cpVect *verts);
/// Set the radius of a poly shape.
CP_EXPORT void cpPolyShapeSetRadius(cpShape *shape, cpFloat radius);

#ifdef __cplusplus
}
#endif
#endif
/// @}

//
//MERGED FILE END: chipmunk_unsafe.h
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpDecompose.h
//

void cpPolyShapeReverse(cpVect *verts, int numVerts);
int cpPolyShapeMergeParallelEdges(cpVect *verts, int numVerts, cpFloat parallelTolerance = MAGIC_EPSILON, cpFloat distanceTolerance = 0);
int cpDecomposeConvexInto(const cpVect *verts, int numVerts, cpSpace *space, cpBody *body, cpFloat radius);

//
//MERGED FILE END: cpDecompose.h
//------------------------------------------------------------------------------------------------------------------------------------------------------
