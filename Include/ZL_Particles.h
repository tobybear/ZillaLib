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

#ifndef __ZL_PARTICLE_ENGINE__
#define __ZL_PARTICLE_ENGINE__

#include <vector>
#include "ZL_Surface.h"
#include "ZL_Application.h"

struct ZL_ParticleEffect
{
	ZL_ParticleEffect();
	ZL_ParticleEffect(scalar lifetimeDuration, scalar lifetimeDurationSpread = 200, scalar spawnProbability = 1);
	~ZL_ParticleEffect();
	ZL_ParticleEffect(const ZL_ParticleEffect &source);
	ZL_ParticleEffect &operator=(const ZL_ParticleEffect &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_ParticleEffect &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_ParticleEffect &b) const { return (impl!=b.impl); }

	void Spawn(int numOfParticles, scalar x, scalar y, int lifetimeStartOffsetTicks = 0, scalar xSpread = 0, scalar ySpread = 0);
	void Spawn(int numOfParticles, const ZL_Vector& pos, int lifetimeStartOffsetTicks = 0, scalar xSpread = 0, scalar ySpread = 0);
	void Draw();

	void SetLifetimeDuration(scalar lifetimeDuration);
	void SetLifetimeDurationSpread(scalar lifetimeDurationSpread);
	void SetSpawnProbability(scalar spawnProbability);
	int CountParticleImage();
	int CountParticles();
	void AddParticleImage(const ZL_Surface &surface, unsigned int maxNumOfParticles, scalar weight = 1);
	void AddBehavior(struct ZL_ParticleBehavior *particleBehavior, bool deleteBehavior = true);
	inline void AddBehavior(struct ZL_ParticleBehavior& particleBehavior) { AddBehavior(&particleBehavior, false); }
	struct ZL_ParticleBehavior* GetBehavior(size_t index);

	private: struct ZL_ParticleEffect_Impl* impl;
};

struct ZL_ParticleBehavior
{
protected:
	friend struct ZL_ParticleEffect_Impl;
	virtual void Calculate(struct ZL_ParticleEffect_CalcData& data) = 0;
	virtual bool ProvidesSpawnSeed() { return false; }
	virtual unsigned int GetSpawnSeed(struct ZL_ParticleEffect_CalcData& /*data*/) { return 0; }

	bool UsesSeed, SpawnSeed, ChangesRotation, ChangesAspectRatio, DeleteAfterUse;
	ZL_ParticleBehavior(bool UsesSeed, bool ChangesRotation, bool ChangesAspectRatio)
							 : UsesSeed(UsesSeed), ChangesRotation(ChangesRotation), ChangesAspectRatio(ChangesAspectRatio) { }
	virtual ~ZL_ParticleBehavior() { }
};

struct ZL_ParticleBehavior_LinearMove : ZL_ParticleBehavior
{
	scalar behaviorAngle, behaviorAngleSpread, behaviorSpeed, behaviorSpeedSpread;
	scalar spawningAngle, spawningAngleSpread, spawningSpeed, spawningSpeedSpread;

	ZL_ParticleBehavior_LinearMove(scalar Speed = 10, scalar SpeedSpread = 3, scalar AngleRad = 0, scalar AngleRadSpread = 2*PI, bool rotateImageToMoveAngle = false);
	void SetSpawnAngle(scalar AngleRad = 0, scalar AngleRadSpread = 2*PI);
	void SetSpawnSpeed(scalar Speed = 10, scalar SpeedSpread = 3);

	protected: void Calculate(struct ZL_ParticleEffect_CalcData& data);
	protected: bool ProvidesSpawnSeed();
	protected: unsigned int GetSpawnSeed(struct ZL_ParticleEffect_CalcData& data);
	private: bool rotateImageToMoveAngle;
};

struct ZL_ParticleBehavior_LinearImageProperties : ZL_ParticleBehavior
{
	ZL_ParticleBehavior_LinearImageProperties(scalar alphaStart = 1, scalar alphaEnd = 0, scalar scaleStart = 1, scalar scaleEnd = 0);
	scalar behaviorAlphaStart, behaviorAlphaEnd, behaviorScaleStart, behaviorScaleEnd;
	protected: void Calculate(struct ZL_ParticleEffect_CalcData& data);
};

struct ZL_ParticleBehavior_LinearColor : public ZL_ParticleBehavior
{
	struct WeightedColor : ZL_Color
	{
		scalar weightIndex; //first entry has 0 and last entry has less than 1
		inline WeightedColor(const ZL_Color &color, scalar weightIndex) : ZL_Color(color), weightIndex(weightIndex) { }
	};
	std::vector<WeightedColor>	colorStarts, colorEnds;

	ZL_ParticleBehavior_LinearColor();

	ZL_ParticleBehavior_LinearColor* AddColorStart(const ZL_Color &color, scalar weight = 1);
	ZL_ParticleBehavior_LinearColor* AddColorEnd(const ZL_Color &color, scalar weight = 1);

	private: scalar totalColorStartWeight, totalColorEndWeight;
	protected: void Calculate(struct ZL_ParticleEffect_CalcData& data);
	protected: bool ProvidesSpawnSeed();
	protected: unsigned int GetSpawnSeed(struct ZL_ParticleEffect_CalcData& data);
};

struct ZL_ParticleBehavior_Gravity : ZL_ParticleBehavior
{
	ZL_ParticleBehavior_Gravity(scalar Angle = -PI/2, scalar Strength = 100);
	scalar behaviorAngle, behaviorStrength, behaviorStrengthSpread;
	protected: void Calculate(struct ZL_ParticleEffect_CalcData& data);
};

#endif //__ZL_PARTICLE_ENGINE__
