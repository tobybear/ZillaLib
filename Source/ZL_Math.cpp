/*
  ZillaLib
  Copyright (C) 2010-2018 Bernhard Schelling

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "ZL_Math.h"
#include "ZL_Math3D.h"
#include <stdlib.h>
#include <ZL_Display.h>
#undef _MSC_EXTENSIONS

//fast floating point functions
//turns out system provided functions are faster than these (at least on WIN32)

//float fsqrt(float number)
//{
//	float x = number * 0.5F;
//	float y  = number;
//	long i  = *(long*)(void*)&y;
//	i  = 0x5f3759df - ( i >> 1 );
//	y  = *(float*)(void*)&i;
//	y  = y * ( 1.5F - ( x * y * y ) );
//	y  = y * ( 1.5F - ( x * y * y ) );
//	return number * y;
//}

//float fatan2(float y, float x)
//{
//	const float coeff_1 = float(PI) / 4.0f;
//	const float coeff_2 = 3.0f * coeff_1;
//	if (y == 0) return (x < 0 ? float(PI) : 0);
//	if (x == 0) return (y < 0 ? -float(PI)/2 : float(PI)/2);
//	float abs_y = (y < 0 ? -y : y);
//	float angle = 0;
//	//if (x >= 0 && x != -abs_y) angle = coeff_1 - coeff_1 * ((x - abs_y) / (x + abs_y)); //less acc
//	//else if (x != abs_y)       angle = coeff_2 - coeff_1 * ((x + abs_y) / (abs_y - x)); //less acc
//	if (x >= 0 && x != -abs_y) { float r = ((x - abs_y) / (x + abs_y)); angle = 0.1963f*r*r*r - 0.9817f*r + coeff_1; }
//	else if (x != abs_y)       { float r = ((x + abs_y) / (abs_y - x)); angle = 0.1963f*r*r*r - 0.9817f*r + coeff_2; }
//	return (y < 0 ? -angle : angle);
//}

const ZL_Vector ZL_Vector::Zero(0, 0), ZL_Vector::One(1, 1), ZL_Vector::Right(1, 0), ZL_Vector::Up(0, 1);

ZL_Vector &ZL_Vector::Rotate(scalar angle_rad)
{
	scalar cosphi = scos(angle_rad), sinphi = ssin(angle_rad), oldx = x;
	x = cosphi*oldx - sinphi*y;
	y = sinphi*oldx + cosphi*y;
	return *this;
}

ZL_Vector &ZL_Vector::Rotate(const ZL_Vector &hotspot, scalar angle_rad)
{
	scalar tmpX = x - hotspot.x, tmpY = y - hotspot.y;
	scalar cosphi = scos(angle_rad), sinphi = ssin(angle_rad);
	x = hotspot.x + cosphi*tmpX - sinphi*tmpY, y = hotspot.y + sinphi*tmpX + cosphi*tmpY;
	return *this;
}

ZL_Vector &ZL_Vector::RotateDeg(scalar angle_deg)
{
	scalar phi = angle_deg * PIOVER180, oldx = x;
	scalar cosphi = scos(phi), sinphi = ssin(phi);
	x = cosphi*oldx - sinphi*y;
	y = sinphi*oldx + cosphi*y;
	return *this;
}

ZL_Vector &ZL_Vector::RotateDeg(const ZL_Vector &hotspot, scalar angle_deg)
{
	scalar tmpX = x - hotspot.x, tmpY = y - hotspot.y, phi = angle_deg * PIOVER180;
	scalar cosphi = scos(phi), sinphi = ssin(phi);
	x = hotspot.x + cosphi*tmpX - sinphi*tmpY, y = hotspot.y + sinphi*tmpX + cosphi*tmpY;
	return *this;
}

ZL_Vector &ZL_Vector::SetLength(scalar setlength)
{
	if (!x && !y) { x = setlength; return *this; }
	return Mul(setlength / GetLength());
}

ZL_Vector &ZL_Vector::AddLength(scalar addlength)
{
	if (!x && !y) { x = addlength; return *this; }
	return Mul(addlength / GetLength() + 1);
}

ZL_Vector &ZL_Vector::SetMaxLength(scalar maxlength)
{
	if (!x && !y)  { return *this; }
	scalar lensq = x*x+y*y;
	return (maxlength*maxlength >= lensq ? *this : Mul(maxlength / ssqrt(lensq)));
}

ZL_Vector &ZL_Vector::SetMinLength(scalar minlength)
{
	if (!x && !y) { x = minlength; return *this; }
	scalar lensq = x*x+y*y;
	return (minlength*minlength <= lensq ? *this : Mul(minlength / ssqrt(lensq)));
}

ZL_Vector ZL_Vector::VecRotate(scalar angle_rad) const
{
	scalar cosphi = scos(angle_rad), sinphi = ssin(angle_rad);
	return ZL_Vector(cosphi*x - sinphi*y, sinphi*x + cosphi*y);
}

ZL_Vector ZL_Vector::VecRotate(const ZL_Vector &hotspot, scalar angle_rad) const
{
	scalar tmpX = x - hotspot.x, tmpY = y - hotspot.y;
	scalar cosphi = scos(angle_rad), sinphi = ssin(angle_rad);
	return ZL_Vector(hotspot.x + cosphi*tmpX - sinphi*tmpY, hotspot.y + sinphi*tmpX + cosphi*tmpY);
}

ZL_Vector ZL_Vector::VecRotateDeg(scalar angle_deg) const
{
	scalar phi = angle_deg * PIOVER180;
	scalar cosphi = scos(phi), sinphi = ssin(phi);
	return ZL_Vector(cosphi*x - sinphi*y, sinphi*x + cosphi*y);
}

ZL_Vector ZL_Vector::VecRotateDeg(const ZL_Vector &hotspot, scalar angle_deg) const
{
	scalar tmpX = x - hotspot.x, tmpY = y - hotspot.y, phi = angle_deg * PIOVER180;
	scalar cosphi = scos(phi), sinphi = ssin(phi);
	return ZL_Vector(hotspot.x + cosphi*tmpX - sinphi*tmpY, hotspot.y + sinphi*tmpX + cosphi*tmpY);
}

ZL_Vector ZL_Vector::VecWithLength(scalar setlength) const
{
	if (!x && !y) return ZL_Vector(setlength, 0);
	return *this * (setlength / GetLength());
}

ZL_Vector ZL_Vector::VecWithAddLength(scalar addlength) const
{
	if (!x && !y) return ZL_Vector(addlength, 0);
	return *this * (addlength / GetLength() + 1);
}

ZL_Vector ZL_Vector::VecWithMaxLength(scalar maxlength) const
{
	if (!x && !y) return *this;
	scalar lensq = x*x+y*y;
	return (maxlength*maxlength >= lensq ? *this : *this * (maxlength / ssqrt(lensq)));
}

ZL_Vector ZL_Vector::VecWithMinLength(scalar minlength) const
{
	if (!x && !y) return ZL_Vector(minlength, 0);
	scalar lensq = x*x+y*y;
	return (minlength*minlength <= lensq ? *this : *this * (minlength / ssqrt(lensq)));
}

ZL_Vector3::ZL_Vector3(const struct ZL_Color &rgb) :  x(rgb.r), y(rgb.g), z(rgb.b) { }
const ZL_Vector3 ZL_Vector3::Zero(0,0,0), ZL_Vector3::One(1,1,1);
ZL_Vector3 ZL_Vector3::Right(1,0,0), ZL_Vector3::Forward(0,1,0), ZL_Vector3::Up(0,0,1);
const ZL_Quat ZL_Quat::Identity;
const ZL_Matrix ZL_Matrix::Identity;

ZL_AABB::ZL_AABB(const ZL_Rectf& rect) : P(rect.Center()), E(rect.Extents()) {}

bool ZL_AABB::Overlaps(const ZL_Vector& c, scalar r) const
{
	scalar s, d = 0; //find the square of the distance from the sphere to the box
	if      (c.x < (P.x - E.x)) { s = c.x - (P.x - E.x); d += s*s; }
	else if (c.x > (P.x + E.x)) { s = c.x - (P.x + E.x); d += s*s; }
	if      (c.y < (P.y - E.y)) { s = c.y - (P.y - E.y); d += s*s; }
	else if (c.y > (P.y + E.y)) { s = c.y - (P.y + E.y); d += s*s; }
	return d <= r*r;
	/*
	ZL_Vector diff(sabs(P.x - c.x) - E.x, sabs(P.y - c.y) - E.y);
	if (diff.x > r || diff.y > r) return false;
	if (diff.x <= 0 || diff.y <= 0) return true;
	return (diff <= r);
	*/
}

ZL_RotBB::ZL_RotBB(const ZL_Rectf& rect, scalar a) : P(rect.Center()), E(rect.Extents()), A(a) {}

bool ZL_RotBB::Overlaps(const ZL_Vector& c, scalar r) const
{
	ZL_Vector diff(c.x - P.x, c.y - P.y);
	scalar difflensq = (diff.x*diff.x + diff.y*diff.y);
	if (difflensq > ((E.x+r)*(E.x+r) + (E.y+r)*(E.y+r))) return false;
	//if (E <= ssqrt(difflensq)-r) return false;
	diff.Rotate(-A);
	scalar s, d = 0; //find the square of the distance from the sphere to the box
	if      (diff.x < - E.x) { s = diff.x + E.x; d += s*s; }
	else if (diff.x > + E.x) { s = diff.x - E.x; d += s*s; }
	if      (diff.y < - E.y) { s = diff.y + E.y; d += s*s; }
	else if (diff.y > + E.y) { s = diff.y - E.y; d += s*s; }
	return d <= r*r;
	/*
	diff.x = sabs(diff.x) - E.x; diff.y = sabs(diff.y) - E.y;
	if (diff.x > r || diff.y > r) return false;
	if (diff.x <= 0 || diff.y <= 0) return true;
	return (diff <= r);
	*/
}

bool ZL_Math::CircleCollision(const ZL_Vector& c1, const scalar r1, const ZL_Vector& c2, const scalar r2, scalar* pIntersectLength)
{
	scalar distx = c1.x - c2.x, disty = c1.y - c2.y, rsum = (r1 + r2);
	scalar squaredist = (distx * distx) + (disty * disty);
	if (squaredist > rsum*rsum) return false;
	if (pIntersectLength) *pIntersectLength = rsum - ssqrt(squaredist);
	return true;
}

bool ZL_Math::LineCollision(const ZL_Vector& a1, const ZL_Vector& a2, const ZL_Vector& b1, const ZL_Vector& b2, ZL_Vector* pCollision)
{
	scalar d = ((b2.y - b1.y) * (a2.x - a1.x)) - ((b2.x - b1.x) * (a2.y - a1.y));
	if (d == 0) return false;
	scalar ua = (((b2.x - b1.x) * (a1.y - b1.y)) - ((b2.y - b1.y) * (a1.x - b1.x))) / d;
	if (ua < 0 || ua > 1) return false;
	scalar ub = (((a2.x - a1.x) * (a1.y - b1.y)) - ((a2.y - a1.y) * (a1.x - b1.x))) / d;
	if (ub < 0 || ub > 1) return false;
	if (pCollision) *pCollision = (((a2 - a1) *= ua) += a1);
	return true;
}

scalar LineIntersectU(const ZL_Vector& p1, const ZL_Vector& p2, const ZL_Vector& circle) //, const scalar rad)
{
	ZL_Vector p1p2 = p2 - p1;
	scalar a = (p1p2.x) * (p1p2.x) + (p1p2.y) * (p1p2.y), b = 2 * ((p1p2.x * (p1.x - circle.x)) + (p1p2.y * (p1.y - circle.y))); //, c = (lp1.x * lp1.x) + (lp1.y * lp1.y) - (rad * rad);
	scalar delta = b * b - (4 * a); // * c);
	if (delta < 0) return -1; // no intersection
	return -b / (2 * a);
}

int ZL_Math::LineCircleCollision(const ZL_Vector& p1, const ZL_Vector& p2, const ZL_Vector& circle, const scalar rad, ZL_Vector* pCollision1, ZL_Vector* pCollision2)
{
	ZL_Vector lp1 = p1 - circle, p1p2 = p2 - p1;
	scalar a = (p1p2.x) * (p1p2.x) + (p1p2.y) * (p1p2.y), b = 2 * ((p1p2.x * lp1.x) + (p1p2.y * lp1.y)), c = (lp1.x * lp1.x) + (lp1.y * lp1.y) - (rad * rad);
	scalar delta = b * b - (4 * a * c);
	if (delta < 0) return 0; // no intersection
	else if (delta == 0) //one intersection
	{
		scalar u = -b / (2 * a);
		if (u < 0 || u > 1) return 0;
		if (!pCollision1 && !pCollision2) return 1;
		if (pCollision1 && pCollision2) *pCollision1 = *pCollision2 = ((p1p2 *= u) += p1);
		else if (pCollision1) *pCollision1 = ((p1p2 *= u) += p1);
		else *pCollision2 = ((p1p2 *= u) += p1);
		return 1;
	}
	// Two intersections
	scalar SquareRootDelta = ssqrt(delta);
	scalar u1 = (-b + SquareRootDelta) / (2 * a);
	scalar u2 = (-b - SquareRootDelta) / (2 * a);
	if (u1 >= 0 && u1 <= 1)
	{
		if (u2 >= 0 && u2 <= 1)
		{
			if (!pCollision1 && !pCollision2) return 2;
			if (pCollision1 && !pCollision2) (((*pCollision1 = p1p2) *= ((u1+u2)/s(2))) += p1);
			else if (pCollision1) (((*pCollision1 = p1p2) *= u1) += p1);
			if (pCollision2) (((*pCollision2 = p1p2) *= u2) += p1);
			return 2;
		}
		else
		{
			if (!pCollision1 && !pCollision2) return 1;
			if (pCollision1 && pCollision2) *pCollision1 = *pCollision2 = ((p1p2 *= u1) += p1);
			else if (pCollision1) *pCollision1 = ((p1p2 *= u1) += p1);
			else *pCollision2 = ((p1p2 *= u1) += p1);
			return 1;
		}
	}
	else if (u2 >= 0 && u2 <= 1)
	{
		if (!pCollision1 && !pCollision2) return 1;
		if (pCollision1 && pCollision2) *pCollision1 = *pCollision2 = ((p1p2 *= u2) += p1);
		else if (pCollision1) *pCollision1 = ((p1p2 *= u2) += p1);
		else *pCollision2 = ((p1p2 *= u2) += p1);
		return 1;
	}
	return 0;
}

int ZL_Math::LineRectCollision(const ZL_Vector& Line1, const ZL_Vector& Line2, const ZL_Rectf& Rectangle, ZL_Vector* pCollision1, ZL_Vector* pCollision2, ZL_Vector* pCollision3, ZL_Vector* pCollision4)
{ // 4 collision points are impossible, but let's implement it anyways, just in case
	ZL_Vector collisions[4];
	ZL_Vector colLeft(0, 0), colHigh(0, 0), colLow(0, 0), colRight(0, 0);
	int i = 0;
	if (LineCollision(Line1, Line2, ZL_Vector(Rectangle.left, Rectangle.high), ZL_Vector(Rectangle.left, Rectangle.low), &colLeft))
		collisions[i++] = colLeft;
	if (LineCollision(Line1, Line2, ZL_Vector(Rectangle.right, Rectangle.high), ZL_Vector(Rectangle.left, Rectangle.high), &colHigh))
		collisions[i++] = colHigh;
	if (LineCollision(Line1, Line2, ZL_Vector(Rectangle.right, Rectangle.low), ZL_Vector(Rectangle.left, Rectangle.low), &colLow))
		collisions[i++] = colLow;
	if (LineCollision(Line1, Line2, ZL_Vector(Rectangle.right, Rectangle.high), ZL_Vector(Rectangle.right, Rectangle.low), &colRight))
		collisions[i++] = colRight;
	switch (i)
	{
		case 4: if (pCollision4) *pCollision4 = collisions[3];
		case 3: if (pCollision3) *pCollision3 = collisions[2];
		case 2: if (pCollision2) *pCollision2 = collisions[1];
		case 1: if (pCollision1) *pCollision1 = collisions[0];
	} // I just love how C++ allows falling from one case to another :D
	return i;
}


// Return true if first root r1 and second root r2 are real
static inline bool QuadraticFormula(const scalar a, const scalar b, const scalar c, scalar& r1, scalar& r2)
{
	const scalar q = b*b - 4*a*c;
	if (q >= 0)
	{
		const scalar sq = ssqrt(q);
		const scalar d = 1 / (2*a);
		r1 = ( -b + sq ) * d;
		r2 = ( -b - sq ) * d;
		return true; //real roots
	}
	else return false; //complex roots
}

bool ZL_Math::CircleSweep(const ZL_Vector& CenterA1, const ZL_Vector& CenterA2, const scalar RadiusA, const ZL_Vector& CenterB1, const ZL_Vector& CenterB2, const scalar RadiusB, scalar* pCollisionTime1, scalar* pCollisionTime2)
{
	const ZL_Vector AB = CenterB1 - CenterA1;
	const scalar rab = RadiusA + RadiusB;
	const scalar c = AB.GetLengthSq() - (rab*rab); //constant term
	if (c <= 0) //check if they're currently overlapping
	{
		if (pCollisionTime1) *pCollisionTime1 = 0;
		if (pCollisionTime2) *pCollisionTime2 = 0;
		return true;
	}
	ZL_Vector vab = CenterB2; vab -= CenterB1; vab -= CenterA2; vab += CenterA1; //relative velocity (in normalized time)
	if (vab.x == 0 && vab.y == 0) return false; //no motion or exact same motion
	const scalar a = vab.DotP(vab); //u*u coefficient
	const scalar b = 2*(vab.DotP(AB));//u coefficient
	scalar u1, u2;
	if (QuadraticFormula(a, b, c, u1, u2)) //check if they hit each other during the frame
	{
		if ((u1 < 0 || u1 > 1) && (u2 <0 || u2 > 1)) return false;
		if (pCollisionTime1) *pCollisionTime1 = (u1 > u2 ? u2 : u1);
		if (pCollisionTime2) *pCollisionTime2 = (u1 > u2 ? u1 : u2);
		return true;
	}
	return false;
}

bool ZL_Math::CirclePointSweep(const ZL_Vector& Center1, const ZL_Vector& Center2, const scalar Radius, const ZL_Vector& Point, ZL_Vector* pCollision, scalar* pCollisionTime)
{
	const ZL_Vector AB = Point - Center1;
	const scalar c = AB.GetLengthSq() - (Radius*Radius); //constant term
	if (c <= 0) //check if they're currently overlapping
	{
		if (pCollision) *pCollision = Center1;
		if (pCollisionTime) *pCollisionTime = 0;
		return true;
	}
	const ZL_Vector vab = Center1 - Center2; //relative velocity (in normalized time)
	if (vab.x == 0 && vab.y == 0) return false; //no motion
	const scalar a = vab.DotP(vab); //u*u coefficient
	const scalar b = 2*(vab.DotP(AB));//u coefficient
	scalar u1, u2;
	if (QuadraticFormula(a, b, c, u1, u2)) //check if they hit each other during the frame
	{
		if ((u1 < 0 || u1 > 1) && (u2 <0 || u2 > 1)) return false;
		if (pCollision) *pCollision = Center1 + (Center2 - Center1) * (u1 > u2 ? u2 : u1);
		if (pCollisionTime) *pCollisionTime = (u1 > u2 ? u2 : u1);
		return true;
	}
	return false;
}

bool ZL_Math::CirclePlaneSweep(const ZL_Vector& Center1, const ZL_Vector& Center2, const scalar Radius, const ZL_Plane& plane, ZL_Vector* pCollision, scalar* pCollisionTime)
{
	const scalar d1 = plane.GetDistanceToPoint(Center1);

	//check if it was touching on previous frame
	if (d1 >= -Radius && d1 <= Radius)
	{
		if (pCollision) *pCollision = Center1;
		if (pCollisionTime) *pCollisionTime = 0;
		return true;
	}

	//check if the sphere penetrated during this frame
	const scalar d2 = plane.GetDistanceToPoint(Center2);
	if (d1 > Radius && d2 < Radius)
	{
		if (!pCollisionTime && !pCollision) return true;
		scalar u = (d1-Radius)/(d1-d2); //normalized time
		if (pCollisionTime) *pCollisionTime = u;
		if (pCollision) *pCollision = Center1*(1-u) + Center2*u; //point of first contact
		return true;
	}
	return false;
}

bool ZL_Math::CircleLineSweep(const ZL_Vector& Center1, const ZL_Vector& Center2, const scalar Radius, const ZL_Vector& p1, const ZL_Vector& p2, ZL_Vector* pCollision, scalar* pCollisionTime)
{
	ZL_Vector Collision;
	scalar CollisionTime;
	ZL_Plane plane(p1, p2);
	if (!CirclePlaneSweep(Center1, Center2, Radius, plane, &Collision, &CollisionTime) &&
	    !CirclePlaneSweep(Center1, Center2, Radius, plane.Invert(), &Collision, &CollisionTime)) return false;
	ZL_Vector p1p2 = p2 - p1;
	scalar a = (p1p2.x) * (p1p2.x) + (p1p2.y) * (p1p2.y), b = 2 * ((p1p2.x * (p1.x - Collision.x)) + (p1p2.y * (p1.y - Collision.y)));
	scalar delta = b * b - (4 * a);
	if (delta < 0) return false; // no intersection
	scalar u = -b / (2 * a);
	if (u >= 0 && u <= 1)
	{
		if (pCollision) *pCollision = Collision;
		if (pCollisionTime) *pCollisionTime = CollisionTime;
		return true;
	}
	return ((u < 0 && CirclePointSweep(Center1, Center2, Radius, p1, pCollision, pCollisionTime)) ||
		    (u > 1 && CirclePointSweep(Center1, Center2, Radius, p2, pCollision, pCollisionTime)));
}

bool ZL_Math::AABBSweep(const ZL_AABB& A, const ZL_Vector& OldPosA, const ZL_AABB& B, const ZL_Vector& OldPosB, scalar* pCollisionTime1, scalar* pCollisionTime2)
{
	if (sabs(OldPosB.x-OldPosA.x) <= (A.E.x + B.E.x) && sabs(OldPosB.y-OldPosA.y) <= (A.E.y + B.E.y)) //check if they were overlapping on the previous frame
	{
		if (pCollisionTime1) *pCollisionTime1 = 0;
		if (pCollisionTime2) *pCollisionTime2 = 0;
		return true;
	}
	//the problem is solved in A's frame of reference
	ZL_Vector v = B.P; v-= OldPosB; v -= A.P; v += OldPosA; //relative velocity (in normalized time)
	ZL_Vector u_1(-999,-999); //first times of overlap along each axis
	ZL_Vector u_2( 999, 999); //last times of overlap along each axis
	//find the possible first and last times of overlap along each axis
	if      (OldPosA.x+A.E.x < OldPosB.x-B.E.x && v.x < 0) u_1.x = ((OldPosA.x+A.E.x) - (OldPosB.x-B.E.x)) / v.x;
	else if (OldPosB.x+B.E.x < OldPosA.x-A.E.x && v.x > 0) u_1.x = ((OldPosA.x-A.E.x) - (OldPosB.x+B.E.x)) / v.x;
	if      (OldPosB.x+B.E.x > OldPosA.x-A.E.x && v.x < 0) u_2.x = ((OldPosA.x-A.E.x) - (OldPosB.x+B.E.x)) / v.x;
	else if (OldPosA.x+A.E.x > OldPosB.x-B.E.x && v.x > 0) u_2.x = ((OldPosA.x+A.E.x) - (OldPosB.x-B.E.x)) / v.x;
	if      (OldPosA.y+A.E.y < OldPosB.y-B.E.y && v.y < 0) u_1.y = ((OldPosA.y+A.E.y) - (OldPosB.y-B.E.y)) / v.y;
	else if (OldPosB.y+B.E.y < OldPosA.y-A.E.y && v.y > 0) u_1.y = ((OldPosA.y-A.E.y) - (OldPosB.y+B.E.y)) / v.y;
	if      (OldPosB.y+B.E.y > OldPosA.y-A.E.y && v.y < 0) u_2.y = ((OldPosA.y-A.E.y) - (OldPosB.y+B.E.y)) / v.y;
	else if (OldPosA.y+A.E.y > OldPosB.y-B.E.y && v.y > 0) u_2.y = ((OldPosA.y+A.E.y) - (OldPosB.y-B.E.y)) / v.y;
	scalar u1 = MAX(u_1.x, u_1.y); //possible first time of overlap
	scalar u2 = MIN(u_2.x, u_2.y); //possible last time of overlap
	if (u1 <= u2 && u1 >= 0 && u1 <= 1) //they could have only collided if the first time of overlap occurred before the last time of overlap
	{
		if (pCollisionTime1) *pCollisionTime1 = u1;
		if (pCollisionTime2) *pCollisionTime2 = u2;
		return true;
	}
	return false;
}

bool ZL_Math::AABBSweep(const ZL_AABB& A, const ZL_Vector& OldPosA, const ZL_AABB& B, ZL_Vector* pCollision, scalar* pCollisionTime)
{
	if (sabs(B.P.x-OldPosA.x) <= (A.E.x + B.E.x) && sabs(B.P.y-OldPosA.y) <= (A.E.y + B.E.y)) //check if they were overlapping on the previous frame
	{
		if (pCollision) *pCollision = OldPosA;
		if (pCollisionTime) *pCollisionTime = 0;
		return true;
	}
	//the problem is solved in A's frame of reference
	ZL_Vector v = OldPosA; v -= A.P; //relative velocity (in normalized time)
	ZL_Vector u_1(-999,-999); //first times of overlap along each axis
	ZL_Vector u_2( 999, 999); //last times of overlap along each axis
	//find the possible first and last times of overlap along each axis
	if      (OldPosA.x+A.E.x < B.P.x-B.E.x && v.x < 0) u_1.x = ((OldPosA.x+A.E.x) - (B.P.x-B.E.x)) / v.x;
	else if (B.P.x+B.E.x < OldPosA.x-A.E.x && v.x > 0) u_1.x = ((OldPosA.x-A.E.x) - (B.P.x+B.E.x)) / v.x;
	if      (B.P.x+B.E.x > OldPosA.x-A.E.x && v.x < 0) u_2.x = ((OldPosA.x-A.E.x) - (B.P.x+B.E.x)) / v.x;
	else if (OldPosA.x+A.E.x > B.P.x-B.E.x && v.x > 0) u_2.x = ((OldPosA.x+A.E.x) - (B.P.x-B.E.x)) / v.x;
	if      (OldPosA.y+A.E.y < B.P.y-B.E.y && v.y < 0) u_1.y = ((OldPosA.y+A.E.y) - (B.P.y-B.E.y)) / v.y;
	else if (B.P.y+B.E.y < OldPosA.y-A.E.y && v.y > 0) u_1.y = ((OldPosA.y-A.E.y) - (B.P.y+B.E.y)) / v.y;
	if      (B.P.y+B.E.y > OldPosA.y-A.E.y && v.y < 0) u_2.y = ((OldPosA.y-A.E.y) - (B.P.y+B.E.y)) / v.y;
	else if (OldPosA.y+A.E.y > B.P.y-B.E.y && v.y > 0) u_2.y = ((OldPosA.y+A.E.y) - (B.P.y-B.E.y)) / v.y;
	scalar u1 = MAX(u_1.x, u_1.y); //possible first time of overlap
	scalar u2 = MIN(u_2.x, u_2.y); //possible last time of overlap
	if (u1 <= u2 && u1 >= 0 && u1 <= 1) //they could have only collided if the first time of overlap occurred before the last time of overlap
	{
		if (pCollision) *pCollision = OldPosA + (A.P - OldPosA) * u1;
		if (pCollisionTime) *pCollisionTime = u1;
		return true;
	}
	return false;
}

bool ZL_Math::CircleAABBSweep(const ZL_Vector& c1, const ZL_Vector& c2, const scalar r, const ZL_AABB& bb, ZL_Vector* pCollision, scalar* pCollisionTime)
{
	ZL_Vector Collision;
	scalar CollisionTime;
	scalar *pGetColTime = (pCollisionTime ? &CollisionTime : NULL);
	if (!AABBSweep(ZL_AABB(c2, ZL_Vector(r, r)), c1, bb, &Collision, pGetColTime)) return false;
	if      (Collision.x > bb.P.x+bb.E.x && Collision.y > bb.P.y+bb.E.y) { if (!ZL_Math::CirclePointSweep(c1, c2, r, ZL_Vector(bb.P.x+bb.E.x, bb.P.y+bb.E.y), &Collision, pGetColTime)) return false; }
	else if (Collision.x > bb.P.x+bb.E.x && Collision.y < bb.P.y-bb.E.y) { if (!ZL_Math::CirclePointSweep(c1, c2, r, ZL_Vector(bb.P.x+bb.E.x, bb.P.y-bb.E.y), &Collision, pGetColTime)) return false; }
	else if (Collision.x < bb.P.x-bb.E.x && Collision.y > bb.P.y+bb.E.y) { if (!ZL_Math::CirclePointSweep(c1, c2, r, ZL_Vector(bb.P.x-bb.E.x, bb.P.y+bb.E.y), &Collision, pGetColTime)) return false; }
	else if (Collision.x < bb.P.x-bb.E.x && Collision.y < bb.P.y-bb.E.y) { if (!ZL_Math::CirclePointSweep(c1, c2, r, ZL_Vector(bb.P.x-bb.E.x, bb.P.y-bb.E.y), &Collision, pGetColTime)) return false; }
	if (pCollision) *pCollision = Collision;
	if (pCollisionTime) *pCollisionTime = CollisionTime;
	return true;
}

bool ZL_Math::CircleRotBBSweep(const ZL_Vector& c1, const ZL_Vector& c2, const scalar r, const ZL_RotBB& bb, ZL_Vector* pCollision, scalar* pCollisionTime)
{
	ZL_Vector lc1 = (c1 - bb.P).Rotate(-bb.A), lc2 = (c2 - bb.P).Rotate(-bb.A);
	ZL_Vector Collision;
	scalar CollisionTime;
	scalar *pGetColTime = (pCollisionTime ? &CollisionTime : NULL);
	if (!AABBSweep(ZL_AABB(lc2, r), lc1, ZL_AABB(ZL_Vector(), bb.E), &Collision, pGetColTime)) return false;
	if      (Collision.x >  bb.E.x && Collision.y >  bb.E.y) { if (!ZL_Math::CirclePointSweep(lc1, lc2, r, bb.E                       , &Collision, pGetColTime)) return false; }
	else if (Collision.x >  bb.E.x && Collision.y < -bb.E.y) { if (!ZL_Math::CirclePointSweep(lc1, lc2, r, ZL_Vector( bb.E.x, -bb.E.y), &Collision, pGetColTime)) return false; }
	else if (Collision.x < -bb.E.x && Collision.y >  bb.E.y) { if (!ZL_Math::CirclePointSweep(lc1, lc2, r, ZL_Vector(-bb.E.x,  bb.E.y), &Collision, pGetColTime)) return false; }
	else if (Collision.x < -bb.E.x && Collision.y < -bb.E.y) { if (!ZL_Math::CirclePointSweep(lc1, lc2, r, ZL_Vector(-bb.E.x, -bb.E.y), &Collision, pGetColTime)) return false; }
	if (pCollision) (*pCollision = Collision).Rotate(bb.A) += bb.P;
	if (pCollisionTime) *pCollisionTime = CollisionTime;
	return true;
}

ZL_Vector ZL_Rand::PointIn(const ZL_Rectf& rec) { return ZL_Vector(Range(rec.left, rec.right), Range(rec.low, rec.high)); }
ZL_Vector ZL_SeededRand::PointIn(const ZL_Rectf& rec) { return ZL_Vector(Range(rec.left, rec.right), Range(rec.low, rec.high)); }

ZL_SeededRand::ZL_SeededRand(unsigned int rand_seed) : w(rand_seed), z((rand_seed ^ 362436069) ^ 521288629) { }
ZL_SeededRand::ZL_SeededRand(unsigned int rand_seed, int seed_xor) : w(rand_seed), z((rand_seed ^ 362436069) ^ seed_xor) { }
ZL_SeededRand& ZL_SeededRand::Seed(unsigned int rand_seed) { w = rand_seed, z = (rand_seed ^ 362436069) ^ 521288629; return *this; }
ZL_SeededRand& ZL_SeededRand::Seed(unsigned int rand_seed, int seed_xor) { w = rand_seed, z = (rand_seed ^ 362436069) ^ seed_xor; return *this; }

unsigned int ZL_SeededRand::UInt()
{
	z = 36969 * (z & 65535) + (z >> 16);
	w = 18000 * (w & 65535) + (w >> 16);
	return (z << 16) + w;
}

scalar ZL_SeededRand::FactorEx()
{
	z = 36969 * (z & 65535) + (z >> 16);
	w = 18000 * (w & 65535) + (w >> 16);
#ifndef ZL_DOUBLE_PRECISCION
	return (scalar)(((z << 16) + w) * 2.3283063671497593e-010); //(1.0/(0xFFFFFFFF+1+0x80))
#else
	return (scalar)(((z << 16) + w) * 2.3283064365386963e-010); //(1.0/(0xFFFFFFFF+1))
#endif
}

scalar ZL_SeededRand::Factor()
{
	z = 36969 * (z & 65535) + (z >> 16);
	w = 18000 * (w & 65535) + (w >> 16);
	return (scalar)(((z << 16) + w) * 2.3283064370807974e-010); //(1.0/(0xFFFFFFFF))
}
