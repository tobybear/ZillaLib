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

#include "ZL_Model.h"
#include "../../Source/ZL_Impl.h"
#include <assert.h>

#define gravity s(0.0091)
#define TimeFric s(0.99441370080879759137351322324801)

static struct ZL_ModelManager_Data
{
	std::vector<ZL_Model::Node*> Nodes;
	std::vector<ZL_Model::Constraint*> Constraints;
	std::vector<ZL_Model::Joint*> Joints;
	std::vector<ZL_Model::Muscle*> Muscles;
	std::vector<ZL_Model::Animation*> Animations;
	std::vector<ZL_Model::Attachment*> Attachments;

	scalar ElapsedSum;
	bool Gravity, Paused;

	void Update();
	void Animate();
	void Verlet();
	void Constrain();
} *ZLMM_Data = NULL;

template <typename t> static inline void EraseMatch(std::vector<t>& vec, const t& value) { vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end()); }

struct ZL_Model_Impl : ZL_Impl
{
	std::map<ZL_String, ZL_Model::Animation*> mapAnimations;
	std::map<ZL_String, ZL_Model::Node*> mapNodes;
	std::map<ZL_String, ZL_Model::Constraint*> mapConstraints;
	std::vector<ZL_Model::Joint*> Joints;
	std::vector<ZL_Model::Muscle*> Muscles;
	std::vector<ZL_Model::Attachment*> Attachments;

	std::map<ZL_Model::Muscle*, scalar> LookMuscleAngleOffsets;

	ZL_Model_Impl *pLinkModel;
	ZL_FileLink FileLink;

	ZL_String ModelName;

	ZL_Vector pos;
	scalar scalefactor;
	bool flip;

	ZL_Model::Node* FixedNode;

	ZL_Model_Impl(const ZL_File& file, const ZL_String& ModelName, ZL_Model_Impl *pLinkModel = NULL);
	ZL_Model_Impl(ZL_Xml& xmlModel, ZL_Model_Impl *pLinkModel = NULL);
	~ZL_Model_Impl();
	void Load(const ZL_File& file);
	void Load(ZL_Xml& xmlModel);
	void Reload();
	void Clear();

	static ZL_Xml Prepare(const ZL_File& file, const ZL_String& ModelName);
};

ZL_Model::Node::Node(const ZL_Vector &pos, scalar Mass, bool IsFixed) :
	ZL_Vector(pos), old(pos), Mass(Mass), IsFixed(IsFixed)
{ }

ZL_Model::Node::Node(scalar x, scalar y, scalar Mass, bool IsFixed) :
	ZL_Vector(x,y), old(x,y), Mass(Mass), IsFixed(IsFixed)
{ }

void ZL_Model::Node::DebugDraw(ZL_Model_Impl* pModel)
{
	scalar x = pModel->pos.x + this->x * pModel->scalefactor;
	scalar y = pModel->pos.y + this->y * pModel->scalefactor;
	if (Mass > 1.0)      ZL_Display::DrawRect(x-2, y-2, x+2, y+2, ZL_Color::Transparent, ZL_Color(1,0,0,1));
	else if (Mass > 0.9) ZL_Display::DrawRect(x-2, y-2, x+2, y+2, ZL_Color::Transparent, ZL_Color(0,1,0,1));
	else if (Mass > 0.8) ZL_Display::DrawRect(x-2, y-2, x+2, y+2, ZL_Color::Transparent, ZL_Color(0,0,1,1));
	else if (Mass > 0.7) ZL_Display::DrawRect(x-2, y-2, x+2, y+2, ZL_Color::Transparent, ZL_Color(1,1,0,1));
	else if (Mass > 0.1) ZL_Display::DrawRect(x-2, y-2, x+2, y+2, ZL_Color::Transparent, ZL_Color(1,0,1,1));
}

ZL_Model::Constraint::Constraint(Node *n1, Node *n2, scalar Strength) :
	n1(n1), n2(n2), Strength(Strength)
{
	Length = RestLength = n1->GetDistance(*n2);
}

void ZL_Model::Constraint::Satisfy()
{
	scalar XDiff = n1->x - n2->x;
	scalar YDiff = n1->y - n2->y;
	Length = ssqrt((XDiff) * (XDiff) + ((YDiff) * (YDiff)));
	scalar SumMass = n1->Mass + n2->Mass;
	scalar P1MassPart = n1->Mass / (SumMass);
	scalar P2MassPart = n2->Mass / (SumMass);
	scalar Factor = (Strength); // * s(8 * 0.125)); //s(8 * TimeFactor)); //0.5
	if (Factor > 1) Factor = 1;
	scalar rl3 = (Length - RestLength) / Length * Factor;
	scalar P1LenMass = rl3 * (n1->IsFixed ? 1 : P1MassPart);
	scalar P2LenMass = rl3 * (n2->IsFixed ? 1 : P2MassPart);
	if (!n1->IsFixed) n1->x -= XDiff * P2LenMass;
	if (!n1->IsFixed) n1->y -= YDiff * P2LenMass;
	if (!n2->IsFixed) n2->x += XDiff * P1LenMass;
	if (!n2->IsFixed) n2->y += YDiff * P1LenMass;
	Length -= (n2->IsFixed ? 0 : Length*P1LenMass)+(n1->IsFixed ? 0 : Length*P2LenMass);
}

void ZL_Model::Constraint::DebugDraw(ZL_Model_Impl* pModel)
{
	ZL_Display::DrawLine(pModel->pos + *n1 * pModel->scalefactor, pModel->pos + *n2 * pModel->scalefactor, ZL_Color(1,1,1,0.2f));
}

ZL_Model::Joint::Joint(Constraint *c12, Constraint *c23, scalar MinAngle, scalar MaxAngle, bool ConstrainAngleReverse, scalar Strength) :
	c12(c12), c23(c23), MinAngle(MinAngle), MaxAngle(MaxAngle), Strength(Strength), ConstrainAngleReverse(ConstrainAngleReverse)
{
	if      (c12->n2 == c23->n1) { n1 = c12->n1; n2 = c12->n2; n3 = c23->n2; }
	else if (c12->n2 == c23->n2) { n1 = c12->n1; n2 = c12->n2; n3 = c23->n1; }
	else if (c12->n1 == c23->n1) { n1 = c12->n2; n2 = c12->n1; n3 = c23->n2; }
	else if (c12->n1 == c23->n2) { n1 = c12->n2; n2 = c12->n1; n3 = c23->n1; }
	//else { assert(0); }
	if (MinAngle > MaxAngle) { this->MinAngle = MaxAngle; this->MaxAngle = MinAngle; }
}
void ZL_Model::Joint::Satisfy(bool ReversedCalc, scalar Suspension)
{
	if (Strength <= 0) return;

	Node *n1 = (ReversedCalc ? this->n1 : n3), *n3 = (ReversedCalc ? this->n3 : this->n1);
	scalar Length = (ReversedCalc ? c12->Length : c23->Length);
	scalar Ang12 = satan2(n2->y - n1->y,n2->x - n1->x);
	scalar Ang23 = satan2(n3->y - n2->y,n3->x - n2->x);

	scalar AngTemp1 = Ang12 - Ang23;
	while (AngTemp1 >  PI) AngTemp1 -= PI*2;
	while (AngTemp1 < -PI) AngTemp1 += PI*2;

	scalar AvgAngleNeg = (ReversedCalc ? ( MaxAngle - MinAngle) : (MaxAngle - MinAngle)) / 2;
	scalar AvgAnglePos = (ReversedCalc ? (-MinAngle - MaxAngle) : (MaxAngle + MinAngle)) / 2;

	scalar AngTemp2 = AvgAnglePos - AngTemp1;
	while (AngTemp2 >  PI) AngTemp2 -= PI*2;
	while (AngTemp2 < -PI) AngTemp2 += PI*2;

	scalar AngTemp;
	if      (AngTemp2 >   AvgAngleNeg) AngTemp = AngTemp2 - AvgAngleNeg;
	else if (AngTemp2 < - AvgAngleNeg) AngTemp = AngTemp2 + AvgAngleNeg;
	else                               AngTemp = 0;

	scalar Factor = (Strength * 10); //s(80 * 0.125)); //s(80 * TimeFactor)); //0.03
	if (Factor > 1) Factor = 1;
	scalar AngTemp3 = (AngTemp * Factor ) + Ang12;

	scalar SumMass = n1->Mass + n2->Mass;
	scalar P1MassPart = n1->Mass / (SumMass);
	scalar P2MassPart = n2->Mass / (SumMass);

	if (n2->IsFixed) { P2MassPart = 1.0; P1MassPart = 0; }
	if (n1->IsFixed) { P1MassPart = 1.0; P2MassPart = 0; }

	scalar TmpX = n1->x + ((n2->x - n1->x) * (P2MassPart));
	scalar TmpY = n1->y + ((n2->y - n1->y) * (P2MassPart));

	if (!n1->IsFixed) n1->x = TmpX + (scos(AngTemp3 + PI) * Length * P2MassPart );
	if (!n1->IsFixed) n1->y = TmpY + (ssin(AngTemp3 + PI) * Length * P2MassPart );
	if (!n2->IsFixed) n2->x = TmpX + (scos(AngTemp3)      * Length * P1MassPart );
	if (!n2->IsFixed) n2->y = TmpY + (ssin(AngTemp3)      * Length * P1MassPart );
	if (Suspension > 0)
	{
		Factor = (Suspension * 10); //s(80 * 0.125)); //s(80 * TimeFactor));  //0.01
		if (Factor > 1) Factor = 1;

		if (!n1->IsFixed) n1->old.x += ((n1->x - n1->old.x) * Factor);
		if (!n1->IsFixed) n1->old.y += ((n1->y - n1->old.y) * Factor);
		if (!n2->IsFixed) n2->old.x += ((n2->x - n2->old.x) * Factor);
		if (!n2->IsFixed) n2->old.y += ((n2->y - n2->old.y) * Factor);
	}
}

void ZL_Model::Joint::DebugDraw(ZL_Model_Impl* pModel)
{
	ZL_Display::DrawLine(*c12->n1*pModel->scalefactor + pModel->pos , *c12->n2*pModel->scalefactor + pModel->pos, ZL_Color(1,0,0,s(0.7)));
	ZL_Display::DrawLine(*c23->n1*pModel->scalefactor + pModel->pos , *c23->n1*pModel->scalefactor + pModel->pos, ZL_Color(1,0,0,s(0.7)));

	//double dAng12 = satan2(n2->y - n1->y,n2->x - n1->x);
	//double dAng23 = satan2(n3->y - n2->y,n3->x - n2->x) + 2*PI;
	//char cText[64];
	//double dAngTemp = (dAng12-dAng23);
	//while (dAngTemp > PI)  { dAngTemp -= PI*2; }
	//while (dAngTemp < -PI) { dAngTemp += PI*2; }
	//sprintf(cText, "%4.1f", dAngTemp/PIOVER180);
	//static CL_Font *fnt = new CL_Font("Arial",10);
	//fnt->draw(p3->x+5, p3->y+5, cText);
}

ZL_Model::Muscle::Muscle(Constraint *c, scalar MuscleAngle, scalar Strength, scalar Suspension) :
 c(c), j(NULL), Strength(Strength), Suspension(Suspension), pActiveMotion(NULL)
{
	if (MuscleAngle == 999) this->MuscleAngle =  satan2(c->n2->y - c->n1->y , c->n2->x - c->n1->x);
	else this->MuscleAngle = MuscleAngle;
}

ZL_Model::Muscle::Muscle(Joint *j, scalar MuscleAngle, scalar Strength, scalar Suspension) :
 c(NULL), j(j), Strength(Strength), Suspension(Suspension), pActiveMotion(NULL)
{
	if (MuscleAngle == 999) this->MuscleAngle = satan2(j->n3->y - j->n2->y , j->n3->x - j->n2->x) - satan2(j->n2->y - j->n1->y , j->n2->x - j->n1->x);
	else this->MuscleAngle = MuscleAngle;
}

void ZL_Model::Muscle::Satisfy()
{
	Constraint *c12, *c23, cother;
	Node nother;
	if (c)
	{
		nother = Node(ZL_Vector(c->n1->old.x - 100, c->n1->old.y), c->n1->Mass);
		cother = Constraint(&nother, c->n1);
		c12 = &cother;
		c23 = c;
	}
	else
	{
		c12 = this->j->c12;
		c23 = this->j->c23;
	}
	Joint j(c12, c23,
		(!pActiveMotion ? MuscleAngle : pActiveMotion->Angle) - s(0.03),
		(!pActiveMotion ? MuscleAngle : pActiveMotion->Angle) + s(0.03),
		false, (!pActiveMotion ? Strength : pActiveMotion->Strength));
	while (j.MinAngle >  PI) { j.MinAngle -= PI*2; j.MaxAngle -= PI*2; }
	while (j.MinAngle < -PI) { j.MinAngle += PI*2; j.MaxAngle += PI*2; }
	//if (j.MaxAngle < j.MinAngle) { /* thats not good */ }
	j.Satisfy(false, (!pActiveMotion ? Suspension : pActiveMotion->Suspension) );
	if (!c && this->j->ConstrainAngleReverse) j.Satisfy(true, (!pActiveMotion ? Suspension : pActiveMotion->Suspension) );
}

void ZL_Model::Muscle::DebugDraw(ZL_Model_Impl* pModel)
{
	if (c)
	{
		ZL_Vector p = *c->n1*pModel->scalefactor + pModel->pos;
		ZL_Display::DrawLine(p.x, p.y, p.x+scos(MuscleAngle)*50, p.y-ssin(MuscleAngle)*80, ZL_Color(0,1,1,0.7f));
	}
	else if (j)
	{
		scalar ang12 = satan2(j->n2->y - j->n3->y,j->n2->x - j->n3->x);
		ZL_Vector p = *j->n2*pModel->scalefactor + pModel->pos;
		ZL_Display::DrawLine(p.x, p.y, p.x+scos(MuscleAngle-ang12)*80, p.y-ssin(MuscleAngle-ang12)*80, ZL_Color(0,1,1,0.7f));
	}
}

ZL_Model::Animation::Motion::Motion(ZL_Model::Muscle *muscle, scalar Angle, scalar TimeFrom, scalar TimeTo, scalar Strength, scalar Suspension)
 : pMuscle(muscle), Angle(Angle), Strength(Strength), Suspension(Suspension), TimeFrom(TimeFrom), TimeTo(TimeTo), Left(0)
{ }

void ZL_Model::Animation::DeactivateMotion(Motion *motion)
{
	if (motion->pMuscle->pActiveMotion == motion) motion->pMuscle->pActiveMotion = NULL;
}

ZL_Model::Animation::Animation(int Loop) :
 Loop(Loop), Timer(0), LoopLeft(Loop), Running(false)
{ }

void ZL_Model::Animation::Continue()
{
	if (Running) return;
	Running = true;
	ZLMM_Data->Animations.insert(ZLMM_Data->Animations.begin(), this);
}

void ZL_Model::Animation::Run()
{
	if (Motions.empty()) return;
	this->LoopLeft = Loop;
	Timer = 0;
	Continue();
}

void ZL_Model::Animation::Pause()
{
	if (!Running) return;
	EraseMatch(ZLMM_Data->Animations, this);
	Running = false;
}

void ZL_Model::Animation::Stop()
{
	for(std::vector<Motion*>::iterator itMotion = Motions.begin(); itMotion != Motions.end(); ++itMotion)
		DeactivateMotion(*itMotion);
	Pause();
}

bool ZL_Model::Animation::Calculate(scalar Elapsed)
{
	scalar TimerOld = Timer;
	scalar TimerMax = 0;
	Timer += Elapsed;
	for(std::vector<Motion*>::iterator itMotion = Motions.begin(); itMotion != Motions.end(); ++itMotion)
	{
		Motion *m = (*itMotion);
		if (m->TimeTo > TimerMax) TimerMax = m->TimeTo;

		if (m->TimeFrom >= Timer || m->TimeTo < TimerOld) continue;
		else if ((m->TimeTo >= TimerOld) && (m->TimeTo < Timer))
		{
			DeactivateMotion(m);
		}
		else
		{
			//if ((m->TimeFrom >= TimerOld) && (m->TimeFrom < Timer))
			//{
				if (m->pMuscle->pActiveMotion == NULL || (m->pMuscle->pActiveMotion != m && m->pMuscle->motionLastActive < ZLTICKS))
					m->pMuscle->pActiveMotion = m;
			//}
			if (m->pMuscle->pActiveMotion == m)
			{
				m->pMuscle->motionLastActive = ZLTICKS;
			}
		}
	}
	if (Timer > TimerMax)
	{
		if (LoopLeft > 0) LoopLeft--;
		if (LoopLeft == 0) { Running = false; return true; }
		Timer -= TimerMax;
		TimerMax = Timer;
		Timer = 0;
		return Calculate(TimerMax);
	}
	return false;
}

ZL_Model::Attachment::Attachment(Constraint *pConstraint, Node *pNode, const ZL_Surface& Surface, const ZL_Vector& offset, scalar rotation, scalar scalex, scalar scaley)
 : pConstraint(pConstraint), pNode(pNode), Surface(Surface), offset(offset), color(ZL_Color::White), rotation(rotation), scalex(scalex), scaley(scaley)
{
	this->Surface.SetDrawOrigin(ZL_Origin::CenterLeft);
	this->Surface.SetRotateOrigin(ZL_Origin::CenterLeft);
}

void ZL_Model::Attachment::Draw(ZL_Model_Impl* pModel)
{
	scalar ang;
	if (pConstraint->n1 == pNode)
		ang = satan2(pConstraint->n2->y - pNode->y, pConstraint->n2->x - pNode->x);
	else if (pConstraint->n2 == pNode)
		ang = satan2(pNode->y - pConstraint->n1->y, pNode->x - pConstraint->n1->x);
	else
		ang = satan2(pConstraint->n2->y - pConstraint->n1->y, pConstraint->n2->x - pConstraint->n1->x);

	scalar dx = pNode->x;
	scalar dy = pNode->y;
	scalar scalew = scalex; //pConstraint->Length / pConstraint->RestLength;

	if (!pModel->flip)
	{
		ang = ang+rotation;
		dx -= scos(ang)*offset.x;
		dy -= ssin(ang)*offset.x;
	}
	else
	{
		ang = PI-((ang+rotation)-PI) ;
		dx *= -1;
		scalew *= -1;
		dx += scos(ang)*offset.x;
		dy += ssin(ang)*offset.x;
	}
	dx += ssin(ang)*offset.y;
	dy -= scos(ang)*offset.y;

	//Surface.SetAlpha(0.6f);
	Surface.Draw(pModel->pos.x + dx*pModel->scalefactor, pModel->pos.y + dy*pModel->scalefactor, ang, scalew*pModel->scalefactor, scaley*pModel->scalefactor, color);
	//ZL_Display::DrawLine(dx,dy-10,dx,dy+10,ZL_Color::Red);
	//ZL_Display::DrawLine(dx-10,dy,dx+10,dy,ZL_Color::Red);
}

void ZL_ModelManager::SetPaused(bool Paused)
{
	ZLMM_Data->Paused = Paused;
}

static void _ZL_ModelManager_KeepAlive()
{
	ZLMM_Data->Update();
}

bool ZL_ModelManager::Init(bool Gravity)
{
	if (ZLMM_Data) return false;
	ZLMM_Data = new ZL_ModelManager_Data();
	ZLMM_Data->Gravity = Gravity;
	ZLMM_Data->Paused = false;
	ZLMM_Data->ElapsedSum = 0;

	ZL_Application::sigKeepAlive.connect(&_ZL_ModelManager_KeepAlive);
	return true;
}

/*
void ZL_ModelManager::DebugDraw()
{
	for(std::vector<ZL_Model::Node*>::iterator       itNode = data->Nodes.begin(); itNode != data->Nodes.end(); ++itNode) (*itNode)->DebugDraw(impl);
	for(std::vector<ZL_Model::Constraint*>::iterator itConstraint = data->Constraints.begin(); itConstraint != data->Constraints.end(); ++itConstraint) (*itConstraint)->DebugDraw(impl);
	for(std::vector<ZL_Model::Joint*>::iterator      itJoint = data->Joints.begin(); itJoint != data->Joints.end(); ++itJoint) (*itJoint)->DebugDraw(impl);
	//for(std::vector<ZL_Model::Muscle*>::iterator     itMuscle = data->Muscles.begin(); itMuscle != data->Muscles.end(); ++itMuscle) (*itMuscle)->DebugDraw();
}
*/

void ZL_ModelManager_Data::Update()
{
	if (Paused) return;
	ElapsedSum += ZL_Application::Elapsed;
	Animate();

	while (ElapsedSum > s(0.04))
	{
		ElapsedSum -= s(0.04);
		Verlet();
		Constrain();
	}
}

void ZL_ModelManager_Data::Verlet()
{
	for(std::vector<ZL_Model::Node*>::iterator itNode = Nodes.begin(); itNode != Nodes.end(); ++itNode)
	{
		ZL_Model::Node *n = (*itNode);
		if (n->IsFixed) continue;
		ZL_Vector old = *n;
		n->x += (n->x - n->old.x) * TimeFric;
		n->y += (n->y - n->old.y) * TimeFric - (Gravity ? (gravity * 64) : 0);
		n->old = old;
	}
}

void ZL_ModelManager_Data::Constrain()
{
	for(std::vector<ZL_Model::Constraint*>::iterator itConstraint = Constraints.begin(); itConstraint != Constraints.end(); ++itConstraint)
		(*itConstraint)->Satisfy();

	for(std::vector<ZL_Model::Muscle*>::iterator itMuscle = Muscles.begin(); itMuscle != Muscles.end(); ++itMuscle)
		(*itMuscle)->Satisfy();

	for(std::vector<ZL_Model::Joint*>::iterator itJoint = Joints.begin(); itJoint != Joints.end(); ++itJoint)
	{
		(*itJoint)->Satisfy(false);
		if ((*itJoint)->ConstrainAngleReverse)
			(*itJoint)->Satisfy(true);
	}
}

void ZL_ModelManager_Data::Animate()
{
	for(std::vector<ZL_Model::Animation*>::iterator itAnimation = Animations.begin(); itAnimation != Animations.end();)
		if ((*itAnimation)->Calculate(ZL_Application::Elapsed)) itAnimation = Animations.erase(itAnimation); else ++itAnimation;
}

ZL_Model_Impl::ZL_Model_Impl(const ZL_File& file, const ZL_String& ModelName, ZL_Model_Impl *pLinkModel)
	: pLinkModel(pLinkModel), FileLink(file), ModelName(ModelName), pos(0, 0), scalefactor(1), flip(false)
{
	Load(file);
}

ZL_Model_Impl::ZL_Model_Impl(ZL_Xml& xmlModel, ZL_Model_Impl *pLinkModel)
	: pLinkModel(pLinkModel), pos(0, 0), scalefactor(1), flip(false)
{
	Load(xmlModel);
}

ZL_Model_Impl::~ZL_Model_Impl()
{
	Clear();
}

ZL_Xml ZL_Model_Impl::Prepare(const ZL_File& file, const ZL_String& ModelName)
{
	ZL_Xml xml(file), xmlModel;
	std::vector<ZL_Xml> xmlModels = xml.GetChildrenByName("model");
	for (std::vector<ZL_Xml>::iterator it = xmlModels.begin(); it != xmlModels.end(); ++it)
		if ((*it)["name"] == ModelName) { xmlModel = *it; break; }
	return xmlModel;
}

void ZL_Model_Impl::Load(ZL_Xml& xmlModel)
{
	if (!xmlModel) return;
	assert(ZLMM_Data || (ZL_Application::Log("PHYSICS", "*** ERROR: ZL_ModelManager::Init has not been run!"), 0));

	std::map<ZL_String, ZL_Model::Joint*> mapJoints;
	std::map<ZL_String, ZL_Model::Muscle*> mapMuscles;
	std::vector<ZL_Xml>::iterator it;

	std::vector<ZL_Xml> xmlNodes = xmlModel.GetChildrenByName("node");
	for (it = xmlNodes.begin(); it != xmlNodes.end(); ++it)
	{
		ZL_Model::Node* n = new ZL_Model::Node((*it)["x"], (*it)["y"], (*it)["mass"], (*it)["fixed"]);
		mapNodes[(*it)["id"]] = n;
		ZLMM_Data->Nodes.push_back(n);
		if (n->IsFixed) FixedNode = n;
	}

	std::vector<ZL_Xml> xmlConstraints = xmlModel.GetChildrenByName("constraint");
	for (it = xmlConstraints.begin(); it != xmlConstraints.end(); ++it)
	{
		ZL_Model::Node* n1 = mapNodes[(*it)["n1"]];
		ZL_Model::Node* n2 = mapNodes[(*it)["n2"]];
		ZL_Model::Constraint* c = new ZL_Model::Constraint(n1, n2, (it->HasParameter("strength") ? s((*it)["strength"]) : s(0.5)));
		mapConstraints[(*it)["id"]] = c;
		ZLMM_Data->Constraints.push_back(c);
	}

	std::vector<ZL_Xml> xmlJoints = xmlModel.GetChildrenByName("joint");
	for (it = xmlJoints.begin(); it != xmlJoints.end(); ++it)
	{
		ZL_Model::Joint* j = new ZL_Model::Joint(
			mapConstraints[(*it)["c1"]],
			mapConstraints[(*it)["c2"]],
			(it->HasParameter("minang") ? s((*it)["minang"]) : s(-170))*PIOVER180,
			(it->HasParameter("maxang") ? s((*it)["maxang"]) : s( 170))*PIOVER180,
			!it->HasParameter("DoNotConstrainReverse"),
			(it->HasParameter("strength") ? s((*it)["strength"]) : s(0.03))
		);
		mapJoints[(*it)["id"]] = j;
		ZLMM_Data->Joints.push_back(j);
		Joints.push_back(j);
	}

	std::vector<ZL_Xml> xmlMuscles = xmlModel.GetChildrenByName("muscle");
	for (it = xmlMuscles.begin(); it != xmlMuscles.end(); ++it)
	{
		//if (it->HasParameter("constraint")) printf("Adding Constraint-Muscle: %s\n", elmSub.get_attribute("constraint").c_str());
		//if (it->HasParameter("joint")) printf("Adding AngledConstraint-Muscle: %s\n", elmSub.get_attribute("angledconstraint").c_str());
		ZL_Model::Muscle* m = (it->HasParameter("constraint") ?
			new ZL_Model::Muscle(
				mapConstraints[(*it)["constraint"]],
				(it->HasParameter("angle") ? s((*it)["angle"])*PIOVER180 : 999),
				(it->HasParameter("strength") ? s((*it)["strength"]) : s(0.01)),
				(it->HasParameter("suspension") ? s((*it)["suspension"]) : s(0.01))
			) :
			new ZL_Model::Muscle(
				mapJoints[(*it)["joint"]],
				(it->HasParameter("angle") ? s((*it)["angle"])*PIOVER180 : 999),
				(it->HasParameter("strength") ? s((*it)["strength"]) : s(0.01)),
				(it->HasParameter("suspension") ? s((*it)["suspension"]) : s(0))
			)
		);
		mapMuscles[(*it)["id"]] = m;
		ZLMM_Data->Muscles.push_back(m);
		Muscles.push_back(m);

		if (it->HasParameter("lookangleoffset"))
			LookMuscleAngleOffsets[m] = s((*it)["lookangleoffset"]);
	}

	std::vector<ZL_Xml> xmlAnimations = xmlModel.GetChildrenByName("animation");
	for (it = xmlAnimations.begin(); it != xmlAnimations.end(); ++it)
	{
		ZL_Model::Animation* a = new ZL_Model::Animation((*it)["loop"]);
		std::vector<ZL_Xml> xmlMotions = (*it).GetChildrenByName("motion");
		for(std::vector<ZL_Xml>::iterator itMotion = xmlMotions.begin(); itMotion != xmlMotions.end(); ++itMotion)
		{
			//printf("Adding Motion to Animation on Muscle: %s\n", (*itMotion)["muscle"].c_str());
			a->Motions.push_back(new ZL_Model::Animation::Motion(mapMuscles[(*itMotion)["muscle"]],
				(itMotion->HasParameter("angle") ? s((*itMotion)["angle"])*PIOVER180 : 0),
				(itMotion->HasParameter("from") ? s((*itMotion)["from"]) : 0),
				(itMotion->HasParameter("to") ? s((*itMotion)["to"]) :
					(itMotion->HasParameter("from") ? s((*itMotion)["from"]) : 0) + (itMotion->HasParameter("duration") ? s((*itMotion)["duration"]) : 0)),
				(itMotion->HasParameter("strength") ? s((*itMotion)["strength"]) : s(0)),
				(itMotion->HasParameter("suspension") ? s((*itMotion)["suspension"]) : s(0))
			));
		}
		mapAnimations[(*it)["id"]] = a;
	}

	std::vector<ZL_Xml> xmlAttachments = xmlModel.GetChildrenByName("attachment");
	for (it = xmlAttachments.begin(); it != xmlAttachments.end(); ++it)
	{
		//printf("Adding Attachment to Constraint: %s\n", (*it)["constraint"].c_str());
		ZL_Model::Attachment *a = new ZL_Model::Attachment(
			(mapConstraints.count((*it)["constraint"]) ? mapConstraints[(*it)["constraint"]] : pLinkModel->mapConstraints[(*it)["constraint"]]),
			(mapNodes.count((*it)["node"]) ? mapNodes[(*it)["node"]] : pLinkModel->mapNodes[(*it)["node"]]),
			((*it)["texture"].length() ? ZL_Surface((*it)["texture"]) : ZL_Surface()),
			ZL_Vector((it->HasParameter("x") ? s((*it)["x"]) : 0), (it->HasParameter("y") ? s((*it)["y"]) : 0)),
			(it->HasParameter("rot") ? s((*it)["rot"])*PIOVER180 : 0),
			(it->HasParameter("scalex") ? s((*it)["scalex"]) : 1),
			(it->HasParameter("scaley") ? s((*it)["scaley"]) : 1)
		);
		if (it->HasParameter("id")) a->id = (*it)["id"];
		ZLMM_Data->Attachments.push_back(a);
		Attachments.push_back(a);
	}

	std::vector<ZL_Xml> xmlLinks = xmlModel.GetChildrenByName("link");
	for (it = xmlLinks.begin(); it != xmlLinks.end(); ++it)
	{
		ZL_Model::Node *nLink = mapNodes[(*it)["node"]], *nForeign = pLinkModel->mapNodes[(*it)["foreign"]];
		*((ZL_Vector*)nLink) = *nForeign;
		nLink->old = nForeign->old;
		ZL_Model::Constraint *c = new ZL_Model::Constraint(nLink, nForeign,
			(it->HasParameter("strength") ? s((*it)["strength"]) : s(1.0)));
		c->RestLength = 0;
		ZLMM_Data->Constraints.push_back(c);
		mapConstraints[ZL_String("link_")<<mapConstraints.size()] = c;
	}
}

void ZL_Model_Impl::Load(const ZL_File& file)
{
	ZL_Xml xml = ZL_Model_Impl::Prepare(file, ModelName);
	Load(xml);
}

void ZL_Model_Impl::Reload()
{
	Clear();
	Load(FileLink.Open());
}

void ZL_Model_Impl::Clear()
{
	for(std::vector<ZL_Model::Attachment*>::iterator itAttachment = Attachments.begin(); itAttachment != Attachments.end(); ++itAttachment) { EraseMatch(ZLMM_Data->Attachments, *itAttachment); delete *itAttachment; }
	for(std::map<ZL_String, ZL_Model::Animation*>::iterator itAnimation = mapAnimations.begin(); itAnimation != mapAnimations.end(); ++itAnimation) { itAnimation->second->Pause(); delete itAnimation->second; }
	for(std::vector<ZL_Model::Muscle*>::iterator itMuscle = Muscles.begin(); itMuscle != Muscles.end(); ++itMuscle) { EraseMatch(ZLMM_Data->Muscles, *itMuscle); delete *itMuscle; }
	for(std::vector<ZL_Model::Joint*>::iterator itJoint = Joints.begin(); itJoint != Joints.end(); ++itJoint) { EraseMatch(ZLMM_Data->Joints, *itJoint); delete *itJoint; }
	for(std::map<ZL_String, ZL_Model::Constraint*>::iterator itConstraint = mapConstraints.begin(); itConstraint != mapConstraints.end(); ++itConstraint) { EraseMatch(ZLMM_Data->Constraints, itConstraint->second); delete itConstraint->second; }
	for(std::map<ZL_String, ZL_Model::Node*>::iterator itNode = mapNodes.begin(); itNode != mapNodes.end(); ++itNode) { EraseMatch(ZLMM_Data->Nodes, itNode->second); delete itNode->second; }
	Attachments.clear();
	mapAnimations.clear();
	Muscles.clear();
	Joints.clear();
	mapConstraints.clear();
	mapNodes.clear();
	LookMuscleAngleOffsets.clear();
}

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Model)

ZL_Model::ZL_Model(const ZL_File& file, const ZL_String& ModelName) : impl(new ZL_Model_Impl(file, ModelName)) { }
ZL_Model::ZL_Model(const ZL_File& file, const ZL_String& ModelName, const ZL_Model& LinkModel) : impl(new ZL_Model_Impl(file, ModelName, LinkModel.impl)) { }
ZL_Model::ZL_Model(ZL_Xml& xmlModel) : impl(new ZL_Model_Impl(xmlModel)) { }
ZL_Model::ZL_Model(ZL_Xml& xmlModel, const ZL_Model& LinkModel) : impl(new ZL_Model_Impl(xmlModel, LinkModel.impl)) { }

void ZL_Model::AnimationRun(const ZL_String& strAnimation)
{
	std::map<ZL_String, Animation*>::iterator it = impl->mapAnimations.find(strAnimation);
	if (it != impl->mapAnimations.end()) it->second->Run();
}

void ZL_Model::AnimationStop(const ZL_String& strAnimation)
{
	std::map<ZL_String, Animation*>::iterator it = impl->mapAnimations.find(strAnimation);
	if (it != impl->mapAnimations.end()) it->second->Stop();
}

bool ZL_Model::AnimationIsRunning(const ZL_String& strAnimation)
{
	std::map<ZL_String, Animation*>::iterator it = impl->mapAnimations.find(strAnimation);
	return (it != impl->mapAnimations.end() ? it->second->IsRunning() : false);
}

ZL_Model::Animation* ZL_Model::GetAnimation(const ZL_String& strAnimation)
{
	std::map<ZL_String, Animation*>::iterator it = impl->mapAnimations.find(strAnimation);
	return (it != impl->mapAnimations.end() ? it->second : NULL);
}

ZL_Model::Constraint* ZL_Model::GetConstraint(const ZL_String& strId)
{
	std::map<ZL_String, Constraint*>::iterator it = impl->mapConstraints.find(strId);
	return (it != impl->mapConstraints.end() ? it->second : NULL);
}

ZL_Model::Node* ZL_Model::GetNode(const ZL_String& strId)
{
	std::map<ZL_String, Node*>::iterator it = impl->mapNodes.find(strId);
	return (it != impl->mapNodes.end() ? it->second : NULL);
}

ZL_Model::Node* ZL_Model::FixedNode()
{
	return (impl ? impl->FixedNode : NULL);
}

void ZL_Model::LookAt(scalar Angle)
{
	if (!impl) return;
	for(std::map<Muscle*, scalar>::iterator lmao = impl->LookMuscleAngleOffsets.begin(); lmao != impl->LookMuscleAngleOffsets.end(); ++lmao)
	{
		lmao->first->MuscleAngle = 0 - Angle + lmao->second;
	}
}

void ZL_Model::Reload()
{
	if (impl) impl->Reload();
}

void ZL_Model::SetFlip(bool flip) { if (impl) impl->flip = flip; }
bool ZL_Model::GetFlip() { return (impl ? impl->flip : false); }
void ZL_Model::SetPos(const ZL_Vector& pos) { if (impl) impl->pos = pos; }
ZL_Vector ZL_Model::GetPos() { return (impl ? impl->pos : ZL_Vector()); }
scalar ZL_Model::GetScaleFactor() { return (impl ? impl->scalefactor : s(0)); }
void ZL_Model::SetScaleFactor(scalar scalefactor) { if (impl) impl->scalefactor = scalefactor; }

void ZL_Model::Draw()
{
	if (!impl) return;
	ZL_Model_Impl* refer = (impl->pLinkModel ? impl->pLinkModel : impl);
	for(std::vector<Attachment*>::iterator it = impl->Attachments.begin(); it != impl->Attachments.end(); ++it)
		(*it)->Draw(refer);
}

void ZL_Model::SetAttachmentColor(const ZL_String& id, const ZL_Color& col)
{
	if (!impl) return;
	for(std::vector<Attachment*>::iterator it = impl->Attachments.begin(); it != impl->Attachments.end(); ++it)
		if ((*it)->id == id) (*it)->color = col;
}

void ZL_Model::SetAttachmentSurface(const ZL_String& id, const ZL_Surface& srf)
{
	if (!impl) return;
	for(std::vector<Attachment*>::iterator it = impl->Attachments.begin(); it != impl->Attachments.end(); ++it)
		if ((*it)->id == id) (*it)->Surface = srf;
}

ZL_Model::Attachment* ZL_Model::GetAttachment(const ZL_String& id)
{
	if (!impl) return NULL;
	for(std::vector<Attachment*>::iterator it = impl->Attachments.begin(); it != impl->Attachments.end(); ++it)
		if ((*it)->id == id) return (*it);
	return NULL;
}

ZL_Xml ZL_Model::Prepare(const ZL_File& file, const ZL_String& ModelName)
{
	return ZL_Model_Impl::Prepare(file, ModelName);
}

void ZL_Model::DebugDraw()
{
	//ZL_Display::PushMatrix();
	//ZL_Display::Translate(30, 30);
	//ZL_Display::Scale(2,2);
	for(std::map<ZL_String, Node*>::iterator itNode = impl->mapNodes.begin(); itNode != impl->mapNodes.end(); ++itNode) itNode->second->DebugDraw(impl);
	for(std::map<ZL_String, Constraint*>::iterator itConstraint = impl->mapConstraints.begin(); itConstraint != impl->mapConstraints.end(); ++itConstraint) itConstraint->second->DebugDraw(impl);
	for(std::vector<Joint*>::iterator itJoint = impl->Joints.begin(); itJoint != impl->Joints.end(); ++itJoint) (*itJoint)->DebugDraw(impl);
	//for(std::vector<Muscle*>::iterator itMuscle = data->Muscles.begin(); itMuscle != data->Muscles.end(); ++itMuscle) (*itMuscle)->DebugDraw(impl);
	//ZL_Display::PopMatrix();
}

ZL_Model::Node* ZL_Model::FindNode(const ZL_Vector& pos)
{
	ZL_Vector p = ((pos - impl->pos) /= impl->scalefactor);
	if (impl->flip) p.x *= -1;
	scalar NearestLen = 10000000; Node *pNearestNode = NULL;
	for(std::map<ZL_String, Node*>::iterator itNode = impl->mapNodes.begin(); itNode != impl->mapNodes.end(); ++itNode)
	{
		scalar len = p.GetDistance(*itNode->second);
		if (len < NearestLen)
		{
			NearestLen = len;
			pNearestNode = itNode->second;
		}
	}
	return pNearestNode;
}

ZL_Model::Constraint* ZL_Model::FindConstraint(const ZL_Vector& pos)
{
	ZL_Vector p = ((pos - impl->pos) /= (impl->scalefactor / 2));
	if (impl->flip) p.x *= -1;
	scalar NearestLen = 10000000; Constraint *pNearestConstraint = NULL;
	for(std::map<ZL_String, Constraint*>::iterator itConstraint = impl->mapConstraints.begin(); itConstraint != impl->mapConstraints.end(); ++itConstraint)
	{
		scalar len = p.GetDistance(*itConstraint->second->n1+*itConstraint->second->n2-p);
		if (len < NearestLen)
		{
			NearestLen = len;
			pNearestConstraint = itConstraint->second;
		}
	}
	return pNearestConstraint;
}

void ZL_Model::Kill(bool KillMuscles, bool UnfixNodes, bool UnLinkModel)
{
	if (KillMuscles)
	{
		for(std::map<ZL_String, Animation*>::iterator itAni = impl->mapAnimations.begin(); itAni != impl->mapAnimations.end(); ++itAni) { itAni->second->Pause(); delete itAni->second; }
		for(std::vector<Muscle*>::iterator itMus = impl->Muscles.begin(); itMus != impl->Muscles.end(); ++itMus) { EraseMatch(ZLMM_Data->Muscles, *itMus); delete *itMus; }
		impl->mapAnimations.clear();
		impl->Muscles.clear();
		impl->LookMuscleAngleOffsets.clear();
	}
	if (UnfixNodes)
	{
		for(std::map<ZL_String, Node*>::iterator itNode = impl->mapNodes.begin(); itNode != impl->mapNodes.end(); ++itNode) itNode->second->IsFixed = false;
	}
	if (UnLinkModel)
	{
		for(std::map<ZL_String, Constraint*>::iterator itConstraint = impl->mapConstraints.begin(); itConstraint != impl->mapConstraints.end();)
			if (itConstraint->first.find("link_") == 0) { EraseMatch(ZLMM_Data->Constraints, itConstraint->second); delete itConstraint->second; impl->mapConstraints.erase(itConstraint++); }
			else ++itConstraint;
	}
}

void ZL_Model::CollideFloor(scalar y)
{
	for(std::map<ZL_String, Node*>::iterator itNode = impl->mapNodes.begin(); itNode != impl->mapNodes.end(); ++itNode)
		if (itNode->second->y < y)
		{
			//Node* n = itNode->second;
			itNode->second->y = y;
			itNode->second->old.y = y+y-itNode->second->old.y;
			itNode->second->old.x += (itNode->second->x - itNode->second->old.x)*0.1f;
		}
}

void ZL_Model::CollideBox(ZL_Rectf box)
{
	for(std::map<ZL_String, Node*>::iterator itNode = impl->mapNodes.begin(); itNode != impl->mapNodes.end(); ++itNode)
	{
		Node& n = *(itNode->second);
		if (n.x < box.left) n.x = box.left;
		if (n.x > box.right) n.x = box.right;
		if (n.y < box.low) n.y = box.low;
		if (n.y > box.high) n.y = box.high;
	}

}

ZL_Vector ZL_Model::GetAbsPosOf(Node* pNode)
{
	ZL_Model_Impl* refer = (impl->pLinkModel ? impl->pLinkModel : impl);
	ZL_Vector p(pNode->x*refer->scalefactor, pNode->y*refer->scalefactor);
	if (refer->flip) p.x *= -1;
	return p += refer->pos;
}

ZL_Vector ZL_Model::GetAbsPosOf(Constraint* c, scalar f)
{
	ZL_Model_Impl* refer = (impl->pLinkModel ? impl->pLinkModel : impl);
	ZL_Vector p(
		(f == 0.5 ? c->n1->x+c->n2->x : (f == 0 ? c->n1->x*2 : (f == 1 ? c->n2->x*2 : c->n1->x*2*(s(1)-f) + c->n2->x*2*f))) * (refer->scalefactor/2),
		(f == 0.5 ? c->n1->y+c->n2->y : (f == 0 ? c->n1->y*2 : (f == 1 ? c->n2->y*2 : c->n1->y*2*(s(1)-f) + c->n2->y*2*f))) * (refer->scalefactor/2));
	if (refer->flip) p.x *= -1;
	return p += refer->pos;
}

scalar ZL_Model::GetAngleOf(Constraint* pConstraint)
{
	if ((impl->pLinkModel ? impl->pLinkModel : impl)->flip) return satan2(pConstraint->n2->y - pConstraint->n1->y, pConstraint->n1->x - pConstraint->n2->x);
	return satan2(pConstraint->n2->y - pConstraint->n1->y, pConstraint->n2->x - pConstraint->n1->x);
}

void ZL_Model::ApplyForce(const ZL_String& NodeId, scalar angle, scalar force)
{
	std::map<ZL_String, Node*>::iterator it = impl->mapNodes.find(NodeId);
	if (it == impl->mapNodes.end()) return;
	it->second->old.x -= ((impl->pLinkModel ? impl->pLinkModel : impl)->flip ? scos(angle) * force * -1 : scos(angle) * force);
	it->second->old.y -= ssin(angle) * force;
}

void ZL_Model::ApplyForce(const ZL_String& NodeId, const ZL_Vector& force)
{
	std::map<ZL_String, Node*>::iterator it = impl->mapNodes.find(NodeId);
	if (it == impl->mapNodes.end()) return;
	it->second->old.x -= ((impl->pLinkModel ? impl->pLinkModel : impl)->flip ? force.x * -1 : force.x);
	it->second->old.y -= force.y;
}

void ZL_Model::ApplyForce(Node* pNode, scalar angle, scalar force)
{
	pNode->old.x -= ((impl->pLinkModel ? impl->pLinkModel : impl)->flip ? scos(angle) * force * -1 : scos(angle) * force);
	pNode->old.y -= ssin(angle) * force;
}

void ZL_Model::ApplyForce(Node* pNode, const ZL_Vector& force)
{
	pNode->old.x -= ((impl->pLinkModel ? impl->pLinkModel : impl)->flip ? force.x * -1 : force.x);
	pNode->old.y -= force.y;
}

/* -- random old code for reference

//old definitions
//#define TimeFric = spow(fric, VerletTime);
//#define fric s(0.9993)
//scalar VerletTime = s(8); //TimeFactor * MAX_CALC_FRAME_TIME;
//#define MAX_CALC_FRAME_TIME 40

void ZL_Model::Reload()
{
	//printf("pEngine Number of Particles BEFORE reload: %d\n", pEngine->vecParticles.size());
	Remove();
	Load(ModelName);
	//printf("pEngine Number of Particles AFTER  reload: %d\n", pEngine->vecParticles.size());
}

void ZL_Model::ChopOff(Particle *p)
{
	//printf("\nChop Off Start: Particles: %d - Contraints: %d - AngledConstraints: %d\n", vecParticles.size(), vecConstraints.size(), vecAngledConstraints.size());
	if (!p) return;
	Particle *pnew;
	bool bNeedNewP = false;
	for(std::vector<Muscle*>::iterator itMuscle = vecMuscles.begin(); itMuscle != vecMuscles.end(); itMuscle++)
	{
		if ((*itMuscle)->a)
		{
			Particle *p1,*p2,*p3;
			(*itMuscle)->a->GetParticles(&p1,&p2,&p3);
			if (p == p2) { DeleteMuscle(itMuscle); printf("Removed one Muscle\n"); if (itMuscle == vecMuscles.end()) break; }
		}
	}
	for(std::vector<AngledConstraint*>::iterator itAngledConstraint = vecAngledConstraints.begin(); itAngledConstraint != vecAngledConstraints.end(); itAngledConstraint++)
	{
		Particle *p1,*p2,*p3;
		(*itAngledConstraint)->GetParticles(&p1,&p2,&p3);
		if (p == p2) { DeleteAngledConstraint(itAngledConstraint); printf("Removed one AngledContraint\n"); if (itAngledConstraint == vecAngledConstraints.end()) break; }
	}
	for(std::vector<Constraint*>::iterator itConstraint = vecConstraints.begin(); itConstraint != vecConstraints.end(); itConstraint++)
	{
		Constraint *pConstraint = *itConstraint;
		if (bNeedNewP)
		{
			if (pJoint->n1 == p)
			{
				pnew = new Particle();
				memcpy(pnew, p, sizeof(Particle));
				pEngine->vecParticles.push_back(pnew);
				vecParticles.push_back(pnew);
				pJoint->n1 = vecParticles.back();
				printf("Created new Particle (=Chop) (for P1)\n");
			}
			if (pJoint->n2 == p)
			{
				pnew = new Particle();
				memcpy(pnew, p, sizeof(Particle));
				pEngine->vecParticles.push_back(pnew);
				vecParticles.push_back(pnew);
				pJoint->n2 = vecParticles.back();
				printf("Created new Particle (=Chop) (for P2)\n");
			}
		}
		else
		{
			if (pJoint->n1 == p) { bNeedNewP = true; printf("Found First attached Constraint (Next will be chopped) (P1)\n"); }
			if (pJoint->n2 == p) { bNeedNewP = true; printf("Found First attached Constraint (Next will be chopped) (P2)\n"); }
		}
	}
	printf("\n");
}

void ZL_Model::ChopOff(Constraint *c)
{
	if (!c) return;
	Particle *p1,*p2,*pnew;
	bool bNeedNewP1 = false, bNeedNewP2 = false;
	p1 = c->p1;
	p2 = c->p2;
	for(std::vector<Muscle*>::iterator itMuscle = vecMuscles.begin(); itMuscle != vecMuscles.end(); itMuscle++)
	{
		Muscle *pMuscle = (*itMuscle);
		if      (pMuscle->c == c)                        { DeleteMuscle(itMuscle); if (itMuscle == vecMuscles.end()) break; }
		else if ((pMuscle->a) && (pMuscle->a->c12 == c)) { DeleteMuscle(itMuscle); if (itMuscle == vecMuscles.end()) break; }
		else if ((pMuscle->a) && (pMuscle->a->c23 == c)) { DeleteMuscle(itMuscle); if (itMuscle == vecMuscles.end()) break; }
	}
	for(std::vector<AngledConstraint*>::iterator itAngledConstraint = vecAngledConstraints.begin(); itAngledConstraint != vecAngledConstraints.end(); itAngledConstraint++)
	{
		AngledConstraint *pAngledConstraint = (*itAngledConstraint);
		if      (pAngledConstraint->c12 == c) { DeleteAngledConstraint(itAngledConstraint); if (itAngledConstraint == vecAngledConstraints.end()) break; }
		else if (pAngledConstraint->c23 == c) { DeleteAngledConstraint(itAngledConstraint); if (itAngledConstraint == vecAngledConstraints.end()) break; }
	}
	for(std::vector<Constraint*>::iterator itConstraint = vecConstraints.begin(); itConstraint != vecConstraints.end(); itConstraint++)
	{
		Constraint *pConstraint = (*itConstraint);
		if (pConstraint != c)
		{
			if      (pJoint->n1 == p1) bNeedNewP1 = true;
			else if (pJoint->n2 == p1) bNeedNewP1 = true;
			else if (pJoint->n1 == p2) bNeedNewP2 = true;
			else if (pJoint->n2 == p2) bNeedNewP2 = true;
		}
	}
	if (bNeedNewP1)
	{
		pnew = new Particle();
		memcpy(pnew, c->p1, sizeof(Particle));
		pEngine->vecParticles.push_back(pnew);
		vecParticles.push_back(pnew);
		c->p1 = vecParticles.back();
	}
	if (bNeedNewP2)
	{
		pnew = new Particle();
		memcpy(pnew, c->p2, sizeof(Particle));
		pEngine->vecParticles.push_back(pnew);
		vecParticles.push_back(pnew);
		c->p2 = vecParticles.back();
	}
}

void ZL_Model::DeleteAnimation(std::map<ZL_String, Animation*>::iterator &itAnimation)
{
	pEngine->DeleteAnimation(itAnimation->second);
	mapAnimations.erase(itAnimation);
}

void ZL_Model::DeleteParticle(std::vector<Particle*>::iterator &itParticle)
{
	pEngine->DeleteParticle(*itParticle);
	for(std::map<ZL_String, Particle*>::iterator itMapParticle = mapParticles.begin(); itMapParticle != mapParticles.end(); itMapParticle++)
		if (*itParticle == itMapParticle->second) { mapParticles.erase(itMapParticle); break; }
	vecParticles.erase(itParticle);
}

void ZL_Model::DeleteConstraint(std::vector<Constraint*>::iterator &itConstraint)
{
	pEngine->DeleteConstraint(*itConstraint);
	vecConstraints.erase(itConstraint);
}

void ZL_Model::DeleteAngledConstraint(std::vector<AngledConstraint*>::iterator &itAngledConstraint)
{
	pEngine->DeleteAngledConstraint(*itAngledConstraint);
	vecAngledConstraints.erase(itAngledConstraint);
}

void ZL_Model::DeleteMuscle(std::vector<Muscle*>::iterator &itMuscle)
{
	pEngine->DeleteMuscle(*itMuscle);
	vecMuscles.erase(itMuscle);
}

void ZL_Model::DeleteAttachment(std::vector<Attachment*>::iterator &itAttachment)
{
	pEngine->DeleteAttachment(*itAttachment);
	vecAttachments.erase(itAttachment);
}

ZL_ModelManager::Particle * ZL_ModelManager::Joint::p1()
{
	if (c12->p2 == c23->p1) { return c12->p1; }
	if (c12->p2 == c23->p2) { return c12->p1; }
	if (c12->p1 == c23->p1) { return c12->p2; }
	if (c12->p1 == c23->p2) { return c12->p2; }
	return NULL;
}

ZL_ModelManager::Particle * ZL_ModelManager::Joint::p2()
{
	if (c12->p2 == c23->p1) { return c12->p2; }
	if (c12->p2 == c23->p2) { return c12->p2; }
	if (c12->p1 == c23->p1) { return c12->p1; }
	if (c12->p1 == c23->p2) { return c12->p1; }
	return NULL;
}

ZL_ModelManager::Particle * ZL_ModelManager::Joint::p3()
{
	if (c12->p2 == c23->p1) { return c23->p2; }
	if (c12->p2 == c23->p2) { return c23->p1; }
	if (c12->p1 == c23->p1) { return c23->p2; }
	if (c12->p1 == c23->p2) { return c23->p1; }
	return NULL;
}

void ZL_ModelManager::Joint::GetParticles( cParticle **p1,cParticle **p2,cParticle **p3 )
{
	if (c12->p2 == c23->p1) { *p1 = c12->p1; *p2 = c12->p2; *p3 = c23->p2; }
	if (c12->p2 == c23->p2) { *p1 = c12->p1; *p2 = c12->p2; *p3 = c23->p1; }
	if (c12->p1 == c23->p1) { *p1 = c12->p2; *p2 = c12->p1; *p3 = c23->p2; }
	if (c12->p1 == c23->p2) { *p1 = c12->p2; *p2 = c12->p1; *p3 = c23->p1; }
}

static int getTimer()
{
	return CL_System::get_time();
}

void ZL_ModelManager::Hold(int x, int y)
{
	for(std::vector<cParticle*>::iterator itOnHoldParticle = vecOnHold.begin(); itOnHoldParticle != vecOnHold.end(); itOnHoldParticle++)
	{
		cParticle *pOnHoldParticle = (*itOnHoldParticle);
		int bx = x - pOnHoldParticle->pModel->dBaseX;
		int by = y - pOnHoldParticle->pModel->dBaseY;
    pOnHoldParticle->x += ((bx - pOnHoldParticle->x) * 0.3);
    pOnHoldParticle->y += ((by - pOnHoldParticle->y) * 0.3);
    //pOnHoldParticle->oldx += ((x - pOnHoldParticle->oldx) * 0.3);
    //pOnHoldParticle->oldy += ((y - pOnHoldParticle->oldy) * 0.3);
	}
}

ZL_ModelManager::ZL_ModelManager()
{
	iOldTime = 0; //getTimer();
		iAnimationLastCalcTime = 0;
		dTimeFactor = 0;
		bGravity = false;
}

ZL_ModelManager::AngledConstraint* ZL_ModelManager::addAngledConstraint( cConstraint *c12, cConstraint *c23, double dMinAngDeg, double dMaxAngDeg, bool bInversed, bool bConstrainAngleReverse, double dStrength)
{
	cParticle *pPartHinge = NULL;
		if      (c12->p1 == c23->p1) pPartHinge = c12->p1;
		else if (c12->p1 == c23->p2) pPartHinge = c12->p1;
		else if (c12->p2 == c23->p1) pPartHinge = c12->p2;
		else if (c12->p2 == c23->p2) pPartHinge = c12->p2;
		if (!pPartHinge) return NULL;
		cAngledConstraint *a = new cAngledConstraint();
		a->c12 = c12;
		a->c23 = c23;
		a->dMaxAng = dMaxAngDeg*PIOVER180;
		a->dMinAng = dMinAngDeg*PIOVER180;
		if (a->dMinAng > a->dMaxAng) { a->dMinAng = a->dMaxAng; a->dMaxAng = dMinAngDeg*PIOVER180; }
		a->dStrength = dStrength;
		a->bInversed = bInversed;
		a->bConstrainAngleReverse = bConstrainAngleReverse;
		vecAngledConstraints.push_back(a);
		return a;
}

ZL_ModelManager::Particle* ZL_ModelManager::addParticle( double x, double y, double dMass )
{
	cParticle *p = new cParticle();
		p->x = p->oldx = x;
		p->y = p->oldy = y;
		p->dMass = dMass;
		vecParticles.push_back(p);
		return p;
}

ZL_ModelManager::Constraint* ZL_ModelManager::addConstraint( cParticle *p1, cParticle *p2, double dStrength)
{
	cConstraint *c = new cConstraint();
		c->p1 = p1;
		c->p2 = p2;
		c->dStrength = dStrength;
		c->CalcRestLength();
		vecConstraints.push_back(c);
		return c;
}

ZL_ModelManager::Muscle* ZL_ModelManager::addMuscle( cConstraint *c, double dMuscleAngle, double dStrength, double dSuspension)
{
	cMuscle *m = new cMuscle();
		m->a = NULL;
		m->c = c;
		if (dMuscleAngle == 999) m->dMuscleAngle =  satan2(c->p1->y - c->p2->y , c->p2->x - c->p1->x);
		else m->dMuscleAngle = dMuscleAngle*PIOVER180;
		m->dStrength = dStrength;
		m->dSuspension = dSuspension;
		vecMuscles.push_back(m);
		return m;
}

ZL_ModelManager::Muscle* ZL_ModelManager::addMuscle( cAngledConstraint *a, double dMuscleAngle, double dStrength, double dSuspension)
{
	cMuscle *m = new cMuscle();
	m->a = a;
	m->c = NULL;
	if (dMuscleAngle == 999)
	{
		cParticle *p1, *p2, *p3;
		a->GetParticles(&p1,&p2,&p3);
		double dAngleBase = satan2( p2->y - p1->y , p2->x - p1->x );
		double dAngle = satan2( p3->y - p2->y , p3->x - p2->x );
		m->dMuscleAngle = dAngleBase - dAngle ;
	}
	else m->dMuscleAngle = dMuscleAngle*PIOVER180;

	m->dStrength = dStrength;
	m->dSuspension = dSuspension;
	vecMuscles.push_back(m);
	return m;
}

ZL_ModelManager::Animation* ZL_ModelManager::addAnimation(int iLoop)
{
	cAnimation *n = new cAnimation();
	n->iLoop = iLoop;
	vecAnimations.push_back(n);
	return n;
}

ZL_ModelManager::Attachment* ZL_ModelManager::addAttachment( cConstraint *pConstraint, cParticle *pParticle, CL_Surface *pSurface, double dOffsetX, double dOffsetY, double dRotation)
{
	cAttachment *a = new cAttachment();
	a->pConstraint = pConstraint;
	a->pParticle = pParticle;
	a->pSurface = pSurface;
	a->dOffsetX = dOffsetX;
	a->dOffsetY = dOffsetY;
	a->dRotation = dRotation*PIOVER180;
	vecAttachments.push_back(a);
	return a;
}

void ZL_ModelManager::DeleteAnimation( cAnimation* pAnimation )
{
	delete pAnimation;
	for(std::vector<cAnimation*>::iterator itAnimation = vecAnimations.begin(); itAnimation != vecAnimations.end(); itAnimation++)
		if (*itAnimation == pAnimation) {
			vecAnimations.erase(itAnimation); break;
		}
}

void ZL_ModelManager::DeleteAttachment( cAttachment* pAttachment )
{
	delete pAttachment;
	for(std::vector<cAttachment*>::iterator itAttachment = vecAttachments.begin(); itAttachment != vecAttachments.end(); itAttachment++)
		if (*itAttachment == pAttachment) { vecAttachments.erase(itAttachment); break; }
}

void ZL_ModelManager::DeleteConstraint( cConstraint* pConstraint )
{
	delete pConstraint;
		for(std::vector<cConstraint*>::iterator itConstraint = vecConstraints.begin(); itConstraint != vecConstraints.end(); itConstraint++)
			if (*itConstraint == pConstraint) { vecConstraints.erase(itConstraint); break; }
}

void ZL_ModelManager::DeleteAngledConstraint( cAngledConstraint* pAngledConstraint )
{
	delete pAngledConstraint;
		for(std::vector<cAngledConstraint*>::iterator itAngledConstraint = vecAngledConstraints.begin(); itAngledConstraint != vecAngledConstraints.end(); itAngledConstraint++)
			if (*itAngledConstraint == pAngledConstraint) { vecAngledConstraints.erase(itAngledConstraint); break; }
}

void ZL_ModelManager::DeleteMuscle( cMuscle* pMuscle )
{
	delete pMuscle;
		for(std::vector<cMuscle*>::iterator itMuscle = vecMuscles.begin(); itMuscle != vecMuscles.end(); itMuscle++)
			if (*itMuscle == pMuscle) { vecMuscles.erase(itMuscle); break; }
}

void ZL_ModelManager::DeleteParticle( cParticle* pParticle )
{
	delete pParticle;
		for(std::vector<cParticle*>::iterator itParticle = vecParticles.begin(); itParticle != vecParticles.end(); itParticle++)
			if (*itParticle == pParticle) { vecParticles.erase(itParticle); break; }
}

void ZL_ModelManager::Forces()
{
	for(std::vector<cParticle*>::iterator itParticle = vecParticles.begin(); itParticle != vecParticles.end(); itParticle++)
		{
		cParticle *pParticle = (*itParticle);
		}
}

void ZL_ModelManager::Shake()
{
	for(std::vector<cParticle*>::iterator itParticle = vecParticles.begin(); itParticle != vecParticles.end(); itParticle++)
		{
		cParticle *pParticle = (*itParticle);
		if (!pParticle->bFixed)
		{
			pParticle->x += ((double(rand()) / double(RAND_MAX)) - (double(rand()) / double(RAND_MAX))) * 15;
			pParticle->y += ((double(rand()) / double(RAND_MAX)) - (double(rand()) / double(RAND_MAX))) * 8;
		}
		}
}

void ZL_ModelManager::SatisfyMuscleNotGood( cMuscle *pMuscle )
{
	cParticle *p1, *p2, *p3;
	cConstraint *c;
	double dAngle, dAngTemp;
	if (pMuscle->c)
	{
		p1 = pMuscle->c->p1;
		p2 = pMuscle->c->p2;
		c = pMuscle->c;
		dAngle = satan2( p2->y - p1->y , p2->x - p1->x );
		dAngTemp = -pMuscle->dMuscleAngle - dAngle;
	}
	else
	{
		pMuscle->a->GetParticles(&p3,&p1,&p2);
		c = pMuscle->a->c23;
		dAngle = satan2( p1->y - p2->y , p1->x - p2->x );
		double dAngleBase = atan2( p3->y - p1->y , p3->x - p1->x );
		dAngTemp = dAngleBase - pMuscle->dMuscleAngle - dAngle;
		dAngle += PI;
	}
	//double dSumMass = p1->dMass + p2->dMass;
	//double dP1MassPart = p1->dMass / (dSumMass);
	//double dP2MassPart = p2->dMass / (dSumMass);
	//double dP2MassLen = (dP2MassPart) * c->dRestLength;
	//double dP1MassLen = (dP1MassPart) * c->dRestLength;
	//double dTmpX = p1->x + ((p2->x - p1->x) * (dP2MassPart) );
	//double dTmpY = p1->y + ((p2->y - p1->y) * (dP2MassPart) );
	while (dAngTemp > PI)  { dAngTemp -= PI*2; }
	while (dAngTemp < -PI) { dAngTemp += PI*2; }
	double dFactor = ( pMuscle->dStrength * 4 * dTimeFactor ); //0.01
	if (dFactor > 1) dFactor = 1;
	double dPowerAngle = dAngle + ( dAngTemp * dFactor );
	////if (!p1->bFixed) p1->x = (dTmpX) + (scos((dPowerAngle) + PI) * (dP2MassLen));
	////if (!p1->bFixed) p1->y = (dTmpY) + (ssin((dPowerAngle) + PI) * (dP2MassLen));
	if (!p2->bFixed)
	{
		//p2->x = (dTmpX) + (scos(dPowerAngle) * dP1MassLen));
		//p2->y = (dTmpY) + (ssin(dPowerAngle) * dP1MassLen));
		double dXDiff = p1->x - p2->x;
		double dYDiff = p1->y - p2->y;
		double dLength = ssqrt((dXDiff) * (dXDiff) + ((dYDiff) * (dYDiff)));
		p2->x = p1->x + (scos(dPowerAngle) * dLength);
		p2->y = p1->y + (ssin(dPowerAngle) * dLength);
		//dFactor = ( pMuscle->dSuspension * 8 * dTimeFactor ); //0.01
		//if (dFactor > 1) dFactor = 1;
		//p2->oldx += ((p2->x - p2->oldx) * dFactor);
		//p2->oldy += ((p2->y - p2->oldy) * dFactor);
	}
	//if (!p1->bFixed)
	//{
	//	p1->x = (dTmpX) + (scos(dPowerAngle + PI) * (dP1MassLen));
	//	p1->y = (dTmpY) + (ssin(dPowerAngle + PI) * (dP1MassLen));
	//	dFactor = ( pMuscle->dSuspension * 8 * dTimeFactor ); //0.01
	//	if (dFactor > 1) dFactor = 1;
	//	p1->oldx += ((p1->x - p1->oldx) * dFactor);
	//	p1->oldy += ((p1->y - p1->oldy) * dFactor);
	//}
}

double ZL_ModelManager::ang( double dAngle )
{
	while (dAngle > PI)  { dAngle -= PI*2; }
	while (dAngle < -PI) { dAngle += PI*2; }
	return dAngle;
}

ZL_ModelManager::Particle * ZL_ModelManager::FindParticle( int x, int y )
{
	double dNearestLen = 10000000; cParticle *pNearestParticle = NULL;
	for(std::vector<cParticle*>::iterator itParticle = vecParticles.begin(); itParticle != vecParticles.end(); itParticle++)
	{
		cParticle *pParticle = (*itParticle);
		double dDiffX = x - pParticle->x;
		double dDiffY = y - pParticle->y;
		double dLen = ssqrt((dDiffX * dDiffX) + (dDiffY * dDiffY));
		if (dLen < dNearestLen)
		{
			dNearestLen = dLen;
			pNearestParticle = pParticle;
		}
	}
	return pNearestParticle;
}

void ZL_ModelManager::HoldParticles( int x, int y )
{
	double dHoldLen = 20;
	vecOnHold.clear();
	for(std::vector<cParticle*>::iterator itParticle = vecParticles.begin(); itParticle != vecParticles.end(); itParticle++)
	{
		cParticle *pParticle = (*itParticle);
		double dDiffX = x - pParticle->x;
		double dDiffY = y - pParticle->y;
		double dLen = ssqrt((dDiffX * dDiffX) + (dDiffY * dDiffY));
		if (dLen <= dHoldLen)
		{
			vecOnHold.push_back(pParticle);
		}
	}
}
*/
