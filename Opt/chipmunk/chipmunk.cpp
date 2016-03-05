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
// (USED IN FILES: cpDecompose.cpp)
//------------------------------------------------------------------------------

#include "chipmunk.h"

#if defined(_MSC_VER)
#pragma warning(disable:4307)
#endif

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: chipmunk.c
//

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void
cpMessage(const char *condition, const char *file, int line, int isError, int isHardError, const char *message, ...)
{
	fprintf(stderr, (isError ? "Aborting due to Chipmunk error: " : "Chipmunk warning: "));

	va_list vargs;
	va_start(vargs, message); {
		vfprintf(stderr, message, vargs);
		fprintf(stderr, "\n");
	} va_end(vargs);

	fprintf(stderr, "\tFailed condition: %s\n", condition);
	fprintf(stderr, "\tSource:%s:%d\n", file, line);
}

#define STR(sss) #sss
#define XSTR(sss) STR(sss)

const char *cpVersionString = XSTR(CP_VERSION_MAJOR) "." XSTR(CP_VERSION_MINOR) "." XSTR(CP_VERSION_RELEASE);

cpFloat
cpMomentForCircle(cpFloat m, cpFloat r1, cpFloat r2, cpVect offset)
{
	return m*(0.5f*(r1*r1 + r2*r2) + cpvlengthsq(offset));
}

cpFloat
cpAreaForCircle(cpFloat r1, cpFloat r2)
{
	return (cpFloat)CP_PI*cpfabs(r1*r1 - r2*r2);
}

cpFloat
cpMomentForSegment(cpFloat m, cpVect a, cpVect b, cpFloat r)
{
	cpVect offset = cpvlerp(a, b, 0.5f);

	cpFloat length = cpvdist(b, a) + 2.0f*r;
	return m*((length*length + 4.0f*r*r)/12.0f + cpvlengthsq(offset));
}

cpFloat
cpAreaForSegment(cpVect a, cpVect b, cpFloat r)
{
	return r*((cpFloat)CP_PI*r + 2.0f*cpvdist(a, b));
}

cpFloat
cpMomentForPoly(cpFloat m, const int count, const cpVect *verts, cpVect offset, cpFloat r)
{
	if(count == 2) return cpMomentForSegment(m, verts[0], verts[1], 0.0f);

	cpFloat sum1 = 0.0f;
	cpFloat sum2 = 0.0f;
	for(int i=0; i<count; i++){
		cpVect v1 = cpvadd(verts[i], offset);
		cpVect v2 = cpvadd(verts[(i+1)%count], offset);

		cpFloat a = cpvcross(v2, v1);
		cpFloat b = cpvdot(v1, v1) + cpvdot(v1, v2) + cpvdot(v2, v2);

		sum1 += a*b;
		sum2 += a;
	}

	return (m*sum1)/(6.0f*sum2);
}

cpFloat
cpAreaForPoly(const int count, const cpVect *verts, cpFloat r)
{
	cpFloat area = 0.0f;
	cpFloat perimeter = 0.0f;
	for(int i=0; i<count; i++){
		cpVect v1 = verts[i];
		cpVect v2 = verts[(i+1)%count];

		area += cpvcross(v1, v2);
		perimeter += cpvdist(v1, v2);
	}

	return r*(CP_PI*cpfabs(r) + perimeter) + area/2.0f;
}

cpVect
cpCentroidForPoly(const int count, const cpVect *verts)
{
	cpFloat sum = 0.0f;
	cpVect vsum = cpvzero;

	for(int i=0; i<count; i++){
		cpVect v1 = verts[i];
		cpVect v2 = verts[(i+1)%count];
		cpFloat cross = cpvcross(v1, v2);

		sum += cross;
		vsum = cpvadd(vsum, cpvmult(cpvadd(v1, v2), cross));
	}

	return cpvmult(vsum, 1.0f/(3.0f*sum));
}

cpFloat
cpMomentForBox(cpFloat m, cpFloat width, cpFloat height)
{
	return m*(width*width + height*height)/12.0f;
}

cpFloat
cpMomentForBox2(cpFloat m, cpBB box)
{
	cpFloat width = box.r - box.l;
	cpFloat height = box.t - box.b;
	cpVect offset = cpvmult(cpv(box.l + box.r, box.b + box.t), 0.5f);

	return cpMomentForBox(m, width, height) + m*cpvlengthsq(offset);
}

void
cpLoopIndexes(const cpVect *verts, int count, int *start, int *end)
{
	(*start) = (*end) = 0;
	cpVect min = verts[0];
	cpVect max = min;

  for(int i=1; i<count; i++){
    cpVect v = verts[i];

    if(v.x < min.x || (v.x == min.x && v.y < min.y)){
      min = v;
      (*start) = i;
    } else if(v.x > max.x || (v.x == max.x && v.y > max.y)){
			max = v;
			(*end) = i;
		}
	}
}

#define SWAP(__A__, __B__) {cpVect __TMP__ = __A__; __A__ = __B__; __B__ = __TMP__;}

static int
QHullPartition(cpVect *verts, int count, cpVect a, cpVect b, cpFloat tol)
{
	if(count == 0) return 0;

	cpFloat max = 0;
	int pivot = 0;

	cpVect delta = cpvsub(b, a);
	cpFloat valueTol = tol*cpvlength(delta);

	int head = 0;
	for(int tail = count-1; head <= tail;){
		cpFloat value = cpvcross(cpvsub(verts[head], a), delta);
		if(value > valueTol){
			if(value > max){
				max = value;
				pivot = head;
			}

			head++;
		} else {
			SWAP(verts[head], verts[tail]);
			tail--;
		}
	}

	if(pivot != 0) SWAP(verts[0], verts[pivot]);
	return head;
}

static int
QHullReduce(cpFloat tol, cpVect *verts, int count, cpVect a, cpVect pivot, cpVect b, cpVect *result)
{
	if(count < 0){
		return 0;
	} else if(count == 0) {
		result[0] = pivot;
		return 1;
	} else {
		int left_count = QHullPartition(verts, count, a, pivot, tol);
		int index = QHullReduce(tol, verts + 1, left_count - 1, a, verts[0], pivot, result);

		result[index++] = pivot;

		int right_count = QHullPartition(verts + left_count, count - left_count, pivot, b, tol);
		return index + QHullReduce(tol, verts + left_count + 1, right_count - 1, pivot, verts[left_count], b, result + index);
	}
}

int
cpConvexHull(int count, const cpVect *verts, cpVect *result, int *first, cpFloat tol)
{
	if(verts != result){
		memcpy(result, verts, count*sizeof(cpVect));
	}

	int start, end;
	cpLoopIndexes(verts, count, &start, &end);
	if(start == end){
		if(first) (*first) = 0;
		return 1;
	}

	SWAP(result[0], result[start]);
	SWAP(result[1], result[end == 0 ? start : end]);

	cpVect a = result[0];
	cpVect b = result[1];

	if(first) (*first) = start;
	return QHullReduce(tol, result + 2, count - 2, a, b, a, result + 1) + 1;
}

#if defined(__has_extension)
#if __has_extension(blocks)

static void IteratorFunc(void *ptr, void (^block)(void *ptr)){block(ptr);}

void cpSpaceEachBody_b(cpSpace *space, void (^block)(cpBody *body)){
	cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)IteratorFunc, block);
}

void cpSpaceEachShape_b(cpSpace *space, void (^block)(cpShape *shape)){
	cpSpaceEachShape(space, (cpSpaceShapeIteratorFunc)IteratorFunc, block);
}

void cpSpaceEachConstraint_b(cpSpace *space, void (^block)(cpConstraint *constraint)){
	cpSpaceEachConstraint(space, (cpSpaceConstraintIteratorFunc)IteratorFunc, block);
}

static void BodyIteratorFunc(cpBody *body, void *ptr, void (^block)(void *ptr)){block(ptr);}

void cpBodyEachShape_b(cpBody *body, void (^block)(cpShape *shape)){
	cpBodyEachShape(body, (cpBodyShapeIteratorFunc)BodyIteratorFunc, block);
}

void cpBodyEachConstraint_b(cpBody *body, void (^block)(cpConstraint *constraint)){
	cpBodyEachConstraint(body, (cpBodyConstraintIteratorFunc)BodyIteratorFunc, block);
}

void cpBodyEachArbiter_b(cpBody *body, void (^block)(cpArbiter *arbiter)){
	cpBodyEachArbiter(body, (cpBodyArbiterIteratorFunc)BodyIteratorFunc, block);
}

static void PointQueryIteratorFunc(cpShape *shape, cpVect p, cpFloat d, cpVect g, cpSpacePointQueryBlock block){block(shape, p, d, g);}
void cpSpacePointQuery_b(cpSpace *space, cpVect point, cpFloat maxDistance, cpShapeFilter filter, cpSpacePointQueryBlock block){
	cpSpacePointQuery(space, point, maxDistance, filter, (cpSpacePointQueryFunc)PointQueryIteratorFunc, block);
}

static void SegmentQueryIteratorFunc(cpShape *shape, cpVect p, cpVect n, cpFloat t, cpSpaceSegmentQueryBlock block){block(shape, p, n, t);}
void cpSpaceSegmentQuery_b(cpSpace *space, cpVect start, cpVect end, cpFloat radius, cpShapeFilter filter, cpSpaceSegmentQueryBlock block){
	cpSpaceSegmentQuery(space, start, end, radius, filter, (cpSpaceSegmentQueryFunc)SegmentQueryIteratorFunc, block);
}

void cpSpaceBBQuery_b(cpSpace *space, cpBB bb, cpShapeFilter filter, cpSpaceBBQueryBlock block){
	cpSpaceBBQuery(space, bb, filter, (cpSpaceBBQueryFunc)IteratorFunc, block);
}

static void ShapeQueryIteratorFunc(cpShape *shape, cpContactPointSet *points, cpSpaceShapeQueryBlock block){block(shape, points);}
cpBool cpSpaceShapeQuery_b(cpSpace *space, cpShape *shape, cpSpaceShapeQueryBlock block){
	return cpSpaceShapeQuery(space, shape, (cpSpaceShapeQueryFunc)ShapeQueryIteratorFunc, block);
}

#endif
#endif

#ifdef CHIPMUNK_FFI

#ifdef _MSC_VER
 #if _MSC_VER >= 1600
  #define MAKE_REF(name) CP_EXPORT decltype(name) *_##name = name
 #else
  #define MAKE_REF(name)
 #endif
#else
 #define MAKE_REF(name) __typeof__(name) *_##name = name
#endif

#ifdef __cplusplus
extern "C" {
#endif

MAKE_REF(cpv);MAKE_REF(cpveql);
MAKE_REF(cpvadd);
MAKE_REF(cpvneg);
MAKE_REF(cpvsub);
MAKE_REF(cpvmult);
MAKE_REF(cpvdot);
MAKE_REF(cpvcross);
MAKE_REF(cpvperp);
MAKE_REF(cpvrperp);
MAKE_REF(cpvproject);
MAKE_REF(cpvforangle);
MAKE_REF(cpvtoangle);
MAKE_REF(cpvrotate);
MAKE_REF(cpvunrotate);
MAKE_REF(cpvlengthsq);
MAKE_REF(cpvlength);
MAKE_REF(cpvlerp);
MAKE_REF(cpvnormalize);
MAKE_REF(cpvclamp);
MAKE_REF(cpvlerpconst);
MAKE_REF(cpvdist);
MAKE_REF(cpvdistsq);
MAKE_REF(cpvnear);

MAKE_REF(cpfmax);
MAKE_REF(cpfmin);
MAKE_REF(cpfabs);
MAKE_REF(cpfclamp);
MAKE_REF(cpflerp);
MAKE_REF(cpflerpconst);

MAKE_REF(cpBBNew);
MAKE_REF(cpBBNewForExtents);
MAKE_REF(cpBBNewForCircle);
MAKE_REF(cpBBIntersects);
MAKE_REF(cpBBContainsBB);
MAKE_REF(cpBBContainsVect);
MAKE_REF(cpBBMerge);
MAKE_REF(cpBBExpand);
MAKE_REF(cpBBCenter);
MAKE_REF(cpBBArea);
MAKE_REF(cpBBMergedArea);
MAKE_REF(cpBBSegmentQuery);
MAKE_REF(cpBBIntersectsSegment);
MAKE_REF(cpBBClampVect);

MAKE_REF(cpSpatialIndexDestroy);
MAKE_REF(cpSpatialIndexCount);
MAKE_REF(cpSpatialIndexEach);
MAKE_REF(cpSpatialIndexContains);
MAKE_REF(cpSpatialIndexInsert);
MAKE_REF(cpSpatialIndexRemove);
MAKE_REF(cpSpatialIndexReindex);
MAKE_REF(cpSpatialIndexReindexObject);
MAKE_REF(cpSpatialIndexSegmentQuery);
MAKE_REF(cpSpatialIndexQuery);
MAKE_REF(cpSpatialIndexReindexQuery);

#ifdef __cplusplus
}
#endif

#endif

//
//MERGED FILE END: chipmunk.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpArbiter.c
//

static inline void
unthreadHelper(cpArbiter *arb, cpBody *body)
{
	struct cpArbiterThread *thread = cpArbiterThreadForBody(arb, body);
	cpArbiter *prev = thread->prev;
	cpArbiter *next = thread->next;

	if(prev){
		cpArbiterThreadForBody(prev, body)->next = next;
	} else if(body->arbiterList == arb) {
		body->arbiterList = next;
	}

	if(next) cpArbiterThreadForBody(next, body)->prev = prev;

	thread->prev = NULL;
	thread->next = NULL;
}

void
cpArbiterUnthread(cpArbiter *arb)
{
	unthreadHelper(arb, arb->body_a);
	unthreadHelper(arb, arb->body_b);
}

cpBool cpArbiterIsFirstContact(const cpArbiter *arb)
{
	return arb->state == CP_ARBITER_STATE_FIRST_COLLISION;
}

cpBool cpArbiterIsRemoval(const cpArbiter *arb)
{
	return arb->state == CP_ARBITER_STATE_INVALIDATED;
}

int cpArbiterGetCount(const cpArbiter *arb)
{
	return (arb->state < CP_ARBITER_STATE_CACHED ? arb->count : 0);
}

cpVect
cpArbiterGetNormal(const cpArbiter *arb)
{
	return cpvmult(arb->n, arb->swapped ? -1.0f : 1.0f);
}

cpVect
cpArbiterGetPointA(const cpArbiter *arb, int i)
{
	cpAssertHard(0 <= i && i < cpArbiterGetCount(arb), "Index error: The specified contact index is invalid for this arbiter");
	return cpvadd(arb->body_a->p, arb->contacts[i].r1);
}

cpVect
cpArbiterGetPointB(const cpArbiter *arb, int i)
{
	cpAssertHard(0 <= i && i < cpArbiterGetCount(arb), "Index error: The specified contact index is invalid for this arbiter");
	return cpvadd(arb->body_b->p, arb->contacts[i].r2);
}

cpFloat
cpArbiterGetDepth(const cpArbiter *arb, int i)
{
	cpAssertHard(0 <= i && i < cpArbiterGetCount(arb), "Index error: The specified contact index is invalid for this arbiter");

	struct cpContact *con = &arb->contacts[i];
	return cpvdot(cpvadd(cpvsub(con->r2, con->r1), cpvsub(arb->body_b->p, arb->body_a->p)), arb->n);
}

cpContactPointSet
cpArbiterGetContactPointSet(const cpArbiter *arb)
{
	cpContactPointSet set;
	set.count = cpArbiterGetCount(arb);

	cpBool swapped = arb->swapped;
	cpVect n = arb->n;
	set.normal = (swapped ? cpvneg(n) : n);

	for(int i=0; i<set.count; i++){
		cpVect p1 = cpvadd(arb->body_a->p, arb->contacts[i].r1);
		cpVect p2 = cpvadd(arb->body_b->p, arb->contacts[i].r2);

		set.points[i].pointA = (swapped ? p2 : p1);
		set.points[i].pointB = (swapped ? p1 : p2);
		set.points[i].distance = cpvdot(cpvsub(p2, p1), n);
	}

	return set;
}

void
cpArbiterSetContactPointSet(cpArbiter *arb, cpContactPointSet *set)
{
	int count = set->count;
	cpAssertHard(count == arb->count, "The number of contact points cannot be changed.");

	cpBool swapped = arb->swapped;
	arb->n = (swapped ? cpvneg(set->normal) : set->normal);

	for(int i=0; i<count; i++){
		cpVect p1 = set->points[i].pointA;
		cpVect p2 = set->points[i].pointB;

		arb->contacts[i].r1 = cpvsub(swapped ? p2 : p1, arb->body_a->p);
		arb->contacts[i].r2 = cpvsub(swapped ? p1 : p2, arb->body_b->p);
	}
}

cpVect
cpArbiterTotalImpulse(const cpArbiter *arb)
{
	struct cpContact *contacts = arb->contacts;
	cpVect n = arb->n;
	cpVect sum = cpvzero;

	for(int i=0, count=cpArbiterGetCount(arb); i<count; i++){
		struct cpContact *con = &contacts[i];
		sum = cpvadd(sum, cpvrotate(n, cpv(con->jnAcc, con->jtAcc)));
	}

	return (arb->swapped ? sum : cpvneg(sum));
	return cpvzero;
}

cpFloat
cpArbiterTotalKE(const cpArbiter *arb)
{
	cpFloat eCoef = (1 - arb->e)/(1 + arb->e);
	cpFloat sum = 0.0;

	struct cpContact *contacts = arb->contacts;
	for(int i=0, count=cpArbiterGetCount(arb); i<count; i++){
		struct cpContact *con = &contacts[i];
		cpFloat jnAcc = con->jnAcc;
		cpFloat jtAcc = con->jtAcc;

		sum += eCoef*jnAcc*jnAcc/con->nMass + jtAcc*jtAcc/con->tMass;
	}

	return sum;
}

cpBool
cpArbiterIgnore(cpArbiter *arb)
{
	arb->state = CP_ARBITER_STATE_IGNORE;
	return cpFalse;
}

cpFloat
cpArbiterGetRestitution(const cpArbiter *arb)
{
	return arb->e;
}

void
cpArbiterSetRestitution(cpArbiter *arb, cpFloat restitution)
{
	arb->e = restitution;
}

cpFloat
cpArbiterGetFriction(const cpArbiter *arb)
{
	return arb->u;
}

void
cpArbiterSetFriction(cpArbiter *arb, cpFloat friction)
{
	arb->u = friction;
}

cpVect
cpArbiterGetSurfaceVelocity(cpArbiter *arb)
{
	return cpvmult(arb->surface_vr, arb->swapped ? -1.0f : 1.0f);
}

void
cpArbiterSetSurfaceVelocity(cpArbiter *arb, cpVect vr)
{
	arb->surface_vr = cpvmult(vr, arb->swapped ? -1.0f : 1.0f);
}

cpDataPointer
cpArbiterGetUserData(const cpArbiter *arb)
{
	return arb->data;
}

void
cpArbiterSetUserData(cpArbiter *arb, cpDataPointer userData)
{
	arb->data = userData;
}

void
cpArbiterGetShapes(const cpArbiter *arb, cpShape **a, cpShape **b)
{
	if(arb->swapped){
		(*a) = (cpShape *)arb->b, (*b) = (cpShape *)arb->a;
	} else {
		(*a) = (cpShape *)arb->a, (*b) = (cpShape *)arb->b;
	}
}

void cpArbiterGetBodies(const cpArbiter *arb, cpBody **a, cpBody **b)
{
	CP_ARBITER_GET_SHAPES(arb, shape_a, shape_b);
	(*a) = shape_a->body;
	(*b) = shape_b->body;
}

cpBool
cpArbiterCallWildcardBeginA(cpArbiter *arb, cpSpace *space)
{
	cpCollisionHandler *handler = arb->handlerA;
	return handler->beginFunc(arb, space, handler->userData);
}

cpBool
cpArbiterCallWildcardBeginB(cpArbiter *arb, cpSpace *space)
{
	cpCollisionHandler *handler = arb->handlerB;
	arb->swapped = !arb->swapped;
	cpBool retval = handler->beginFunc(arb, space, handler->userData);
	arb->swapped = !arb->swapped;
	return retval;
}

cpBool
cpArbiterCallWildcardPreSolveA(cpArbiter *arb, cpSpace *space)
{
	cpCollisionHandler *handler = arb->handlerA;
	return handler->preSolveFunc(arb, space, handler->userData);
}

cpBool
cpArbiterCallWildcardPreSolveB(cpArbiter *arb, cpSpace *space)
{
	cpCollisionHandler *handler = arb->handlerB;
	arb->swapped = !arb->swapped;
	cpBool retval = handler->preSolveFunc(arb, space, handler->userData);
	arb->swapped = !arb->swapped;
	return retval;
}

void
cpArbiterCallWildcardPostSolveA(cpArbiter *arb, cpSpace *space)
{
	cpCollisionHandler *handler = arb->handlerA;
	handler->postSolveFunc(arb, space, handler->userData);
}

void
cpArbiterCallWildcardPostSolveB(cpArbiter *arb, cpSpace *space)
{
	cpCollisionHandler *handler = arb->handlerB;
	arb->swapped = !arb->swapped;
	handler->postSolveFunc(arb, space, handler->userData);
	arb->swapped = !arb->swapped;
}

void
cpArbiterCallWildcardSeparateA(cpArbiter *arb, cpSpace *space)
{
	cpCollisionHandler *handler = arb->handlerA;
	handler->separateFunc(arb, space, handler->userData);
}

void
cpArbiterCallWildcardSeparateB(cpArbiter *arb, cpSpace *space)
{
	cpCollisionHandler *handler = arb->handlerB;
	arb->swapped = !arb->swapped;
	handler->separateFunc(arb, space, handler->userData);
	arb->swapped = !arb->swapped;
}

cpArbiter*
cpArbiterInit(cpArbiter *arb, cpShape *a, cpShape *b)
{
	arb->handler = NULL;
	arb->swapped = cpFalse;

	arb->handler = NULL;
	arb->handlerA = NULL;
	arb->handlerB = NULL;

	arb->e = 0.0f;
	arb->u = 0.0f;
	arb->surface_vr = cpvzero;

	arb->count = 0;
	arb->contacts = NULL;

	arb->a = a; arb->body_a = a->body;
	arb->b = b; arb->body_b = b->body;

	arb->thread_a.next = NULL;
	arb->thread_b.next = NULL;
	arb->thread_a.prev = NULL;
	arb->thread_b.prev = NULL;

	arb->stamp = 0;
	arb->state = CP_ARBITER_STATE_FIRST_COLLISION;

	arb->data = NULL;

	return arb;
}

static inline cpCollisionHandler *
cpSpaceLookupHandler(cpSpace *space, cpCollisionType a, cpCollisionType b, cpCollisionHandler *defaultValue)
{
	cpCollisionType types[] = {a, b};
	cpCollisionHandler *handler = (cpCollisionHandler *)cpHashSetFind(space->collisionHandlers, CP_HASH_PAIR(a, b), types);
	return (handler ? handler : defaultValue);
}

void
cpArbiterUpdate(cpArbiter *arb, struct cpCollisionInfo *info, cpSpace *space)
{
	const cpShape *a = info->a, *b = info->b;

	arb->a = a; arb->body_a = a->body;
	arb->b = b; arb->body_b = b->body;

	for(int i=0; i<info->count; i++){
		struct cpContact *con = &info->arr[i];

		con->r1 = cpvsub(con->r1, a->body->p);
		con->r2 = cpvsub(con->r2, b->body->p);

		con->jnAcc = con->jtAcc = 0.0f;

		for(int j=0; j<arb->count; j++){
			struct cpContact *old = &arb->contacts[j];

			if(con->hash == old->hash){
				con->jnAcc = old->jnAcc;
				con->jtAcc = old->jtAcc;
			}
		}
	}

	arb->contacts = info->arr;
	arb->count = info->count;
	arb->n = info->n;

	arb->e = a->e * b->e;
	arb->u = a->u * b->u;

	cpVect surface_vr = cpvsub(b->surfaceV, a->surfaceV);
	arb->surface_vr = cpvsub(surface_vr, cpvmult(info->n, cpvdot(surface_vr, info->n)));

	cpCollisionType typeA = info->a->type, typeB = info->b->type;
	cpCollisionHandler *defaultHandler = &space->defaultHandler;
	cpCollisionHandler *handler = arb->handler = cpSpaceLookupHandler(space, typeA, typeB, defaultHandler);

	cpBool swapped = arb->swapped = (typeA != handler->typeA && handler->typeA != CP_WILDCARD_COLLISION_TYPE);

	if(handler != defaultHandler || space->usesWildcards){
		arb->handlerA = cpSpaceLookupHandler(space, (swapped ? typeB : typeA), CP_WILDCARD_COLLISION_TYPE, &cpCollisionHandlerDoNothing);
		arb->handlerB = cpSpaceLookupHandler(space, (swapped ? typeA : typeB), CP_WILDCARD_COLLISION_TYPE, &cpCollisionHandlerDoNothing);
	}

	if(arb->state == CP_ARBITER_STATE_CACHED) arb->state = CP_ARBITER_STATE_FIRST_COLLISION;
}

void
cpArbiterPreStep(cpArbiter *arb, cpFloat dt, cpFloat slop, cpFloat bias)
{
	cpBody *a = arb->body_a;
	cpBody *b = arb->body_b;
	cpVect n = arb->n;
	cpVect body_delta = cpvsub(b->p, a->p);

	for(int i=0; i<arb->count; i++){
		struct cpContact *con = &arb->contacts[i];

		con->nMass = 1.0f/k_scalar(a, b, con->r1, con->r2, n);
		con->tMass = 1.0f/k_scalar(a, b, con->r1, con->r2, cpvperp(n));

		cpFloat dist = cpvdot(cpvadd(cpvsub(con->r2, con->r1), body_delta), n);
		con->bias = -bias*cpfmin(0.0f, dist + slop)/dt;
		con->jBias = 0.0f;

		con->bounce = normal_relative_velocity(a, b, con->r1, con->r2, n)*arb->e;
	}
}

void
cpArbiterApplyCachedImpulse(cpArbiter *arb, cpFloat dt_coef)
{
	if(cpArbiterIsFirstContact(arb)) return;

	cpBody *a = arb->body_a;
	cpBody *b = arb->body_b;
	cpVect n = arb->n;

	for(int i=0; i<arb->count; i++){
		struct cpContact *con = &arb->contacts[i];
		cpVect j = cpvrotate(n, cpv(con->jnAcc, con->jtAcc));
		apply_impulses(a, b, con->r1, con->r2, cpvmult(j, dt_coef));
	}
}

void
cpArbiterApplyImpulse(cpArbiter *arb)
{
	cpBody *a = arb->body_a;
	cpBody *b = arb->body_b;
	cpVect n = arb->n;
	cpVect surface_vr = arb->surface_vr;
	cpFloat friction = arb->u;

	for(int i=0; i<arb->count; i++){
		struct cpContact *con = &arb->contacts[i];
		cpFloat nMass = con->nMass;
		cpVect r1 = con->r1;
		cpVect r2 = con->r2;

		cpVect vb1 = cpvadd(a->v_bias, cpvmult(cpvperp(r1), a->w_bias));
		cpVect vb2 = cpvadd(b->v_bias, cpvmult(cpvperp(r2), b->w_bias));
		cpVect vr = cpvadd(relative_velocity(a, b, r1, r2), surface_vr);

		cpFloat vbn = cpvdot(cpvsub(vb2, vb1), n);
		cpFloat vrn = cpvdot(vr, n);
		cpFloat vrt = cpvdot(vr, cpvperp(n));

		cpFloat jbn = (con->bias - vbn)*nMass;
		cpFloat jbnOld = con->jBias;
		con->jBias = cpfmax(jbnOld + jbn, 0.0f);

		cpFloat jn = -(con->bounce + vrn)*nMass;
		cpFloat jnOld = con->jnAcc;
		con->jnAcc = cpfmax(jnOld + jn, 0.0f);

		cpFloat jtMax = friction*con->jnAcc;
		cpFloat jt = -vrt*con->tMass;
		cpFloat jtOld = con->jtAcc;
		con->jtAcc = cpfclamp(jtOld + jt, -jtMax, jtMax);

		apply_bias_impulses(a, b, r1, r2, cpvmult(n, con->jBias - jbnOld));
		apply_impulses(a, b, r1, r2, cpvrotate(n, cpv(con->jnAcc - jnOld, con->jtAcc - jtOld)));
	}
}

//
//MERGED FILE END: cpArbiter.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpArray.c
//

#include <string.h>

cpArray *
cpArrayNew(int size)
{
	cpArray *arr = (cpArray *)cpcalloc(1, sizeof(cpArray));

	arr->num = 0;
	arr->max = (size ? size : 4);
	arr->arr = (void **)cpcalloc(arr->max, sizeof(void*));

	return arr;
}

void
cpArrayFree(cpArray *arr)
{
	if(arr){
		cpfree(arr->arr);
		arr->arr = NULL;

		cpfree(arr);
	}
}

void
cpArrayPush(cpArray *arr, void *object)
{
	if(arr->num == arr->max){
		arr->max *= 2;
		arr->arr = (void **)cprealloc(arr->arr, arr->max*sizeof(void*));
	}

	arr->arr[arr->num] = object;
	arr->num++;
}

void *
cpArrayPop(cpArray *arr)
{
	arr->num--;

	void *value = arr->arr[arr->num];
	arr->arr[arr->num] = NULL;

	return value;
}

void
cpArrayDeleteObj(cpArray *arr, void *obj)
{
	for(int i=0; i<arr->num; i++){
		if(arr->arr[i] == obj){
			arr->num--;

			arr->arr[i] = arr->arr[arr->num];
			arr->arr[arr->num] = NULL;

			return;
		}
	}
}

void
cpArrayFreeEach(cpArray *arr, void (freeFunc)(void*))
{
	for(int i=0; i<arr->num; i++) freeFunc(arr->arr[i]);
}

cpBool
cpArrayContains(cpArray *arr, void *ptr)
{
	for(int i=0; i<arr->num; i++)
		if(arr->arr[i] == ptr) return cpTrue;

	return cpFalse;
}

//
//MERGED FILE END: cpArray.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpBBTree.c
//

#include "stdlib.h"
#include "stdio.h"

static inline cpSpatialIndexClass *KlasscpBBTree();

typedef struct Node Node;
typedef struct Pair Pair;

struct cpBBTree {
	cpSpatialIndex spatialIndex;
	cpBBTreeVelocityFunc velocityFunc;

	cpHashSet *leaves;
	Node *root;

	Node *pooledNodes;
	Pair *pooledPairs;
	cpArray *allocatedBuffers;

	cpTimestamp stamp;
};

struct Node {
	void *obj;
	cpBB bb;
	Node *parent;

	union {
		struct { Node *a, *b; } children;

		struct {
			cpTimestamp stamp;
			Pair *pairs;
		} leaf;
	} node;
};

#define A node.children.a
#define B node.children.b
#define STAMP node.leaf.stamp
#define PAIRS node.leaf.pairs

typedef struct Thread {
	Pair *prev;
	Node *leaf;
	Pair *next;
} Thread;

struct Pair {
	Thread a, b;
	cpCollisionID id;
};

static inline cpBB
GetBB(cpBBTree *tree, void *obj)
{
	cpBB bb = tree->spatialIndex.bbfunc(obj);

	cpBBTreeVelocityFunc velocityFunc = tree->velocityFunc;
	if(velocityFunc){
		cpFloat coef = 0.1f;
		cpFloat x = (bb.r - bb.l)*coef;
		cpFloat y = (bb.t - bb.b)*coef;

		cpVect v = cpvmult(velocityFunc(obj), 0.1f);
		return cpBBNew(bb.l + cpfmin(-x, v.x), bb.b + cpfmin(-y, v.y), bb.r + cpfmax(x, v.x), bb.t + cpfmax(y, v.y));
	} else {
		return bb;
	}
}

static inline cpBBTree *
GetTree(cpSpatialIndex *index)
{
	return (index && index->klass == KlasscpBBTree() ? (cpBBTree *)index : NULL);
}

static inline Node *
GetRootIfTree(cpSpatialIndex *index){
	return (index && index->klass == KlasscpBBTree() ? ((cpBBTree *)index)->root : NULL);
}

static inline cpBBTree *
GetMasterTree(cpBBTree *tree)
{
	cpBBTree *dynamicTree = GetTree(tree->spatialIndex.dynamicIndex);
	return (dynamicTree ? dynamicTree : tree);
}

static inline void
IncrementStamp(cpBBTree *tree)
{
	cpBBTree *dynamicTree = GetTree(tree->spatialIndex.dynamicIndex);
	if(dynamicTree){
		dynamicTree->stamp++;
	} else {
		tree->stamp++;
	}
}

static void
PairRecycle(cpBBTree *tree, Pair *pair)
{
	tree = GetMasterTree(tree);

	pair->a.next = tree->pooledPairs;
	tree->pooledPairs = pair;
}

static Pair *
PairFromPool(cpBBTree *tree)
{
	tree = GetMasterTree(tree);

	Pair *pair = tree->pooledPairs;

	if(pair){
		tree->pooledPairs = pair->a.next;
		return pair;
	} else {
		int count = CP_BUFFER_BYTES/sizeof(Pair);
		cpAssertHard(count, "Internal Error: Buffer size is too small.");

		Pair *buffer = (Pair *)cpcalloc(1, CP_BUFFER_BYTES);
		cpArrayPush(tree->allocatedBuffers, buffer);

		for(int i=1; i<count; i++) PairRecycle(tree, buffer + i);
		return buffer;
	}
}

static inline void
ThreadUnlink(Thread thread)
{
	Pair *next = thread.next;
	Pair *prev = thread.prev;

	if(next){
		if(next->a.leaf == thread.leaf) next->a.prev = prev; else next->b.prev = prev;
	}

	if(prev){
		if(prev->a.leaf == thread.leaf) prev->a.next = next; else prev->b.next = next;
	} else {
		thread.leaf->PAIRS = next;
	}
}

static void
PairsClear(Node *leaf, cpBBTree *tree)
{
	Pair *pair = leaf->PAIRS;
	leaf->PAIRS = NULL;

	while(pair){
		if(pair->a.leaf == leaf){
			Pair *next = pair->a.next;
			ThreadUnlink(pair->b);
			PairRecycle(tree, pair);
			pair = next;
		} else {
			Pair *next = pair->b.next;
			ThreadUnlink(pair->a);
			PairRecycle(tree, pair);
			pair = next;
		}
	}
}

static void
PairInsert(Node *a, Node *b, cpBBTree *tree)
{
	Pair *nextA = a->PAIRS, *nextB = b->PAIRS;
	Pair *pair = PairFromPool(tree);
	Pair temp = {{NULL, a, nextA},{NULL, b, nextB}, 0};

	a->PAIRS = b->PAIRS = pair;
	*pair = temp;

	if(nextA){
		if(nextA->a.leaf == a) nextA->a.prev = pair; else nextA->b.prev = pair;
	}

	if(nextB){
		if(nextB->a.leaf == b) nextB->a.prev = pair; else nextB->b.prev = pair;
	}
}

static void
NodeRecycle(cpBBTree *tree, Node *node)
{
	node->parent = tree->pooledNodes;
	tree->pooledNodes = node;
}

static Node *
NodeFromPool(cpBBTree *tree)
{
	Node *node = tree->pooledNodes;

	if(node){
		tree->pooledNodes = node->parent;
		return node;
	} else {
		int count = CP_BUFFER_BYTES/sizeof(Node);
		cpAssertHard(count, "Internal Error: Buffer size is too small.");

		Node *buffer = (Node *)cpcalloc(1, CP_BUFFER_BYTES);
		cpArrayPush(tree->allocatedBuffers, buffer);

		for(int i=1; i<count; i++) NodeRecycle(tree, buffer + i);
		return buffer;
	}
}

static inline void
NodeSetA(Node *node, Node *value)
{
	node->A = value;
	value->parent = node;
}

static inline void
NodeSetB(Node *node, Node *value)
{
	node->B = value;
	value->parent = node;
}

static Node *
NodeNew(cpBBTree *tree, Node *a, Node *b)
{
	Node *node = NodeFromPool(tree);

	node->obj = NULL;
	node->bb = cpBBMerge(a->bb, b->bb);
	node->parent = NULL;

	NodeSetA(node, a);
	NodeSetB(node, b);

	return node;
}

static inline cpBool
NodeIsLeaf(Node *node)
{
	return (node->obj != NULL);
}

static inline Node *
NodeOther(Node *node, Node *child)
{
	return (node->A == child ? node->B : node->A);
}

static inline void
NodeReplaceChild(Node *parent, Node *child, Node *value, cpBBTree *tree)
{
	cpAssertSoft(!NodeIsLeaf(parent), "Internal Error: Cannot replace child of a leaf.");
	cpAssertSoft(child == parent->A || child == parent->B, "Internal Error: Node is not a child of parent.");

	if(parent->A == child){
		NodeRecycle(tree, parent->A);
		NodeSetA(parent, value);
	} else {
		NodeRecycle(tree, parent->B);
		NodeSetB(parent, value);
	}

	for(Node *node=parent; node; node = node->parent){
		node->bb = cpBBMerge(node->A->bb, node->B->bb);
	}
}

static inline cpFloat
cpBBProximity(cpBB a, cpBB b)
{
	return cpfabs(a.l + a.r - b.l - b.r) + cpfabs(a.b + a.t - b.b - b.t);
}

static Node *
SubtreeInsert(Node *subtree, Node *leaf, cpBBTree *tree)
{
	if(subtree == NULL){
		return leaf;
	} else if(NodeIsLeaf(subtree)){
		return NodeNew(tree, leaf, subtree);
	} else {
		cpFloat cost_a = cpBBArea(subtree->B->bb) + cpBBMergedArea(subtree->A->bb, leaf->bb);
		cpFloat cost_b = cpBBArea(subtree->A->bb) + cpBBMergedArea(subtree->B->bb, leaf->bb);

		if(cost_a == cost_b){
			cost_a = cpBBProximity(subtree->A->bb, leaf->bb);
			cost_b = cpBBProximity(subtree->B->bb, leaf->bb);
		}

		if(cost_b < cost_a){
			NodeSetB(subtree, SubtreeInsert(subtree->B, leaf, tree));
		} else {
			NodeSetA(subtree, SubtreeInsert(subtree->A, leaf, tree));
		}

		subtree->bb = cpBBMerge(subtree->bb, leaf->bb);
		return subtree;
	}
}

static void
SubtreeQuery(Node *subtree, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data)
{
	if(cpBBIntersects(subtree->bb, bb)){
		if(NodeIsLeaf(subtree)){
			func(obj, subtree->obj, 0, data);
		} else {
			SubtreeQuery(subtree->A, obj, bb, func, data);
			SubtreeQuery(subtree->B, obj, bb, func, data);
		}
	}
}

static cpFloat
SubtreeSegmentQuery(Node *subtree, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data)
{
	if(NodeIsLeaf(subtree)){
		return func(obj, subtree->obj, data);
	} else {
		cpFloat t_a = cpBBSegmentQuery(subtree->A->bb, a, b);
		cpFloat t_b = cpBBSegmentQuery(subtree->B->bb, a, b);

		if(t_a < t_b){
			if(t_a < t_exit) t_exit = cpfmin(t_exit, SubtreeSegmentQuery(subtree->A, obj, a, b, t_exit, func, data));
			if(t_b < t_exit) t_exit = cpfmin(t_exit, SubtreeSegmentQuery(subtree->B, obj, a, b, t_exit, func, data));
		} else {
			if(t_b < t_exit) t_exit = cpfmin(t_exit, SubtreeSegmentQuery(subtree->B, obj, a, b, t_exit, func, data));
			if(t_a < t_exit) t_exit = cpfmin(t_exit, SubtreeSegmentQuery(subtree->A, obj, a, b, t_exit, func, data));
		}

		return t_exit;
	}
}

static void
SubtreeRecycle(cpBBTree *tree, Node *node)
{
	if(!NodeIsLeaf(node)){
		SubtreeRecycle(tree, node->A);
		SubtreeRecycle(tree, node->B);
		NodeRecycle(tree, node);
	}
}

static inline Node *
SubtreeRemove(Node *subtree, Node *leaf, cpBBTree *tree)
{
	if(leaf == subtree){
		return NULL;
	} else {
		Node *parent = leaf->parent;
		if(parent == subtree){
			Node *other = NodeOther(subtree, leaf);
			other->parent = subtree->parent;
			NodeRecycle(tree, subtree);
			return other;
		} else {
			NodeReplaceChild(parent->parent, parent, NodeOther(parent, leaf), tree);
			return subtree;
		}
	}
}

typedef struct MarkContext {
	cpBBTree *tree;
	Node *staticRoot;
	cpSpatialIndexQueryFunc func;
	void *data;
} MarkContext;

static void
MarkLeafQuery(Node *subtree, Node *leaf, cpBool left, MarkContext *context)
{
	if(cpBBIntersects(leaf->bb, subtree->bb)){
		if(NodeIsLeaf(subtree)){
			if(left){
				PairInsert(leaf, subtree, context->tree);
			} else {
				if(subtree->STAMP < leaf->STAMP) PairInsert(subtree, leaf, context->tree);
				context->func(leaf->obj, subtree->obj, 0, context->data);
			}
		} else {
			MarkLeafQuery(subtree->A, leaf, left, context);
			MarkLeafQuery(subtree->B, leaf, left, context);
		}
	}
}

static void
MarkLeaf(Node *leaf, MarkContext *context)
{
	cpBBTree *tree = context->tree;
	if(leaf->STAMP == GetMasterTree(tree)->stamp){
		Node *staticRoot = context->staticRoot;
		if(staticRoot) MarkLeafQuery(staticRoot, leaf, cpFalse, context);

		for(Node *node = leaf; node->parent; node = node->parent){
			if(node == node->parent->A){
				MarkLeafQuery(node->parent->B, leaf, cpTrue, context);
			} else {
				MarkLeafQuery(node->parent->A, leaf, cpFalse, context);
			}
		}
	} else {
		Pair *pair = leaf->PAIRS;
		while(pair){
			if(leaf == pair->b.leaf){
				pair->id = context->func(pair->a.leaf->obj, leaf->obj, pair->id, context->data);
				pair = pair->b.next;
			} else {
				pair = pair->a.next;
			}
		}
	}
}

static void
MarkSubtree(Node *subtree, MarkContext *context)
{
	if(NodeIsLeaf(subtree)){
		MarkLeaf(subtree, context);
	} else {
		MarkSubtree(subtree->A, context);
		MarkSubtree(subtree->B, context);	}
}

static Node *
LeafNew(cpBBTree *tree, void *obj, cpBB bb)
{
	Node *node = NodeFromPool(tree);
	node->obj = obj;
	node->bb = GetBB(tree, obj);

	node->parent = NULL;
	node->STAMP = 0;
	node->PAIRS = NULL;

	return node;
}

static cpBool
LeafUpdate(Node *leaf, cpBBTree *tree)
{
	Node *root = tree->root;
	cpBB bb = tree->spatialIndex.bbfunc(leaf->obj);

	if(!cpBBContainsBB(leaf->bb, bb)){
		leaf->bb = GetBB(tree, leaf->obj);

		root = SubtreeRemove(root, leaf, tree);
		tree->root = SubtreeInsert(root, leaf, tree);

		PairsClear(leaf, tree);
		leaf->STAMP = GetMasterTree(tree)->stamp;

		return cpTrue;
	} else {
		return cpFalse;
	}
}

static cpCollisionID VoidQueryFunc(void *obj1, void *obj2, cpCollisionID id, void *data){return id;}

static void
LeafAddPairs(Node *leaf, cpBBTree *tree)
{
	cpSpatialIndex *dynamicIndex = tree->spatialIndex.dynamicIndex;
	if(dynamicIndex){
		Node *dynamicRoot = GetRootIfTree(dynamicIndex);
		if(dynamicRoot){
			cpBBTree *dynamicTree = GetTree(dynamicIndex);
			MarkContext context = {dynamicTree, NULL, NULL, NULL};
			MarkLeafQuery(dynamicRoot, leaf, cpTrue, &context);
		}
	} else {
		Node *staticRoot = GetRootIfTree(tree->spatialIndex.staticIndex);
		MarkContext context = {tree, staticRoot, VoidQueryFunc, NULL};
		MarkLeaf(leaf, &context);
	}
}

cpBBTree *
cpBBTreeAlloc(void)
{
	return (cpBBTree *)cpcalloc(1, sizeof(cpBBTree));
}

static int
leafSetEql(void *obj, Node *node)
{
	return (obj == node->obj);
}

static void *
leafSetTrans(void *obj, cpBBTree *tree)
{
	return LeafNew(tree, obj, tree->spatialIndex.bbfunc(obj));
}

cpSpatialIndex *
cpBBTreeInit(cpBBTree *tree, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	cpSpatialIndexInit((cpSpatialIndex *)tree, KlasscpBBTree(), bbfunc, staticIndex);

	tree->velocityFunc = NULL;

	tree->leaves = cpHashSetNew(0, (cpHashSetEqlFunc)leafSetEql);
	tree->root = NULL;

	tree->pooledNodes = NULL;
	tree->allocatedBuffers = cpArrayNew(0);

	tree->stamp = 0;

	return (cpSpatialIndex *)tree;
}

void
cpBBTreeSetVelocityFunc(cpSpatialIndex *index, cpBBTreeVelocityFunc func)
{
	if(index->klass != KlasscpBBTree()){
		cpAssertWarn(cpFalse, "Ignoring cpBBTreeSetVelocityFunc() call to non-tree spatial index.");
		return;
	}

	((cpBBTree *)index)->velocityFunc = func;
}

cpSpatialIndex *
cpBBTreeNew(cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	return cpBBTreeInit(cpBBTreeAlloc(), bbfunc, staticIndex);
}

static void
cpBBTreeDestroy(cpBBTree *tree)
{
	cpHashSetFree(tree->leaves);

	if(tree->allocatedBuffers) cpArrayFreeEach(tree->allocatedBuffers, cpfree);
	cpArrayFree(tree->allocatedBuffers);
}

static void
cpBBTreeInsert(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	Node *leaf = (Node *)cpHashSetInsert(tree->leaves, hashid, obj, (cpHashSetTransFunc)leafSetTrans, tree);

	Node *root = tree->root;
	tree->root = SubtreeInsert(root, leaf, tree);

	leaf->STAMP = GetMasterTree(tree)->stamp;
	LeafAddPairs(leaf, tree);
	IncrementStamp(tree);
}

static void
cpBBTreeRemove(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	Node *leaf = (Node *)cpHashSetRemove(tree->leaves, hashid, obj);

	tree->root = SubtreeRemove(tree->root, leaf, tree);
	PairsClear(leaf, tree);
	NodeRecycle(tree, leaf);
}

static cpBool
cpBBTreeContains(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	return (cpHashSetFind(tree->leaves, hashid, obj) != NULL);
}

static void LeafUpdateWrap(Node *leaf, cpBBTree *tree) {LeafUpdate(leaf, tree);}

static void
cpBBTreeReindexQuery(cpBBTree *tree, cpSpatialIndexQueryFunc func, void *data)
{
	if(!tree->root) return;

	cpHashSetEach(tree->leaves, (cpHashSetIteratorFunc)LeafUpdateWrap, tree);

	cpSpatialIndex *staticIndex = tree->spatialIndex.staticIndex;
	Node *staticRoot = (staticIndex && staticIndex->klass == KlasscpBBTree() ? ((cpBBTree *)staticIndex)->root : NULL);

	MarkContext context = {tree, staticRoot, func, data};
	MarkSubtree(tree->root, &context);
	if(staticIndex && !staticRoot) cpSpatialIndexCollideStatic((cpSpatialIndex *)tree, staticIndex, func, data);

	IncrementStamp(tree);
}

static void
cpBBTreeReindex(cpBBTree *tree)
{
	cpBBTreeReindexQuery(tree, VoidQueryFunc, NULL);
}

static void
cpBBTreeReindexObject(cpBBTree *tree, void *obj, cpHashValue hashid)
{
	Node *leaf = (Node *)cpHashSetFind(tree->leaves, hashid, obj);
	if(leaf){
		if(LeafUpdate(leaf, tree)) LeafAddPairs(leaf, tree);
		IncrementStamp(tree);
	}
}

static void
cpBBTreeSegmentQuery(cpBBTree *tree, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data)
{
	Node *root = tree->root;
	if(root) SubtreeSegmentQuery(root, obj, a, b, t_exit, func, data);
}

static void
cpBBTreeQuery(cpBBTree *tree, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data)
{
	if(tree->root) SubtreeQuery(tree->root, obj, bb, func, data);
}

static int
cpBBTreeCount(cpBBTree *tree)
{
	return cpHashSetCount(tree->leaves);
}

typedef struct eachContextcpBBTree {
	cpSpatialIndexIteratorFunc func;
	void *data;
} eachContextcpBBTree;

static void each_helper(Node *node, eachContextcpBBTree *context){context->func(node->obj, context->data);}

static void
cpBBTreeEach(cpBBTree *tree, cpSpatialIndexIteratorFunc func, void *data)
{
	eachContextcpBBTree context = {func, data};
	cpHashSetEach(tree->leaves, (cpHashSetIteratorFunc)each_helper, &context);
}

static cpSpatialIndexClass klasscpBBTree = {
	(cpSpatialIndexDestroyImpl)cpBBTreeDestroy,

	(cpSpatialIndexCountImpl)cpBBTreeCount,
	(cpSpatialIndexEachImpl)cpBBTreeEach,

	(cpSpatialIndexContainsImpl)cpBBTreeContains,
	(cpSpatialIndexInsertImpl)cpBBTreeInsert,
	(cpSpatialIndexRemoveImpl)cpBBTreeRemove,

	(cpSpatialIndexReindexImpl)cpBBTreeReindex,
	(cpSpatialIndexReindexObjectImpl)cpBBTreeReindexObject,
	(cpSpatialIndexReindexQueryImpl)cpBBTreeReindexQuery,

	(cpSpatialIndexQueryImpl)cpBBTreeQuery,
	(cpSpatialIndexSegmentQueryImpl)cpBBTreeSegmentQuery,
};

static inline cpSpatialIndexClass *KlasscpBBTree(){return &klasscpBBTree;}

static int
cpfcompare(const cpFloat *a, const cpFloat *b){
	return (*a < *b ? -1 : (*b < *a ? 1 : 0));
}

static void
fillNodeArray(Node *node, Node ***cursor){
	(**cursor) = node;
	(*cursor)++;
}

static Node *
partitionNodes(cpBBTree *tree, Node **nodes, int count)
{
	if(count == 1){
		return nodes[0];
	} else if(count == 2) {
		return NodeNew(tree, nodes[0], nodes[1]);
	}

	cpBB bb = nodes[0]->bb;
	for(int i=1; i<count; i++) bb = cpBBMerge(bb, nodes[i]->bb);

	cpBool splitWidth = (bb.r - bb.l > bb.t - bb.b);

	cpFloat *bounds = (cpFloat *)cpcalloc(count*2, sizeof(cpFloat));
	if(splitWidth){
		for(int i=0; i<count; i++){
			bounds[2*i + 0] = nodes[i]->bb.l;
			bounds[2*i + 1] = nodes[i]->bb.r;
		}
	} else {
		for(int i=0; i<count; i++){
			bounds[2*i + 0] = nodes[i]->bb.b;
			bounds[2*i + 1] = nodes[i]->bb.t;
		}
	}

	qsort(bounds, count*2, sizeof(cpFloat), (int (*)(const void *, const void *))cpfcompare);
	cpFloat split = (bounds[count - 1] + bounds[count])*0.5f;	cpfree(bounds);

	cpBB a = bb, b = bb;
	if(splitWidth) a.r = b.l = split; else a.t = b.b = split;

	int right = count;
	for(int left=0; left < right;){
		Node *node = nodes[left];
		if(cpBBMergedArea(node->bb, b) < cpBBMergedArea(node->bb, a)){
			right--;
			nodes[left] = nodes[right];
			nodes[right] = node;
		} else {
			left++;
		}
	}

	if(right == count){
		Node *node = NULL;
		for(int i=0; i<count; i++) node = SubtreeInsert(node, nodes[i], tree);
		return node;
	}

	return NodeNew(tree,
		partitionNodes(tree, nodes, right),
		partitionNodes(tree, nodes + right, count - right)
	);
}

void
cpBBTreeOptimize(cpSpatialIndex *index)
{
	if(index->klass != &klasscpBBTree){
		cpAssertWarn(cpFalse, "Ignoring cpBBTreeOptimize() call to non-tree spatial index.");
		return;
	}

	cpBBTree *tree = (cpBBTree *)index;
	Node *root = tree->root;
	if(!root) return;

	int count = cpBBTreeCount(tree);
	Node **nodes = (Node **)cpcalloc(count, sizeof(Node *));
	Node **cursor = nodes;

	cpHashSetEach(tree->leaves, (cpHashSetIteratorFunc)fillNodeArray, &cursor);

	SubtreeRecycle(tree, root);
	tree->root = partitionNodes(tree, nodes, count);
	cpfree(nodes);
}

#ifdef CP_BBTREE_DEBUG_DRAW
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#include <GLUT/glut.h>

static void
NodeRender(Node *node, int depth)
{
	if(!NodeIsLeaf(node) && depth <= 10){
		NodeRender(node->a, depth + 1);
		NodeRender(node->b, depth + 1);
	}

	cpBB bb = node->bb;

	glLineWidth(cpfmax(5.0f - depth, 1.0f));
	glBegin(GL_LINES); {
		glVertex2f(bb.l, bb.b);
		glVertex2f(bb.l, bb.t);

		glVertex2f(bb.l, bb.t);
		glVertex2f(bb.r, bb.t);

		glVertex2f(bb.r, bb.t);
		glVertex2f(bb.r, bb.b);

		glVertex2f(bb.r, bb.b);
		glVertex2f(bb.l, bb.b);
	}; glEnd();
}

void
cpBBTreeRenderDebug(cpSpatialIndex *index){
	if(index->klass != &klasscpBBTree){
		cpAssertWarn(cpFalse, "Ignoring cpBBTreeRenderDebug() call to non-tree spatial index.");
		return;
	}

	cpBBTree *tree = (cpBBTree *)index;
	if(tree->root) NodeRender(tree->root, 0);
}
#endif

//
//MERGED FILE END: cpBBTree.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpBody.c
//

#include <float.h>
#include <stdarg.h>

cpBody*
cpBodyAlloc(void)
{
	return (cpBody *)cpcalloc(1, sizeof(cpBody));
}

cpBody *
cpBodyInit(cpBody *body, cpFloat mass, cpFloat moment)
{
	body->space = NULL;
	body->shapeList = NULL;
	body->arbiterList = NULL;
	body->constraintList = NULL;

	body->velocity_func = cpBodyUpdateVelocity;
	body->position_func = cpBodyUpdatePosition;

	body->sleeping.root = NULL;
	body->sleeping.next = NULL;
	body->sleeping.idleTime = 0.0f;

	body->p = cpvzero;
	body->v = cpvzero;
	body->f = cpvzero;

	body->w = 0.0f;
	body->t = 0.0f;

	body->v_bias = cpvzero;
	body->w_bias = 0.0f;

	body->userData = NULL;

	cpBodySetMass(body, mass);
	cpBodySetMoment(body, moment);
	cpBodySetAngle(body, 0.0f);

	return body;
}

cpBody*
cpBodyNew(cpFloat mass, cpFloat moment)
{
	return cpBodyInit(cpBodyAlloc(), mass, moment);
}

cpBody*
cpBodyNewKinematic()
{
	cpBody *body = cpBodyNew(0.0f, 0.0f);
	cpBodySetType(body, CP_BODY_TYPE_KINEMATIC);

	return body;
}

cpBody*
cpBodyNewStatic()
{
	cpBody *body = cpBodyNew(0.0f, 0.0f);
	cpBodySetType(body, CP_BODY_TYPE_STATIC);

	return body;
}

void cpBodyDestroy(cpBody *body){}

void
cpBodyFree(cpBody *body)
{
	if(body){
		cpBodyDestroy(body);
		cpfree(body);
	}
}

#ifdef NDEBUG
	#define	cpAssertSaneBody(body)
#else
	static void cpv_assert_nan(cpVect v, const char *message){cpAssertHard(v.x == v.x && v.y == v.y, message);}
	static void cpv_assert_infinite(cpVect v, const char *message){cpAssertHard(cpfabs(v.x) != INFINITY && cpfabs(v.y) != INFINITY, message);}
	static void cpv_assert_sane(cpVect v, const char *message){cpv_assert_nan(v, message); cpv_assert_infinite(v, message);}

	static void
	cpBodySanityCheck(const cpBody *body)
	{
		cpAssertHard(body->m == body->m && body->m_inv == body->m_inv, "Body's mass is NaN.");
		cpAssertHard(body->i == body->i && body->i_inv == body->i_inv, "Body's moment is NaN.");
		cpAssertHard(body->m >= 0.0f, "Body's mass is negative.");
		cpAssertHard(body->i >= 0.0f, "Body's moment is negative.");

		cpv_assert_sane(body->p, "Body's position is invalid.");
		cpv_assert_sane(body->v, "Body's velocity is invalid.");
		cpv_assert_sane(body->f, "Body's force is invalid.");

		cpAssertHard(body->a == body->a && cpfabs(body->a) != INFINITY, "Body's angle is invalid.");
		cpAssertHard(body->w == body->w && cpfabs(body->w) != INFINITY, "Body's angular velocity is invalid.");
		cpAssertHard(body->t == body->t && cpfabs(body->t) != INFINITY, "Body's torque is invalid.");
	}

	#define	cpAssertSaneBody(body) cpBodySanityCheck(body)
#endif

cpBool
cpBodyIsSleeping(const cpBody *body)
{
	return (body->sleeping.root != ((cpBody*)0));
}

cpBodyType
cpBodyGetType(cpBody *body)
{
	if(body->sleeping.idleTime == INFINITY){
		return CP_BODY_TYPE_STATIC;
	} else if(body->m == INFINITY){
		return CP_BODY_TYPE_KINEMATIC;
	} else {
		return CP_BODY_TYPE_DYNAMIC;
	}
}

void
cpBodySetType(cpBody *body, cpBodyType type)
{
	cpBodyType oldType = cpBodyGetType(body);
	if(oldType == type) return;

	body->sleeping.idleTime = (type == CP_BODY_TYPE_STATIC ? INFINITY : 0.0f);

	if(type == CP_BODY_TYPE_DYNAMIC){
		body->m = body->i = 0.0f;
		body->m_inv = body->i_inv = INFINITY;

		cpBodyAccumulateMassFromShapes(body);
	} else {
		body->m = body->i = INFINITY;
		body->m_inv = body->i_inv = 0.0f;

		body->v = cpvzero;
		body->w = 0.0f;
	}

	cpSpace *space = cpBodyGetSpace(body);
	if(space != NULL){
		cpAssertSpaceUnlocked(space);

		if(oldType == CP_BODY_TYPE_STATIC){
		} else {
			cpBodyActivate(body);
		}

		cpArray *fromArray = cpSpaceArrayForBodyType(space, oldType);
		cpArray *toArray = cpSpaceArrayForBodyType(space, type);
		if(fromArray != toArray){
			cpArrayDeleteObj(fromArray, body);
			cpArrayPush(toArray, body);
		}

		cpSpatialIndex *fromIndex = (oldType == CP_BODY_TYPE_STATIC ? space->staticShapes : space->dynamicShapes);
		cpSpatialIndex *toIndex = (type == CP_BODY_TYPE_STATIC ? space->staticShapes : space->dynamicShapes);
		if(fromIndex != toIndex){
			CP_BODY_FOREACH_SHAPE(body, shape){
				cpSpatialIndexRemove(fromIndex, shape, shape->hashid);
				cpSpatialIndexInsert(toIndex, shape, shape->hashid);
			}
		}
	}
}

void
cpBodyAccumulateMassFromShapes(cpBody *body)
{
	if(body == NULL || cpBodyGetType(body) != CP_BODY_TYPE_DYNAMIC) return;

	body->m = body->i = 0.0f;
	body->cog = cpvzero;

	cpVect pos = cpBodyGetPosition(body);

	CP_BODY_FOREACH_SHAPE(body, shape){
		struct cpShapeMassInfo *info = &shape->massInfo;
		cpFloat m = info->m;

		if(m > 0.0f){
			cpFloat msum = body->m + m;

			body->i += m*info->i + cpvdistsq(body->cog, info->cog)*(m*body->m)/msum;
			body->cog = cpvlerp(body->cog, info->cog, m/msum);
			body->m = msum;
		}
	}

	body->m_inv = 1.0f/body->m;
	body->i_inv = 1.0f/body->i;

	cpBodySetPosition(body, pos);
	cpAssertSaneBody(body);
}

cpSpace *
cpBodyGetSpace(const cpBody *body)
{
	return body->space;
}

cpFloat
cpBodyGetMass(const cpBody *body)
{
	return body->m;
}

void
cpBodySetMass(cpBody *body, cpFloat mass)
{
	cpAssertHard(cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC, "You cannot set the mass of kinematic or static bodies.");
	cpAssertHard(0.0f <= mass && mass < INFINITY, "Mass must be positive and finite.");

	cpBodyActivate(body);
	body->m = mass;
	body->m_inv = 1.0f/mass;
	cpAssertSaneBody(body);
}

cpFloat
cpBodyGetMoment(const cpBody *body)
{
	return body->i;
}

void
cpBodySetMoment(cpBody *body, cpFloat moment)
{
	cpAssertHard(moment >= 0.0f, "Moment of Inertia must be positive.");

	cpBodyActivate(body);
	body->i = moment;
	body->i_inv = 1.0f/moment;
	cpAssertSaneBody(body);
}

cpVect
cpBodyGetRotation(const cpBody *body)
{
	return cpv(body->transform.a, body->transform.b);
}

void
cpBodyAddShape(cpBody *body, cpShape *shape)
{
	cpShape *next = body->shapeList;
	if(next) next->prev = shape;

	shape->next = next;
	body->shapeList = shape;

	if(shape->massInfo.m > 0.0f){
		cpBodyAccumulateMassFromShapes(body);
	}
}

void
cpBodyRemoveShape(cpBody *body, cpShape *shape)
{
  cpShape *prev = shape->prev;
  cpShape *next = shape->next;

  if(prev){
		prev->next = next;
  } else {
		body->shapeList = next;
  }

  if(next){
		next->prev = prev;
	}

  shape->prev = NULL;
  shape->next = NULL;

	if(cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC && shape->massInfo.m > 0.0f){
		cpBodyAccumulateMassFromShapes(body);
	}
}

static cpConstraint *
filterConstraints(cpConstraint *node, cpBody *body, cpConstraint *filter)
{
	if(node == filter){
		return cpConstraintNext(node, body);
	} else if(node->a == body){
		node->next_a = filterConstraints(node->next_a, body, filter);
	} else {
		node->next_b = filterConstraints(node->next_b, body, filter);
	}

	return node;
}

void
cpBodyRemoveConstraint(cpBody *body, cpConstraint *constraint)
{
	body->constraintList = filterConstraints(body->constraintList, body, constraint);
}

static void
SetTransform(cpBody *body, cpVect p, cpFloat a)
{
	cpVect rot = cpvforangle(a);
	cpVect c = body->cog;

	body->transform = cpTransformNewTranspose(
		rot.x, -rot.y, p.x - (c.x*rot.x - c.y*rot.y),
		rot.y,  rot.x, p.y - (c.x*rot.y + c.y*rot.x)
	);
}

static inline cpFloat
SetAngle(cpBody *body, cpFloat a)
{
	body->a = a;
	cpAssertSaneBody(body);

	return a;
}

cpVect
cpBodyGetPosition(const cpBody *body)
{
	return cpTransformPoint(body->transform, cpvzero);
}

void
cpBodySetPosition(cpBody *body, cpVect position)
{
	cpBodyActivate(body);
	cpVect p = body->p = cpvadd(cpTransformVect(body->transform, body->cog), position);
	cpAssertSaneBody(body);

	SetTransform(body, p, body->a);
}

cpVect
cpBodyGetCenterOfGravity(const cpBody *body)
{
	return body->cog;
}

void
cpBodySetCenterOfGravity(cpBody *body, cpVect cog)
{
	cpBodyActivate(body);
	body->cog = cog;
	cpAssertSaneBody(body);
}

cpVect
cpBodyGetVelocity(const cpBody *body)
{
	return body->v;
}

void
cpBodySetVelocity(cpBody *body, cpVect velocity)
{
	cpBodyActivate(body);
	body->v = velocity;
	cpAssertSaneBody(body);
}

cpVect
cpBodyGetForce(const cpBody *body)
{
	return body->f;
}

void
cpBodySetForce(cpBody *body, cpVect force)
{
	cpBodyActivate(body);
	body->f = force;
	cpAssertSaneBody(body);
}

cpFloat
cpBodyGetAngle(const cpBody *body)
{
	return body->a;
}

void
cpBodySetAngle(cpBody *body, cpFloat angle)
{
	cpBodyActivate(body);
	SetAngle(body, angle);

	SetTransform(body, body->p, angle);
}

cpFloat
cpBodyGetAngularVelocity(const cpBody *body)
{
	return body->w;
}

void
cpBodySetAngularVelocity(cpBody *body, cpFloat angularVelocity)
{
	cpBodyActivate(body);
	body->w = angularVelocity;
	cpAssertSaneBody(body);
}

cpFloat
cpBodyGetTorque(const cpBody *body)
{
	return body->t;
}

void
cpBodySetTorque(cpBody *body, cpFloat torque)
{
	cpBodyActivate(body);
	body->t = torque;
	cpAssertSaneBody(body);
}

cpDataPointer
cpBodyGetUserData(const cpBody *body)
{
	return body->userData;
}

void
cpBodySetUserData(cpBody *body, cpDataPointer userData)
{
	body->userData = userData;
}

void
cpBodySetVelocityUpdateFunc(cpBody *body, cpBodyVelocityFunc velocityFunc)
{
	body->velocity_func = velocityFunc;
}

void
cpBodySetPositionUpdateFunc(cpBody *body, cpBodyPositionFunc positionFunc)
{
	body->position_func = positionFunc;
}

void
cpBodyUpdateVelocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
	if(cpBodyGetType(body) == CP_BODY_TYPE_KINEMATIC) return;

	cpAssertSoft3(body->m > 0.0f && body->i > 0.0f, "Body's mass and moment must be positive to simulate. (Mass: %f Moment: %f)", body->m, body->i);

	body->v = cpvadd(cpvmult(body->v, damping), cpvmult(cpvadd(gravity, cpvmult(body->f, body->m_inv)), dt));
	body->w = body->w*damping + body->t*body->i_inv*dt;

	body->f = cpvzero;
	body->t = 0.0f;

	cpAssertSaneBody(body);
}

void
cpBodyUpdatePosition(cpBody *body, cpFloat dt)
{
	cpVect p = body->p = cpvadd(body->p, cpvmult(cpvadd(body->v, body->v_bias), dt));
	cpFloat a = SetAngle(body, body->a + (body->w + body->w_bias)*dt);
	SetTransform(body, p, a);

	body->v_bias = cpvzero;
	body->w_bias = 0.0f;

	cpAssertSaneBody(body);
}

cpVect
cpBodyLocalToWorld(const cpBody *body, const cpVect point)
{
	return cpTransformPoint(body->transform, point);
}

cpVect
cpBodyWorldToLocal(const cpBody *body, const cpVect point)
{
	return cpTransformPoint(cpTransformRigidInverse(body->transform), point);
}

void
cpBodyApplyForceAtWorldPoint(cpBody *body, cpVect force, cpVect point)
{
	cpBodyActivate(body);
	body->f = cpvadd(body->f, force);

	cpVect r = cpvsub(point, cpTransformPoint(body->transform, body->cog));
	body->t += cpvcross(r, force);
}

void
cpBodyApplyForceAtLocalPoint(cpBody *body, cpVect force, cpVect point)
{
	cpBodyApplyForceAtWorldPoint(body, cpTransformVect(body->transform, force), cpTransformPoint(body->transform, point));
}

void
cpBodyApplyImpulseAtWorldPoint(cpBody *body, cpVect impulse, cpVect point)
{
	cpBodyActivate(body);

	cpVect r = cpvsub(point, cpTransformPoint(body->transform, body->cog));
	apply_impulse(body, impulse, r);
}

void
cpBodyApplyImpulseAtLocalPoint(cpBody *body, cpVect impulse, cpVect point)
{
	cpBodyApplyImpulseAtWorldPoint(body, cpTransformVect(body->transform, impulse), cpTransformPoint(body->transform, point));
}

cpVect
cpBodyGetVelocityAtLocalPoint(const cpBody *body, cpVect point)
{
	cpVect r = cpTransformVect(body->transform, cpvsub(point, body->cog));
	return cpvadd(body->v, cpvmult(cpvperp(r), body->w));
}

cpVect
cpBodyGetVelocityAtWorldPoint(const cpBody *body, cpVect point)
{
	cpVect r = cpvsub(point, cpTransformPoint(body->transform, body->cog));
	return cpvadd(body->v, cpvmult(cpvperp(r), body->w));
}

cpFloat
cpBodyKineticEnergy(const cpBody *body)
{
	cpFloat vsq = cpvdot(body->v, body->v);
	cpFloat wsq = body->w*body->w;
	return (vsq ? vsq*body->m : 0.0f) + (wsq ? wsq*body->i : 0.0f);
}

void
cpBodyEachShape(cpBody *body, cpBodyShapeIteratorFunc func, void *data)
{
	cpShape *shape = body->shapeList;
	while(shape){
		cpShape *next = shape->next;
		func(body, shape, data);
		shape = next;
	}
}

void
cpBodyEachConstraint(cpBody *body, cpBodyConstraintIteratorFunc func, void *data)
{
	cpConstraint *constraint = body->constraintList;
	while(constraint){
		cpConstraint *next = cpConstraintNext(constraint, body);
		func(body, constraint, data);
		constraint = next;
	}
}

void
cpBodyEachArbiter(cpBody *body, cpBodyArbiterIteratorFunc func, void *data)
{
	cpArbiter *arb = body->arbiterList;
	while(arb){
		cpArbiter *next = cpArbiterNext(arb, body);

		cpBool swapped = arb->swapped; {
			arb->swapped = (body == arb->body_b);
			func(body, arb, data);
		} arb->swapped = swapped;

		arb = next;
	}
}

//
//MERGED FILE END: cpBody.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpCollision.c
//

#include <stdio.h>
#include <string.h>

cpBool cpCheckSignedArea(const cpVect a, const cpVect b, const cpVect c);

#if DEBUG && 0
#include "ChipmunkDemo.h"
#define DRAW_ALL 0
#define DRAW_GJK (0 || DRAW_ALL)
#define DRAW_EPA (0 || DRAW_ALL)
#define DRAW_CLOSEST (0 || DRAW_ALL)
#define DRAW_CLIP (0 || DRAW_ALL)

#define PRINT_LOG 0
#endif

#define MAX_GJK_ITERATIONS 30
#define MAX_EPA_ITERATIONS 30
#define WARN_GJK_ITERATIONS 20
#define WARN_EPA_ITERATIONS 20

static inline void
cpCollisionInfoPushContact(struct cpCollisionInfo *info, cpVect p1, cpVect p2, cpHashValue hash)
{
	cpAssertSoft(info->count <= CP_MAX_CONTACTS_PER_ARBITER, "Internal error: Tried to push too many contacts.");

	struct cpContact *con = &info->arr[info->count];
	con->r1 = p1;
	con->r2 = p2;
	con->hash = hash;

	info->count++;
}

static inline int
PolySupportPointIndex(const int count, const struct cpSplittingPlane *planes, const cpVect n)
{
	cpFloat max = -INFINITY;
	int index = 0;

	for(int i=0; i<count; i++){
		cpVect v = planes[i].v0;
		cpFloat d = cpvdot(v, n);
		if(d > max){
			max = d;
			index = i;
		}
	}

	return index;
}

struct SupportPoint {
	cpVect p;
	cpCollisionID index;
};

static inline struct SupportPoint
SupportPointNew(cpVect p, cpCollisionID index)
{
	struct SupportPoint point = {{p.x, p.y}, index};
	return point;
}

typedef struct SupportPoint (*SupportPointFunc)(const cpShape *shape, const cpVect n);

static inline struct SupportPoint
CircleSupportPoint(const cpCircleShape *circle, const cpVect n)
{
	return SupportPointNew(circle->tc, 0);
}

static inline struct SupportPoint
SegmentSupportPoint(const cpSegmentShape *seg, const cpVect n)
{
	if(cpvdot(seg->ta, n) > cpvdot(seg->tb, n)){
		return SupportPointNew(seg->ta, 0);
	} else {
		return SupportPointNew(seg->tb, 1);
	}
}

static inline struct SupportPoint
PolySupportPoint(const cpPolyShape *poly, const cpVect n)
{
	const struct cpSplittingPlane *planes = poly->planes;
	int i = PolySupportPointIndex(poly->count, planes, n);
	return SupportPointNew(planes[i].v0, i);
}

struct MinkowskiPoint {
	cpVect a, b;
	cpVect ab;
	cpCollisionID id;
};

static inline struct MinkowskiPoint
MinkowskiPointNew(const struct SupportPoint a, const struct SupportPoint b)
{
	struct MinkowskiPoint point = {{a.p.x, a.p.y}, {b.p.x, b.p.y}, {b.p.x-a.p.x, b.p.y-a.p.y}, (a.index & 0xFF)<<8 | (b.index & 0xFF)};
	return point;
}

struct SupportContext {
	const cpShape *shape1, *shape2;
	SupportPointFunc func1, func2;
};

static inline struct MinkowskiPoint
Support(const struct SupportContext *ctx, const cpVect n)
{
	struct SupportPoint a = ctx->func1(ctx->shape1, cpvneg(n));
	struct SupportPoint b = ctx->func2(ctx->shape2, n);
	return MinkowskiPointNew(a, b);
}

struct EdgePoint {
	cpVect p;
	cpHashValue hash;
};

struct Edge {
	struct EdgePoint a, b;
	cpFloat r;
	cpVect n;
};

static struct Edge
SupportEdgeForPoly(const cpPolyShape *poly, const cpVect n)
{
	int count = poly->count;
	int i1 = PolySupportPointIndex(poly->count, poly->planes, n);

	int i0 = (i1 - 1 + count)%count;
	int i2 = (i1 + 1)%count;

	const struct cpSplittingPlane *planes = poly->planes;
	cpHashValue hashid = poly->shape.hashid;
	if(cpvdot(n, planes[i1].n) > cpvdot(n, planes[i2].n)){
		struct Edge edge = {{{planes[i0].v0.x, planes[i0].v0.y}, CP_HASH_PAIR(hashid, i0)}, {{planes[i1].v0.x, planes[i1].v0.y}, CP_HASH_PAIR(hashid, i1)}, poly->r, {planes[i1].n.x, planes[i1].n.y}};
		return edge;
	} else {
		struct Edge edge = {{{planes[i1].v0.x, planes[i1].v0.y}, CP_HASH_PAIR(hashid, i1)}, {{planes[i2].v0.x, planes[i2].v0.y}, CP_HASH_PAIR(hashid, i2)}, poly->r, {planes[i2].n.x, planes[i2].n.y}};
		return edge;
	}
}

static struct Edge
SupportEdgeForSegment(const cpSegmentShape *seg, const cpVect n)
{
	cpHashValue hashid = seg->shape.hashid;
	if(cpvdot(seg->tn, n) > 0.0){
		struct Edge edge = {{{seg->ta.x, seg->ta.y}, CP_HASH_PAIR(hashid, 0)}, {{seg->tb.x, seg->tb.y}, CP_HASH_PAIR(hashid, 1)}, seg->r, {seg->tn.x, seg->tn.y}};
		return edge;
	} else {
		struct Edge edge = {{{seg->tb.x, seg->tb.y}, CP_HASH_PAIR(hashid, 1)}, {{seg->ta.x, seg->ta.y}, CP_HASH_PAIR(hashid, 0)}, seg->r, {-seg->tn.x, -seg->tn.y}};
		return edge;
	}
}

static inline cpFloat
ClosestT(const cpVect a, const cpVect b)
{
	cpVect delta = cpvsub(b, a);
	return -cpfclamp(cpvdot(delta, cpvadd(a, b))/cpvlengthsq(delta), -1.0f, 1.0f);
}

static inline cpVect
LerpT(const cpVect a, const cpVect b, const cpFloat t)
{
	cpFloat ht = 0.5f*t;
	return cpvadd(cpvmult(a, 0.5f - ht), cpvmult(b, 0.5f + ht));
}

struct ClosestPoints {
	cpVect a, b;
	cpVect n;
	cpFloat d;
	cpCollisionID id;
};

static inline struct ClosestPoints
ClosestPointsNew(const struct MinkowskiPoint v0, const struct MinkowskiPoint v1)
{
	cpFloat t = ClosestT(v0.ab, v1.ab);
	cpVect p = LerpT(v0.ab, v1.ab, t);

	cpVect pa = LerpT(v0.a, v1.a, t);
	cpVect pb = LerpT(v0.b, v1.b, t);
	cpCollisionID id = (v0.id & 0xFFFF)<<16 | (v1.id & 0xFFFF);

	cpVect delta = cpvsub(v1.ab, v0.ab);
	cpVect n = cpvnormalize(cpvrperp(delta));
	cpFloat d = cpvdot(n, p);

	if(d <= 0.0f || (-1.0f < t && t < 1.0f)){
		struct ClosestPoints points = {{pa.x, pa.y}, {pb.x, pb.y}, {n.x, n.y}, d, id};
		return points;
	} else {
		cpFloat d2 = cpvlength(p);
		cpVect n2 = cpvmult(p, 1.0f/(d2 + CPFLOAT_MIN));

		struct ClosestPoints points = {{pa.x, pa.y}, {pb.x, pb.y}, {n2.x, n2.y}, d2, id};
		return points;
	}
}

static inline cpFloat
ClosestDist(const cpVect v0,const cpVect v1)
{
	return cpvlengthsq(LerpT(v0, v1, ClosestT(v0, v1)));
}

static struct ClosestPoints
EPARecurse(const struct SupportContext *ctx, const int count, const struct MinkowskiPoint *hull, const int iteration)
{
	int mini = 0;
	cpFloat minDist = INFINITY;

	for(int j=0, i=count-1; j<count; i=j, j++){
		cpFloat d = ClosestDist(hull[i].ab, hull[j].ab);
		if(d < minDist){
			minDist = d;
			mini = i;
		}
	}

	struct MinkowskiPoint v0 = hull[mini];
	struct MinkowskiPoint v1 = hull[(mini + 1)%count];
	cpAssertSoft3(!cpveql(v0.ab, v1.ab), "Internal Error: EPA vertexes are the same (%d and %d)", mini, (mini + 1)%count);

	struct MinkowskiPoint p = Support(ctx, cpvperp(cpvsub(v1.ab, v0.ab)));

#if DRAW_EPA
	cpVect verts[count];
	for(int i=0; i<count; i++) verts[i] = hull[i].ab;

	ChipmunkDebugDrawPolygon(count, verts, 0.0, RGBAColor(1, 1, 0, 1), RGBAColor(1, 1, 0, 0.25));
	ChipmunkDebugDrawSegment(v0.ab, v1.ab, RGBAColor(1, 0, 0, 1));

	ChipmunkDebugDrawDot(5, p.ab, LAColor(1, 1));
#endif

	cpBool duplicate = (p.id == v0.id || p.id == v1.id);

	if(!duplicate && cpCheckSignedArea(v0.ab, v1.ab, p.ab) && iteration < MAX_EPA_ITERATIONS){
		struct MinkowskiPoint *hull2 = (struct MinkowskiPoint *)alloca((count + 1)*sizeof(struct MinkowskiPoint));
		int count2 = 1;
		hull2[0] = p;

		for(int i=0; i<count; i++){
			int index = (mini + 1 + i)%count;

			cpVect h0 = hull2[count2 - 1].ab;
			cpVect h1 = hull[index].ab;
			cpVect h2 = (i + 1 < count ? hull[(index + 1)%count] : p).ab;

			if(cpCheckSignedArea(h0, h2, h1)){
				hull2[count2] = hull[index];
				count2++;
			}
		}

		return EPARecurse(ctx, count2, hull2, iteration + 1);
	} else {
		cpAssertWarn2(iteration < WARN_EPA_ITERATIONS, "High EPA iterations: %d", iteration);
		return ClosestPointsNew(v0, v1);
	}
}

static struct ClosestPoints
EPA(const struct SupportContext *ctx, const struct MinkowskiPoint v0, const struct MinkowskiPoint v1, const struct MinkowskiPoint v2)
{
	struct MinkowskiPoint hull[3] = {{{v0.a.x, v0.a.y}, {v0.b.x, v0.b.y}, {v0.ab.x, v0.ab.y}, v0.id}, {{v1.a.x, v1.a.y}, {v1.b.x, v1.b.y}, {v1.ab.x, v1.ab.y}, v1.id}, {{v2.a.x, v2.a.y}, {v2.b.x, v2.b.y}, {v2.ab.x, v2.ab.y}, v2.id}};
	return EPARecurse(ctx, 3, hull, 1);
}

static inline cpBool
CheckArea(cpVect v1, cpVect v2)
{
	return (v1.x*v2.y) > (v1.y*v2.x);
}

static inline struct ClosestPoints
GJKRecurse(const struct SupportContext *ctx, const struct MinkowskiPoint v0, const struct MinkowskiPoint v1, const int iteration)
{
	if(iteration > MAX_GJK_ITERATIONS){
		cpAssertWarn2(iteration < WARN_GJK_ITERATIONS, "High GJK iterations: %d", iteration);
		return ClosestPointsNew(v0, v1);
	}

	cpVect delta = cpvsub(v1.ab, v0.ab);
	if(CheckArea(delta, cpvadd(v0.ab, v1.ab))){
		return GJKRecurse(ctx, v1, v0, iteration);
	} else {
		cpFloat t = ClosestT(v0.ab, v1.ab);
		cpVect n = (-1.0f < t && t < 1.0f ? cpvperp(delta) : cpvneg(LerpT(v0.ab, v1.ab, t)));
		struct MinkowskiPoint p = Support(ctx, n);

#if DRAW_GJK
		ChipmunkDebugDrawSegment(v0.ab, v1.ab, RGBAColor(1, 1, 1, 1));
		cpVect c = cpvlerp(v0.ab, v1.ab, 0.5);
		ChipmunkDebugDrawSegment(c, cpvadd(c, cpvmult(cpvnormalize(n), 5.0)), RGBAColor(1, 0, 0, 1));

		ChipmunkDebugDrawDot(5.0, p.ab, LAColor(1, 1));
#endif

		if(
			CheckArea(cpvsub(v1.ab, p.ab), cpvadd(v1.ab, p.ab)) &&
			CheckArea(cpvadd(v0.ab, p.ab), cpvsub(v0.ab, p.ab))
		){
			cpAssertWarn2(iteration < WARN_GJK_ITERATIONS, "High GJK->EPA iterations: %d", iteration);
			return EPA(ctx, v0, p, v1);
		} else {
			if(cpvdot(p.ab, n) <= cpfmax(cpvdot(v0.ab, n), cpvdot(v1.ab, n))){
				cpAssertWarn2(iteration < WARN_GJK_ITERATIONS, "High GJK iterations: %d", iteration);
				return ClosestPointsNew(v0, v1);
			} else {
				if(ClosestDist(v0.ab, p.ab) < ClosestDist(p.ab, v1.ab)){
					return GJKRecurse(ctx, v0, p, iteration + 1);
				} else {
					return GJKRecurse(ctx, p, v1, iteration + 1);
				}
			}
		}
	}
}

static struct SupportPoint
ShapePoint(const cpShape *shape, const int i)
{
	switch(shape->klass->type){
		case CP_CIRCLE_SHAPE: {
			return SupportPointNew(((cpCircleShape *)shape)->tc, 0);
		} case CP_SEGMENT_SHAPE: {
			cpSegmentShape *seg = (cpSegmentShape *)shape;
			return SupportPointNew(i == 0 ? seg->ta : seg->tb, i);
		} case CP_POLY_SHAPE: {
			cpPolyShape *poly = (cpPolyShape *)shape;
			int index = (i < poly->count ? i : 0);
			return SupportPointNew(poly->planes[index].v0, index);
		} default: {
			return SupportPointNew(cpvzero, 0);
		}
	}
}

static struct ClosestPoints
GJK(const struct SupportContext *ctx, cpCollisionID *id)
{
#if DRAW_GJK || DRAW_EPA
	int count1 = 1;
	int count2 = 1;

	switch(ctx->shape1->klass->type){
		case CP_SEGMENT_SHAPE: count1 = 2; break;
		case CP_POLY_SHAPE: count1 = ((cpPolyShape *)ctx->shape1)->count; break;
		default: break;
	}

	switch(ctx->shape2->klass->type){
		case CP_SEGMENT_SHAPE: count1 = 2; break;
		case CP_POLY_SHAPE: count2 = ((cpPolyShape *)ctx->shape2)->count; break;
		default: break;
	}

	cpVect origin = cpvzero;
	ChipmunkDebugDrawDot(5.0, origin, RGBAColor(1,0,0,1));

	int mdiffCount = count1*count2;
	cpVect *mdiffVerts = alloca(mdiffCount*sizeof(cpVect));

	for(int i=0; i<count1; i++){
		for(int j=0; j<count2; j++){
			cpVect v = cpvsub(ShapePoint(ctx->shape2, j).p, ShapePoint(ctx->shape1, i).p);
			mdiffVerts[i*count2 + j] = v;
			ChipmunkDebugDrawDot(2.0, v, RGBAColor(1, 0, 0, 1));
		}
	}

	cpVect *hullVerts = alloca(mdiffCount*sizeof(cpVect));
	int hullCount = cpConvexHull(mdiffCount, mdiffVerts, hullVerts, NULL, 0.0);

	ChipmunkDebugDrawPolygon(hullCount, hullVerts, 0.0, RGBAColor(1, 0, 0, 1), RGBAColor(1, 0, 0, 0.25));
#endif

	struct MinkowskiPoint v0, v1;
	if(*id){
		v0 = MinkowskiPointNew(ShapePoint(ctx->shape1, (*id>>24)&0xFF), ShapePoint(ctx->shape2, (*id>>16)&0xFF));
		v1 = MinkowskiPointNew(ShapePoint(ctx->shape1, (*id>> 8)&0xFF), ShapePoint(ctx->shape2, (*id    )&0xFF));
	} else {
		cpVect axis = cpvperp(cpvsub(cpBBCenter(ctx->shape1->bb), cpBBCenter(ctx->shape2->bb)));
		v0 = Support(ctx, axis);
		v1 = Support(ctx, cpvneg(axis));
	}

	struct ClosestPoints points = GJKRecurse(ctx, v0, v1, 1);
	*id = points.id;
	return points;
}

static inline void
ContactPoints(const struct Edge e1, const struct Edge e2, const struct ClosestPoints points, struct cpCollisionInfo *info)
{
	cpFloat mindist = e1.r + e2.r;
	if(points.d <= mindist){
#ifdef DRAW_CLIP
	ChipmunkDebugDrawFatSegment(e1.a.p, e1.b.p, e1.r, RGBAColor(0, 1, 0, 1), LAColor(0, 0));
	ChipmunkDebugDrawFatSegment(e2.a.p, e2.b.p, e2.r, RGBAColor(1, 0, 0, 1), LAColor(0, 0));
#endif
		cpVect n = info->n = points.n;

		cpFloat d_e1_a = cpvcross(e1.a.p, n);
		cpFloat d_e1_b = cpvcross(e1.b.p, n);
		cpFloat d_e2_a = cpvcross(e2.a.p, n);
		cpFloat d_e2_b = cpvcross(e2.b.p, n);

		cpFloat e1_denom = 1.0f/(d_e1_b - d_e1_a);
		cpFloat e2_denom = 1.0f/(d_e2_b - d_e2_a);

		{
			cpVect p1 = cpvadd(cpvmult(n,  e1.r), cpvlerp(e1.a.p, e1.b.p, cpfclamp01((d_e2_b - d_e1_a)*e1_denom)));
			cpVect p2 = cpvadd(cpvmult(n, -e2.r), cpvlerp(e2.a.p, e2.b.p, cpfclamp01((d_e1_a - d_e2_a)*e2_denom)));
			cpFloat dist = cpvdot(cpvsub(p2, p1), n);
			if(dist <= 0.0f){
				cpHashValue hash_1a2b = CP_HASH_PAIR(e1.a.hash, e2.b.hash);
				cpCollisionInfoPushContact(info, p1, p2, hash_1a2b);
			}
		}{
			cpVect p1 = cpvadd(cpvmult(n,  e1.r), cpvlerp(e1.a.p, e1.b.p, cpfclamp01((d_e2_a - d_e1_a)*e1_denom)));
			cpVect p2 = cpvadd(cpvmult(n, -e2.r), cpvlerp(e2.a.p, e2.b.p, cpfclamp01((d_e1_b - d_e2_a)*e2_denom)));
			cpFloat dist = cpvdot(cpvsub(p2, p1), n);
			if(dist <= 0.0f){
				cpHashValue hash_1b2a = CP_HASH_PAIR(e1.b.hash, e2.a.hash);
				cpCollisionInfoPushContact(info, p1, p2, hash_1b2a);
			}
		}
	}
}

typedef void (*CollisionFunc)(const cpShape *a, const cpShape *b, struct cpCollisionInfo *info);

static void
CircleToCircle(const cpCircleShape *c1, const cpCircleShape *c2, struct cpCollisionInfo *info)
{
	cpFloat mindist = c1->r + c2->r;
	cpVect delta = cpvsub(c2->tc, c1->tc);
	cpFloat distsq = cpvlengthsq(delta);

	if(distsq < mindist*mindist){
		cpFloat dist = cpfsqrt(distsq);
		cpVect n = info->n = (dist ? cpvmult(delta, 1.0f/dist) : cpv(1.0f, 0.0f));
		cpCollisionInfoPushContact(info, cpvadd(c1->tc, cpvmult(n, c1->r)), cpvadd(c2->tc, cpvmult(n, -c2->r)), 0);
	}
}

static void
CircleToSegment(const cpCircleShape *circle, const cpSegmentShape *segment, struct cpCollisionInfo *info)
{
	cpVect seg_a = segment->ta;
	cpVect seg_b = segment->tb;
	cpVect center = circle->tc;

	cpVect seg_delta = cpvsub(seg_b, seg_a);
	cpFloat closest_t = cpfclamp01(cpvdot(seg_delta, cpvsub(center, seg_a))/cpvlengthsq(seg_delta));
	cpVect closest = cpvadd(seg_a, cpvmult(seg_delta, closest_t));

	cpFloat mindist = circle->r + segment->r;
	cpVect delta = cpvsub(closest, center);
	cpFloat distsq = cpvlengthsq(delta);
	if(distsq < mindist*mindist){
		cpFloat dist = cpfsqrt(distsq);
		cpVect n = info->n = (dist ? cpvmult(delta, 1.0f/dist) : segment->tn);

		cpVect rot = cpBodyGetRotation(segment->shape.body);
		if(
			(closest_t != 0.0f || cpvdot(n, cpvrotate(segment->a_tangent, rot)) >= 0.0) &&
			(closest_t != 1.0f || cpvdot(n, cpvrotate(segment->b_tangent, rot)) >= 0.0)
		){
			cpCollisionInfoPushContact(info, cpvadd(center, cpvmult(n, circle->r)), cpvadd(closest, cpvmult(n, -segment->r)), 0);
		}
	}
}

static void
SegmentToSegment(const cpSegmentShape *seg1, const cpSegmentShape *seg2, struct cpCollisionInfo *info)
{
	struct SupportContext context = {(cpShape *)seg1, (cpShape *)seg2, (SupportPointFunc)SegmentSupportPoint, (SupportPointFunc)SegmentSupportPoint};
	struct ClosestPoints points = GJK(&context, &info->id);

#if DRAW_CLOSEST
#if PRINT_LOG
#endif

	ChipmunkDebugDrawDot(6.0, points.a, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawDot(6.0, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif

	cpVect n = points.n;
	cpVect rot1 = cpBodyGetRotation(seg1->shape.body);
	cpVect rot2 = cpBodyGetRotation(seg2->shape.body);

	if(
		points.d <= (seg1->r + seg2->r) &&
		(
			(!cpveql(points.a, seg1->ta) || cpvdot(n, cpvrotate(seg1->a_tangent, rot1)) <= 0.0) &&
			(!cpveql(points.a, seg1->tb) || cpvdot(n, cpvrotate(seg1->b_tangent, rot1)) <= 0.0) &&
			(!cpveql(points.b, seg2->ta) || cpvdot(n, cpvrotate(seg2->a_tangent, rot2)) >= 0.0) &&
			(!cpveql(points.b, seg2->tb) || cpvdot(n, cpvrotate(seg2->b_tangent, rot2)) >= 0.0)
		)
	){
		ContactPoints(SupportEdgeForSegment(seg1, n), SupportEdgeForSegment(seg2, cpvneg(n)), points, info);
	}
}

static void
PolyToPoly(const cpPolyShape *poly1, const cpPolyShape *poly2, struct cpCollisionInfo *info)
{
	struct SupportContext context = {(cpShape *)poly1, (cpShape *)poly2, (SupportPointFunc)PolySupportPoint, (SupportPointFunc)PolySupportPoint};
	struct ClosestPoints points = GJK(&context, &info->id);

#if DRAW_CLOSEST
#if PRINT_LOG
#endif

	ChipmunkDebugDrawDot(3.0, points.a, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawDot(3.0, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif

	if(points.d - poly1->r - poly2->r <= 0.0){
		ContactPoints(SupportEdgeForPoly(poly1, points.n), SupportEdgeForPoly(poly2, cpvneg(points.n)), points, info);
	}
}

static void
SegmentToPoly(const cpSegmentShape *seg, const cpPolyShape *poly, struct cpCollisionInfo *info)
{
	struct SupportContext context = {(cpShape *)seg, (cpShape *)poly, (SupportPointFunc)SegmentSupportPoint, (SupportPointFunc)PolySupportPoint};
	struct ClosestPoints points = GJK(&context, &info->id);

#if DRAW_CLOSEST
#if PRINT_LOG
#endif

	ChipmunkDebugDrawDot(3.0, points.a, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawDot(3.0, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif

	cpVect n = points.n;
	cpVect rot = cpBodyGetRotation(seg->shape.body);

	if(
		points.d - seg->r - poly->r <= 0.0 &&
		(
			(!cpveql(points.a, seg->ta) || cpvdot(n, cpvrotate(seg->a_tangent, rot)) <= 0.0) &&
			(!cpveql(points.a, seg->tb) || cpvdot(n, cpvrotate(seg->b_tangent, rot)) <= 0.0)
		)
	){
		ContactPoints(SupportEdgeForSegment(seg, n), SupportEdgeForPoly(poly, cpvneg(n)), points, info);
	}
}

static void
CircleToPoly(const cpCircleShape *circle, const cpPolyShape *poly, struct cpCollisionInfo *info)
{
	struct SupportContext context = {(cpShape *)circle, (cpShape *)poly, (SupportPointFunc)CircleSupportPoint, (SupportPointFunc)PolySupportPoint};
	struct ClosestPoints points = GJK(&context, &info->id);

#if DRAW_CLOSEST
	ChipmunkDebugDrawDot(3.0, points.a, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawDot(3.0, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, points.b, RGBAColor(1, 1, 1, 1));
	ChipmunkDebugDrawSegment(points.a, cpvadd(points.a, cpvmult(points.n, 10.0)), RGBAColor(1, 0, 0, 1));
#endif

	if(points.d <= circle->r + poly->r){
		cpVect n = info->n = points.n;
		cpCollisionInfoPushContact(info, cpvadd(points.a, cpvmult(n, circle->r)), cpvadd(points.b, cpvmult(n, poly->r)), 0);
	}
}

static void
CollisionError(const cpShape *circle, const cpShape *poly, struct cpCollisionInfo *info)
{
	cpAssertHard(cpFalse, "Internal Error: Shape types are not sorted.");
}

static const CollisionFunc BuiltinCollisionFuncs[9] = {
	(CollisionFunc)CircleToCircle,
	CollisionError,
	CollisionError,
	(CollisionFunc)CircleToSegment,
	(CollisionFunc)SegmentToSegment,
	CollisionError,
	(CollisionFunc)CircleToPoly,
	(CollisionFunc)SegmentToPoly,
	(CollisionFunc)PolyToPoly,
};
static const CollisionFunc *CollisionFuncs = BuiltinCollisionFuncs;

struct cpCollisionInfo
cpCollide(const cpShape *a, const cpShape *b, cpCollisionID id, struct cpContact *contacts)
{
	struct cpCollisionInfo info = {a, b, id, {0, 0}, 0, contacts};

	if(a->klass->type > b->klass->type){
		info.a = b;
		info.b = a;
	}

	CollisionFuncs[info.a->klass->type + info.b->klass->type*CP_NUM_SHAPES](info.a, info.b, &info);

	return info;
}

//
//MERGED FILE END: cpCollision.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpConstraint.c
//

void cpConstraintDestroy(cpConstraint *constraint){}

void
cpConstraintFree(cpConstraint *constraint)
{
	if(constraint){
		cpConstraintDestroy(constraint);
		cpfree(constraint);
	}
}

void
cpConstraintInit(cpConstraint *constraint, const cpConstraintClass *klass, cpBody *a, cpBody *b)
{
	constraint->klass = klass;

	constraint->a = a;
	constraint->b = b;
	constraint->space = NULL;

	constraint->next_a = NULL;
	constraint->next_b = NULL;

	constraint->maxForce = (cpFloat)INFINITY;
	constraint->errorBias = cpfpow(1.0f - 0.1f, 60.0f);
	constraint->maxBias = (cpFloat)INFINITY;

	constraint->collideBodies = cpTrue;

	constraint->preSolve = NULL;
	constraint->postSolve = NULL;
}

cpSpace *
cpConstraintGetSpace(const cpConstraint *constraint)
{
	return constraint->space;
}

cpBody *
cpConstraintGetBodyA(const cpConstraint *constraint)
{
	return constraint->a;
}

cpBody *
cpConstraintGetBodyB(const cpConstraint *constraint)
{
	return constraint->b;
}

cpFloat
cpConstraintGetMaxForce(const cpConstraint *constraint)
{
	return constraint->maxForce;
}

void
cpConstraintSetMaxForce(cpConstraint *constraint, cpFloat maxForce)
{
	cpAssertHard(maxForce >= 0.0f, "maxForce must be positive.");
	cpConstraintActivateBodies(constraint);
	constraint->maxForce = maxForce;
}

cpFloat
cpConstraintGetErrorBias(const cpConstraint *constraint)
{
	return constraint->errorBias;
}

void
cpConstraintSetErrorBias(cpConstraint *constraint, cpFloat errorBias)
{
	cpAssertHard(errorBias >= 0.0f, "errorBias must be positive.");
	cpConstraintActivateBodies(constraint);
	constraint->errorBias = errorBias;
}

cpFloat
cpConstraintGetMaxBias(const cpConstraint *constraint)
{
	return constraint->maxBias;
}

void
cpConstraintSetMaxBias(cpConstraint *constraint, cpFloat maxBias)
{
	cpAssertHard(maxBias >= 0.0f, "maxBias must be positive.");
	cpConstraintActivateBodies(constraint);
	constraint->maxBias = maxBias;
}

cpBool
cpConstraintGetCollideBodies(const cpConstraint *constraint)
{
	return constraint->collideBodies;
}

void
cpConstraintSetCollideBodies(cpConstraint *constraint, cpBool collideBodies)
{
	cpConstraintActivateBodies(constraint);
	constraint->collideBodies = collideBodies;
}

cpConstraintPreSolveFunc
cpConstraintGetPreSolveFunc(const cpConstraint *constraint)
{
	return constraint->preSolve;
}

void
cpConstraintSetPreSolveFunc(cpConstraint *constraint, cpConstraintPreSolveFunc preSolveFunc)
{
	constraint->preSolve = preSolveFunc;
}

cpConstraintPostSolveFunc
cpConstraintGetPostSolveFunc(const cpConstraint *constraint)
{
	return constraint->postSolve;
}

void
cpConstraintSetPostSolveFunc(cpConstraint *constraint, cpConstraintPostSolveFunc postSolveFunc)
{
	constraint->postSolve = postSolveFunc;
}

cpDataPointer
cpConstraintGetUserData(const cpConstraint *constraint)
{
	return constraint->userData;
}

void
cpConstraintSetUserData(cpConstraint *constraint, cpDataPointer userData)
{
	constraint->userData = userData;
}

cpFloat
cpConstraintGetImpulse(cpConstraint *constraint)
{
	return constraint->klass->getImpulse(constraint);
}

//
//MERGED FILE END: cpConstraint.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpDampedRotarySpring.c
//

static cpFloat
defaultSpringTorque(cpDampedRotarySpring *spring, cpFloat relativeAngle){
	return (relativeAngle - spring->restAngle)*spring->stiffness;
}

static void
preStepcpDampedRotarySpring(cpDampedRotarySpring *spring, cpFloat dt)
{
	cpBody *a = spring->constraint.a;
	cpBody *b = spring->constraint.b;

	cpFloat moment = a->i_inv + b->i_inv;
	cpAssertSoft(moment != 0.0, "Unsolvable spring.");
	spring->iSum = 1.0f/moment;

	spring->w_coef = 1.0f - cpfexp(-spring->damping*dt*moment);
	spring->target_wrn = 0.0f;

	cpFloat j_spring = spring->springTorqueFunc((cpConstraint *)spring, a->a - b->a)*dt;
	spring->jAcc = j_spring;

	a->w -= j_spring*a->i_inv;
	b->w += j_spring*b->i_inv;
}

static void applyCachedImpulsecpDampedRotarySpring(cpDampedRotarySpring *spring, cpFloat dt_coef){}

static void
applyImpulsecpDampedRotarySpring(cpDampedRotarySpring *spring, cpFloat dt)
{
	cpBody *a = spring->constraint.a;
	cpBody *b = spring->constraint.b;

	cpFloat wrn = a->w - b->w;
	cpFloat w_damp = (spring->target_wrn - wrn)*spring->w_coef;
	spring->target_wrn = wrn + w_damp;

	cpFloat j_damp = w_damp*spring->iSum;
	spring->jAcc += j_damp;

	a->w += j_damp*a->i_inv;
	b->w -= j_damp*b->i_inv;
}

static cpFloat
getImpulsecpDampedRotarySpring(cpDampedRotarySpring *spring)
{
	return spring->jAcc;
}

static const cpConstraintClass klasscpDampedRotarySpring = {
	(cpConstraintPreStepImpl)preStepcpDampedRotarySpring,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpDampedRotarySpring,
	(cpConstraintApplyImpulseImpl)applyImpulsecpDampedRotarySpring,
	(cpConstraintGetImpulseImpl)getImpulsecpDampedRotarySpring,
};

cpDampedRotarySpring *
cpDampedRotarySpringAlloc(void)
{
	return (cpDampedRotarySpring *)cpcalloc(1, sizeof(cpDampedRotarySpring));
}

cpDampedRotarySpring *
cpDampedRotarySpringInit(cpDampedRotarySpring *spring, cpBody *a, cpBody *b, cpFloat restAngle, cpFloat stiffness, cpFloat damping)
{
	cpConstraintInit((cpConstraint *)spring, &klasscpDampedRotarySpring, a, b);

	spring->restAngle = restAngle;
	spring->stiffness = stiffness;
	spring->damping = damping;
	spring->springTorqueFunc = (cpDampedRotarySpringTorqueFunc)defaultSpringTorque;

	spring->jAcc = 0.0f;

	return spring;
}

cpConstraint *
cpDampedRotarySpringNew(cpBody *a, cpBody *b, cpFloat restAngle, cpFloat stiffness, cpFloat damping)
{
	return (cpConstraint *)cpDampedRotarySpringInit(cpDampedRotarySpringAlloc(), a, b, restAngle, stiffness, damping);
}

cpBool
cpConstraintIsDampedRotarySpring(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpDampedRotarySpring);
}

cpFloat
cpDampedRotarySpringGetRestAngle(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedRotarySpring(constraint), "Constraint is not a damped rotary spring.");
	return ((cpDampedRotarySpring *)constraint)->restAngle;
}

void
cpDampedRotarySpringSetRestAngle(cpConstraint *constraint, cpFloat restAngle)
{
	cpAssertHard(cpConstraintIsDampedRotarySpring(constraint), "Constraint is not a damped rotary spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedRotarySpring *)constraint)->restAngle = restAngle;
}

cpFloat
cpDampedRotarySpringGetStiffness(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedRotarySpring(constraint), "Constraint is not a damped rotary spring.");
	return ((cpDampedRotarySpring *)constraint)->stiffness;
}

void
cpDampedRotarySpringSetStiffness(cpConstraint *constraint, cpFloat stiffness)
{
	cpAssertHard(cpConstraintIsDampedRotarySpring(constraint), "Constraint is not a damped rotary spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedRotarySpring *)constraint)->stiffness = stiffness;
}

cpFloat
cpDampedRotarySpringGetDamping(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedRotarySpring(constraint), "Constraint is not a damped rotary spring.");
	return ((cpDampedRotarySpring *)constraint)->damping;
}

void
cpDampedRotarySpringSetDamping(cpConstraint *constraint, cpFloat damping)
{
	cpAssertHard(cpConstraintIsDampedRotarySpring(constraint), "Constraint is not a damped rotary spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedRotarySpring *)constraint)->damping = damping;
}

cpDampedRotarySpringTorqueFunc
cpDampedRotarySpringGetSpringTorqueFunc(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedRotarySpring(constraint), "Constraint is not a damped rotary spring.");
	return ((cpDampedRotarySpring *)constraint)->springTorqueFunc;
}

void
cpDampedRotarySpringSetSpringTorqueFunc(cpConstraint *constraint, cpDampedRotarySpringTorqueFunc springTorqueFunc)
{
	cpAssertHard(cpConstraintIsDampedRotarySpring(constraint), "Constraint is not a damped rotary spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedRotarySpring *)constraint)->springTorqueFunc = springTorqueFunc;
}

//
//MERGED FILE END: cpDampedRotarySpring.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpDampedSpring.c
//

static cpFloat
defaultSpringForce(cpDampedSpring *spring, cpFloat dist){
	return (spring->restLength - dist)*spring->stiffness;
}

static void
preStepcpDampedSpring(cpDampedSpring *spring, cpFloat dt)
{
	cpBody *a = spring->constraint.a;
	cpBody *b = spring->constraint.b;

	spring->r1 = cpTransformVect(a->transform, cpvsub(spring->anchorA, a->cog));
	spring->r2 = cpTransformVect(b->transform, cpvsub(spring->anchorB, b->cog));

	cpVect delta = cpvsub(cpvadd(b->p, spring->r2), cpvadd(a->p, spring->r1));
	cpFloat dist = cpvlength(delta);
	spring->n = cpvmult(delta, 1.0f/(dist ? dist : INFINITY));

	cpFloat k = k_scalar(a, b, spring->r1, spring->r2, spring->n);
	cpAssertSoft(k != 0.0, "Unsolvable spring.");
	spring->nMass = 1.0f/k;

	spring->target_vrn = 0.0f;
	spring->v_coef = 1.0f - cpfexp(-spring->damping*dt*k);

	cpFloat f_spring = spring->springForceFunc((cpConstraint *)spring, dist);
	cpFloat j_spring = spring->jAcc = f_spring*dt;
	apply_impulses(a, b, spring->r1, spring->r2, cpvmult(spring->n, j_spring));
}

static void applyCachedImpulsecpDampedSpring(cpDampedSpring *spring, cpFloat dt_coef){}

static void
applyImpulsecpDampedSpring(cpDampedSpring *spring, cpFloat dt)
{
	cpBody *a = spring->constraint.a;
	cpBody *b = spring->constraint.b;

	cpVect n = spring->n;
	cpVect r1 = spring->r1;
	cpVect r2 = spring->r2;

	cpFloat vrn = normal_relative_velocity(a, b, r1, r2, n);

	cpFloat v_damp = (spring->target_vrn - vrn)*spring->v_coef;
	spring->target_vrn = vrn + v_damp;

	cpFloat j_damp = v_damp*spring->nMass;
	spring->jAcc += j_damp;
	apply_impulses(a, b, spring->r1, spring->r2, cpvmult(spring->n, j_damp));
}

static cpFloat
getImpulsecpDampedSpring(cpDampedSpring *spring)
{
	return spring->jAcc;
}

static const cpConstraintClass klasscpDampedSpring = {
	(cpConstraintPreStepImpl)preStepcpDampedSpring,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpDampedSpring,
	(cpConstraintApplyImpulseImpl)applyImpulsecpDampedSpring,
	(cpConstraintGetImpulseImpl)getImpulsecpDampedSpring,
};

cpDampedSpring *
cpDampedSpringAlloc(void)
{
	return (cpDampedSpring *)cpcalloc(1, sizeof(cpDampedSpring));
}

cpDampedSpring *
cpDampedSpringInit(cpDampedSpring *spring, cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB, cpFloat restLength, cpFloat stiffness, cpFloat damping)
{
	cpConstraintInit((cpConstraint *)spring, &klasscpDampedSpring, a, b);

	spring->anchorA = anchorA;
	spring->anchorB = anchorB;

	spring->restLength = restLength;
	spring->stiffness = stiffness;
	spring->damping = damping;
	spring->springForceFunc = (cpDampedSpringForceFunc)defaultSpringForce;

	spring->jAcc = 0.0f;

	return spring;
}

cpConstraint *
cpDampedSpringNew(cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB, cpFloat restLength, cpFloat stiffness, cpFloat damping)
{
	return (cpConstraint *)cpDampedSpringInit(cpDampedSpringAlloc(), a, b, anchorA, anchorB, restLength, stiffness, damping);
}

cpBool
cpConstraintIsDampedSpring(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpDampedSpring);
}

cpVect
cpDampedSpringGetAnchorA(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	return ((cpDampedSpring *)constraint)->anchorA;
}

void
cpDampedSpringSetAnchorA(cpConstraint *constraint, cpVect anchorA)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedSpring *)constraint)->anchorA = anchorA;
}

cpVect
cpDampedSpringGetAnchorB(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	return ((cpDampedSpring *)constraint)->anchorB;
}

void
cpDampedSpringSetAnchorB(cpConstraint *constraint, cpVect anchorB)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedSpring *)constraint)->anchorB = anchorB;
}

cpFloat
cpDampedSpringGetRestLength(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	return ((cpDampedSpring *)constraint)->restLength;
}

void
cpDampedSpringSetRestLength(cpConstraint *constraint, cpFloat restLength)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedSpring *)constraint)->restLength = restLength;
}

cpFloat
cpDampedSpringGetStiffness(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	return ((cpDampedSpring *)constraint)->stiffness;
}

void
cpDampedSpringSetStiffness(cpConstraint *constraint, cpFloat stiffness)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedSpring *)constraint)->stiffness = stiffness;
}

cpFloat
cpDampedSpringGetDamping(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	return ((cpDampedSpring *)constraint)->damping;
}

void
cpDampedSpringSetDamping(cpConstraint *constraint, cpFloat damping)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedSpring *)constraint)->damping = damping;
}

cpDampedSpringForceFunc
cpDampedSpringGetSpringForceFunc(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	return ((cpDampedSpring *)constraint)->springForceFunc;
}

void
cpDampedSpringSetSpringForceFunc(cpConstraint *constraint, cpDampedSpringForceFunc springForceFunc)
{
	cpAssertHard(cpConstraintIsDampedSpring(constraint), "Constraint is not a damped spring.");
	cpConstraintActivateBodies(constraint);
	((cpDampedSpring *)constraint)->springForceFunc = springForceFunc;
}

//
//MERGED FILE END: cpDampedSpring.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpGearJoint.c
//

static void
preStepcpGearJoint(cpGearJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	joint->iSum = 1.0f/(a->i_inv*joint->ratio_inv + joint->ratio*b->i_inv);

	cpFloat maxBias = joint->constraint.maxBias;
	joint->bias = cpfclamp(-bias_coef(joint->constraint.errorBias, dt)*(b->a*joint->ratio - a->a - joint->phase)/dt, -maxBias, maxBias);
}

static void
applyCachedImpulsecpGearJoint(cpGearJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat j = joint->jAcc*dt_coef;
	a->w -= j*a->i_inv*joint->ratio_inv;
	b->w += j*b->i_inv;
}

static void
applyImpulsecpGearJoint(cpGearJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat wr = b->w*joint->ratio - a->w;

	cpFloat jMax = joint->constraint.maxForce*dt;

	cpFloat j = (joint->bias - wr)*joint->iSum;
	cpFloat jOld = joint->jAcc;
	joint->jAcc = cpfclamp(jOld + j, -jMax, jMax);
	j = joint->jAcc - jOld;

	a->w -= j*a->i_inv*joint->ratio_inv;
	b->w += j*b->i_inv;
}

static cpFloat
getImpulsecpGearJoint(cpGearJoint *joint)
{
	return cpfabs(joint->jAcc);
}

static const cpConstraintClass klasscpGearJoint = {
	(cpConstraintPreStepImpl)preStepcpGearJoint,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpGearJoint,
	(cpConstraintApplyImpulseImpl)applyImpulsecpGearJoint,
	(cpConstraintGetImpulseImpl)getImpulsecpGearJoint,
};

cpGearJoint *
cpGearJointAlloc(void)
{
	return (cpGearJoint *)cpcalloc(1, sizeof(cpGearJoint));
}

cpGearJoint *
cpGearJointInit(cpGearJoint *joint, cpBody *a, cpBody *b, cpFloat phase, cpFloat ratio)
{
	cpConstraintInit((cpConstraint *)joint, &klasscpGearJoint, a, b);

	joint->phase = phase;
	joint->ratio = ratio;
	joint->ratio_inv = 1.0f/ratio;

	joint->jAcc = 0.0f;

	return joint;
}

cpConstraint *
cpGearJointNew(cpBody *a, cpBody *b, cpFloat phase, cpFloat ratio)
{
	return (cpConstraint *)cpGearJointInit(cpGearJointAlloc(), a, b, phase, ratio);
}

cpBool
cpConstraintIsGearJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpGearJoint);
}

cpFloat
cpGearJointGetPhase(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsGearJoint(constraint), "Constraint is not a ratchet joint.");
	return ((cpGearJoint *)constraint)->phase;
}

void
cpGearJointSetPhase(cpConstraint *constraint, cpFloat phase)
{
	cpAssertHard(cpConstraintIsGearJoint(constraint), "Constraint is not a ratchet joint.");
	cpConstraintActivateBodies(constraint);
	((cpGearJoint *)constraint)->phase = phase;
}

cpFloat
cpGearJointGetRatio(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsGearJoint(constraint), "Constraint is not a ratchet joint.");
	return ((cpGearJoint *)constraint)->ratio;
}

void
cpGearJointSetRatio(cpConstraint *constraint, cpFloat ratio)
{
	cpAssertHard(cpConstraintIsGearJoint(constraint), "Constraint is not a ratchet joint.");
	cpConstraintActivateBodies(constraint);
	((cpGearJoint *)constraint)->ratio = ratio;
	((cpGearJoint *)constraint)->ratio_inv = 1.0f/ratio;
}

//
//MERGED FILE END: cpGearJoint.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpGrooveJoint.c
//

static void
preStepcpGrooveJoint(cpGrooveJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpVect ta = cpTransformPoint(a->transform, joint->grv_a);
	cpVect tb = cpTransformPoint(a->transform, joint->grv_b);

	cpVect n = cpTransformVect(a->transform, joint->grv_n);
	cpFloat d = cpvdot(ta, n);

	joint->grv_tn = n;
	joint->r2 = cpTransformVect(b->transform, cpvsub(joint->anchorB, b->cog));

	cpFloat td = cpvcross(cpvadd(b->p, joint->r2), n);
	if(td <= cpvcross(ta, n)){
		joint->clamp = 1.0f;
		joint->r1 = cpvsub(ta, a->p);
	} else if(td >= cpvcross(tb, n)){
		joint->clamp = -1.0f;
		joint->r1 = cpvsub(tb, a->p);
	} else {
		joint->clamp = 0.0f;
		joint->r1 = cpvsub(cpvadd(cpvmult(cpvperp(n), -td), cpvmult(n, d)), a->p);
	}

	joint->k = k_tensor(a, b, joint->r1, joint->r2);

	cpVect delta = cpvsub(cpvadd(b->p, joint->r2), cpvadd(a->p, joint->r1));
	joint->bias = cpvclamp(cpvmult(delta, -bias_coef(joint->constraint.errorBias, dt)/dt), joint->constraint.maxBias);
}

static void
applyCachedImpulsecpGrooveJoint(cpGrooveJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	apply_impulses(a, b, joint->r1, joint->r2, cpvmult(joint->jAcc, dt_coef));
}

static inline cpVect
grooveConstrain(cpGrooveJoint *joint, cpVect j, cpFloat dt){
	cpVect n = joint->grv_tn;
	cpVect jClamp = (joint->clamp*cpvcross(j, n) > 0.0f) ? j : cpvproject(j, n);
	return cpvclamp(jClamp, joint->constraint.maxForce*dt);
}

static void
applyImpulsecpGrooveJoint(cpGrooveJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpVect r1 = joint->r1;
	cpVect r2 = joint->r2;

	cpVect vr = relative_velocity(a, b, r1, r2);

	cpVect j = cpMat2x2Transform(joint->k, cpvsub(joint->bias, vr));
	cpVect jOld = joint->jAcc;
	joint->jAcc = grooveConstrain(joint, cpvadd(jOld, j), dt);
	j = cpvsub(joint->jAcc, jOld);

	apply_impulses(a, b, joint->r1, joint->r2, j);
}

static cpFloat
getImpulsecpGrooveJoint(cpGrooveJoint *joint)
{
	return cpvlength(joint->jAcc);
}

static const cpConstraintClass klasscpGrooveJoint = {
	(cpConstraintPreStepImpl)preStepcpGrooveJoint,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpGrooveJoint,
	(cpConstraintApplyImpulseImpl)applyImpulsecpGrooveJoint,
	(cpConstraintGetImpulseImpl)getImpulsecpGrooveJoint,
};

cpGrooveJoint *
cpGrooveJointAlloc(void)
{
	return (cpGrooveJoint *)cpcalloc(1, sizeof(cpGrooveJoint));
}

cpGrooveJoint *
cpGrooveJointInit(cpGrooveJoint *joint, cpBody *a, cpBody *b, cpVect groove_a, cpVect groove_b, cpVect anchorB)
{
	cpConstraintInit((cpConstraint *)joint, &klasscpGrooveJoint, a, b);

	joint->grv_a = groove_a;
	joint->grv_b = groove_b;
	joint->grv_n = cpvperp(cpvnormalize(cpvsub(groove_b, groove_a)));
	joint->anchorB = anchorB;

	joint->jAcc = cpvzero;

	return joint;
}

cpConstraint *
cpGrooveJointNew(cpBody *a, cpBody *b, cpVect groove_a, cpVect groove_b, cpVect anchorB)
{
	return (cpConstraint *)cpGrooveJointInit(cpGrooveJointAlloc(), a, b, groove_a, groove_b, anchorB);
}

cpBool
cpConstraintIsGrooveJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpGrooveJoint);
}

cpVect
cpGrooveJointGetGrooveA(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	return ((cpGrooveJoint *)constraint)->grv_a;
}

void
cpGrooveJointSetGrooveA(cpConstraint *constraint, cpVect value)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	cpGrooveJoint *g = (cpGrooveJoint *)constraint;

	g->grv_a = value;
	g->grv_n = cpvperp(cpvnormalize(cpvsub(g->grv_b, value)));

	cpConstraintActivateBodies(constraint);
}

cpVect
cpGrooveJointGetGrooveB(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	return ((cpGrooveJoint *)constraint)->grv_b;
}

void
cpGrooveJointSetGrooveB(cpConstraint *constraint, cpVect value)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	cpGrooveJoint *g = (cpGrooveJoint *)constraint;

	g->grv_b = value;
	g->grv_n = cpvperp(cpvnormalize(cpvsub(value, g->grv_a)));

	cpConstraintActivateBodies(constraint);
}

cpVect
cpGrooveJointGetAnchorB(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	return ((cpGrooveJoint *)constraint)->anchorB;
}

void
cpGrooveJointSetAnchorB(cpConstraint *constraint, cpVect anchorB)
{
	cpAssertHard(cpConstraintIsGrooveJoint(constraint), "Constraint is not a groove joint.");
	cpConstraintActivateBodies(constraint);
	((cpGrooveJoint *)constraint)->anchorB = anchorB;
}

//
//MERGED FILE END: cpGrooveJoint.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpHashSet.c
//

static int primes[] = {
	5,
	13,
	23,
	47,
	97,
	193,
	389,
	769,
	1543,
	3079,
	6151,
	12289,
	24593,
	49157,
	98317,
	196613,
	393241,
	786433,
	1572869,
	3145739,
	6291469,
	12582917,
	25165843,
	50331653,
	100663319,
	201326611,
	402653189,
	805306457,
	1610612741,
	0,
};

static inline int
next_prime(int n)
{
	int i = 0;
	while(n > primes[i]){
		i++;
		cpAssertHard(primes[i], "Tried to resize a hash table to a size greater than 1610612741 O_o");	}

	return primes[i];
}

typedef struct cpHashSetBin {
	void *elt;
	cpHashValue hash;
	struct cpHashSetBin *next;
} cpHashSetBin;

struct cpHashSet {
	unsigned int entries, size;

	cpHashSetEqlFunc eql;
	void *default_value;

	cpHashSetBin **table;
	cpHashSetBin *pooledBins;

	cpArray *allocatedBuffers;
};

void
cpHashSetFree(cpHashSet *set)
{
	if(set){
		cpfree(set->table);

		cpArrayFreeEach(set->allocatedBuffers, cpfree);
		cpArrayFree(set->allocatedBuffers);

		cpfree(set);
	}
}

cpHashSet *
cpHashSetNew(int size, cpHashSetEqlFunc eqlFunc)
{
	cpHashSet *set = (cpHashSet *)cpcalloc(1, sizeof(cpHashSet));

	set->size = next_prime(size);
	set->entries = 0;

	set->eql = eqlFunc;
	set->default_value = NULL;

	set->table = (cpHashSetBin **)cpcalloc(set->size, sizeof(cpHashSetBin *));
	set->pooledBins = NULL;

	set->allocatedBuffers = cpArrayNew(0);

	return set;
}

void
cpHashSetSetDefaultValue(cpHashSet *set, void *default_value)
{
	set->default_value = default_value;
}

static int
setIsFull(cpHashSet *set)
{
	return (set->entries >= set->size);
}

static void
cpHashSetResize(cpHashSet *set)
{
	unsigned int newSize = next_prime(set->size + 1);
	cpHashSetBin **newTable = (cpHashSetBin **)cpcalloc(newSize, sizeof(cpHashSetBin *));

	for(unsigned int i=0; i<set->size; i++){
		cpHashSetBin *bin = set->table[i];
		while(bin){
			cpHashSetBin *next = bin->next;

			cpHashValue idx = bin->hash%newSize;
			bin->next = newTable[idx];
			newTable[idx] = bin;

			bin = next;
		}
	}

	cpfree(set->table);

	set->table = newTable;
	set->size = newSize;
}

static inline void
recycleBin(cpHashSet *set, cpHashSetBin *bin)
{
	bin->next = set->pooledBins;
	set->pooledBins = bin;
	bin->elt = NULL;
}

static cpHashSetBin *
getUnusedBin(cpHashSet *set)
{
	cpHashSetBin *bin = set->pooledBins;

	if(bin){
		set->pooledBins = bin->next;
		return bin;
	} else {
		int count = CP_BUFFER_BYTES/sizeof(cpHashSetBin);
		cpAssertHard(count, "Internal Error: Buffer size is too small.");

		cpHashSetBin *buffer = (cpHashSetBin *)cpcalloc(1, CP_BUFFER_BYTES);
		cpArrayPush(set->allocatedBuffers, buffer);

		for(int i=1; i<count; i++) recycleBin(set, buffer + i);
		return buffer;
	}
}

int
cpHashSetCount(cpHashSet *set)
{
	return set->entries;
}

void *
cpHashSetInsert(cpHashSet *set, cpHashValue hash, void *ptr, cpHashSetTransFunc trans, void *data)
{
	cpHashValue idx = hash%set->size;

	cpHashSetBin *bin = set->table[idx];
	while(bin && !set->eql(ptr, bin->elt))
		bin = bin->next;

	if(!bin){
		bin = getUnusedBin(set);
		bin->hash = hash;
		bin->elt = (trans ? trans(ptr, data) : data);

		bin->next = set->table[idx];
		set->table[idx] = bin;

		set->entries++;
		if(setIsFull(set)) cpHashSetResize(set);
	}

	return bin->elt;
}

void *
cpHashSetRemove(cpHashSet *set, cpHashValue hash, void *ptr)
{
	cpHashValue idx = hash%set->size;

	cpHashSetBin **prev_ptr = &set->table[idx];
	cpHashSetBin *bin = set->table[idx];

	while(bin && !set->eql(ptr, bin->elt)){
		prev_ptr = &bin->next;
		bin = bin->next;
	}

	if(bin){
		(*prev_ptr) = bin->next;
		set->entries--;

		void *elt = bin->elt;
		recycleBin(set, bin);

		return elt;
	}

	return NULL;
}

void *
cpHashSetFind(cpHashSet *set, cpHashValue hash, void *ptr)
{
	cpHashValue idx = hash%set->size;
	cpHashSetBin *bin = set->table[idx];
	while(bin && !set->eql(ptr, bin->elt))
		bin = bin->next;

	return (bin ? bin->elt : set->default_value);
}

void
cpHashSetEach(cpHashSet *set, cpHashSetIteratorFunc func, void *data)
{
	for(unsigned int i=0; i<set->size; i++){
		cpHashSetBin *bin = set->table[i];
		while(bin){
			cpHashSetBin *next = bin->next;
			func(bin->elt, data);
			bin = next;
		}
	}
}

void
cpHashSetFilter(cpHashSet *set, cpHashSetFilterFunc func, void *data)
{
	for(unsigned int i=0; i<set->size; i++){
		cpHashSetBin **prev_ptr = &set->table[i];
		cpHashSetBin *bin = set->table[i];
		while(bin){
			cpHashSetBin *next = bin->next;

			if(func(bin->elt, data)){
				prev_ptr = &bin->next;
			} else {
				(*prev_ptr) = next;

				set->entries--;
				recycleBin(set, bin);
			}

			bin = next;
		}
	}
}

//
//MERGED FILE END: cpHashSet.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpPinJoint.c
//

static void
preStepcpPinJoint(cpPinJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	joint->r1 = cpTransformVect(a->transform, cpvsub(joint->anchorA, a->cog));
	joint->r2 = cpTransformVect(b->transform, cpvsub(joint->anchorB, b->cog));

	cpVect delta = cpvsub(cpvadd(b->p, joint->r2), cpvadd(a->p, joint->r1));
	cpFloat dist = cpvlength(delta);
	joint->n = cpvmult(delta, 1.0f/(dist ? dist : (cpFloat)INFINITY));

	joint->nMass = 1.0f/k_scalar(a, b, joint->r1, joint->r2, joint->n);

	cpFloat maxBias = joint->constraint.maxBias;
	joint->bias = cpfclamp(-bias_coef(joint->constraint.errorBias, dt)*(dist - joint->dist)/dt, -maxBias, maxBias);
}

static void
applyCachedImpulsecpPinJoint(cpPinJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpVect j = cpvmult(joint->n, joint->jnAcc*dt_coef);
	apply_impulses(a, b, joint->r1, joint->r2, j);
}

static void
applyImpulsecpPinJoint(cpPinJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	cpVect n = joint->n;

	cpFloat vrn = normal_relative_velocity(a, b, joint->r1, joint->r2, n);

	cpFloat jnMax = joint->constraint.maxForce*dt;

	cpFloat jn = (joint->bias - vrn)*joint->nMass;
	cpFloat jnOld = joint->jnAcc;
	joint->jnAcc = cpfclamp(jnOld + jn, -jnMax, jnMax);
	jn = joint->jnAcc - jnOld;

	apply_impulses(a, b, joint->r1, joint->r2, cpvmult(n, jn));
}

static cpFloat
getImpulsecpPinJoint(cpPinJoint *joint)
{
	return cpfabs(joint->jnAcc);
}

static const cpConstraintClass klasscpPinJoint = {
	(cpConstraintPreStepImpl)preStepcpPinJoint,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpPinJoint,
	(cpConstraintApplyImpulseImpl)applyImpulsecpPinJoint,
	(cpConstraintGetImpulseImpl)getImpulsecpPinJoint,
};

cpPinJoint *
cpPinJointAlloc(void)
{
	return (cpPinJoint *)cpcalloc(1, sizeof(cpPinJoint));
}

cpPinJoint *
cpPinJointInit(cpPinJoint *joint, cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB)
{
	cpConstraintInit((cpConstraint *)joint, &klasscpPinJoint, a, b);

	joint->anchorA = anchorA;
	joint->anchorB = anchorB;

	cpVect p1 = (a ? cpTransformPoint(a->transform, anchorA) : anchorA);
	cpVect p2 = (b ? cpTransformPoint(b->transform, anchorB) : anchorB);
	joint->dist = cpvlength(cpvsub(p2, p1));

	cpAssertWarn(joint->dist > 0.0, "You created a 0 length pin joint. A pivot joint will be much more stable.");

	joint->jnAcc = 0.0f;

	return joint;
}

cpConstraint *
cpPinJointNew(cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB)
{
	return (cpConstraint *)cpPinJointInit(cpPinJointAlloc(), a, b, anchorA, anchorB);
}

cpBool
cpConstraintIsPinJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpPinJoint);
}

cpVect
cpPinJointGetAnchorA(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsPinJoint(constraint), "Constraint is not a pin joint.");
	return ((cpPinJoint *)constraint)->anchorA;
}

void
cpPinJointSetAnchorA(cpConstraint *constraint, cpVect anchorA)
{
	cpAssertHard(cpConstraintIsPinJoint(constraint), "Constraint is not a pin joint.");
	cpConstraintActivateBodies(constraint);
	((cpPinJoint *)constraint)->anchorA = anchorA;
}

cpVect
cpPinJointGetAnchorB(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsPinJoint(constraint), "Constraint is not a pin joint.");
	return ((cpPinJoint *)constraint)->anchorB;
}

void
cpPinJointSetAnchorB(cpConstraint *constraint, cpVect anchorB)
{
	cpAssertHard(cpConstraintIsPinJoint(constraint), "Constraint is not a pin joint.");
	cpConstraintActivateBodies(constraint);
	((cpPinJoint *)constraint)->anchorB = anchorB;
}

cpFloat
cpPinJointGetDist(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsPinJoint(constraint), "Constraint is not a pin joint.");
	return ((cpPinJoint *)constraint)->dist;
}

void
cpPinJointSetDist(cpConstraint *constraint, cpFloat dist)
{
	cpAssertHard(cpConstraintIsPinJoint(constraint), "Constraint is not a pin joint.");
	cpConstraintActivateBodies(constraint);
	((cpPinJoint *)constraint)->dist = dist;
}

//
//MERGED FILE END: cpPinJoint.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpPivotJoint.c
//

static void
preStepcpPivotJoint(cpPivotJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	joint->r1 = cpTransformVect(a->transform, cpvsub(joint->anchorA, a->cog));
	joint->r2 = cpTransformVect(b->transform, cpvsub(joint->anchorB, b->cog));

	joint-> k = k_tensor(a, b, joint->r1, joint->r2);

	cpVect delta = cpvsub(cpvadd(b->p, joint->r2), cpvadd(a->p, joint->r1));
	joint->bias = cpvclamp(cpvmult(delta, -bias_coef(joint->constraint.errorBias, dt)/dt), joint->constraint.maxBias);
}

static void
applyCachedImpulsecpPivotJoint(cpPivotJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	apply_impulses(a, b, joint->r1, joint->r2, cpvmult(joint->jAcc, dt_coef));
}

static void
applyImpulsecpPivotJoint(cpPivotJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpVect r1 = joint->r1;
	cpVect r2 = joint->r2;

	cpVect vr = relative_velocity(a, b, r1, r2);

	cpVect j = cpMat2x2Transform(joint->k, cpvsub(joint->bias, vr));
	cpVect jOld = joint->jAcc;
	joint->jAcc = cpvclamp(cpvadd(joint->jAcc, j), joint->constraint.maxForce*dt);
	j = cpvsub(joint->jAcc, jOld);

	apply_impulses(a, b, joint->r1, joint->r2, j);
}

static cpFloat
getImpulsecpPivotJoint(cpConstraint *joint)
{
	return cpvlength(((cpPivotJoint *)joint)->jAcc);
}

static const cpConstraintClass klasscpPivotJoint = {
	(cpConstraintPreStepImpl)preStepcpPivotJoint,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpPivotJoint,
	(cpConstraintApplyImpulseImpl)applyImpulsecpPivotJoint,
	(cpConstraintGetImpulseImpl)getImpulsecpPivotJoint,
};

cpPivotJoint *
cpPivotJointAlloc(void)
{
	return (cpPivotJoint *)cpcalloc(1, sizeof(cpPivotJoint));
}

cpPivotJoint *
cpPivotJointInit(cpPivotJoint *joint, cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB)
{
	cpConstraintInit((cpConstraint *)joint, &klasscpPivotJoint, a, b);

	joint->anchorA = anchorA;
	joint->anchorB = anchorB;

	joint->jAcc = cpvzero;

	return joint;
}

cpConstraint *
cpPivotJointNew2(cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB)
{
	return (cpConstraint *)cpPivotJointInit(cpPivotJointAlloc(), a, b, anchorA, anchorB);
}

cpConstraint *
cpPivotJointNew(cpBody *a, cpBody *b, cpVect pivot)
{
	cpVect anchorA = (a ? cpBodyWorldToLocal(a, pivot) : pivot);
	cpVect anchorB = (b ? cpBodyWorldToLocal(b, pivot) : pivot);
	return cpPivotJointNew2(a, b, anchorA, anchorB);
}

cpBool
cpConstraintIsPivotJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpPivotJoint);
}

cpVect
cpPivotJointGetAnchorA(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsPivotJoint(constraint), "Constraint is not a pivot joint.");
	return ((cpPivotJoint *)constraint)->anchorA;
}

void
cpPivotJointSetAnchorA(cpConstraint *constraint, cpVect anchorA)
{
	cpAssertHard(cpConstraintIsPivotJoint(constraint), "Constraint is not a pivot joint.");
	cpConstraintActivateBodies(constraint);
	((cpPivotJoint *)constraint)->anchorA = anchorA;
}

cpVect
cpPivotJointGetAnchorB(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsPivotJoint(constraint), "Constraint is not a pivot joint.");
	return ((cpPivotJoint *)constraint)->anchorB;
}

void
cpPivotJointSetAnchorB(cpConstraint *constraint, cpVect anchorB)
{
	cpAssertHard(cpConstraintIsPivotJoint(constraint), "Constraint is not a pivot joint.");
	cpConstraintActivateBodies(constraint);
	((cpPivotJoint *)constraint)->anchorB = anchorB;
}

//
//MERGED FILE END: cpPivotJoint.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpPolyShape.c
//

cpPolyShape *
cpPolyShapeAlloc(void)
{
	return (cpPolyShape *)cpcalloc(1, sizeof(cpPolyShape));
}

static void
cpPolyShapeDestroy(cpPolyShape *poly)
{
	if(poly->count > CP_POLY_SHAPE_INLINE_ALLOC){
		cpfree(poly->planes);
	}
}

static cpBB
cpPolyShapeCacheData(cpPolyShape *poly, cpTransform transform)
{
	int count = poly->count;
	struct cpSplittingPlane *dst = poly->planes;
	struct cpSplittingPlane *src = dst + count;

	cpFloat l = (cpFloat)INFINITY, r = -(cpFloat)INFINITY;
	cpFloat b = (cpFloat)INFINITY, t = -(cpFloat)INFINITY;

	for(int i=0; i<count; i++){
		cpVect v = cpTransformPoint(transform, src[i].v0);
		cpVect n = cpTransformVect(transform, src[i].n);

		dst[i].v0 = v;
		dst[i].n = n;

		l = cpfmin(l, v.x);
		r = cpfmax(r, v.x);
		b = cpfmin(b, v.y);
		t = cpfmax(t, v.y);
	}

	cpFloat radius = poly->r;
	return (poly->shape.bb = cpBBNew(l - radius, b - radius, r + radius, t + radius));
}

static void
cpPolyShapePointQuery(cpPolyShape *poly, cpVect p, cpPointQueryInfo *info){
	int count = poly->count;
	struct cpSplittingPlane *planes = poly->planes;
	cpFloat r = poly->r;

	cpVect v0 = planes[count - 1].v0;
	cpFloat minDist = INFINITY;
	cpVect closestPoint = cpvzero;
	cpVect closestNormal = cpvzero;
	cpBool outside = cpFalse;

	for(int i=0; i<count; i++){
		cpVect v1 = planes[i].v0;
		outside = outside || (cpvdot(planes[i].n, cpvsub(p,v1)) > 0.0f);

		cpVect closest = cpClosetPointOnSegment(p, v0, v1);

		cpFloat dist = cpvdist(p, closest);
		if(dist < minDist){
			minDist = dist;
			closestPoint = closest;
			closestNormal = planes[i].n;
		}

		v0 = v1;
	}

	cpFloat dist = (outside ? minDist : -minDist);
	cpVect g = cpvmult(cpvsub(p, closestPoint), 1.0f/dist);

	info->shape = (cpShape *)poly;
	info->point = cpvadd(closestPoint, cpvmult(g, r));
	info->distance = dist - r;

	info->gradient = (minDist > MAGIC_EPSILON ? g : closestNormal);
}

static void
cpPolyShapeSegmentQuery(cpPolyShape *poly, cpVect a, cpVect b, cpFloat r2, cpSegmentQueryInfo *info)
{
	struct cpSplittingPlane *planes = poly->planes;
	int count = poly->count;
	cpFloat r = poly->r;
	cpFloat rsum = r + r2;

	for(int i=0; i<count; i++){
		cpVect n = planes[i].n;
		cpFloat an = cpvdot(a, n);
		cpFloat d =  an - cpvdot(planes[i].v0, n) - rsum;
		if(d < 0.0f) continue;

		cpFloat bn = cpvdot(b, n);
		cpFloat t = d/(an - bn);
		if(t < 0.0f || 1.0f < t) continue;

		cpVect point = cpvlerp(a, b, t);
		cpFloat dt = cpvcross(n, point);
		cpFloat dtMin = cpvcross(n, planes[(i - 1 + count)%count].v0);
		cpFloat dtMax = cpvcross(n, planes[i].v0);

		if(dtMin <= dt && dt <= dtMax){
			info->shape = (cpShape *)poly;
			info->point = cpvsub(cpvlerp(a, b, t), cpvmult(n, r2));
			info->normal = n;
			info->alpha = t;
		}
	}

	if(rsum > 0.0f){
		for(int i=0; i<count; i++){
			cpSegmentQueryInfo circle_info = {NULL, {b.x, b.y}, {0, 0}, 1.0f};
			CircleSegmentQuery(&poly->shape, planes[i].v0, r, a, b, r2, &circle_info);
			if(circle_info.alpha < info->alpha) (*info) = circle_info;
		}
	}
}

static void
SetVerts(cpPolyShape *poly, int count, const cpVect *verts)
{
	poly->count = count;
	if(count <= CP_POLY_SHAPE_INLINE_ALLOC){
		poly->planes = poly->_planes;
	} else {
		poly->planes = (struct cpSplittingPlane *)cpcalloc(2*count, sizeof(struct cpSplittingPlane));
	}

	for(int i=0; i<count; i++){
		cpVect a = verts[(i - 1 + count)%count];
		cpVect b = verts[i];
		cpVect n = cpvnormalize(cpvrperp(cpvsub(b, a)));

		poly->planes[i + count].v0 = b;
		poly->planes[i + count].n = n;
	}
}

static struct cpShapeMassInfo
cpPolyShapeMassInfo(cpFloat mass, int count, const cpVect *verts, cpFloat radius)
{

	cpVect centroid = cpCentroidForPoly(count, verts);
	struct cpShapeMassInfo info = {
		mass, cpMomentForPoly(1.0f, count, verts, cpvneg(centroid), radius),
		{centroid.x, centroid.y},
		cpAreaForPoly(count, verts, radius),
	};

	return info;
}

static const cpShapeClass polyClass = {
	CP_POLY_SHAPE,
	(cpShapeCacheDataImpl)cpPolyShapeCacheData,
	(cpShapeDestroyImpl)cpPolyShapeDestroy,
	(cpShapePointQueryImpl)cpPolyShapePointQuery,
	(cpShapeSegmentQueryImpl)cpPolyShapeSegmentQuery,
};

cpPolyShape *
cpPolyShapeInit(cpPolyShape *poly, cpBody *body, int count, const cpVect *verts, cpTransform transform, cpFloat radius)
{
	cpVect *hullVerts = (cpVect *)alloca(count*sizeof(cpVect));

	for(int i=0; i<count; i++) hullVerts[i] = cpTransformPoint(transform, verts[i]);

	unsigned int hullCount = cpConvexHull(count, hullVerts, hullVerts, NULL, 0.0);
	return cpPolyShapeInitRaw(poly, body, hullCount, hullVerts, radius);
}

cpPolyShape *
cpPolyShapeInitRaw(cpPolyShape *poly, cpBody *body, int count, const cpVect *verts, cpFloat radius)
{
	cpShapeInit((cpShape *)poly, &polyClass, body, cpPolyShapeMassInfo(0.0f, count, verts, radius));

	SetVerts(poly, count, verts);
	poly->r = radius;

	return poly;
}

cpShape *
cpPolyShapeNew(cpBody *body, int count, const cpVect *verts, cpTransform transform, cpFloat radius)
{
	return (cpShape *)cpPolyShapeInit(cpPolyShapeAlloc(), body, count, verts, transform, radius);
}

cpShape *
cpPolyShapeNewRaw(cpBody *body, int count, const cpVect *verts, cpFloat radius)
{
	return (cpShape *)cpPolyShapeInitRaw(cpPolyShapeAlloc(), body, count, verts, radius);
}

cpPolyShape *
cpBoxShapeInit(cpPolyShape *poly, cpBody *body, cpFloat width, cpFloat height, cpFloat radius)
{
	cpFloat hw = width/2.0f;
	cpFloat hh = height/2.0f;

	return cpBoxShapeInit2(poly, body, cpBBNew(-hw, -hh, hw, hh), radius);
}

cpPolyShape *
cpBoxShapeInit2(cpPolyShape *poly, cpBody *body, cpBB box, cpFloat radius)
{
	cpVect verts[] = {
		{box.r, box.b},
		{box.r, box.t},
		{box.l, box.t},
		{box.l, box.b},
	};

	return cpPolyShapeInitRaw(poly, body, 4, verts, radius);
}

cpShape *
cpBoxShapeNew(cpBody *body, cpFloat width, cpFloat height, cpFloat radius)
{
	return (cpShape *)cpBoxShapeInit(cpPolyShapeAlloc(), body, width, height, radius);
}

cpShape *
cpBoxShapeNew2(cpBody *body, cpBB box, cpFloat radius)
{
	return (cpShape *)cpBoxShapeInit2(cpPolyShapeAlloc(), body, box, radius);
}

int
cpPolyShapeGetCount(const cpShape *shape)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	return ((cpPolyShape *)shape)->count;
}

cpVect
cpPolyShapeGetVert(const cpShape *shape, int i)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");

	int count = cpPolyShapeGetCount(shape);
	cpAssertHard(0 <= i && i < count, "Index out of range.");

	return ((cpPolyShape *)shape)->planes[i + count].v0;
}

cpFloat
cpPolyShapeGetRadius(const cpShape *shape)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	return ((cpPolyShape *)shape)->r;
}

void
cpPolyShapeSetVerts(cpShape *shape, int count, cpVect *verts, cpTransform transform)
{
	cpVect *hullVerts = (cpVect *)alloca(count*sizeof(cpVect));

	for(int i=0; i<count; i++) hullVerts[i] = cpTransformPoint(transform, verts[i]);

	unsigned int hullCount = cpConvexHull(count, hullVerts, hullVerts, NULL, 0.0);
	cpPolyShapeSetVertsRaw(shape, hullCount, hullVerts);
}

void
cpPolyShapeSetVertsRaw(cpShape *shape, int count, cpVect *verts)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	cpPolyShape *poly = (cpPolyShape *)shape;
	cpPolyShapeDestroy(poly);

	SetVerts(poly, count, verts);

	cpFloat mass = shape->massInfo.m;
	shape->massInfo = cpPolyShapeMassInfo(shape->massInfo.m, count, verts, poly->r);
	if(mass > 0.0f) cpBodyAccumulateMassFromShapes(shape->body);
}

void
cpPolyShapeSetRadius(cpShape *shape, cpFloat radius)
{
	cpAssertHard(shape->klass == &polyClass, "Shape is not a poly shape.");
	cpPolyShape *poly = (cpPolyShape *)shape;
	poly->r = radius;

}

//
//MERGED FILE END: cpPolyShape.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpRatchetJoint.c
//

static void
preStepcpRatchetJoint(cpRatchetJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat angle = joint->angle;
	cpFloat phase = joint->phase;
	cpFloat ratchet = joint->ratchet;

	cpFloat delta = b->a - a->a;
	cpFloat diff = angle - delta;
	cpFloat pdist = 0.0f;

	if(diff*ratchet > 0.0f){
		pdist = diff;
	} else {
		joint->angle = cpffloor((delta - phase)/ratchet)*ratchet + phase;
	}

	joint->iSum = 1.0f/(a->i_inv + b->i_inv);

	cpFloat maxBias = joint->constraint.maxBias;
	joint->bias = cpfclamp(-bias_coef(joint->constraint.errorBias, dt)*pdist/dt, -maxBias, maxBias);

	if(!joint->bias) joint->jAcc = 0.0f;
}

static void
applyCachedImpulsecpRatchetJoint(cpRatchetJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat j = joint->jAcc*dt_coef;
	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static void
applyImpulsecpRatchetJoint(cpRatchetJoint *joint, cpFloat dt)
{
	if(!joint->bias) return;
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat wr = b->w - a->w;
	cpFloat ratchet = joint->ratchet;

	cpFloat jMax = joint->constraint.maxForce*dt;

	cpFloat j = -(joint->bias + wr)*joint->iSum;
	cpFloat jOld = joint->jAcc;
	joint->jAcc = cpfclamp((jOld + j)*ratchet, 0.0f, jMax*cpfabs(ratchet))/ratchet;
	j = joint->jAcc - jOld;

	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static cpFloat
getImpulsecpRatchetJoint(cpRatchetJoint *joint)
{
	return cpfabs(joint->jAcc);
}

static const cpConstraintClass klasscpRatchetJoint = {
	(cpConstraintPreStepImpl)preStepcpRatchetJoint,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpRatchetJoint,
	(cpConstraintApplyImpulseImpl)applyImpulsecpRatchetJoint,
	(cpConstraintGetImpulseImpl)getImpulsecpRatchetJoint,
};

cpRatchetJoint *
cpRatchetJointAlloc(void)
{
	return (cpRatchetJoint *)cpcalloc(1, sizeof(cpRatchetJoint));
}

cpRatchetJoint *
cpRatchetJointInit(cpRatchetJoint *joint, cpBody *a, cpBody *b, cpFloat phase, cpFloat ratchet)
{
	cpConstraintInit((cpConstraint *)joint, &klasscpRatchetJoint, a, b);

	joint->angle = 0.0f;
	joint->phase = phase;
	joint->ratchet = ratchet;

	joint->angle = (b ? b->a : 0.0f) - (a ? a->a : 0.0f);

	return joint;
}

cpConstraint *
cpRatchetJointNew(cpBody *a, cpBody *b, cpFloat phase, cpFloat ratchet)
{
	return (cpConstraint *)cpRatchetJointInit(cpRatchetJointAlloc(), a, b, phase, ratchet);
}

cpBool
cpConstraintIsRatchetJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpRatchetJoint);
}

cpFloat
cpRatchetJointGetAngle(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	return ((cpRatchetJoint *)constraint)->angle;
}

void
cpRatchetJointSetAngle(cpConstraint *constraint, cpFloat angle)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	cpConstraintActivateBodies(constraint);
	((cpRatchetJoint *)constraint)->angle = angle;
}

cpFloat
cpRatchetJointGetPhase(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	return ((cpRatchetJoint *)constraint)->phase;
}

void
cpRatchetJointSetPhase(cpConstraint *constraint, cpFloat phase)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	cpConstraintActivateBodies(constraint);
	((cpRatchetJoint *)constraint)->phase = phase;
}
cpFloat
cpRatchetJointGetRatchet(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	return ((cpRatchetJoint *)constraint)->ratchet;
}

void
cpRatchetJointSetRatchet(cpConstraint *constraint, cpFloat ratchet)
{
	cpAssertHard(cpConstraintIsRatchetJoint(constraint), "Constraint is not a ratchet joint.");
	cpConstraintActivateBodies(constraint);
	((cpRatchetJoint *)constraint)->ratchet = ratchet;
}

//
//MERGED FILE END: cpRatchetJoint.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpRobust.c
//

cpBool
cpCheckSignedArea(const cpVect a, const cpVect b, const cpVect c)
{
	const cpVect v0 = cpvsub(b, a);
	const cpVect v1 = cpvadd(cpvsub(c, a), cpvsub(c, b));
	return (v0.x*v1.y) > (v1.x*v0.y);
}

//
//MERGED FILE END: cpRobust.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpRotaryLimitJoint.c
//

static void
preStepcpRotaryLimitJoint(cpRotaryLimitJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat dist = b->a - a->a;
	cpFloat pdist = 0.0f;
	if(dist > joint->max) {
		pdist = joint->max - dist;
	} else if(dist < joint->min) {
		pdist = joint->min - dist;
	}

	joint->iSum = 1.0f/(a->i_inv + b->i_inv);

	cpFloat maxBias = joint->constraint.maxBias;
	joint->bias = cpfclamp(-bias_coef(joint->constraint.errorBias, dt)*pdist/dt, -maxBias, maxBias);

	if(!joint->bias) joint->jAcc = 0.0f;
}

static void
applyCachedImpulsecpRotaryLimitJoint(cpRotaryLimitJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat j = joint->jAcc*dt_coef;
	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static void
applyImpulsecpRotaryLimitJoint(cpRotaryLimitJoint *joint, cpFloat dt)
{
	if(!joint->bias) return;
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat wr = b->w - a->w;

	cpFloat jMax = joint->constraint.maxForce*dt;

	cpFloat j = -(joint->bias + wr)*joint->iSum;
	cpFloat jOld = joint->jAcc;
	if(joint->bias < 0.0f){
		joint->jAcc = cpfclamp(jOld + j, 0.0f, jMax);
	} else {
		joint->jAcc = cpfclamp(jOld + j, -jMax, 0.0f);
	}
	j = joint->jAcc - jOld;

	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static cpFloat
getImpulsecpRotaryLimitJoint(cpRotaryLimitJoint *joint)
{
	return cpfabs(joint->jAcc);
}

static const cpConstraintClass klasscpRotaryLimitJoint = {
	(cpConstraintPreStepImpl)preStepcpRotaryLimitJoint,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpRotaryLimitJoint,
	(cpConstraintApplyImpulseImpl)applyImpulsecpRotaryLimitJoint,
	(cpConstraintGetImpulseImpl)getImpulsecpRotaryLimitJoint,
};

cpRotaryLimitJoint *
cpRotaryLimitJointAlloc(void)
{
	return (cpRotaryLimitJoint *)cpcalloc(1, sizeof(cpRotaryLimitJoint));
}

cpRotaryLimitJoint *
cpRotaryLimitJointInit(cpRotaryLimitJoint *joint, cpBody *a, cpBody *b, cpFloat min, cpFloat max)
{
	cpConstraintInit((cpConstraint *)joint, &klasscpRotaryLimitJoint, a, b);

	joint->min = min;
	joint->max  = max;

	joint->jAcc = 0.0f;

	return joint;
}

cpConstraint *
cpRotaryLimitJointNew(cpBody *a, cpBody *b, cpFloat min, cpFloat max)
{
	return (cpConstraint *)cpRotaryLimitJointInit(cpRotaryLimitJointAlloc(), a, b, min, max);
}

cpBool
cpConstraintIsRotaryLimitJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpRotaryLimitJoint);
}

cpFloat
cpRotaryLimitJointGetMin(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsRotaryLimitJoint(constraint), "Constraint is not a rotary limit joint.");
	return ((cpRotaryLimitJoint *)constraint)->min;
}

void
cpRotaryLimitJointSetMin(cpConstraint *constraint, cpFloat min)
{
	cpAssertHard(cpConstraintIsRotaryLimitJoint(constraint), "Constraint is not a rotary limit joint.");
	cpConstraintActivateBodies(constraint);
	((cpRotaryLimitJoint *)constraint)->min = min;
}

cpFloat
cpRotaryLimitJointGetMax(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsRotaryLimitJoint(constraint), "Constraint is not a rotary limit joint.");
	return ((cpRotaryLimitJoint *)constraint)->max;
}

void
cpRotaryLimitJointSetMax(cpConstraint *constraint, cpFloat max)
{
	cpAssertHard(cpConstraintIsRotaryLimitJoint(constraint), "Constraint is not a rotary limit joint.");
	cpConstraintActivateBodies(constraint);
	((cpRotaryLimitJoint *)constraint)->max = max;
}

//
//MERGED FILE END: cpRotaryLimitJoint.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpShape.c
//

#define CP_DefineShapeGetter(struct, type, member, name) \
CP_DeclareShapeGetter(struct, type, name){ \
	cpAssertHard(shape->klass == &struct##Class, "shape is not a "#struct); \
	return ((struct *)shape)->member; \
}

cpShape *
cpShapeInit(cpShape *shape, const cpShapeClass *klass, cpBody *body, struct cpShapeMassInfo massInfo)
{
	shape->klass = klass;

	shape->body = body;
	shape->massInfo = massInfo;

	shape->sensor = 0;

	shape->e = 0.0f;
	shape->u = 0.0f;
	shape->surfaceV = cpvzero;

	shape->type = 0;
	shape->filter.group = CP_NO_GROUP;
	shape->filter.categories = CP_ALL_CATEGORIES;
	shape->filter.mask = CP_ALL_CATEGORIES;

	shape->userData = NULL;

	shape->space = NULL;

	shape->next = NULL;
	shape->prev = NULL;

	return shape;
}

void
cpShapeDestroy(cpShape *shape)
{
	if(shape->klass && shape->klass->destroy) shape->klass->destroy(shape);
}

void
cpShapeFree(cpShape *shape)
{
	if(shape){
		cpShapeDestroy(shape);
		cpfree(shape);
	}
}

cpSpace *
cpShapeGetSpace(const cpShape *shape)
{
	return shape->space;
}

cpBody *
cpShapeGetBody(const cpShape *shape)
{
	return shape->body;
}

void
cpShapeSetBody(cpShape *shape, cpBody *body)
{
	cpAssertHard(!cpShapeActive(shape), "You cannot change the body on an active shape. You must remove the shape from the space before changing the body.");
	shape->body = body;
}

cpFloat cpShapeGetMass(cpShape *shape){ return shape->massInfo.m; }

void
cpShapeSetMass(cpShape *shape, cpFloat mass){
	cpBody *body = shape->body;
	cpBodyActivate(body);

	shape->massInfo.m = mass;
	cpBodyAccumulateMassFromShapes(body);
}

cpFloat cpShapeGetDensity(cpShape *shape){ return shape->massInfo.m/shape->massInfo.area; }
void cpShapeSetDensity(cpShape *shape, cpFloat density){ cpShapeSetMass(shape, density*shape->massInfo.area); }

cpFloat cpShapeGetMoment(cpShape *shape){ return shape->massInfo.m*shape->massInfo.i; }
cpFloat cpShapeGetArea(cpShape *shape){ return shape->massInfo.area; }
cpVect cpShapeGetCenterOfGravity(cpShape *shape) { return shape->massInfo.cog; }

cpBB
cpShapeGetBB(const cpShape *shape)
{
	return shape->bb;
}

cpBool
cpShapeGetSensor(const cpShape *shape)
{
	return shape->sensor;
}

void
cpShapeSetSensor(cpShape *shape, cpBool sensor)
{
	cpBodyActivate(shape->body);
	shape->sensor = sensor;
}

cpFloat
cpShapeGetElasticity(const cpShape *shape)
{
	return shape->e;
}

void
cpShapeSetElasticity(cpShape *shape, cpFloat elasticity)
{
	cpAssertHard(elasticity >= 0.0f, "Elasticity must be positive and non-zero.");
	cpBodyActivate(shape->body);
	shape->e = elasticity;
}

cpFloat
cpShapeGetFriction(const cpShape *shape)
{
	return shape->u;
}

void
cpShapeSetFriction(cpShape *shape, cpFloat friction)
{
	cpAssertHard(friction >= 0.0f, "Friction must be postive and non-zero.");
	cpBodyActivate(shape->body);
	shape->u = friction;
}

cpVect
cpShapeGetSurfaceVelocity(const cpShape *shape)
{
	return shape->surfaceV;
}

void
cpShapeSetSurfaceVelocity(cpShape *shape, cpVect surfaceVelocity)
{
	cpBodyActivate(shape->body);
	shape->surfaceV = surfaceVelocity;
}

cpDataPointer
cpShapeGetUserData(const cpShape *shape)
{
	return shape->userData;
}

void
cpShapeSetUserData(cpShape *shape, cpDataPointer userData)
{
	shape->userData = userData;
}

cpCollisionType
cpShapeGetCollisionType(const cpShape *shape)
{
	return shape->type;
}

void
cpShapeSetCollisionType(cpShape *shape, cpCollisionType collisionType)
{
	cpBodyActivate(shape->body);
	shape->type = collisionType;
}

cpShapeFilter
cpShapeGetFilter(const cpShape *shape)
{
	return shape->filter;
}

void
cpShapeSetFilter(cpShape *shape, cpShapeFilter filter)
{
	cpBodyActivate(shape->body);
	shape->filter = filter;
}

cpBB
cpShapeCacheBB(cpShape *shape)
{
	return cpShapeUpdate(shape, shape->body->transform);
}

cpBB
cpShapeUpdate(cpShape *shape, cpTransform transform)
{
	return (shape->bb = shape->klass->cacheData(shape, transform));
}

cpFloat
cpShapePointQuery(const cpShape *shape, cpVect p, cpPointQueryInfo *info)
{
	cpPointQueryInfo blank = {NULL, {0, 0}, (cpFloat)INFINITY, {0, 0}};
	if(info){
		(*info) = blank;
	} else {
		info = &blank;
	}

	shape->klass->pointQuery(shape, p, info);
	return info->distance;
}

cpBool
cpShapeSegmentQuery(const cpShape *shape, cpVect a, cpVect b, cpFloat radius, cpSegmentQueryInfo *info){
	cpSegmentQueryInfo blank = {NULL, {b.x, b.y}, {0, 0}, 1.0f};
	if(info){
		(*info) = blank;
	} else {
		info = &blank;
	}

	cpPointQueryInfo nearest;
	shape->klass->pointQuery(shape, a, &nearest);
	if(nearest.distance <= radius){
		info->shape = shape;
		info->alpha = 0.0;
		info->normal = cpvnormalize(cpvsub(a, nearest.point));
	} else {
		shape->klass->segmentQuery(shape, a, b, radius, info);
	}

	return (info->shape != NULL);
}

cpContactPointSet
cpShapesCollide(const cpShape *a, const cpShape *b)
{
	struct cpContact contacts[CP_MAX_CONTACTS_PER_ARBITER];
	struct cpCollisionInfo info = cpCollide(a, b, 0, contacts);

	cpContactPointSet set;
	set.count = info.count;

	cpBool swapped = (a != info.a);
	set.normal = (swapped ? cpvneg(info.n) : info.n);

	for(int i=0; i<info.count; i++){
		cpVect p1 = contacts[i].r1;
		cpVect p2 = contacts[i].r2;

		set.points[i].pointA = (swapped ? p2 : p1);
		set.points[i].pointB = (swapped ? p1 : p2);
		set.points[i].distance = cpvdot(cpvsub(p2, p1), set.normal);
	}

	return set;
}

cpCircleShape *
cpCircleShapeAlloc(void)
{
	return (cpCircleShape *)cpcalloc(1, sizeof(cpCircleShape));
}

static cpBB
cpCircleShapeCacheData(cpCircleShape *circle, cpTransform transform)
{
	cpVect c = circle->tc = cpTransformPoint(transform, circle->c);
	return cpBBNewForCircle(c, circle->r);
}

static void
cpCircleShapePointQuery(cpCircleShape *circle, cpVect p, cpPointQueryInfo *info)
{
	cpVect delta = cpvsub(p, circle->tc);
	cpFloat d = cpvlength(delta);
	cpFloat r = circle->r;

	info->shape = (cpShape *)circle;
	info->point = cpvadd(circle->tc, cpvmult(delta, r/d));	info->distance = d - r;

	info->gradient = (d > MAGIC_EPSILON ? cpvmult(delta, 1.0f/d) : cpv(0.0f, 1.0f));
}

static void
cpCircleShapeSegmentQuery(cpCircleShape *circle, cpVect a, cpVect b, cpFloat radius, cpSegmentQueryInfo *info)
{
	CircleSegmentQuery((cpShape *)circle, circle->tc, circle->r, a, b, radius, info);
}

static struct cpShapeMassInfo
cpCircleShapeMassInfo(cpFloat mass, cpFloat radius, cpVect center)
{
	struct cpShapeMassInfo info = {
		mass, cpMomentForCircle(1.0f, 0.0f, radius, cpvzero),
		{center.x, center.y},
		cpAreaForCircle(0.0f, radius),
	};

	return info;
}

static const cpShapeClass cpCircleShapeClass = {
	CP_CIRCLE_SHAPE,
	(cpShapeCacheDataImpl)cpCircleShapeCacheData,
	NULL,
	(cpShapePointQueryImpl)cpCircleShapePointQuery,
	(cpShapeSegmentQueryImpl)cpCircleShapeSegmentQuery,
};

cpCircleShape *
cpCircleShapeInit(cpCircleShape *circle, cpBody *body, cpFloat radius, cpVect offset)
{
	circle->c = offset;
	circle->r = radius;

	cpShapeInit((cpShape *)circle, &cpCircleShapeClass, body, cpCircleShapeMassInfo(0.0f, radius, offset));

	return circle;
}

cpShape *
cpCircleShapeNew(cpBody *body, cpFloat radius, cpVect offset)
{
	return (cpShape *)cpCircleShapeInit(cpCircleShapeAlloc(), body, radius, offset);
}

cpVect
cpCircleShapeGetOffset(const cpShape *shape)
{
	cpAssertHard(shape->klass == &cpCircleShapeClass, "Shape is not a circle shape.");
	return ((cpCircleShape *)shape)->c;
}

cpFloat
cpCircleShapeGetRadius(const cpShape *shape)
{
	cpAssertHard(shape->klass == &cpCircleShapeClass, "Shape is not a circle shape.");
	return ((cpCircleShape *)shape)->r;
}

cpSegmentShape *
cpSegmentShapeAlloc(void)
{
	return (cpSegmentShape *)cpcalloc(1, sizeof(cpSegmentShape));
}

static cpBB
cpSegmentShapeCacheData(cpSegmentShape *seg, cpTransform transform)
{
	seg->ta = cpTransformPoint(transform, seg->a);
	seg->tb = cpTransformPoint(transform, seg->b);
	seg->tn = cpTransformVect(transform, seg->n);

	cpFloat l,r,b,t;

	if(seg->ta.x < seg->tb.x){
		l = seg->ta.x;
		r = seg->tb.x;
	} else {
		l = seg->tb.x;
		r = seg->ta.x;
	}

	if(seg->ta.y < seg->tb.y){
		b = seg->ta.y;
		t = seg->tb.y;
	} else {
		b = seg->tb.y;
		t = seg->ta.y;
	}

	cpFloat rad = seg->r;
	return cpBBNew(l - rad, b - rad, r + rad, t + rad);
}

static void
cpSegmentShapePointQuery(cpSegmentShape *seg, cpVect p, cpPointQueryInfo *info)
{
	cpVect closest = cpClosetPointOnSegment(p, seg->ta, seg->tb);

	cpVect delta = cpvsub(p, closest);
	cpFloat d = cpvlength(delta);
	cpFloat r = seg->r;
	cpVect g = cpvmult(delta, 1.0f/d);

	info->shape = (cpShape *)seg;
	info->point = (d ? cpvadd(closest, cpvmult(g, r)) : closest);
	info->distance = d - r;

	info->gradient = (d > MAGIC_EPSILON ? g : seg->n);
}

static void
cpSegmentShapeSegmentQuery(cpSegmentShape *seg, cpVect a, cpVect b, cpFloat r2, cpSegmentQueryInfo *info)
{
	cpVect n = seg->tn;
	cpFloat d = cpvdot(cpvsub(seg->ta, a), n);
	cpFloat r = seg->r + r2;

	cpVect flipped_n = (d > 0.0f ? cpvneg(n) : n);
	cpVect seg_offset = cpvsub(cpvmult(flipped_n, r), a);

	cpVect seg_a = cpvadd(seg->ta, seg_offset);
	cpVect seg_b = cpvadd(seg->tb, seg_offset);
	cpVect delta = cpvsub(b, a);

	if(cpvcross(delta, seg_a)*cpvcross(delta, seg_b) <= 0.0f){
		cpFloat d_offset = d + (d > 0.0f ? -r : r);
		cpFloat ad = -d_offset;
		cpFloat bd = cpvdot(delta, n) - d_offset;

		if(ad*bd < 0.0f){
			cpFloat t = ad/(ad - bd);

			info->shape = (cpShape *)seg;
			info->point = cpvsub(cpvlerp(a, b, t), cpvmult(flipped_n, r2));
			info->normal = flipped_n;
			info->alpha = t;
		}
	} else if(r != 0.0f){
		cpSegmentQueryInfo info1 = {NULL, {b.x, b.y}, {0, 0}, 1.0f};
		cpSegmentQueryInfo info2 = {NULL, {b.x, b.y}, {0, 0}, 1.0f};
		CircleSegmentQuery((cpShape *)seg, seg->ta, seg->r, a, b, r2, &info1);
		CircleSegmentQuery((cpShape *)seg, seg->tb, seg->r, a, b, r2, &info2);

		if(info1.alpha < info2.alpha){
			(*info) = info1;
		} else {
			(*info) = info2;
		}
	}
}

static struct cpShapeMassInfo
cpSegmentShapeMassInfo(cpFloat mass, cpVect a, cpVect b, cpFloat r)
{
	struct cpShapeMassInfo info = {
		mass, cpMomentForBox(1.0f, cpvdist(a, b) + 2.0f*r, 2.0f*r), {(a.x+b.x)*0.5f, (b.x+b.y)*0.5f},
		cpAreaForSegment(a, b, r),
	};

	return info;
}

static const cpShapeClass cpSegmentShapeClass = {
	CP_SEGMENT_SHAPE,
	(cpShapeCacheDataImpl)cpSegmentShapeCacheData,
	NULL,
	(cpShapePointQueryImpl)cpSegmentShapePointQuery,
	(cpShapeSegmentQueryImpl)cpSegmentShapeSegmentQuery,
};

cpSegmentShape *
cpSegmentShapeInit(cpSegmentShape *seg, cpBody *body, cpVect a, cpVect b, cpFloat r)
{
	seg->a = a;
	seg->b = b;
	seg->n = cpvrperp(cpvnormalize(cpvsub(b, a)));

	seg->r = r;

	seg->a_tangent = cpvzero;
	seg->b_tangent = cpvzero;

	cpShapeInit((cpShape *)seg, &cpSegmentShapeClass, body, cpSegmentShapeMassInfo(0.0f, a, b, r));

	return seg;
}

cpShape*
cpSegmentShapeNew(cpBody *body, cpVect a, cpVect b, cpFloat r)
{
	return (cpShape *)cpSegmentShapeInit(cpSegmentShapeAlloc(), body, a, b, r);
}

cpVect
cpSegmentShapeGetA(const cpShape *shape)
{
	cpAssertHard(shape->klass == &cpSegmentShapeClass, "Shape is not a segment shape.");
	return ((cpSegmentShape *)shape)->a;
}

cpVect
cpSegmentShapeGetB(const cpShape *shape)
{
	cpAssertHard(shape->klass == &cpSegmentShapeClass, "Shape is not a segment shape.");
	return ((cpSegmentShape *)shape)->b;
}

cpVect
cpSegmentShapeGetNormal(const cpShape *shape)
{
	cpAssertHard(shape->klass == &cpSegmentShapeClass, "Shape is not a segment shape.");
	return ((cpSegmentShape *)shape)->n;
}

cpFloat
cpSegmentShapeGetRadius(const cpShape *shape)
{
	cpAssertHard(shape->klass == &cpSegmentShapeClass, "Shape is not a segment shape.");
	return ((cpSegmentShape *)shape)->r;
}

void
cpSegmentShapeSetNeighbors(cpShape *shape, cpVect prev, cpVect next)
{
	cpAssertHard(shape->klass == &cpSegmentShapeClass, "Shape is not a segment shape.");
	cpSegmentShape *seg = (cpSegmentShape *)shape;

	seg->a_tangent = cpvsub(prev, seg->a);
	seg->b_tangent = cpvsub(next, seg->b);
}

void
cpCircleShapeSetRadius(cpShape *shape, cpFloat radius)
{
	cpAssertHard(shape->klass == &cpCircleShapeClass, "Shape is not a circle shape.");
	cpCircleShape *circle = (cpCircleShape *)shape;

	circle->r = radius;

	cpFloat mass = shape->massInfo.m;
	shape->massInfo = cpCircleShapeMassInfo(mass, circle->r, circle->c);
	if(mass > 0.0f) cpBodyAccumulateMassFromShapes(shape->body);
}

void
cpCircleShapeSetOffset(cpShape *shape, cpVect offset)
{
	cpAssertHard(shape->klass == &cpCircleShapeClass, "Shape is not a circle shape.");
	cpCircleShape *circle = (cpCircleShape *)shape;

	circle->c = offset;

	cpFloat mass = shape->massInfo.m;
	shape->massInfo = cpCircleShapeMassInfo(shape->massInfo.m, circle->r, circle->c);
	if(mass > 0.0f) cpBodyAccumulateMassFromShapes(shape->body);
}

void
cpSegmentShapeSetEndpoints(cpShape *shape, cpVect a, cpVect b)
{
	cpAssertHard(shape->klass == &cpSegmentShapeClass, "Shape is not a segment shape.");
	cpSegmentShape *seg = (cpSegmentShape *)shape;

	seg->a = a;
	seg->b = b;
	seg->n = cpvperp(cpvnormalize(cpvsub(b, a)));

	cpFloat mass = shape->massInfo.m;
	shape->massInfo = cpSegmentShapeMassInfo(shape->massInfo.m, seg->a, seg->b, seg->r);
	if(mass > 0.0f) cpBodyAccumulateMassFromShapes(shape->body);
}

void
cpSegmentShapeSetRadius(cpShape *shape, cpFloat radius)
{
	cpAssertHard(shape->klass == &cpSegmentShapeClass, "Shape is not a segment shape.");
	cpSegmentShape *seg = (cpSegmentShape *)shape;

	seg->r = radius;

	cpFloat mass = shape->massInfo.m;
	shape->massInfo = cpSegmentShapeMassInfo(shape->massInfo.m, seg->a, seg->b, seg->r);
	if(mass > 0.0f) cpBodyAccumulateMassFromShapes(shape->body);
}

//
//MERGED FILE END: cpShape.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSimpleMotor.c
//

static void
preStepcpSimpleMotor(cpSimpleMotor *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	joint->iSum = 1.0f/(a->i_inv + b->i_inv);
}

static void
applyCachedImpulsecpSimpleMotor(cpSimpleMotor *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat j = joint->jAcc*dt_coef;
	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static void
applyImpulsecpSimpleMotor(cpSimpleMotor *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpFloat wr = b->w - a->w + joint->rate;

	cpFloat jMax = joint->constraint.maxForce*dt;

	cpFloat j = -wr*joint->iSum;
	cpFloat jOld = joint->jAcc;
	joint->jAcc = cpfclamp(jOld + j, -jMax, jMax);
	j = joint->jAcc - jOld;

	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static cpFloat
getImpulsecpSimpleMotor(cpSimpleMotor *joint)
{
	return cpfabs(joint->jAcc);
}

static const cpConstraintClass klasscpSimpleMotor = {
	(cpConstraintPreStepImpl)preStepcpSimpleMotor,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpSimpleMotor,
	(cpConstraintApplyImpulseImpl)applyImpulsecpSimpleMotor,
	(cpConstraintGetImpulseImpl)getImpulsecpSimpleMotor,
};

cpSimpleMotor *
cpSimpleMotorAlloc(void)
{
	return (cpSimpleMotor *)cpcalloc(1, sizeof(cpSimpleMotor));
}

cpSimpleMotor *
cpSimpleMotorInit(cpSimpleMotor *joint, cpBody *a, cpBody *b, cpFloat rate)
{
	cpConstraintInit((cpConstraint *)joint, &klasscpSimpleMotor, a, b);

	joint->rate = rate;

	joint->jAcc = 0.0f;

	return joint;
}

cpConstraint *
cpSimpleMotorNew(cpBody *a, cpBody *b, cpFloat rate)
{
	return (cpConstraint *)cpSimpleMotorInit(cpSimpleMotorAlloc(), a, b, rate);
}

cpBool
cpConstraintIsSimpleMotor(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpSimpleMotor);
}

cpFloat
cpSimpleMotorGetRate(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsSimpleMotor(constraint), "Constraint is not a pin joint.");
	return ((cpSimpleMotor *)constraint)->rate;
}

void
cpSimpleMotorSetRate(cpConstraint *constraint, cpFloat rate)
{
	cpAssertHard(cpConstraintIsSimpleMotor(constraint), "Constraint is not a pin joint.");
	cpConstraintActivateBodies(constraint);
	((cpSimpleMotor *)constraint)->rate = rate;
}

//
//MERGED FILE END: cpSimpleMotor.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSlideJoint.c
//

static void
preStepcpSlideJoint(cpSlideJoint *joint, cpFloat dt)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	joint->r1 = cpTransformVect(a->transform, cpvsub(joint->anchorA, a->cog));
	joint->r2 = cpTransformVect(b->transform, cpvsub(joint->anchorB, b->cog));

	cpVect delta = cpvsub(cpvadd(b->p, joint->r2), cpvadd(a->p, joint->r1));
	cpFloat dist = cpvlength(delta);
	cpFloat pdist = 0.0f;
	if(dist > joint->max) {
		pdist = dist - joint->max;
		joint->n = cpvnormalize(delta);
	} else if(dist < joint->min) {
		pdist = joint->min - dist;
		joint->n = cpvneg(cpvnormalize(delta));
	} else {
		joint->n = cpvzero;
		joint->jnAcc = 0.0f;
	}

	joint->nMass = 1.0f/k_scalar(a, b, joint->r1, joint->r2, joint->n);

	cpFloat maxBias = joint->constraint.maxBias;
	joint->bias = cpfclamp(-bias_coef(joint->constraint.errorBias, dt)*pdist/dt, -maxBias, maxBias);
}

static void
applyCachedImpulsecpSlideJoint(cpSlideJoint *joint, cpFloat dt_coef)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpVect j = cpvmult(joint->n, joint->jnAcc*dt_coef);
	apply_impulses(a, b, joint->r1, joint->r2, j);
}

static void
applyImpulsecpSlideJoint(cpSlideJoint *joint, cpFloat dt)
{
	if(cpveql(joint->n, cpvzero)) return;
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;

	cpVect n = joint->n;
	cpVect r1 = joint->r1;
	cpVect r2 = joint->r2;

	cpVect vr = relative_velocity(a, b, r1, r2);
	cpFloat vrn = cpvdot(vr, n);

	cpFloat jn = (joint->bias - vrn)*joint->nMass;
	cpFloat jnOld = joint->jnAcc;
	joint->jnAcc = cpfclamp(jnOld + jn, -joint->constraint.maxForce*dt, 0.0f);
	jn = joint->jnAcc - jnOld;

	apply_impulses(a, b, joint->r1, joint->r2, cpvmult(n, jn));
}

static cpFloat
getImpulsecpSlideJoint(cpConstraint *joint)
{
	return cpfabs(((cpSlideJoint *)joint)->jnAcc);
}

static const cpConstraintClass klasscpSlideJoint = {
	(cpConstraintPreStepImpl)preStepcpSlideJoint,
	(cpConstraintApplyCachedImpulseImpl)applyCachedImpulsecpSlideJoint,
	(cpConstraintApplyImpulseImpl)applyImpulsecpSlideJoint,
	(cpConstraintGetImpulseImpl)getImpulsecpSlideJoint,
};

cpSlideJoint *
cpSlideJointAlloc(void)
{
	return (cpSlideJoint *)cpcalloc(1, sizeof(cpSlideJoint));
}

cpSlideJoint *
cpSlideJointInit(cpSlideJoint *joint, cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB, cpFloat min, cpFloat max)
{
	cpConstraintInit((cpConstraint *)joint, &klasscpSlideJoint, a, b);

	joint->anchorA = anchorA;
	joint->anchorB = anchorB;
	joint->min = min;
	joint->max = max;

	joint->jnAcc = 0.0f;

	return joint;
}

cpConstraint *
cpSlideJointNew(cpBody *a, cpBody *b, cpVect anchorA, cpVect anchorB, cpFloat min, cpFloat max)
{
	return (cpConstraint *)cpSlideJointInit(cpSlideJointAlloc(), a, b, anchorA, anchorB, min, max);
}

cpBool
cpConstraintIsSlideJoint(const cpConstraint *constraint)
{
	return (constraint->klass == &klasscpSlideJoint);
}

cpVect
cpSlideJointGetAnchorA(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsSlideJoint(constraint), "Constraint is not a slide joint.");
	return ((cpSlideJoint *)constraint)->anchorA;
}

void
cpSlideJointSetAnchorA(cpConstraint *constraint, cpVect anchorA)
{
	cpAssertHard(cpConstraintIsSlideJoint(constraint), "Constraint is not a slide joint.");
	cpConstraintActivateBodies(constraint);
	((cpSlideJoint *)constraint)->anchorA = anchorA;
}

cpVect
cpSlideJointGetAnchorB(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsSlideJoint(constraint), "Constraint is not a slide joint.");
	return ((cpSlideJoint *)constraint)->anchorB;
}

void
cpSlideJointSetAnchorB(cpConstraint *constraint, cpVect anchorB)
{
	cpAssertHard(cpConstraintIsSlideJoint(constraint), "Constraint is not a slide joint.");
	cpConstraintActivateBodies(constraint);
	((cpSlideJoint *)constraint)->anchorB = anchorB;
}

cpFloat
cpSlideJointGetMin(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsSlideJoint(constraint), "Constraint is not a slide joint.");
	return ((cpSlideJoint *)constraint)->min;
}

void
cpSlideJointSetMin(cpConstraint *constraint, cpFloat min)
{
	cpAssertHard(cpConstraintIsSlideJoint(constraint), "Constraint is not a slide joint.");
	cpConstraintActivateBodies(constraint);
	((cpSlideJoint *)constraint)->min = min;
}

cpFloat
cpSlideJointGetMax(const cpConstraint *constraint)
{
	cpAssertHard(cpConstraintIsSlideJoint(constraint), "Constraint is not a slide joint.");
	return ((cpSlideJoint *)constraint)->max;
}

void
cpSlideJointSetMax(cpConstraint *constraint, cpFloat max)
{
	cpAssertHard(cpConstraintIsSlideJoint(constraint), "Constraint is not a slide joint.");
	cpConstraintActivateBodies(constraint);
	((cpSlideJoint *)constraint)->max = max;
}

//
//MERGED FILE END: cpSlideJoint.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSpace.c
//

#include <stdio.h>
#include <string.h>

static cpBool
arbiterSetEql(cpShape **shapes, cpArbiter *arb)
{
	cpShape *a = shapes[0];
	cpShape *b = shapes[1];

	return ((a == arb->a && b == arb->b) || (b == arb->a && a == arb->b));
}

static cpBool
handlerSetEql(cpCollisionHandler *check, cpCollisionHandler *pair)
{
	return (
		(check->typeA == pair->typeA && check->typeB == pair->typeB) ||
		(check->typeB == pair->typeA && check->typeA == pair->typeB)
	);
}

static void *
handlerSetTrans(cpCollisionHandler *handler, void *unused)
{
	cpCollisionHandler *copy = (cpCollisionHandler *)cpcalloc(1, sizeof(cpCollisionHandler));
	memcpy(copy, handler, sizeof(cpCollisionHandler));

	return copy;
}

static cpBool
DefaultBegin(cpArbiter *arb, cpSpace *space, void *data){
	cpBool retA = cpArbiterCallWildcardBeginA(arb, space);
	cpBool retB = cpArbiterCallWildcardBeginB(arb, space);
	return retA && retB;
}

static cpBool
DefaultPreSolve(cpArbiter *arb, cpSpace *space, void *data){
	cpBool retA = cpArbiterCallWildcardPreSolveA(arb, space);
	cpBool retB = cpArbiterCallWildcardPreSolveB(arb, space);
	return retA && retB;
}

static void
DefaultPostSolve(cpArbiter *arb, cpSpace *space, void *data){
	cpArbiterCallWildcardPostSolveA(arb, space);
	cpArbiterCallWildcardPostSolveB(arb, space);
}

static void
DefaultSeparate(cpArbiter *arb, cpSpace *space, void *data){
	cpArbiterCallWildcardSeparateA(arb, space);
	cpArbiterCallWildcardSeparateB(arb, space);
}

static cpCollisionHandler cpCollisionHandlerDefault = {
	CP_WILDCARD_COLLISION_TYPE, CP_WILDCARD_COLLISION_TYPE,
	DefaultBegin, DefaultPreSolve, DefaultPostSolve, DefaultSeparate, NULL
};

static cpBool AlwaysCollide(cpArbiter *arb, cpSpace *space, void *data){return cpTrue;}
static void DoNothing(cpArbiter *arb, cpSpace *space, void *data){}

cpCollisionHandler cpCollisionHandlerDoNothing = {
	CP_WILDCARD_COLLISION_TYPE, CP_WILDCARD_COLLISION_TYPE,
	AlwaysCollide, AlwaysCollide, DoNothing, DoNothing, NULL
};

static cpVect ShapeVelocityFunc(cpShape *shape){return shape->body->v;}

static void FreeWrap(void *ptr, void *unused){cpfree(ptr);}

cpSpace *
cpSpaceAlloc(void)
{
	return (cpSpace *)cpcalloc(1, sizeof(cpSpace));
}

cpSpace*
cpSpaceInit(cpSpace *space)
{
#ifndef NDEBUG
	static cpBool done = cpFalse;
	if(!done){
		printf("Initializing cpSpace - Chipmunk v%s (Debug Enabled)\n", cpVersionString);
		printf("Compile with -DNDEBUG defined to disable debug mode and runtime assertion checks\n");
		done = cpTrue;
	}
#endif

	space->iterations = 10;

	space->gravity = cpvzero;
	space->damping = 1.0f;

	space->collisionSlop = 0.1f;
	space->collisionBias = cpfpow(1.0f - 0.1f, 60.0f);
	space->collisionPersistence = 3;

	space->locked = 0;
	space->stamp = 0;

	space->shapeIDCounter = 0;
	space->staticShapes = cpBBTreeNew((cpSpatialIndexBBFunc)cpShapeGetBB, NULL);
	space->dynamicShapes = cpBBTreeNew((cpSpatialIndexBBFunc)cpShapeGetBB, space->staticShapes);
	cpBBTreeSetVelocityFunc(space->dynamicShapes, (cpBBTreeVelocityFunc)ShapeVelocityFunc);

	space->allocatedBuffers = cpArrayNew(0);

	space->dynamicBodies = cpArrayNew(0);
	space->staticBodies = cpArrayNew(0);
	space->sleepingComponents = cpArrayNew(0);
	space->rousedBodies = cpArrayNew(0);

	space->sleepTimeThreshold = INFINITY;
	space->idleSpeedThreshold = 0.0f;

	space->arbiters = cpArrayNew(0);
	space->pooledArbiters = cpArrayNew(0);

	space->contactBuffersHead = NULL;
	space->cachedArbiters = cpHashSetNew(0, (cpHashSetEqlFunc)arbiterSetEql);

	space->constraints = cpArrayNew(0);

	space->usesWildcards = cpFalse;
	memcpy(&space->defaultHandler, &cpCollisionHandlerDoNothing, sizeof(cpCollisionHandler));
	space->collisionHandlers = cpHashSetNew(0, (cpHashSetEqlFunc)handlerSetEql);

	space->postStepCallbacks = cpArrayNew(0);
	space->skipPostStep = cpFalse;

	cpBody *staticBody = cpBodyInit(&space->_staticBody, 0.0f, 0.0f);
	cpBodySetType(staticBody, CP_BODY_TYPE_STATIC);
	cpSpaceSetStaticBody(space, staticBody);

	return space;
}

cpSpace*
cpSpaceNew(void)
{
	return cpSpaceInit(cpSpaceAlloc());
}

static void cpBodyActivateWrap(cpBody *body, void *unused){cpBodyActivate(body);}

void
cpSpaceDestroy(cpSpace *space)
{
	cpSpaceEachBody(space, (cpSpaceBodyIteratorFunc)cpBodyActivateWrap, NULL);

	cpSpatialIndexFree(space->staticShapes);
	cpSpatialIndexFree(space->dynamicShapes);

	cpArrayFree(space->dynamicBodies);
	cpArrayFree(space->staticBodies);
	cpArrayFree(space->sleepingComponents);
	cpArrayFree(space->rousedBodies);

	cpArrayFree(space->constraints);

	cpHashSetFree(space->cachedArbiters);

	cpArrayFree(space->arbiters);
	cpArrayFree(space->pooledArbiters);

	if(space->allocatedBuffers){
		cpArrayFreeEach(space->allocatedBuffers, cpfree);
		cpArrayFree(space->allocatedBuffers);
	}

	if(space->postStepCallbacks){
		cpArrayFreeEach(space->postStepCallbacks, cpfree);
		cpArrayFree(space->postStepCallbacks);
	}

	if(space->collisionHandlers) cpHashSetEach(space->collisionHandlers, FreeWrap, NULL);
	cpHashSetFree(space->collisionHandlers);
}

void
cpSpaceFree(cpSpace *space)
{
	if(space){
		cpSpaceDestroy(space);
		cpfree(space);
	}
}

int
cpSpaceGetIterations(const cpSpace *space)
{
	return space->iterations;
}

void
cpSpaceSetIterations(cpSpace *space, int iterations)
{
	cpAssertHard(iterations > 0, "Iterations must be positive and non-zero.");
	space->iterations = iterations;
}

cpVect
cpSpaceGetGravity(const cpSpace *space)
{
	return space->gravity;
}

void
cpSpaceSetGravity(cpSpace *space, cpVect gravity)
{
	space->gravity = gravity;

	cpArray *components = space->sleepingComponents;
	for(int i=0; i<components->num; i++){
		cpBodyActivate((cpBody *)components->arr[i]);
	}
}

cpFloat
cpSpaceGetDamping(const cpSpace *space)
{
	return space->damping;
}

void
cpSpaceSetDamping(cpSpace *space, cpFloat damping)
{
	cpAssertHard(damping >= 0.0, "Damping must be positive.");
	space->damping = damping;
}

cpFloat
cpSpaceGetIdleSpeedThreshold(const cpSpace *space)
{
	return space->idleSpeedThreshold;
}

void
cpSpaceSetIdleSpeedThreshold(cpSpace *space, cpFloat idleSpeedThreshold)
{
	space->idleSpeedThreshold = idleSpeedThreshold;
}

cpFloat
cpSpaceGetSleepTimeThreshold(const cpSpace *space)
{
	return space->sleepTimeThreshold;
}

void
cpSpaceSetSleepTimeThreshold(cpSpace *space, cpFloat sleepTimeThreshold)
{
	space->sleepTimeThreshold = sleepTimeThreshold;
}

cpFloat
cpSpaceGetCollisionSlop(const cpSpace *space)
{
	return space->collisionSlop;
}

void
cpSpaceSetCollisionSlop(cpSpace *space, cpFloat collisionSlop)
{
	space->collisionSlop = collisionSlop;
}

cpFloat
cpSpaceGetCollisionBias(const cpSpace *space)
{
	return space->collisionBias;
}

void
cpSpaceSetCollisionBias(cpSpace *space, cpFloat collisionBias)
{
	space->collisionBias = collisionBias;
}

cpTimestamp
cpSpaceGetCollisionPersistence(const cpSpace *space)
{
	return space->collisionPersistence;
}

void
cpSpaceSetCollisionPersistence(cpSpace *space, cpTimestamp collisionPersistence)
{
	space->collisionPersistence = collisionPersistence;
}

cpDataPointer
cpSpaceGetUserData(const cpSpace *space)
{
	return space->userData;
}

void
cpSpaceSetUserData(cpSpace *space, cpDataPointer userData)
{
	space->userData = userData;
}

cpBody *
cpSpaceGetStaticBody(const cpSpace *space)
{
	return space->staticBody;
}

cpFloat
cpSpaceGetCurrentTimeStep(const cpSpace *space)
{
	return space->curr_dt;
}

void
cpSpaceSetStaticBody(cpSpace *space, cpBody *body)
{
	if(space->staticBody != NULL){
		cpAssertHard(space->staticBody->shapeList == NULL, "Internal Error: Changing the designated static body while the old one still had shapes attached.");
		space->staticBody->space = NULL;
	}

	space->staticBody = body;
	body->space = space;
}

cpBool
cpSpaceIsLocked(cpSpace *space)
{
	return (space->locked > 0);
}

static void
cpSpaceUseWildcardDefaultHandler(cpSpace *space)
{
	if(!space->usesWildcards){
		space->usesWildcards = cpTrue;
		memcpy(&space->defaultHandler, &cpCollisionHandlerDefault, sizeof(cpCollisionHandler));
	}
}

cpCollisionHandler *cpSpaceAddDefaultCollisionHandler(cpSpace *space)
{
	cpSpaceUseWildcardDefaultHandler(space);
	return &space->defaultHandler;
}

cpCollisionHandler *cpSpaceAddCollisionHandler(cpSpace *space, cpCollisionType a, cpCollisionType b)
{
	cpHashValue hash = CP_HASH_PAIR(a, b);
	cpCollisionHandler handler = {a, b, DefaultBegin, DefaultPreSolve, DefaultPostSolve, DefaultSeparate, NULL};
	return (cpCollisionHandler*)cpHashSetInsert(space->collisionHandlers, hash, &handler, (cpHashSetTransFunc)handlerSetTrans, NULL);
}

cpCollisionHandler *
cpSpaceAddWildcardHandler(cpSpace *space, cpCollisionType type)
{
	cpSpaceUseWildcardDefaultHandler(space);

	cpHashValue hash = CP_HASH_PAIR(type, CP_WILDCARD_COLLISION_TYPE);
	cpCollisionHandler handler = {type, CP_WILDCARD_COLLISION_TYPE, AlwaysCollide, AlwaysCollide, DoNothing, DoNothing, NULL};
	return (cpCollisionHandler*)cpHashSetInsert(space->collisionHandlers, hash, &handler, (cpHashSetTransFunc)handlerSetTrans, NULL);
}

cpShape *
cpSpaceAddShape(cpSpace *space, cpShape *shape)
{
	cpBody *body = shape->body;

	cpAssertHard(shape->space != space, "You have already added this shape to this space. You must not add it a second time.");
	cpAssertHard(!shape->space, "You have already added this shape to another space. You cannot add it to a second.");
	cpAssertSpaceUnlocked(space);

	cpBool isStatic = (cpBodyGetType(body) == CP_BODY_TYPE_STATIC);
	if(!isStatic) cpBodyActivate(body);
	cpBodyAddShape(body, shape);

	shape->hashid = space->shapeIDCounter++;
	cpShapeUpdate(shape, body->transform);
	cpSpatialIndexInsert(isStatic ? space->staticShapes : space->dynamicShapes, shape, shape->hashid);
	shape->space = space;

	return shape;
}

cpBody *
cpSpaceAddBody(cpSpace *space, cpBody *body)
{
	cpAssertHard(body->space != space, "You have already added this body to this space. You must not add it a second time.");
	cpAssertHard(!body->space, "You have already added this body to another space. You cannot add it to a second.");
	cpAssertSpaceUnlocked(space);

	cpArrayPush(cpSpaceArrayForBodyType(space, cpBodyGetType(body)), body);
	body->space = space;

	return body;
}

cpConstraint *
cpSpaceAddConstraint(cpSpace *space, cpConstraint *constraint)
{
	cpAssertHard(constraint->space != space, "You have already added this constraint to this space. You must not add it a second time.");
	cpAssertHard(!constraint->space, "You have already added this constraint to another space. You cannot add it to a second.");
	cpAssertSpaceUnlocked(space);

	cpBody *a = constraint->a, *b = constraint->b;
	cpAssertHard(a != NULL && b != NULL, "Constraint is attached to a NULL body.");

	cpBodyActivate(a);
	cpBodyActivate(b);
	cpArrayPush(space->constraints, constraint);

	constraint->next_a = a->constraintList; a->constraintList = constraint;
	constraint->next_b = b->constraintList; b->constraintList = constraint;
	constraint->space = space;

	return constraint;
}

struct arbiterFilterContext {
	cpSpace *space;
	cpBody *body;
	cpShape *shape;
};

static cpBool
cachedArbitersFilter(cpArbiter *arb, struct arbiterFilterContext *context)
{
	cpShape *shape = context->shape;
	cpBody *body = context->body;

	if(
		(body == arb->body_a && (shape == arb->a || shape == NULL)) ||
		(body == arb->body_b && (shape == arb->b || shape == NULL))
	){
		if(shape && arb->state != CP_ARBITER_STATE_CACHED){
			arb->state = CP_ARBITER_STATE_INVALIDATED;

			cpCollisionHandler *handler = arb->handler;
			handler->separateFunc(arb, context->space, handler->userData);
		}

		cpArbiterUnthread(arb);
		cpArrayDeleteObj(context->space->arbiters, arb);
		cpArrayPush(context->space->pooledArbiters, arb);

		return cpFalse;
	}

	return cpTrue;
}

void
cpSpaceFilterArbiters(cpSpace *space, cpBody *body, cpShape *filter)
{
	cpSpaceLock(space); {
		struct arbiterFilterContext context = {space, body, filter};
		cpHashSetFilter(space->cachedArbiters, (cpHashSetFilterFunc)cachedArbitersFilter, &context);
	} cpSpaceUnlock(space, cpTrue);
}

void
cpSpaceRemoveShape(cpSpace *space, cpShape *shape)
{
	cpBody *body = shape->body;
	cpAssertHard(cpSpaceContainsShape(space, shape), "Cannot remove a shape that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);

	cpBool isStatic = (cpBodyGetType(body) == CP_BODY_TYPE_STATIC);
	if(isStatic){
		cpBodyActivateStatic(body, shape);
	} else {
		cpBodyActivate(body);
	}

	cpBodyRemoveShape(body, shape);
	cpSpaceFilterArbiters(space, body, shape);
	cpSpatialIndexRemove(isStatic ? space->staticShapes : space->dynamicShapes, shape, shape->hashid);
	shape->space = NULL;
	shape->hashid = 0;
}

void
cpSpaceRemoveBody(cpSpace *space, cpBody *body)
{
	cpAssertHard(body != cpSpaceGetStaticBody(space), "Cannot remove the designated static body for the space.");
	cpAssertHard(cpSpaceContainsBody(space, body), "Cannot remove a body that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);

	cpBodyActivate(body);
	cpArrayDeleteObj(cpSpaceArrayForBodyType(space, cpBodyGetType(body)), body);
	body->space = NULL;
}

void
cpSpaceRemoveConstraint(cpSpace *space, cpConstraint *constraint)
{
	cpAssertHard(cpSpaceContainsConstraint(space, constraint), "Cannot remove a constraint that was not added to the space. (Removed twice maybe?)");
	cpAssertSpaceUnlocked(space);

	cpBodyActivate(constraint->a);
	cpBodyActivate(constraint->b);
	cpArrayDeleteObj(space->constraints, constraint);

	cpBodyRemoveConstraint(constraint->a, constraint);
	cpBodyRemoveConstraint(constraint->b, constraint);
	constraint->space = NULL;
}

cpBool cpSpaceContainsShape(cpSpace *space, cpShape *shape)
{
	return (shape->space == space);
}

cpBool cpSpaceContainsBody(cpSpace *space, cpBody *body)
{
	return (body->space == space);
}

cpBool cpSpaceContainsConstraint(cpSpace *space, cpConstraint *constraint)
{
	return (constraint->space == space);
}

void
cpSpaceEachBody(cpSpace *space, cpSpaceBodyIteratorFunc func, void *data)
{
	cpSpaceLock(space); {
		cpArray *bodies = space->dynamicBodies;
		int i;
		for(i=0; i<bodies->num; i++){
			func((cpBody *)bodies->arr[i], data);
		}

		cpArray *otherBodies = space->staticBodies;
		for(i=0; i<otherBodies->num; i++){
			func((cpBody *)otherBodies->arr[i], data);
		}

		cpArray *components = space->sleepingComponents;
		for(i=0; i<components->num; i++){
			cpBody *root = (cpBody *)components->arr[i];

			cpBody *body = root;
			while(body){
				cpBody *next = body->sleeping.next;
				func(body, data);
				body = next;
			}
		}
	} cpSpaceUnlock(space, cpTrue);
}

typedef struct spaceShapeContext {
	cpSpaceShapeIteratorFunc func;
	void *data;
} spaceShapeContext;

static void
spaceEachShapeIterator(cpShape *shape, spaceShapeContext *context)
{
	context->func(shape, context->data);
}

void
cpSpaceEachShape(cpSpace *space, cpSpaceShapeIteratorFunc func, void *data)
{
	cpSpaceLock(space); {
		spaceShapeContext context = {func, data};
		cpSpatialIndexEach(space->dynamicShapes, (cpSpatialIndexIteratorFunc)spaceEachShapeIterator, &context);
		cpSpatialIndexEach(space->staticShapes, (cpSpatialIndexIteratorFunc)spaceEachShapeIterator, &context);
	} cpSpaceUnlock(space, cpTrue);
}

void
cpSpaceEachConstraint(cpSpace *space, cpSpaceConstraintIteratorFunc func, void *data)
{
	cpSpaceLock(space); {
		cpArray *constraints = space->constraints;

		for(int i=0; i<constraints->num; i++){
			func((cpConstraint *)constraints->arr[i], data);
		}
	} cpSpaceUnlock(space, cpTrue);
}

void
cpSpaceReindexStatic(cpSpace *space)
{
	cpAssertHard(!space->locked, "You cannot manually reindex objects while the space is locked. Wait until the current query or step is complete.");

	cpSpatialIndexEach(space->staticShapes, (cpSpatialIndexIteratorFunc)&cpShapeUpdateFunc, NULL);
	cpSpatialIndexReindex(space->staticShapes);
}

void
cpSpaceReindexShape(cpSpace *space, cpShape *shape)
{
	cpAssertHard(!space->locked, "You cannot manually reindex objects while the space is locked. Wait until the current query or step is complete.");

	cpShapeCacheBB(shape);

	cpSpatialIndexReindexObject(space->dynamicShapes, shape, shape->hashid);
	cpSpatialIndexReindexObject(space->staticShapes, shape, shape->hashid);
}

void
cpSpaceReindexShapesForBody(cpSpace *space, cpBody *body)
{
	CP_BODY_FOREACH_SHAPE(body, shape) cpSpaceReindexShape(space, shape);
}

static void
copyShapes(cpShape *shape, cpSpatialIndex *index)
{
	cpSpatialIndexInsert(index, shape, shape->hashid);
}

void
cpSpaceUseSpatialHash(cpSpace *space, cpFloat dim, int count)
{
	cpSpatialIndex *staticShapes = cpSpaceHashNew(dim, count, (cpSpatialIndexBBFunc)cpShapeGetBB, NULL);
	cpSpatialIndex *dynamicShapes = cpSpaceHashNew(dim, count, (cpSpatialIndexBBFunc)cpShapeGetBB, staticShapes);

	cpSpatialIndexEach(space->staticShapes, (cpSpatialIndexIteratorFunc)copyShapes, staticShapes);
	cpSpatialIndexEach(space->dynamicShapes, (cpSpatialIndexIteratorFunc)copyShapes, dynamicShapes);

	cpSpatialIndexFree(space->staticShapes);
	cpSpatialIndexFree(space->dynamicShapes);

	space->staticShapes = staticShapes;
	space->dynamicShapes = dynamicShapes;
}

//
//MERGED FILE END: cpSpace.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSpaceComponent.c
//

#include <string.h>

void
cpSpaceActivateBody(cpSpace *space, cpBody *body)
{
	cpAssertHard(cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC, "Internal error: Attempting to activate a non-dynamic body.");

	if(space->locked){
		if(!cpArrayContains(space->rousedBodies, body)) cpArrayPush(space->rousedBodies, body);
	} else {
		cpAssertSoft(body->sleeping.root == NULL && body->sleeping.next == NULL, "Internal error: Activating body non-NULL node pointers.");
		cpArrayPush(space->dynamicBodies, body);

		CP_BODY_FOREACH_SHAPE(body, shape){
			cpSpatialIndexRemove(space->staticShapes, shape, shape->hashid);
			cpSpatialIndexInsert(space->dynamicShapes, shape, shape->hashid);
		}

		CP_BODY_FOREACH_ARBITER(body, arb){
			cpBody *bodyA = arb->body_a;

			if(body == bodyA || cpBodyGetType(bodyA) == CP_BODY_TYPE_STATIC){
				int numContacts = arb->count;
				struct cpContact *contacts = arb->contacts;

				arb->contacts = cpContactBufferGetArray(space);
				memcpy(arb->contacts, contacts, numContacts*sizeof(struct cpContact));
				cpSpacePushContacts(space, numContacts);

				const cpShape *a = arb->a, *b = arb->b;
				const cpShape *shape_pair[] = {a, b};
				cpHashValue arbHashID = CP_HASH_PAIR((cpHashValue)a, (cpHashValue)b);
				cpHashSetInsert(space->cachedArbiters, arbHashID, shape_pair, NULL, arb);

				arb->stamp = space->stamp;
				cpArrayPush(space->arbiters, arb);

				cpfree(contacts);
			}
		}

		CP_BODY_FOREACH_CONSTRAINT(body, constraint){
			cpBody *bodyA = constraint->a;
			if(body == bodyA || cpBodyGetType(bodyA) == CP_BODY_TYPE_STATIC) cpArrayPush(space->constraints, constraint);
		}
	}
}

static void
cpSpaceDeactivateBody(cpSpace *space, cpBody *body)
{
	cpAssertHard(cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC, "Internal error: Attempting to deactivate a non-dynamic body.");

	cpArrayDeleteObj(space->dynamicBodies, body);

	CP_BODY_FOREACH_SHAPE(body, shape){
		cpSpatialIndexRemove(space->dynamicShapes, shape, shape->hashid);
		cpSpatialIndexInsert(space->staticShapes, shape, shape->hashid);
	}

	CP_BODY_FOREACH_ARBITER(body, arb){
		cpBody *bodyA = arb->body_a;
		if(body == bodyA || cpBodyGetType(bodyA) == CP_BODY_TYPE_STATIC){
			cpSpaceUncacheArbiter(space, arb);

			size_t bytes = arb->count*sizeof(struct cpContact);
			struct cpContact *contacts = (struct cpContact *)cpcalloc(1, bytes);
			memcpy(contacts, arb->contacts, bytes);
			arb->contacts = contacts;
		}
	}

	CP_BODY_FOREACH_CONSTRAINT(body, constraint){
		cpBody *bodyA = constraint->a;
		if(body == bodyA || cpBodyGetType(bodyA) == CP_BODY_TYPE_STATIC) cpArrayDeleteObj(space->constraints, constraint);
	}
}

static inline cpBody *
ComponentRoot(cpBody *body)
{
	return (body ? body->sleeping.root : NULL);
}

void
cpBodyActivate(cpBody *body)
{
	if(body != NULL && cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC){
		body->sleeping.idleTime = 0.0f;

		cpBody *root = ComponentRoot(body);
		if(root && cpBodyIsSleeping(root)){
			cpAssertSoft(cpBodyGetType(root) == CP_BODY_TYPE_DYNAMIC, "Internal Error: Non-dynamic body component root detected.");

			cpSpace *space = root->space;
			cpBody *body = root;
			while(body){
				cpBody *next = body->sleeping.next;

				body->sleeping.idleTime = 0.0f;
				body->sleeping.root = NULL;
				body->sleeping.next = NULL;
				cpSpaceActivateBody(space, body);

				body = next;
			}

			cpArrayDeleteObj(space->sleepingComponents, root);
		}

		CP_BODY_FOREACH_ARBITER(body, arb){
			cpBody *other = (arb->body_a == body ? arb->body_b : arb->body_a);
			if(cpBodyGetType(other) != CP_BODY_TYPE_STATIC) other->sleeping.idleTime = 0.0f;
		}
	}
}

void
cpBodyActivateStatic(cpBody *body, cpShape *filter)
{
	cpAssertHard(cpBodyGetType(body) == CP_BODY_TYPE_STATIC, "cpBodyActivateStatic() called on a non-static body.");

	CP_BODY_FOREACH_ARBITER(body, arb){
		if(!filter || filter == arb->a || filter == arb->b){
			cpBodyActivate(arb->body_a == body ? arb->body_b : arb->body_a);
		}
	}

}

static inline void
cpBodyPushArbiter(cpBody *body, cpArbiter *arb)
{
	cpAssertSoft(cpArbiterThreadForBody(arb, body)->next == NULL, "Internal Error: Dangling contact graph pointers detected. (A)");
	cpAssertSoft(cpArbiterThreadForBody(arb, body)->prev == NULL, "Internal Error: Dangling contact graph pointers detected. (B)");

	cpArbiter *next = body->arbiterList;
	cpAssertSoft(next == NULL || cpArbiterThreadForBody(next, body)->prev == NULL, "Internal Error: Dangling contact graph pointers detected. (C)");
	cpArbiterThreadForBody(arb, body)->next = next;

	if(next) cpArbiterThreadForBody(next, body)->prev = arb;
	body->arbiterList = arb;
}

static inline void
ComponentAdd(cpBody *root, cpBody *body){
	body->sleeping.root = root;

	if(body != root){
		body->sleeping.next = root->sleeping.next;
		root->sleeping.next = body;
	}
}

static inline void
FloodFillComponent(cpBody *root, cpBody *body)
{
	if(cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC){
		cpBody *other_root = ComponentRoot(body);
		if(other_root == NULL){
			ComponentAdd(root, body);
			CP_BODY_FOREACH_ARBITER(body, arb) FloodFillComponent(root, (body == arb->body_a ? arb->body_b : arb->body_a));
			CP_BODY_FOREACH_CONSTRAINT(body, constraint) FloodFillComponent(root, (body == constraint->a ? constraint->b : constraint->a));
		} else {
			cpAssertSoft(other_root == root, "Internal Error: Inconsistency dectected in the contact graph.");
		}
	}
}

static inline cpBool
ComponentActive(cpBody *root, cpFloat threshold)
{
	CP_BODY_FOREACH_COMPONENT(root, body){
		if(body->sleeping.idleTime < threshold) return cpTrue;
	}

	return cpFalse;
}

void
cpSpaceProcessComponents(cpSpace *space, cpFloat dt)
{
	cpBool sleep = (space->sleepTimeThreshold != INFINITY);
	cpArray *bodies = space->dynamicBodies;

	int i;
#ifndef NDEBUG
	for(i=0; i<bodies->num; i++){
		cpBody *body = (cpBody*)bodies->arr[i];

		cpAssertSoft(body->sleeping.next == NULL, "Internal Error: Dangling next pointer detected in contact graph.");
		cpAssertSoft(body->sleeping.root == NULL, "Internal Error: Dangling root pointer detected in contact graph.");
	}
#endif

	if(sleep){
		cpFloat dv = space->idleSpeedThreshold;
		cpFloat dvsq = (dv ? dv*dv : cpvlengthsq(space->gravity)*dt*dt);

		for(i=0; i<bodies->num; i++){
			cpBody *body = (cpBody*)bodies->arr[i];

			if(cpBodyGetType(body) != CP_BODY_TYPE_DYNAMIC) continue;

			cpFloat keThreshold = (dvsq ? body->m*dvsq : 0.0f);
			body->sleeping.idleTime = (cpBodyKineticEnergy(body) > keThreshold ? 0.0f : body->sleeping.idleTime + dt);
		}
	}

	cpArray *arbiters = space->arbiters;
	i=0; for(int count=arbiters->num; i<count; i++){
		cpArbiter *arb = (cpArbiter*)arbiters->arr[i];
		cpBody *a = arb->body_a, *b = arb->body_b;

		if(sleep){
			if(cpBodyGetType(b) == CP_BODY_TYPE_KINEMATIC || cpBodyIsSleeping(a)) cpBodyActivate(a);
			if(cpBodyGetType(a) == CP_BODY_TYPE_KINEMATIC || cpBodyIsSleeping(b)) cpBodyActivate(b);
		}

		cpBodyPushArbiter(a, arb);
		cpBodyPushArbiter(b, arb);
	}

	if(sleep){
		cpArray *constraints = space->constraints;
		for(i=0; i<constraints->num; i++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[i];
			cpBody *a = constraint->a, *b = constraint->b;

			if(cpBodyGetType(b) == CP_BODY_TYPE_KINEMATIC) cpBodyActivate(a);
			if(cpBodyGetType(a) == CP_BODY_TYPE_KINEMATIC) cpBodyActivate(b);
		}

		for(i=0; i<bodies->num;){
			cpBody *body = (cpBody*)bodies->arr[i];

			if(ComponentRoot(body) == NULL){
				FloodFillComponent(body, body);

				if(!ComponentActive(body, space->sleepTimeThreshold)){
					cpArrayPush(space->sleepingComponents, body);
					CP_BODY_FOREACH_COMPONENT(body, other) cpSpaceDeactivateBody(space, other);

					continue;
				}
			}

			i++;

			body->sleeping.root = NULL;
			body->sleeping.next = NULL;
		}
	}
}

void
cpBodySleep(cpBody *body)
{
	cpBodySleepWithGroup(body, NULL);
}

void
cpBodySleepWithGroup(cpBody *body, cpBody *group){
	cpAssertHard(cpBodyGetType(body) == CP_BODY_TYPE_DYNAMIC, "Non-dynamic bodies cannot be put to sleep.");

	cpSpace *space = body->space;
	cpAssertHard(!cpSpaceIsLocked(space), "Bodies cannot be put to sleep during a query or a call to cpSpaceStep(). Put these calls into a post-step callback.");
	cpAssertHard(cpSpaceGetSleepTimeThreshold(space) < INFINITY, "Sleeping is not enabled on the space. You cannot sleep a body without setting a sleep time threshold on the space.");
	cpAssertHard(group == NULL || cpBodyIsSleeping(group), "Cannot use a non-sleeping body as a group identifier.");

	if(cpBodyIsSleeping(body)){
		cpAssertHard(ComponentRoot(body) == ComponentRoot(group), "The body is already sleeping and it's group cannot be reassigned.");
		return;
	}

	CP_BODY_FOREACH_SHAPE(body, shape) cpShapeCacheBB(shape);
	cpSpaceDeactivateBody(space, body);

	if(group){
		cpBody *root = ComponentRoot(group);

		body->sleeping.root = root;
		body->sleeping.next = root->sleeping.next;
		body->sleeping.idleTime = 0.0f;

		root->sleeping.next = body;
	} else {
		body->sleeping.root = body;
		body->sleeping.next = NULL;
		body->sleeping.idleTime = 0.0f;

		cpArrayPush(space->sleepingComponents, body);
	}

	cpArrayDeleteObj(space->dynamicBodies, body);
}

//
//MERGED FILE END: cpSpaceComponent.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSpaceHash.c
//

typedef struct cpSpaceHashBin cpSpaceHashBin;
typedef struct cpHandle cpHandle;

struct cpSpaceHash {
	cpSpatialIndex spatialIndex;

	int numcells;
	cpFloat celldim;

	cpSpaceHashBin **table;
	cpHashSet *handleSet;

	cpSpaceHashBin *pooledBins;
	cpArray *pooledHandles;
	cpArray *allocatedBuffers;

	cpTimestamp stamp;
};

struct cpHandle {
	void *obj;
	int retain;
	cpTimestamp stamp;
};

static cpHandle*
cpHandleInit(cpHandle *hand, void *obj)
{
	hand->obj = obj;
	hand->retain = 0;
	hand->stamp = 0;

	return hand;
}

static inline void cpHandleRetain(cpHandle *hand){hand->retain++;}

static inline void
cpHandleRelease(cpHandle *hand, cpArray *pooledHandles)
{
	hand->retain--;
	if(hand->retain == 0) cpArrayPush(pooledHandles, hand);
}

static int handleSetEql(void *obj, cpHandle *hand){return (obj == hand->obj);}

static void *
handleSetTrans(void *obj, cpSpaceHash *hash)
{
	if(hash->pooledHandles->num == 0){
		int count = CP_BUFFER_BYTES/sizeof(cpHandle);
		cpAssertHard(count, "Internal Error: Buffer size is too small.");

		cpHandle *buffer = (cpHandle *)cpcalloc(1, CP_BUFFER_BYTES);
		cpArrayPush(hash->allocatedBuffers, buffer);

		for(int i=0; i<count; i++) cpArrayPush(hash->pooledHandles, buffer + i);
	}

	cpHandle *hand = cpHandleInit((cpHandle *)cpArrayPop(hash->pooledHandles), obj);
	cpHandleRetain(hand);

	return hand;
}

struct cpSpaceHashBin {
	cpHandle *handle;
	cpSpaceHashBin *next;
};

static inline void
recycleBin(cpSpaceHash *hash, cpSpaceHashBin *bin)
{
	bin->next = hash->pooledBins;
	hash->pooledBins = bin;
}

static inline void
clearTableCell(cpSpaceHash *hash, int idx)
{
	cpSpaceHashBin *bin = hash->table[idx];
	while(bin){
		cpSpaceHashBin *next = bin->next;

		cpHandleRelease(bin->handle, hash->pooledHandles);
		recycleBin(hash, bin);

		bin = next;
	}

	hash->table[idx] = NULL;
}

static void
clearTable(cpSpaceHash *hash)
{
	for(int i=0; i<hash->numcells; i++) clearTableCell(hash, i);
}

static inline cpSpaceHashBin *
getEmptyBin(cpSpaceHash *hash)
{
	cpSpaceHashBin *bin = hash->pooledBins;

	if(bin){
		hash->pooledBins = bin->next;
		return bin;
	} else {
		int count = CP_BUFFER_BYTES/sizeof(cpSpaceHashBin);
		cpAssertHard(count, "Internal Error: Buffer size is too small.");

		cpSpaceHashBin *buffer = (cpSpaceHashBin *)cpcalloc(1, CP_BUFFER_BYTES);
		cpArrayPush(hash->allocatedBuffers, buffer);

		for(int i=1; i<count; i++) recycleBin(hash, buffer + i);
		return buffer;
	}
}

cpSpaceHash *
cpSpaceHashAlloc(void)
{
	return (cpSpaceHash *)cpcalloc(1, sizeof(cpSpaceHash));
}

static void
cpSpaceHashAllocTable(cpSpaceHash *hash, int numcells)
{
	cpfree(hash->table);

	hash->numcells = numcells;
	hash->table = (cpSpaceHashBin **)cpcalloc(numcells, sizeof(cpSpaceHashBin *));
}

static inline cpSpatialIndexClass *KlasscpSpaceHash();

cpSpatialIndex *
cpSpaceHashInit(cpSpaceHash *hash, cpFloat celldim, int numcells, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	cpSpatialIndexInit((cpSpatialIndex *)hash, KlasscpSpaceHash(), bbfunc, staticIndex);

	cpSpaceHashAllocTable(hash, next_prime(numcells));
	hash->celldim = celldim;

	hash->handleSet = cpHashSetNew(0, (cpHashSetEqlFunc)handleSetEql);

	hash->pooledHandles = cpArrayNew(0);

	hash->pooledBins = NULL;
	hash->allocatedBuffers = cpArrayNew(0);

	hash->stamp = 1;

	return (cpSpatialIndex *)hash;
}

cpSpatialIndex *
cpSpaceHashNew(cpFloat celldim, int cells, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	return cpSpaceHashInit(cpSpaceHashAlloc(), celldim, cells, bbfunc, staticIndex);
}

static void
cpSpaceHashDestroy(cpSpaceHash *hash)
{
	if(hash->table) clearTable(hash);
	cpfree(hash->table);

	cpHashSetFree(hash->handleSet);

	cpArrayFreeEach(hash->allocatedBuffers, cpfree);
	cpArrayFree(hash->allocatedBuffers);
	cpArrayFree(hash->pooledHandles);
}

static inline cpBool
containsHandle(cpSpaceHashBin *bin, cpHandle *hand)
{
	while(bin){
		if(bin->handle == hand) return cpTrue;
		bin = bin->next;
	}

	return cpFalse;
}

static inline cpHashValue
hash_func(cpHashValue x, cpHashValue y, cpHashValue n)
{
	return (x*1640531513ul ^ y*2654435789ul) % n;
}

static inline int
floor_int(cpFloat f)
{
	int i = (int)f;
	return (f < 0.0f && f != i ? i - 1 : i);
}

static inline void
hashHandle(cpSpaceHash *hash, cpHandle *hand, cpBB bb)
{
	cpFloat dim = hash->celldim;
	int l = floor_int(bb.l/dim);	int r = floor_int(bb.r/dim);
	int b = floor_int(bb.b/dim);
	int t = floor_int(bb.t/dim);

	int n = hash->numcells;
	for(int i=l; i<=r; i++){
		for(int j=b; j<=t; j++){
			cpHashValue idx = hash_func(i,j,n);
			cpSpaceHashBin *bin = hash->table[idx];

			if(containsHandle(bin, hand)) continue;

			cpHandleRetain(hand);
			cpSpaceHashBin *newBin = getEmptyBin(hash);
			newBin->handle = hand;
			newBin->next = bin;
			hash->table[idx] = newBin;
		}
	}
}

static void
cpSpaceHashInsert(cpSpaceHash *hash, void *obj, cpHashValue hashid)
{
	cpHandle *hand = (cpHandle *)cpHashSetInsert(hash->handleSet, hashid, obj, (cpHashSetTransFunc)handleSetTrans, hash);
	hashHandle(hash, hand, hash->spatialIndex.bbfunc(obj));
}

static void
cpSpaceHashRehashObject(cpSpaceHash *hash, void *obj, cpHashValue hashid)
{
	cpHandle *hand = (cpHandle *)cpHashSetRemove(hash->handleSet, hashid, obj);

	if(hand){
		hand->obj = NULL;
		cpHandleRelease(hand, hash->pooledHandles);

		cpSpaceHashInsert(hash, obj, hashid);
	}
}

static void
rehash_helper(cpHandle *hand, cpSpaceHash *hash)
{
	hashHandle(hash, hand, hash->spatialIndex.bbfunc(hand->obj));
}

static void
cpSpaceHashRehash(cpSpaceHash *hash)
{
	clearTable(hash);
	cpHashSetEach(hash->handleSet, (cpHashSetIteratorFunc)rehash_helper, hash);
}

static void
cpSpaceHashRemove(cpSpaceHash *hash, void *obj, cpHashValue hashid)
{
	cpHandle *hand = (cpHandle *)cpHashSetRemove(hash->handleSet, hashid, obj);

	if(hand){
		hand->obj = NULL;
		cpHandleRelease(hand, hash->pooledHandles);
	}
}

typedef struct eachContextcpSpaceHash {
	cpSpatialIndexIteratorFunc func;
	void *data;
} eachContextcpSpaceHash;

static void eachHelper(cpHandle *hand, eachContextcpSpaceHash *context){context->func(hand->obj, context->data);}

static void
cpSpaceHashEach(cpSpaceHash *hash, cpSpatialIndexIteratorFunc func, void *data)
{
	eachContextcpSpaceHash context = {func, data};
	cpHashSetEach(hash->handleSet, (cpHashSetIteratorFunc)eachHelper, &context);
}

static void
remove_orphaned_handles(cpSpaceHash *hash, cpSpaceHashBin **bin_ptr)
{
	cpSpaceHashBin *bin = *bin_ptr;
	while(bin){
		cpHandle *hand = bin->handle;
		cpSpaceHashBin *next = bin->next;

		if(!hand->obj){
			(*bin_ptr) = bin->next;
			recycleBin(hash, bin);

			cpHandleRelease(hand, hash->pooledHandles);
		} else {
			bin_ptr = &bin->next;
		}

		bin = next;
	}
}

static inline void
query_helper(cpSpaceHash *hash, cpSpaceHashBin **bin_ptr, void *obj, cpSpatialIndexQueryFunc func, void *data)
{
	restart:
	for(cpSpaceHashBin *bin = *bin_ptr; bin; bin = bin->next){
		cpHandle *hand = bin->handle;
		void *other = hand->obj;

		if(hand->stamp == hash->stamp || obj == other){
			continue;
		} else if(other){
			func(obj, other, 0, data);
			hand->stamp = hash->stamp;
		} else {
			remove_orphaned_handles(hash, bin_ptr);
			goto restart;		}
	}
}

static void
cpSpaceHashQuery(cpSpaceHash *hash, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data)
{
	cpFloat dim = hash->celldim;
	int l = floor_int(bb.l/dim);	int r = floor_int(bb.r/dim);
	int b = floor_int(bb.b/dim);
	int t = floor_int(bb.t/dim);

	int n = hash->numcells;
	cpSpaceHashBin **table = hash->table;

	for(int i=l; i<=r; i++){
		for(int j=b; j<=t; j++){
			query_helper(hash, &table[hash_func(i,j,n)], obj, func, data);
		}
	}

	hash->stamp++;
}

typedef struct queryRehashContext {
	cpSpaceHash *hash;
	cpSpatialIndexQueryFunc func;
	void *data;
} queryRehashContext;

static void
queryRehash_helper(cpHandle *hand, queryRehashContext *context)
{
	cpSpaceHash *hash = context->hash;
	cpSpatialIndexQueryFunc func = context->func;
	void *data = context->data;

	cpFloat dim = hash->celldim;
	int n = hash->numcells;

	void *obj = hand->obj;
	cpBB bb = hash->spatialIndex.bbfunc(obj);

	int l = floor_int(bb.l/dim);
	int r = floor_int(bb.r/dim);
	int b = floor_int(bb.b/dim);
	int t = floor_int(bb.t/dim);

	cpSpaceHashBin **table = hash->table;

	for(int i=l; i<=r; i++){
		for(int j=b; j<=t; j++){
			cpHashValue idx = hash_func(i,j,n);
			cpSpaceHashBin *bin = table[idx];

			if(containsHandle(bin, hand)) continue;

			cpHandleRetain(hand);			query_helper(hash, &bin, obj, func, data);

			cpSpaceHashBin *newBin = getEmptyBin(hash);
			newBin->handle = hand;
			newBin->next = bin;
			table[idx] = newBin;
		}
	}

	hash->stamp++;
}

static void
cpSpaceHashReindexQuery(cpSpaceHash *hash, cpSpatialIndexQueryFunc func, void *data)
{
	clearTable(hash);

	queryRehashContext context = {hash, func, data};
	cpHashSetEach(hash->handleSet, (cpHashSetIteratorFunc)queryRehash_helper, &context);

	cpSpatialIndexCollideStatic((cpSpatialIndex *)hash, hash->spatialIndex.staticIndex, func, data);
}

static inline cpFloat
segmentQuery_helper(cpSpaceHash *hash, cpSpaceHashBin **bin_ptr, void *obj, cpSpatialIndexSegmentQueryFunc func, void *data)
{
	cpFloat t = 1.0f;

	restart:
	for(cpSpaceHashBin *bin = *bin_ptr; bin; bin = bin->next){
		cpHandle *hand = bin->handle;
		void *other = hand->obj;

		if(hand->stamp == hash->stamp){
			continue;
		} else if(other){
			t = cpfmin(t, func(obj, other, data));
			hand->stamp = hash->stamp;
		} else {
			remove_orphaned_handles(hash, bin_ptr);
			goto restart;		}
	}

	return t;
}

static void
cpSpaceHashSegmentQuery(cpSpaceHash *hash, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data)
{
	a = cpvmult(a, 1.0f/hash->celldim);
	b = cpvmult(b, 1.0f/hash->celldim);

	int cell_x = floor_int(a.x), cell_y = floor_int(a.y);

	cpFloat t = 0;

	int x_inc, y_inc;
	cpFloat temp_v, temp_h;

	if (b.x > a.x){
		x_inc = 1;
		temp_h = (cpffloor(a.x + 1.0f) - a.x);
	} else {
		x_inc = -1;
		temp_h = (a.x - cpffloor(a.x));
	}

	if (b.y > a.y){
		y_inc = 1;
		temp_v = (cpffloor(a.y + 1.0f) - a.y);
	} else {
		y_inc = -1;
		temp_v = (a.y - cpffloor(a.y));
	}

	cpFloat dx = cpfabs(b.x - a.x), dy = cpfabs(b.y - a.y);
	cpFloat dt_dx = (dx ? 1.0f/dx : INFINITY), dt_dy = (dy ? 1.0f/dy : INFINITY);

	cpFloat next_h = (temp_h ? temp_h*dt_dx : dt_dx);
	cpFloat next_v = (temp_v ? temp_v*dt_dy : dt_dy);

	int n = hash->numcells;
	cpSpaceHashBin **table = hash->table;

	while(t < t_exit){
		cpHashValue idx = hash_func(cell_x, cell_y, n);
		t_exit = cpfmin(t_exit, segmentQuery_helper(hash, &table[idx], obj, func, data));

		if (next_v < next_h){
			cell_y += y_inc;
			t = next_v;
			next_v += dt_dy;
		} else {
			cell_x += x_inc;
			t = next_h;
			next_h += dt_dx;
		}
	}

	hash->stamp++;
}

void
cpSpaceHashResize(cpSpaceHash *hash, cpFloat celldim, int numcells)
{
	if(hash->spatialIndex.klass != KlasscpSpaceHash()){
		cpAssertWarn(cpFalse, "Ignoring cpSpaceHashResize() call to non-cpSpaceHash spatial index.");
		return;
	}

	clearTable(hash);

	hash->celldim = celldim;
	cpSpaceHashAllocTable(hash, next_prime(numcells));
}

static int
cpSpaceHashCount(cpSpaceHash *hash)
{
	return cpHashSetCount(hash->handleSet);
}

static int
cpSpaceHashContains(cpSpaceHash *hash, void *obj, cpHashValue hashid)
{
	return cpHashSetFind(hash->handleSet, hashid, obj) != NULL;
}

static cpSpatialIndexClass klasscpSpaceHash = {
	(cpSpatialIndexDestroyImpl)cpSpaceHashDestroy,

	(cpSpatialIndexCountImpl)cpSpaceHashCount,
	(cpSpatialIndexEachImpl)cpSpaceHashEach,
	(cpSpatialIndexContainsImpl)cpSpaceHashContains,

	(cpSpatialIndexInsertImpl)cpSpaceHashInsert,
	(cpSpatialIndexRemoveImpl)cpSpaceHashRemove,

	(cpSpatialIndexReindexImpl)cpSpaceHashRehash,
	(cpSpatialIndexReindexObjectImpl)cpSpaceHashRehashObject,
	(cpSpatialIndexReindexQueryImpl)cpSpaceHashReindexQuery,

	(cpSpatialIndexQueryImpl)cpSpaceHashQuery,
	(cpSpatialIndexSegmentQueryImpl)cpSpaceHashSegmentQuery,
};

static inline cpSpatialIndexClass *KlasscpSpaceHash(){return &klasscpSpaceHash;}

#ifdef CP_BBTREE_DEBUG_DRAW
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#include <GLUT/glut.h>

void
cpSpaceHashRenderDebug(cpSpatialIndex *index)
{
	if(index->klass != &klasscpSpaceHash){
		cpAssertWarn(cpFalse, "Ignoring cpSpaceHashRenderDebug() call to non-spatial hash spatial index.");
		return;
	}

	cpSpaceHash *hash = (cpSpaceHash *)index;
	cpBB bb = cpBBNew(-320, -240, 320, 240);

	cpFloat dim = hash->celldim;
	int n = hash->numcells;

	int l = (int)floor(bb.l/dim);
	int r = (int)floor(bb.r/dim);
	int b = (int)floor(bb.b/dim);
	int t = (int)floor(bb.t/dim);

	for(int i=l; i<=r; i++){
		for(int j=b; j<=t; j++){
			int cell_count = 0;

			int index = hash_func(i,j,n);
			for(cpSpaceHashBin *bin = hash->table[index]; bin; bin = bin->next)
				cell_count++;

			GLfloat v = 1.0f - (GLfloat)cell_count/10.0f;
			glColor3f(v,v,v);
			glRectf(i*dim, j*dim, (i + 1)*dim, (j + 1)*dim);
		}
	}
}
#endif

//
//MERGED FILE END: cpSpaceHash.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSpaceQuery.c
//

struct PointQueryContext {
	cpVect point;
	cpFloat maxDistance;
	cpShapeFilter filter;
	cpSpacePointQueryFunc func;
};

static cpCollisionID
NearestPointQuery(struct PointQueryContext *context, cpShape *shape, cpCollisionID id, void *data)
{
	if(
		!cpShapeFilterReject(shape->filter, context->filter)
	){
		cpPointQueryInfo info;
		cpShapePointQuery(shape, context->point, &info);

		if(info.shape && info.distance < context->maxDistance) context->func(shape, info.point, info.distance, info.gradient, data);
	}

	return id;
}

void
cpSpacePointQuery(cpSpace *space, cpVect point, cpFloat maxDistance, cpShapeFilter filter, cpSpacePointQueryFunc func, void *data)
{
	struct PointQueryContext context = {{point.x, point.y}, maxDistance, {filter.group, filter.categories, filter.mask}, func};
	cpBB bb = cpBBNewForCircle(point, cpfmax(maxDistance, 0.0f));

	cpSpaceLock(space); {
		cpSpatialIndexQuery(space->dynamicShapes, &context, bb, (cpSpatialIndexQueryFunc)NearestPointQuery, data);
		cpSpatialIndexQuery(space->staticShapes, &context, bb, (cpSpatialIndexQueryFunc)NearestPointQuery, data);
	} cpSpaceUnlock(space, cpTrue);
}

static cpCollisionID
NearestPointQueryNearest(struct PointQueryContext *context, cpShape *shape, cpCollisionID id, cpPointQueryInfo *out)
{
	if(
		!cpShapeFilterReject(shape->filter, context->filter) && !shape->sensor
	){
		cpPointQueryInfo info;
		cpShapePointQuery(shape, context->point, &info);

		if(info.distance < out->distance) (*out) = info;
	}

	return id;
}

cpShape *
cpSpacePointQueryNearest(cpSpace *space, cpVect point, cpFloat maxDistance, cpShapeFilter filter, cpPointQueryInfo *out)
{
	cpPointQueryInfo info = {NULL, {0, 0}, maxDistance, {0, 0}};
	if(out){
		(*out) = info;
  } else {
		out = &info;
	}

	struct PointQueryContext context = {
		{point.x, point.y}, maxDistance,
		{filter.group, filter.categories, filter.mask},
		NULL
	};

	cpBB bb = cpBBNewForCircle(point, cpfmax(maxDistance, 0.0f));
	cpSpatialIndexQuery(space->dynamicShapes, &context, bb, (cpSpatialIndexQueryFunc)NearestPointQueryNearest, out);
	cpSpatialIndexQuery(space->staticShapes, &context, bb, (cpSpatialIndexQueryFunc)NearestPointQueryNearest, out);

	return (cpShape *)out->shape;
}

struct SegmentQueryContext {
	cpVect start, end;
	cpFloat radius;
	cpShapeFilter filter;
	cpSpaceSegmentQueryFunc func;
};

static cpFloat
SegmentQuery(struct SegmentQueryContext *context, cpShape *shape, void *data)
{
	cpSegmentQueryInfo info;

	if(
		!cpShapeFilterReject(shape->filter, context->filter) &&
		cpShapeSegmentQuery(shape, context->start, context->end, context->radius, &info)
	){
		context->func(shape, info.point, info.normal, info.alpha, data);
	}

	return 1.0f;
}

void
cpSpaceSegmentQuery(cpSpace *space, cpVect start, cpVect end, cpFloat radius, cpShapeFilter filter, cpSpaceSegmentQueryFunc func, void *data)
{
	struct SegmentQueryContext context = {
		{start.x, start.y}, {end.x, end.y},
		radius,
		{filter.group, filter.categories, filter.mask},
		func,
	};

	cpSpaceLock(space); {
    cpSpatialIndexSegmentQuery(space->staticShapes, &context, start, end, 1.0f, (cpSpatialIndexSegmentQueryFunc)SegmentQuery, data);
    cpSpatialIndexSegmentQuery(space->dynamicShapes, &context, start, end, 1.0f, (cpSpatialIndexSegmentQueryFunc)SegmentQuery, data);
	} cpSpaceUnlock(space, cpTrue);
}

static cpFloat
SegmentQueryFirst(struct SegmentQueryContext *context, cpShape *shape, cpSegmentQueryInfo *out)
{
	cpSegmentQueryInfo info;

	if(
		!cpShapeFilterReject(shape->filter, context->filter) && !shape->sensor &&
		cpShapeSegmentQuery(shape, context->start, context->end, context->radius, &info) &&
		info.alpha < out->alpha
	){
		(*out) = info;
	}

	return out->alpha;
}

cpShape *
cpSpaceSegmentQueryFirst(cpSpace *space, cpVect start, cpVect end, cpFloat radius, cpShapeFilter filter, cpSegmentQueryInfo *out)
{
	cpSegmentQueryInfo info = {NULL, {end.x, end.y}, {0, 0}, 1.0f};
	if(out){
		(*out) = info;
  } else {
		out = &info;
	}

	struct SegmentQueryContext context = {
		{start.x, start.y}, {end.x, end.y},
		radius,
		{filter.group, filter.categories, filter.mask},
		NULL
	};

	cpSpatialIndexSegmentQuery(space->staticShapes, &context, start, end, 1.0f, (cpSpatialIndexSegmentQueryFunc)SegmentQueryFirst, out);
	cpSpatialIndexSegmentQuery(space->dynamicShapes, &context, start, end, out->alpha, (cpSpatialIndexSegmentQueryFunc)SegmentQueryFirst, out);

	return (cpShape *)out->shape;
}

struct BBQueryContext {
	cpBB bb;
	cpShapeFilter filter;
	cpSpaceBBQueryFunc func;
};

static cpCollisionID
BBQuery(struct BBQueryContext *context, cpShape *shape, cpCollisionID id, void *data)
{
	if(
		!cpShapeFilterReject(shape->filter, context->filter) &&
		cpBBIntersects(context->bb, shape->bb)
	){
		context->func(shape, data);
	}

	return id;
}

void
cpSpaceBBQuery(cpSpace *space, cpBB bb, cpShapeFilter filter, cpSpaceBBQueryFunc func, void *data)
{
	struct BBQueryContext context = {{bb.l, bb.b, bb.r, bb.t}, {filter.group, filter.categories, filter.mask}, func};

	cpSpaceLock(space); {
    cpSpatialIndexQuery(space->dynamicShapes, &context, bb, (cpSpatialIndexQueryFunc)BBQuery, data);
    cpSpatialIndexQuery(space->staticShapes, &context, bb, (cpSpatialIndexQueryFunc)BBQuery, data);
	} cpSpaceUnlock(space, cpTrue);
}

struct ShapeQueryContext {
	cpSpaceShapeQueryFunc func;
	void *data;
	cpBool anyCollision;
};

static cpCollisionID
ShapeQuery(cpShape *a, cpShape *b, cpCollisionID id, struct ShapeQueryContext *context)
{
	if(cpShapeFilterReject(a->filter, b->filter) || a == b) return id;

	cpContactPointSet set = cpShapesCollide(a, b);
	if(set.count){
		if(context->func) context->func(b, &set, context->data);
		context->anyCollision = !(a->sensor || b->sensor);
	}

	return id;
}

cpBool
cpSpaceShapeQuery(cpSpace *space, cpShape *shape, cpSpaceShapeQueryFunc func, void *data)
{
	cpBody *body = shape->body;
	cpBB bb = (body ? cpShapeUpdate(shape, body->transform) : shape->bb);
	struct ShapeQueryContext context = {func, data, cpFalse};

	cpSpaceLock(space); {
    cpSpatialIndexQuery(space->dynamicShapes, shape, bb, (cpSpatialIndexQueryFunc)ShapeQuery, &context);
    cpSpatialIndexQuery(space->staticShapes, shape, bb, (cpSpatialIndexQueryFunc)ShapeQuery, &context);
	} cpSpaceUnlock(space, cpTrue);

	return context.anyCollision;
}

//
//MERGED FILE END: cpSpaceQuery.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSpaceStep.c
//

cpPostStepCallback *
cpSpaceGetPostStepCallback(cpSpace *space, void *key)
{
	cpArray *arr = space->postStepCallbacks;
	for(int i=0; i<arr->num; i++){
		cpPostStepCallback *callback = (cpPostStepCallback *)arr->arr[i];
		if(callback && callback->key == key) return callback;
	}

	return NULL;
}

static void PostStepDoNothing(cpSpace *space, void *obj, void *data){}

cpBool
cpSpaceAddPostStepCallback(cpSpace *space, cpPostStepFunc func, void *key, void *data)
{
	cpAssertWarn(space->locked,
		"Adding a post-step callback when the space is not locked is unnecessary. "
		"Post-step callbacks will not called until the end of the next call to cpSpaceStep() or the next query.");

	if(!cpSpaceGetPostStepCallback(space, key)){
		cpPostStepCallback *callback = (cpPostStepCallback *)cpcalloc(1, sizeof(cpPostStepCallback));
		callback->func = (func ? func : PostStepDoNothing);
		callback->key = key;
		callback->data = data;

		cpArrayPush(space->postStepCallbacks, callback);
		return cpTrue;
	} else {
		return cpFalse;
	}
}

void
cpSpaceLock(cpSpace *space)
{
	space->locked++;
}

void
cpSpaceUnlock(cpSpace *space, cpBool runPostStep)
{
	space->locked--;
	cpAssertHard(space->locked >= 0, "Internal Error: Space lock underflow.");

	if(space->locked == 0){
		cpArray *waking = space->rousedBodies;

		for(int i=0, count=waking->num; i<count; i++){
			cpSpaceActivateBody(space, (cpBody *)waking->arr[i]);
			waking->arr[i] = NULL;
		}

		waking->num = 0;

		if(space->locked == 0 && runPostStep && !space->skipPostStep){
			space->skipPostStep = cpTrue;

			cpArray *arr = space->postStepCallbacks;
			for(int i=0; i<arr->num; i++){
				cpPostStepCallback *callback = (cpPostStepCallback *)arr->arr[i];
				cpPostStepFunc func = callback->func;

				callback->func = NULL;
				if(func) func(space, callback->key, callback->data);

				arr->arr[i] = NULL;
				cpfree(callback);
			}

			arr->num = 0;
			space->skipPostStep = cpFalse;
		}
	}
}

struct cpContactBufferHeader {
	cpTimestamp stamp;
	cpContactBufferHeader *next;
	unsigned int numContacts;
};

#define CP_CONTACTS_BUFFER_SIZE ((CP_BUFFER_BYTES - sizeof(cpContactBufferHeader))/sizeof(struct cpContact))
typedef struct cpContactBuffer {
	cpContactBufferHeader header;
	struct cpContact contacts[CP_CONTACTS_BUFFER_SIZE];
} cpContactBuffer;

static cpContactBufferHeader *
cpSpaceAllocContactBuffer(cpSpace *space)
{
	cpContactBuffer *buffer = (cpContactBuffer *)cpcalloc(1, sizeof(cpContactBuffer));
	cpArrayPush(space->allocatedBuffers, buffer);
	return (cpContactBufferHeader *)buffer;
}

static cpContactBufferHeader *
cpContactBufferHeaderInit(cpContactBufferHeader *header, cpTimestamp stamp, cpContactBufferHeader *splice)
{
	header->stamp = stamp;
	header->next = (splice ? splice->next : header);
	header->numContacts = 0;

	return header;
}

void
cpSpacePushFreshContactBuffer(cpSpace *space)
{
	cpTimestamp stamp = space->stamp;

	cpContactBufferHeader *head = space->contactBuffersHead;

	if(!head){
		space->contactBuffersHead = cpContactBufferHeaderInit(cpSpaceAllocContactBuffer(space), stamp, NULL);
	} else if(stamp - head->next->stamp > space->collisionPersistence){
	cpContactBufferHeader *tail = head->next;
		space->contactBuffersHead = cpContactBufferHeaderInit(tail, stamp, tail);
	} else {
		cpContactBufferHeader *buffer = cpContactBufferHeaderInit(cpSpaceAllocContactBuffer(space), stamp, head);
		space->contactBuffersHead = head->next = buffer;
	}
}

struct cpContact *
cpContactBufferGetArray(cpSpace *space)
{
	if(space->contactBuffersHead->numContacts + CP_MAX_CONTACTS_PER_ARBITER > CP_CONTACTS_BUFFER_SIZE){
		cpSpacePushFreshContactBuffer(space);
	}

	cpContactBufferHeader *head = space->contactBuffersHead;
	return ((cpContactBuffer *)head)->contacts + head->numContacts;
}

void
cpSpacePushContacts(cpSpace *space, int count)
{
	cpAssertHard(count <= CP_MAX_CONTACTS_PER_ARBITER, "Internal Error: Contact buffer overflow!");
	space->contactBuffersHead->numContacts += count;
}

static void
cpSpacePopContacts(cpSpace *space, int count){
	space->contactBuffersHead->numContacts -= count;
}

static void *
cpSpaceArbiterSetTrans(cpShape **shapes, cpSpace *space)
{
	if(space->pooledArbiters->num == 0){
		int count = CP_BUFFER_BYTES/sizeof(cpArbiter);
		cpAssertHard(count, "Internal Error: Buffer size too small.");

		cpArbiter *buffer = (cpArbiter *)cpcalloc(1, CP_BUFFER_BYTES);
		cpArrayPush(space->allocatedBuffers, buffer);

		for(int i=0; i<count; i++) cpArrayPush(space->pooledArbiters, buffer + i);
	}

	return cpArbiterInit((cpArbiter *)cpArrayPop(space->pooledArbiters), shapes[0], shapes[1]);
}

static inline cpBool
QueryRejectConstraint(cpBody *a, cpBody *b)
{
	CP_BODY_FOREACH_CONSTRAINT(a, constraint){
		if(
			!constraint->collideBodies && (
				(constraint->a == a && constraint->b == b) ||
				(constraint->a == b && constraint->b == a)
			)
		) return cpTrue;
	}

	return cpFalse;
}

static inline cpBool
QueryReject(cpShape *a, cpShape *b)
{
	return (
		!cpBBIntersects(a->bb, b->bb)
		|| a->body == b->body
		|| cpShapeFilterReject(a->filter, b->filter)
		|| QueryRejectConstraint(a->body, b->body)
	);
}

cpCollisionID
cpSpaceCollideShapes(cpShape *a, cpShape *b, cpCollisionID id, cpSpace *space)
{
	if(QueryReject(a,b)) return id;

	struct cpCollisionInfo info = cpCollide(a, b, id, cpContactBufferGetArray(space));

	if(info.count == 0) return info.id;	cpSpacePushContacts(space, info.count);

	const cpShape *shape_pair[] = {info.a, info.b};
	cpHashValue arbHashID = CP_HASH_PAIR((cpHashValue)info.a, (cpHashValue)info.b);
	cpArbiter *arb = (cpArbiter *)cpHashSetInsert(space->cachedArbiters, arbHashID, shape_pair, (cpHashSetTransFunc)cpSpaceArbiterSetTrans, space);
	cpArbiterUpdate(arb, &info, space);

	cpCollisionHandler *handler = arb->handler;

	if(arb->state == CP_ARBITER_STATE_FIRST_COLLISION && !handler->beginFunc(arb, space, handler->userData)){
		cpArbiterIgnore(arb);	}

	if(
		(arb->state != CP_ARBITER_STATE_IGNORE) &&
		handler->preSolveFunc(arb, space, handler->userData) &&
		arb->state != CP_ARBITER_STATE_IGNORE &&
		!(a->sensor || b->sensor) &&
		!(a->body->m == INFINITY && b->body->m == INFINITY)
	){
		cpArrayPush(space->arbiters, arb);
	} else {
		cpSpacePopContacts(space, info.count);

		arb->contacts = NULL;
		arb->count = 0;

		if(arb->state != CP_ARBITER_STATE_IGNORE) arb->state = CP_ARBITER_STATE_NORMAL;
	}

	arb->stamp = space->stamp;
	return info.id;
}

cpBool
cpSpaceArbiterSetFilter(cpArbiter *arb, cpSpace *space)
{
	cpTimestamp ticks = space->stamp - arb->stamp;

	cpBody *a = arb->body_a, *b = arb->body_b;

	if(
		(cpBodyGetType(a) == CP_BODY_TYPE_STATIC || cpBodyIsSleeping(a)) &&
		(cpBodyGetType(b) == CP_BODY_TYPE_STATIC || cpBodyIsSleeping(b))
	){
		return cpTrue;
	}

	if(ticks >= 1 && arb->state != CP_ARBITER_STATE_CACHED){
		arb->state = CP_ARBITER_STATE_CACHED;
		cpCollisionHandler *handler = arb->handler;
		handler->separateFunc(arb, space, handler->userData);
	}

	if(ticks >= space->collisionPersistence){
		arb->contacts = NULL;
		arb->count = 0;

		cpArrayPush(space->pooledArbiters, arb);
		return cpFalse;
	}

	return cpTrue;
}

 void
cpShapeUpdateFunc(cpShape *shape, void *unused)
{
	cpShapeCacheBB(shape);
}

void
cpSpaceStep(cpSpace *space, cpFloat dt)
{
	if(dt == 0.0f) return;

	space->stamp++;

	cpFloat prev_dt = space->curr_dt;
	space->curr_dt = dt;

	cpArray *bodies = space->dynamicBodies;
	cpArray *constraints = space->constraints;
	cpArray *arbiters = space->arbiters;

	int i;
	for(i=0; i<arbiters->num; i++){
		cpArbiter *arb = (cpArbiter *)arbiters->arr[i];
		arb->state = CP_ARBITER_STATE_NORMAL;

		if(!cpBodyIsSleeping(arb->body_a) && !cpBodyIsSleeping(arb->body_b)){
			cpArbiterUnthread(arb);
		}
	}
	arbiters->num = 0;

	cpSpaceLock(space); {
		for(i=0; i<bodies->num; i++){
			cpBody *body = (cpBody *)bodies->arr[i];
			body->position_func(body, dt);
		}

		cpSpacePushFreshContactBuffer(space);
		cpSpatialIndexEach(space->dynamicShapes, (cpSpatialIndexIteratorFunc)cpShapeUpdateFunc, NULL);
		cpSpatialIndexReindexQuery(space->dynamicShapes, (cpSpatialIndexQueryFunc)cpSpaceCollideShapes, space);
	} cpSpaceUnlock(space, cpFalse);

	cpSpaceProcessComponents(space, dt);

	cpSpaceLock(space); {
		cpHashSetFilter(space->cachedArbiters, (cpHashSetFilterFunc)cpSpaceArbiterSetFilter, space);

		cpFloat slop = space->collisionSlop;
		cpFloat biasCoef = 1.0f - cpfpow(space->collisionBias, dt);
		for(i=0; i<arbiters->num; i++){
			cpArbiterPreStep((cpArbiter *)arbiters->arr[i], dt, slop, biasCoef);
		}

		for(i=0; i<constraints->num; i++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[i];

			cpConstraintPreSolveFunc preSolve = constraint->preSolve;
			if(preSolve) preSolve(constraint, space);

			constraint->klass->preStep(constraint, dt);
		}

		cpFloat damping = cpfpow(space->damping, dt);
		cpVect gravity = space->gravity;
		for(i=0; i<bodies->num; i++){
			cpBody *body = (cpBody *)bodies->arr[i];
			body->velocity_func(body, gravity, damping, dt);
		}

		cpFloat dt_coef = (prev_dt == 0.0f ? 0.0f : dt/prev_dt);
		for(i=0; i<arbiters->num; i++){
			cpArbiterApplyCachedImpulse((cpArbiter *)arbiters->arr[i], dt_coef);
		}

		for(i=0; i<constraints->num; i++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[i];
			constraint->klass->applyCachedImpulse(constraint, dt_coef);
		}

		for(i=0; i<space->iterations; i++){
			int j;
			for(j=0; j<arbiters->num; j++){
				cpArbiterApplyImpulse((cpArbiter *)arbiters->arr[j]);
			}

			for(j=0; j<constraints->num; j++){
				cpConstraint *constraint = (cpConstraint *)constraints->arr[j];
				constraint->klass->applyImpulse(constraint, dt);
			}
		}

		for(i=0; i<constraints->num; i++){
			cpConstraint *constraint = (cpConstraint *)constraints->arr[i];

			cpConstraintPostSolveFunc postSolve = constraint->postSolve;
			if(postSolve) postSolve(constraint, space);
		}

		for(i=0; i<arbiters->num; i++){
			cpArbiter *arb = (cpArbiter *) arbiters->arr[i];

			cpCollisionHandler *handler = arb->handler;
			handler->postSolveFunc(arb, space, handler->userData);
		}
	} cpSpaceUnlock(space, cpTrue);
}

//
//MERGED FILE END: cpSpaceStep.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSpatialIndex.c
//

void
cpSpatialIndexFree(cpSpatialIndex *index)
{
	if(index){
		cpSpatialIndexDestroy(index);
		cpfree(index);
	}
}

cpSpatialIndex *
cpSpatialIndexInit(cpSpatialIndex *index, cpSpatialIndexClass *klass, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	index->klass = klass;
	index->bbfunc = bbfunc;
	index->staticIndex = staticIndex;

	if(staticIndex){
		cpAssertHard(!staticIndex->dynamicIndex, "This static index is already associated with a dynamic index.");
		staticIndex->dynamicIndex = index;
	}

	return index;
}

typedef struct dynamicToStaticContext {
	cpSpatialIndexBBFunc bbfunc;
	cpSpatialIndex *staticIndex;
	cpSpatialIndexQueryFunc queryFunc;
	void *data;
} dynamicToStaticContext;

static void
dynamicToStaticIter(void *obj, dynamicToStaticContext *context)
{
	cpSpatialIndexQuery(context->staticIndex, obj, context->bbfunc(obj), context->queryFunc, context->data);
}

void
cpSpatialIndexCollideStatic(cpSpatialIndex *dynamicIndex, cpSpatialIndex *staticIndex, cpSpatialIndexQueryFunc func, void *data)
{
	if(staticIndex && cpSpatialIndexCount(staticIndex) > 0){
		dynamicToStaticContext context = {dynamicIndex->bbfunc, staticIndex, func, data};
		cpSpatialIndexEach(dynamicIndex, (cpSpatialIndexIteratorFunc)dynamicToStaticIter, &context);
	}
}

//
//MERGED FILE END: cpSpatialIndex.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpSweep1D.c
//

static inline cpSpatialIndexClass *KlasscpSweep1D();

typedef struct Bounds {
	cpFloat min, max;
} Bounds;

typedef struct TableCell {
	void *obj;
	Bounds bounds;
} TableCell;

struct cpSweep1D
{
	cpSpatialIndex spatialIndex;

	int num;
	int max;
	TableCell *table;
};

static inline cpBool
BoundsOverlap(Bounds a, Bounds b)
{
	return (a.min <= b.max && b.min <= a.max);
}

static inline Bounds
BBToBounds(cpSweep1D *sweep, cpBB bb)
{
	Bounds bounds = {bb.l, bb.r};
	return bounds;
}

static inline TableCell
MakeTableCell(cpSweep1D *sweep, void *obj)
{
	TableCell cell = {obj}; cell.bounds = BBToBounds(sweep, sweep->spatialIndex.bbfunc(obj));
	return cell;
}

cpSweep1D *
cpSweep1DAlloc(void)
{
	return (cpSweep1D *)cpcalloc(1, sizeof(cpSweep1D));
}

static void
ResizeTable(cpSweep1D *sweep, int size)
{
	sweep->max = size;
	sweep->table = (TableCell *)cprealloc(sweep->table, size*sizeof(TableCell));
}

cpSpatialIndex *
cpSweep1DInit(cpSweep1D *sweep, cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	cpSpatialIndexInit((cpSpatialIndex *)sweep, KlasscpSweep1D(), bbfunc, staticIndex);

	sweep->num = 0;
	ResizeTable(sweep, 32);

	return (cpSpatialIndex *)sweep;
}

cpSpatialIndex *
cpSweep1DNew(cpSpatialIndexBBFunc bbfunc, cpSpatialIndex *staticIndex)
{
	return cpSweep1DInit(cpSweep1DAlloc(), bbfunc, staticIndex);
}

static void
cpSweep1DDestroy(cpSweep1D *sweep)
{
	cpfree(sweep->table);
	sweep->table = NULL;
}

static int
cpSweep1DCount(cpSweep1D *sweep)
{
	return sweep->num;
}

static void
cpSweep1DEach(cpSweep1D *sweep, cpSpatialIndexIteratorFunc func, void *data)
{
	TableCell *table = sweep->table;
	for(int i=0, count=sweep->num; i<count; i++) func(table[i].obj, data);
}

static int
cpSweep1DContains(cpSweep1D *sweep, void *obj, cpHashValue hashid)
{
	TableCell *table = sweep->table;
	for(int i=0, count=sweep->num; i<count; i++){
		if(table[i].obj == obj) return cpTrue;
	}

	return cpFalse;
}

static void
cpSweep1DInsert(cpSweep1D *sweep, void *obj, cpHashValue hashid)
{
	if(sweep->num == sweep->max) ResizeTable(sweep, sweep->max*2);

	sweep->table[sweep->num] = MakeTableCell(sweep, obj);
	sweep->num++;
}

static void
cpSweep1DRemove(cpSweep1D *sweep, void *obj, cpHashValue hashid)
{
	TableCell *table = sweep->table;
	for(int i=0, count=sweep->num; i<count; i++){
		if(table[i].obj == obj){
			int num = --sweep->num;

			table[i] = table[num];
			table[num].obj = NULL;

			return;
		}
	}
}

static void
cpSweep1DReindexObject(cpSweep1D *sweep, void *obj, cpHashValue hashid)
{
}

static void
cpSweep1DReindex(cpSweep1D *sweep)
{
}

static void
cpSweep1DQuery(cpSweep1D *sweep, void *obj, cpBB bb, cpSpatialIndexQueryFunc func, void *data)
{

	Bounds bounds = BBToBounds(sweep, bb);

	TableCell *table = sweep->table;
	for(int i=0, count=sweep->num; i<count; i++){
		TableCell cell = table[i];
		if(BoundsOverlap(bounds, cell.bounds) && obj != cell.obj) func(obj, cell.obj, 0, data);
	}
}

static void
cpSweep1DSegmentQuery(cpSweep1D *sweep, void *obj, cpVect a, cpVect b, cpFloat t_exit, cpSpatialIndexSegmentQueryFunc func, void *data)
{
	cpBB bb = cpBBExpand(cpBBNew(a.x, a.y, a.x, a.y), b);
	Bounds bounds = BBToBounds(sweep, bb);

	TableCell *table = sweep->table;
	for(int i=0, count=sweep->num; i<count; i++){
		TableCell cell = table[i];
		if(BoundsOverlap(bounds, cell.bounds)) func(obj, cell.obj, data);
	}
}

static int
TableSort(TableCell *a, TableCell *b)
{
	return (a->bounds.min < b->bounds.min ? -1 : (a->bounds.min > b->bounds.min ? 1 : 0));
}

static void
cpSweep1DReindexQuery(cpSweep1D *sweep, cpSpatialIndexQueryFunc func, void *data)
{
	TableCell *table = sweep->table;
	int count = sweep->num;

	int i;
	for(i=0; i<count; i++) table[i] = MakeTableCell(sweep, table[i].obj);
	qsort(table, count, sizeof(TableCell), (int (*)(const void *, const void *))TableSort);
	for(i=0; i<count; i++){
		TableCell cell = table[i];
		cpFloat max = cell.bounds.max;

		for(int j=i+1; table[j].bounds.min < max && j<count; j++){
			func(cell.obj, table[j].obj, 0, data);
		}
	}

	cpSpatialIndexCollideStatic((cpSpatialIndex *)sweep, sweep->spatialIndex.staticIndex, func, data);
}

static cpSpatialIndexClass klasscpSweep1D = {
	(cpSpatialIndexDestroyImpl)cpSweep1DDestroy,

	(cpSpatialIndexCountImpl)cpSweep1DCount,
	(cpSpatialIndexEachImpl)cpSweep1DEach,
	(cpSpatialIndexContainsImpl)cpSweep1DContains,

	(cpSpatialIndexInsertImpl)cpSweep1DInsert,
	(cpSpatialIndexRemoveImpl)cpSweep1DRemove,

	(cpSpatialIndexReindexImpl)cpSweep1DReindex,
	(cpSpatialIndexReindexObjectImpl)cpSweep1DReindexObject,
	(cpSpatialIndexReindexQueryImpl)cpSweep1DReindexQuery,

	(cpSpatialIndexQueryImpl)cpSweep1DQuery,
	(cpSpatialIndexSegmentQueryImpl)cpSweep1DSegmentQuery,
};

static inline cpSpatialIndexClass *KlasscpSweep1D(){return &klasscpSweep1D;}

//
//MERGED FILE END: cpSweep1D.c
//------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------
//MERGED FILE START: cpDecompose.cpp
//

cpBool cpPolyValidate(const cpVect *verts, const int numVerts)
{
	for (int i = 0; i < numVerts; i++) if(cpvcross(cpvsub(verts[(i+1)%numVerts], verts[i]), cpvsub(verts[(i+2)%numVerts], verts[i])) > 0.0f) return cpFalse;
	return cpTrue;
}

struct cpTriangle
{
	cpVect verts[3];
	cpTriangle(const cpVect& v1, const cpVect& v2, const cpVect& v3);
	cpTriangle();
	bool IsInside(const cpVect& v);
};

cpTriangle::cpTriangle(const cpVect& v1, const cpVect& v2, const cpVect& v3)
{
	cpFloat dx1 = v2.x-v1.x;
	cpFloat dx2 = v3.x-v1.x;
	cpFloat dy1 = v2.y-v1.y;
	cpFloat dy2 = v3.y-v1.y;
	cpFloat cross = dx1*dy2-dx2*dy1;
	bool ccw = (cross>0);
	if (!ccw){
		verts[0] = v1; verts[1] = v2; verts[2] = v3;
	} else{
		verts[0] = v1; verts[1] = v3; verts[2] = v2;
	}
}

cpTriangle::cpTriangle() { }

bool cpTriangle::IsInside(const cpVect& point)
{
		cpFloat vx2 = point.x-verts[0].x; cpFloat vy2 = point.y-verts[0].y;
		cpFloat vx1 = verts[1].x-verts[0].x; cpFloat vy1 = verts[1].y-verts[0].y;
		cpFloat vx0 = verts[2].x-verts[0].x; cpFloat vy0 = verts[2].y-verts[0].y;
		cpFloat dot00 = vx0*vx0+vy0*vy0;
		cpFloat dot01 = vx0*vx1+vy0*vy1;
		cpFloat dot02 = vx0*vx2+vy0*vy2;
		cpFloat dot11 = vx1*vx1+vy1*vy1;
		cpFloat dot12 = vx1*vx2+vy1*vy2;
		cpFloat invDenom = 1.0f / (dot00*dot11 - dot01*dot01);
		cpFloat u = (dot11*dot02 - dot01*dot12)*invDenom;
		cpFloat v = (dot00*dot12 - dot01*dot02)*invDenom;
		return ((u>0)&&(v>0)&&(u+v<1));
}

bool cpPolyShapeAddIfConvex(cpVect **pVerts, int *pNumVerts, const cpTriangle& t)
{
		cpVect *verts = *pVerts;
	int numVerts = *pNumVerts;
	int firstP = -1;
	int firstT = -1;
	int secondP = -1;
	int secondT = -1;
	for (int i = 0; i < numVerts; i++)
	{
		if (t.verts[0].x == verts[i].x && t.verts[0].y == verts[i].y) {
			if (firstP == -1) {
				firstP = i;
				firstT = 0;
			}
			else {
				secondP = i;
				secondT = 0;
			}
		}
		else if (t.verts[1].x == verts[i].x && t.verts[1].y == verts[i].y) {
			if (firstP == -1) {
				firstP = i;
				firstT = 1;
			}
			else {
				secondP = i;
				secondT = 1;
			}
		}
		else if (t.verts[2].x == verts[i].x && t.verts[2].y == verts[i].y) {
			if (firstP == -1) {
				firstP = i;
				firstT = 2;
			}
			else {
				secondP = i;
				secondT = 2;
			}
		}
	}

		if (firstP == 0 && secondP == numVerts - 1) {
		firstP = numVerts - 1;
		secondP = 0;
	}

		if (secondP == -1)
		return false;

		int tipT = 0;
	if (tipT == firstT || tipT == secondT) tipT = 1;
	if (tipT == firstT || tipT == secondT) tipT = 2;

	cpVect* newv = (cpVect*)cpcalloc(numVerts + 1, sizeof(cpVect));
	for (int currOut = 0, ip = 0; ip < numVerts; ip++)
	{
		newv[currOut++] = verts[ip];
		if (ip == firstP) newv[currOut++] = t.verts[tipT];
	}

	if (cpPolyValidate(newv, numVerts + 1))
	{
		cpfree(*pVerts);
		*pVerts = newv;
		*pNumVerts = numVerts + 1;
		return true;
	}
	else
	{
		cpfree(newv);
		return false;
	}
}

int cpPolyShapeMergeParallelEdges(cpVect *verts, int numVerts, cpFloat parallelTolerance, cpFloat distanceTolerance)
{
	cpFloat alen, blen = 0; cpVect a, b = cpvzero, *p1, *p2 = NULL, *p3 = &verts[numVerts-1];
	for (int i = 0; i <= numVerts;)
	{
				a = b; alen = blen; p1 = p2; p2 = p3; p3 = &verts[i%numVerts];
		b = cpvsub(*p3, *p2); blen = cpvlength(b);
		if (i && (alen <= distanceTolerance || blen <= distanceTolerance || cpfabs(cpvcross(a, b)/alen/blen) < parallelTolerance))
		{
			for (int imerge = i; imerge < numVerts; imerge++) verts[imerge-1] = verts[imerge];
			numVerts--;
			p3 = p2; p2 = p1; b = cpvsub(*p3, *p2); blen = cpvlength(b);
		}
		else i++;
	}
	return numVerts;
}

int PolygonizeTrianglesInto(cpTriangle* triangulated, int triangulatedLength, cpSpace *space, cpBody *body, cpFloat radius)
{
	if (triangulatedLength == 0) return 0;
	int shapeCount = 0;
	int* covered = (int*)cpcalloc(triangulatedLength, sizeof(int));
	for (int i = 0; i < triangulatedLength; ++i) covered[i] = 0;
	for (int currTri = 0; currTri < triangulatedLength; currTri++)
	{
		while (currTri < triangulatedLength && covered[currTri]) currTri++;
		if (currTri == triangulatedLength) break;

		int numVerts = 3;
		cpVect* verts = (cpVect*)cpcalloc(3, sizeof(cpVect));
		verts[0] = triangulated[currTri].verts[0];
		verts[1] = triangulated[currTri].verts[1];
		verts[2] = triangulated[currTri].verts[2];
		covered[currTri] = 1;
		for (int i = currTri; i < triangulatedLength; i++)
			if (!covered[i] && cpPolyShapeAddIfConvex(&verts, &numVerts, triangulated[i]))
				covered[i] = 1;

		numVerts = cpPolyShapeMergeParallelEdges(verts, numVerts);
		cpSpaceAddShape(space, cpPolyShapeNew(body, numVerts, verts, cpTransformIdentity, radius));
		cpfree(verts);
		shapeCount++;
	}
	cpfree(covered);
	return shapeCount;
}

bool IsEar(int i, cpVect *verts, int xvLength)
{
	cpFloat dx0, dy0, dx1, dy1;
	dx0 = dy0 = dx1 = dy1 = 0;
	if (i >= xvLength || i < 0 || xvLength < 3) return false;
	int upper = i + 1;
	int lower = i - 1;
	if (i == 0)
	{
		dx0 = verts[0].x - verts[xvLength - 1].x;
		dy0 = verts[0].y - verts[xvLength - 1].y;
		dx1 = verts[1].x - verts[0].x;
		dy1 = verts[1].y - verts[0].y;
		lower = xvLength - 1;
	}
	else if (i == xvLength - 1)
	{
		dx0 = verts[i].x - verts[i - 1].x;
		dy0 = verts[i].y - verts[i - 1].y;
		dx1 = verts[0].x - verts[i].x;
		dy1 = verts[0].y - verts[i].y;
		upper = 0;
	}
	else
	{
		dx0 = verts[i].x - verts[i - 1].x;
		dy0 = verts[i].y - verts[i - 1].y;
		dx1 = verts[i + 1].x - verts[i].x;
		dy1 = verts[i + 1].y - verts[i].y;
	}
	cpFloat cross = dx0 * dy1 - dx1 * dy0;
	if (cross > 0)
		return false;
	cpTriangle myTri(verts[i], verts[upper], verts[lower]);
	for (int j = 0; j < xvLength; ++j)
	{
		if (j == i || j == lower || j == upper)
			continue;
		if (myTri.IsInside(verts[j]))
			return false;
	}
	return true;
}

int TriangulatePolygon(const cpVect *verts, int vNum, cpTriangle* results)
{
	if (vNum < 3)
		return 0;

	int bufferSize = 0;
	cpVect* vrem = (cpVect*)cpcalloc(vNum, sizeof(cpVect));
	for (int i = 0; i < vNum; ++i) {
		vrem[i] = verts[i];
	}

	int xremLength = vNum;(void)xremLength;

	while (vNum > 3) {
				int earIndex = -1;
		for (int i = 0; i < vNum; ++i) {
			if (IsEar(i, vrem, vNum)) {
				earIndex = i;
				break;
			}
		}

												if (earIndex == -1) {
			cpfree(vrem);
			return -1;
		}

		--vNum;

				int under = (earIndex == 0) ? (vNum) : (earIndex - 1);
		int over = (earIndex == vNum) ? 0 : (earIndex + 1);
		results[bufferSize++] = cpTriangle(vrem[earIndex], vrem[over], vrem[under]);

		for (int iv = earIndex; iv < vNum; ++iv) {
			vrem[iv] = vrem[iv+1];
		}
	}
	results[bufferSize++] = cpTriangle(vrem[1], vrem[2], vrem[0]);
	cpfree(vrem);
	cpAssertSoft(bufferSize == xremLength-2, "triangulating polygon resulted in wrong polygon size");
	return bufferSize;
}

void cpPolyShapeReverse(cpVect *v, int n)
{
	if (n == 1) return;
	int low = 0;
	int high = n - 1;
	while (low < high)
	{
		cpFloat buffer = v[low].x;
		v[low].x = v[high].x;
		v[high].x = buffer;
		buffer = v[low].y;
		v[low].y = v[high].y;
		v[high].y = buffer;
		++low;
		--high;
	}
}

int cpDecomposeConvexInto(const cpVect *verts, int numVerts, cpSpace *space, cpBody *body, cpFloat radius)
{
	if (numVerts < 3) return 0;
	cpTriangle* triangulated = (cpTriangle*)cpcalloc(numVerts - 2, sizeof(cpTriangle));
	int nTri = TriangulatePolygon(verts, numVerts, triangulated);
	if (nTri < 1)  return -1;
	int nPolys = PolygonizeTrianglesInto(triangulated, nTri, space, body, radius);
	cpfree(triangulated);
	return nPolys;
}

//
//MERGED FILE END: cpDecompose.cpp
//------------------------------------------------------------------------------------------------------------------------------------------------------
