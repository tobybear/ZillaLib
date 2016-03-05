/*
  ZillaLib
  Copyright (C) 2010-2016 Bernhard Schelling

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

#ifndef __ZL_MODEL__
#define __ZL_MODEL__

#include <ZL_Math.h>
#include <ZL_String.h>
#include <ZL_Surface.h>
#include <ZL_Data.h>
#include <map>
#include <vector>

struct ZL_ModelManager
{

	static bool Init(bool Gravity = true);
	static void SetPaused(bool Paused);
};

struct ZL_Model
{
	struct Node : public ZL_Vector
	{
		ZL_Vector old;
		scalar Mass;
		bool IsFixed;
		Node() {}
		Node(const ZL_Vector &pos, scalar Mass, bool IsFixed = false);
		Node(scalar x, scalar y, scalar Mass, bool IsFixed = false);
		void DebugDraw(struct ZL_Model_Impl* pModel);
	};

	struct Constraint
	{
		Node *n1, *n2;
		scalar RestLength, Length, Strength;
		Constraint() {}
		Constraint(Node *n1, Node *n2, scalar Strength = s(0.5));
		void Satisfy();
		void DebugDraw(struct ZL_Model_Impl* pModel);
	};

	struct Joint
	{
		Constraint *c12,*c23;
		Node *n1, *n2, *n3;
		scalar MinAngle, MaxAngle, Strength;
		bool ConstrainAngleReverse;
		Joint(Constraint *c12, Constraint *c23, scalar MinAngle, scalar MaxAngle, bool ConstrainAngleReverse = true, scalar Strength = s(0.03));
		void Satisfy(bool ReversedCalc = false, scalar Suspension = 0.0);
		void DebugDraw(struct ZL_Model_Impl* pModel);
	};

	struct Muscle;
	struct Animation
	{
		struct Motion
		{
			Motion(Muscle *muscle, scalar Angle, scalar TimeFrom, scalar TimeTo, scalar Strength = 0, scalar Suspension = 0);
			Muscle *pMuscle;
			scalar Angle, Strength, Suspension, TimeFrom, TimeTo, Left;
		};
		std::vector<Motion*> Motions;
		int Loop;
		scalar Timer;
		Animation(int Loop = 0);
		void Run();
		void Stop();
		void Pause();
		void Continue();
		bool IsRunning() { return Running; }
		bool Calculate(scalar Elapsed);

	private:
		void DeactivateMotion(Motion *motion);
		int LoopLeft;
		bool Running;
		//pure pointer class
		Animation(const Animation& /*source*/) { }
		Animation& operator=(const Animation& /*source*/) { return *this; }
	};

	struct Muscle
	{
		Constraint *c;
		Joint *j;
		scalar MuscleAngle, Strength, Suspension;
		Animation::Motion *pActiveMotion;
		unsigned int motionLastActive;
		Muscle(Constraint *c, scalar MuscleAngle = 999, scalar Strength = s(0.01), scalar Suspension = s(0.01));
		Muscle(Joint *j, scalar MuscleAngle = 999, scalar Strength = s(0.01), scalar Suspension = s(0.01));
		void Satisfy();
		void DebugDraw(struct ZL_Model_Impl* pModel);
	};

	struct Attachment
	{
		Constraint *pConstraint;
		Node *pNode;
		ZL_Surface Surface;
		ZL_Vector offset;
		ZL_String id;
		ZL_Color color;
		scalar rotation, scalex, scaley;
		Attachment(Constraint *pConstraint, Node *pNode, const ZL_Surface& Surface, const ZL_Vector& offset = ZL_Vector(0, 0), scalar rotation = 0, scalar scalex = 1, scalar scaley = 1);
		void Draw(struct ZL_Model_Impl* pModel);
		void SetSurface(ZL_Surface srf) { Surface = srf.SetDrawOrigin(ZL_Origin::CenterLeft).SetRotateOrigin(ZL_Origin::CenterLeft); }
	};

	ZL_Model();
	ZL_Model(const ZL_File& file, const ZL_String& ModelName);
	ZL_Model(const ZL_File& file, const ZL_String& ModelName, const ZL_Model& LinkModel);
	ZL_Model(ZL_Xml& xmlModel);
	ZL_Model(ZL_Xml& xmlModel, const ZL_Model& LinkModel);
	~ZL_Model();
	ZL_Model(const ZL_Model &source);
	ZL_Model &operator=(const ZL_Model &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Model &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Model &b) const { return (impl!=b.impl); }

	void AnimationRun(const ZL_String& strAnimation);
	void AnimationStop(const ZL_String& strAnimation);
	bool AnimationIsRunning(const ZL_String& strAnimation);
	Animation* GetAnimation(const ZL_String& strAnimation);

	void Reload();
	void Draw();

	Constraint* GetConstraint(const ZL_String& strId);
	Node* GetNode(const ZL_String& strId);
	Node* FindNode(const ZL_Vector& pos);
	Constraint* FindConstraint(const ZL_Vector& pos);
	Node* FixedNode();

	ZL_Vector GetAbsPosOf(Node* pNode);
	ZL_Vector GetAbsPosOf(Constraint* pConstraint, scalar posN1toN2 = 0.5);
	scalar GetAngleOf(Constraint* pConstraint);

 	void ApplyForce(const ZL_String& NodeId, scalar angle, scalar force);
 	void ApplyForce(const ZL_String& NodeId, const ZL_Vector& force);
 	void ApplyForce(Node* pNode, scalar angle, scalar force);
 	void ApplyForce(Node* pNode, const ZL_Vector& force);

	void SetAttachmentColor(const ZL_String& id, const ZL_Color& col);
	void SetAttachmentSurface(const ZL_String& id, const ZL_Surface& srf);
	Attachment* GetAttachment(const ZL_String& id);
	void LookAt(scalar Angle);
	void Kill(bool KillMuscles, bool UnfixNodes = false, bool UnLinkModel = false);
	void CollideBox(ZL_Rectf box);
	void CollideFloor(scalar y = 0);

	void SetFlip(bool flip = true);
	void SetPos(const ZL_Vector& pos);
	void SetScaleFactor(scalar scalefactor);

	bool GetFlip();
	ZL_Vector GetPos();
	scalar GetScaleFactor();

	//void ChopOff(ZL_Model::Node *p);
	//void ChopOff(ZL_Model::Constraint *c);

	static ZL_Xml Prepare(const ZL_File& file, const ZL_String& ModelName);

	void DebugDraw();

	private: struct ZL_Model_Impl* impl;
};

#endif //__ZL_MODEL__
