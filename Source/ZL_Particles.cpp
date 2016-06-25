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

#include "ZL_Application.h"
#include "ZL_Math.h"
#include <assert.h>
#include "ZL_Texture_Impl.h"
#include "ZL_Particles.h"

//--------------------------------------------------------------------
//            Effect Main Class (Calculation and Drawing)
//--------------------------------------------------------------------

struct ZL_ParticleEffect_CalcData
{
	ZL_Vector* startPos;
	ZL_Vector pos;
	ZL_Color col;
	scalar rotation, scalew, scaleh, alpha;
	scalar percentOfLifeTime;
	unsigned int seed, seed_max_full, seed_max_half;
	unsigned char seedbits, seedbytes;
	inline unsigned int getSeedIntFirstHalf() { return seed & (seed_max_half-1); }
	inline unsigned int getSeedIntSecondHalf() { return seed >> (seedbits/2); }
	inline scalar getSeedPercentFirstHalf() { return s(getSeedIntFirstHalf()) / seed_max_half; }
	inline scalar getSeedPercentSecondHalf() { return s(getSeedIntSecondHalf()) / seed_max_half; }
};

struct ZL_ParticleEffect_Impl : ZL_Impl
{
	struct WeightedSurface : ZL_Surface
	{
		struct Particle
		{
			unsigned int lifetimeStart, lifetimeDuration;
			ZL_Vector startPos;
			unsigned char cseed[8];
		};
		scalar weightIndex; //first entry has 0 and last entry has less than 1
		unsigned int maxNumOfParticles, usedParticles;
		Particle *particles;
		WeightedSurface()
		 : weightIndex(1), maxNumOfParticles(0), usedParticles(0), particles(NULL) { }
		WeightedSurface(const ZL_Surface &surface, scalar weightIndex, unsigned int maxNumOfParticles)
		 : ZL_Surface(surface), weightIndex(weightIndex), maxNumOfParticles(maxNumOfParticles), usedParticles(0)
		{
			particles = (Particle*)malloc(sizeof(Particle)*maxNumOfParticles);
		}
	};

	scalar lifetimeDuration, lifetimeDurationSpread, spawnProbability, totalSurfaceWeight;
	WeightedSurface particleImages[4];
	ZL_ParticleBehavior* behaviors[10+1]; //+1 = NULL delimiter
	bool active, allowPointSprites;
	ZL_ParticleEffect_CalcData calcData;

	ZL_ParticleEffect_Impl(scalar lifetimeDuration, scalar lifetimeDurationSpread, scalar spawnProbability)
	 : lifetimeDuration(lifetimeDuration), lifetimeDurationSpread(lifetimeDurationSpread), spawnProbability(spawnProbability), totalSurfaceWeight(0), active(false), allowPointSprites(true)
	{ behaviors[0] = NULL; }


	~ZL_ParticleEffect_Impl()
	{
		for (WeightedSurface* ws = particleImages; ws->particles && ws != particleImages+(sizeof(particleImages)/sizeof(particleImages[0])); ws++)
			free(ws->particles);
		for (ZL_ParticleBehavior** b = behaviors; *b; b++)
			if ((*b)->DeleteAfterUse) delete *b;
	}

	void Spawn(int numOfParticles, scalar x, scalar y, int lifetimeStartOffsetTicks, scalar xSpread, scalar ySpread)
	{
		//make sure we have at least one image
		assert(particleImages[0].particles);

		for (ZL_ParticleBehavior** b = behaviors; *b; b++)
			if ((*b)->UsesSeed) { (*b)->SpawnSeed = (*b)->ProvidesSpawnSeed(); }

		while (numOfParticles--)
		{
			//spawn probability
			if (spawnProbability <= 0 || (spawnProbability < 1 && RAND_FACTOR > spawnProbability))
				continue;

			//select a random surface to be used by the particle
			WeightedSurface* ws = particleImages;
			if (particleImages[1].particles)
			{
				scalar weight = RAND_FACTOR;
				while (ws->weightIndex <= weight && ws != particleImages+(sizeof(particleImages)/sizeof(particleImages[0]))) ws++;
				ws--;
			}

			//select particle if available
			if (ws->usedParticles == ws->maxNumOfParticles) continue;
			WeightedSurface::Particle& newParticle = ws->particles[ws->usedParticles++];

			//base for new spawned particles
			newParticle.lifetimeStart = ZL_Application::Ticks + lifetimeStartOffsetTicks;
			newParticle.startPos.x = (xSpread ? x + RAND_VARIATION(xSpread*HALF) : x);
			newParticle.startPos.y = (xSpread ? y + RAND_VARIATION(ySpread*HALF) : y);

			//particle lifetime duration
			newParticle.lifetimeDuration = (int)(lifetimeDuration + RAND_VARIATION(lifetimeDurationSpread*HALF));

			//define seed data for the particle by checking the needs (and optional provision) of the behaviors
			unsigned char *pseed = newParticle.cseed - calcData.seedbytes;
			for (ZL_ParticleBehavior** b = behaviors; *b; b++)
			{
				if ((*b)->UsesSeed && (*b)->SpawnSeed) *(unsigned int*) (pseed += calcData.seedbytes) = (*b)->GetSpawnSeed(calcData);
				else if ((*b)->UsesSeed) { pseed += calcData.seedbytes; for (int i = 0; i < calcData.seedbytes; i++) pseed[i] = rand()%0x100; }
			}
		}
		active = true;
	}

	void Draw()
	{
		if (!active) return;
		#define VBSIZE 32
		#ifdef ZL_VIDEO_OPENGL_ES1
		GLscalar sizes[1*VBSIZE];
		#endif
		GLscalar vertices[2*6*VBSIZE], colors[4*6*VBSIZE];
		GLscalar texcoordbox[2*6*VBSIZE];

		ZLGL_ENABLE_TEXTURE();
		ZLGL_COLORARRAY_ENABLE();
		ZLGL_VERTEXTPOINTER(2, GL_SCALAR, 0, vertices);
		ZLGL_COLORARRAY_POINTER(4, GL_SCALAR, 0, colors);

		for (WeightedSurface* ws = particleImages; ws->particles && ws != particleImages+(sizeof(particleImages)/sizeof(particleImages[0])); ws++)
		{
			if (!ws->usedParticles) continue;

			ZL_Surface_Impl *s = ZL_ImplFromOwner<ZL_Surface_Impl>(*(ZL_Surface*)ws);
			ZL_Texture_Impl *t = s->tex;
			glBindTexture(GL_TEXTURE_2D, t->gltexid);

			#ifdef ZL_VIDEO_OPENGL_ES1
				bool bDoPointSprite = allowPointSprites && t->w == t->h;
				if (bDoPointSprite)
				{
					glEnable(GL_POINT_SPRITE_OES);
					glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
					glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
					glPointSizePointerOES(GL_SCALAR, 0, sizes);
				}
				else
			#endif
			{
				memcpy(&texcoordbox[0], s->TexCoordBox+0, sizeof(GLscalar)*6);
				memcpy(&texcoordbox[6], s->TexCoordBox+2, sizeof(GLscalar)*6);
				for (int v = 1; v < VBSIZE; v++) memcpy(&texcoordbox[12*v], texcoordbox, sizeof(GLscalar)*12);
				ZLGL_TEXCOORDPOINTER(2, GL_SCALAR, 0, texcoordbox);
			}

			int v = 0;

			for (WeightedSurface::Particle* p = ws->particles; p != ws->particles+ws->usedParticles; p++)
			{
				//lifetime check
				if (p->lifetimeStart > ZL_Application::Ticks) continue;
				if (p->lifetimeStart + p->lifetimeDuration < ZL_Application::Ticks)
				{
					ws->usedParticles--;
					if (p == ws->particles+ws->usedParticles) break;
					*(p--) = ws->particles[ws->usedParticles];
					continue;
				}

				//set calc data
				calcData.startPos = &p->startPos;
				calcData.percentOfLifeTime = (scalar)(ZL_Application::Ticks - p->lifetimeStart)/p->lifetimeDuration;
				calcData.rotation = 0;
				calcData.scalew = calcData.scaleh = calcData.alpha = 1;
				calcData.col = s->color;

				//call behavior calculations with their seed
				unsigned char *pseed = p->cseed - calcData.seedbytes;
				for (ZL_ParticleBehavior** b = behaviors; *b; b++)
				{
					calcData.seed = *(unsigned int*) ((*b)->UsesSeed ? (pseed += calcData.seedbytes) : pseed);
					(*b)->Calculate(calcData);
				}

				//draw particle surface
				calcData.col.a *= calcData.alpha;

				#ifdef ZL_VIDEO_OPENGL_ES1
					if (bDoPointSprite)
					{
						vertices[2*v+0] = calcData.pos.x;
						vertices[2*v+1] = calcData.pos.y;
						memcpy(&colors[4*v], &calcData.col.r, sizeof(scalar)*4);
						sizes[v] = calcData.scalew*t->w*1.1;
						if (++v==VBSIZE) { glDrawArraysUnbuffered(GL_POINTS, 0, VBSIZE); v = 0; }
					}
					else
				#endif
				{
					scalar s = ssin(calcData.rotation), c = scos(calcData.rotation);
					scalar cosW = c*t->w*calcData.scalew/2, sinW = s*t->w*calcData.scalew/2;
					scalar cosH = s*t->h*calcData.scaleh/2, sinH = c*t->h*calcData.scaleh/2;
					vertices[12*v+ 0] = calcData.pos.x-cosW+cosH;
					vertices[12*v+ 1] = calcData.pos.y-sinW-sinH;
					vertices[12*v+ 2] = calcData.pos.x+cosW+cosH;
					vertices[12*v+ 3] = calcData.pos.y+sinW-sinH;
					vertices[12*v+ 4] = calcData.pos.x-cosW-cosH;
					vertices[12*v+ 5] = calcData.pos.y-sinW+sinH;
					vertices[12*v+10] = calcData.pos.x+cosW-cosH;
					vertices[12*v+11] = calcData.pos.y+sinW+sinH;
					memcpy(&vertices[12*v+ 6], &vertices[12*v+ 2], sizeof(GLscalar)*4);
					memcpy(&colors[24*v+ 0], &calcData.col.r, sizeof(scalar)*4);
					memcpy(&colors[24*v +4], &colors[24*v+0], sizeof(scalar)*4);
					memcpy(&colors[24*v+ 8], &colors[24*v+0], sizeof(scalar)*4);
					memcpy(&colors[24*v+12], &colors[24*v+0], sizeof(scalar)*12);
					if (++v==VBSIZE) { glDrawArraysUnbuffered(GL_TRIANGLES, 0, 6*VBSIZE); v = 0; }
				}
			}
			#ifdef ZL_VIDEO_OPENGL_ES1
				if (bDoPointSprite)
				{
					if (v) glDrawArraysUnbuffered(GL_POINTS, 0, v);
					glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
					glDisable(GL_POINT_SPRITE_OES);
				}
				else
			#endif
			{
				if (v) glDrawArraysUnbuffered(GL_TRIANGLES, 0, 6*v);
			}
		}
		ZLGL_COLORARRAY_DISABLE();
		active = ((particleImages[0].usedParticles) || (CountParticles() != 0));
	}

	void AddParticleImage(const ZL_Surface &surface, unsigned int maxNumOfParticles, scalar weight)
	{
		//add weighted entry and update weight indexes
		scalar newIndex = totalSurfaceWeight/(totalSurfaceWeight+weight);
		totalSurfaceWeight += weight;
		WeightedSurface* ws = particleImages;
		if (particleImages[(sizeof(particleImages)/sizeof(particleImages[0]))-1].particles) return;
		for (;ws->particles && ws != particleImages+(sizeof(particleImages)/sizeof(particleImages[0])); ws++)
			ws->weightIndex *= newIndex;
		*ws = WeightedSurface(surface, newIndex, maxNumOfParticles);
	}

	void AddBehavior(ZL_ParticleBehavior *particleBehavior, bool deleteBehavior)
	{
		int numSeededBehaviors = 0;
		ZL_ParticleBehavior** b;
		for (b = behaviors; *b; b++) if ((*b)->UsesSeed) numSeededBehaviors++;
		if (b == behaviors+((sizeof(behaviors)/sizeof(behaviors[0]))-1)) return;
		particleBehavior->DeleteAfterUse = deleteBehavior;
		*b = particleBehavior;
		*++b = NULL;
		calcData.seedbytes = (numSeededBehaviors <= 2 ? 4 : 8 / numSeededBehaviors);
		calcData.seedbits = calcData.seedbytes * 8;
		calcData.seed_max_full = (1 << (calcData.seedbits-1));
		calcData.seed_max_half = (1 << (calcData.seedbits/2));
		if (particleBehavior->ChangesAspectRatio || particleBehavior->ChangesRotation) allowPointSprites = false;
	}

	int CountParticleImage()
	{
		int c = 0;
		while (c != (sizeof(particleImages)/sizeof(particleImages[0])) && particleImages[c].particles) c++;
		return c;
	}

	int CountParticles()
	{
		int pc = 0;
		for (int c = 0; c != (sizeof(particleImages)/sizeof(particleImages[0])) && particleImages[c].particles; c++)
			{ pc += particleImages[c].usedParticles; }
		return pc;
	}
};

//--------------------------------------------------------------------
//            Reference counted wrapper
//--------------------------------------------------------------------

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_ParticleEffect)

ZL_ParticleEffect::ZL_ParticleEffect(scalar lifetimeDuration, scalar lifetimeDurationSpread, scalar spawnProbability)
 : impl(new ZL_ParticleEffect_Impl(lifetimeDuration, lifetimeDurationSpread, spawnProbability)) { }

void ZL_ParticleEffect::Spawn(int numOfParticles, scalar x, scalar y, int lifetimeStartOffsetTicks, scalar xSpread, scalar ySpread)
{
	impl->Spawn(numOfParticles, x, y, lifetimeStartOffsetTicks, xSpread, ySpread);
}

void ZL_ParticleEffect::Spawn(int numOfParticles, const ZL_Vector& pos, int lifetimeStartOffsetTicks, scalar xSpread, scalar ySpread)
{
	impl->Spawn(numOfParticles, pos.x, pos.y, lifetimeStartOffsetTicks, xSpread, ySpread);
}

void ZL_ParticleEffect::Draw()
{
	impl->Draw();
}

void ZL_ParticleEffect::SetLifetimeDuration(scalar lifetimeDuration)
{
	impl->lifetimeDuration = lifetimeDuration;
}

void ZL_ParticleEffect::SetLifetimeDurationSpread(scalar lifetimeDurationSpread)
{
	impl->lifetimeDurationSpread = lifetimeDurationSpread;
}

void ZL_ParticleEffect::SetSpawnProbability(scalar spawnProbability)
{
	impl->spawnProbability = spawnProbability;
}

int ZL_ParticleEffect::CountParticleImage()
{
	return impl->CountParticleImage();
}

int ZL_ParticleEffect::CountParticles()
{
	return impl->CountParticles();
}

void ZL_ParticleEffect::AddParticleImage(const ZL_Surface &surface, unsigned int maxNumOfParticles, scalar weight)
{
	impl->AddParticleImage(surface, maxNumOfParticles, weight);
}

void ZL_ParticleEffect::AddBehavior(ZL_ParticleBehavior *particleBehavior, bool deleteBehavior)
{
	impl->AddBehavior(particleBehavior, deleteBehavior);
}

ZL_ParticleBehavior* ZL_ParticleEffect::GetBehavior(size_t index)
{
	if (!impl) return NULL;
	if (index >= (sizeof(impl->behaviors)/sizeof(impl->behaviors[0]))) return NULL;
	return impl->behaviors[index];
}


//--------------------------------------------------------------------
//            Emitter Class (Grouped Effects)
//--------------------------------------------------------------------
/*
class ZL_ParticleEmitter
{
private:
	std::vector<ZL_ParticleEffect*>	assignedParticleEffects;

public:
	ZL_ParticleEmitter();

	void AddParticleEffect(ZL_ParticleEffect *effect);
	void Spawn(int numOfParticles, ZL_Vector pos);
	void Draw();
};

void ZL_ParticleEmitter::Spawn(int numOfParticles, ZL_Vector pos)
{
	for (std::vector<ZL_ParticleEffect*>::iterator it = assignedParticleEffects.begin(); it != assignedParticleEffects.end(); ++it)
		(*it)->Spawn(numOfParticles, pos);
}

void ZL_ParticleEmitter::Draw()
{
	for (std::vector<ZL_ParticleEffect*>::iterator it = assignedParticleEffects.begin(); it != assignedParticleEffects.end(); ++it)
		(*it)->Draw();
}

void ZL_ParticleEmitter::AddParticleEffect(ZL_ParticleEffect *effect)
{
	assignedParticleEffects.push_back(effect);
}
*/

//--------------------------------------------------------------------
//                Behavior: LinearMove
//--------------------------------------------------------------------
ZL_ParticleBehavior_LinearMove::ZL_ParticleBehavior_LinearMove(scalar Speed, scalar SpeedSpread, scalar AngleRad, scalar AngleRadSpread, bool rotateImageToMoveAngle)
	: ZL_ParticleBehavior(true, rotateImageToMoveAngle, false) , rotateImageToMoveAngle(rotateImageToMoveAngle)
{
	spawningSpeed = behaviorSpeed = Speed;
	spawningSpeedSpread = behaviorSpeedSpread = SpeedSpread;
	spawningAngle = behaviorAngle = AngleRad;
	spawningAngleSpread = behaviorAngleSpread = AngleRadSpread;
}

void ZL_ParticleBehavior_LinearMove::SetSpawnSpeed(scalar Speed, scalar SpeedSpread)
{ spawningSpeed = Speed; spawningSpeedSpread = SpeedSpread; }

void ZL_ParticleBehavior_LinearMove::SetSpawnAngle(scalar AngleRad, scalar AngleRadSpread)
{ spawningAngle = AngleRad; spawningAngleSpread = AngleRadSpread; }

void ZL_ParticleBehavior_LinearMove::Calculate(ZL_ParticleEffect_CalcData& d)
{
	scalar angle =  behaviorAngle + ((d.getSeedPercentSecondHalf() - s(0.5)) * behaviorAngleSpread);
	ZL_Vector vel(scos(angle), ssin(angle));
	vel *= behaviorSpeed + (d.getSeedPercentFirstHalf() - s(0.5)) * behaviorSpeedSpread;

	d.pos = *d.startPos + vel * d.percentOfLifeTime;
	if (rotateImageToMoveAngle) d.rotation = angle;
}

unsigned int ZL_ParticleBehavior_LinearMove::GetSpawnSeed(ZL_ParticleEffect_CalcData& d)
{
	scalar speed = spawningSpeed + RAND_VARIATION(spawningSpeedSpread*HALF) ;
	scalar angle = ZL_Math::AngleSpread(spawningAngle + RAND_VARIATION(spawningAngleSpread*HALF));
	if (speed < behaviorSpeed-behaviorSpeedSpread/2) speed = behaviorSpeed-behaviorSpeedSpread/2;
	if (speed > behaviorSpeed+behaviorSpeedSpread/2) speed = behaviorSpeed+behaviorSpeedSpread/2;
	if (angle < behaviorAngle-behaviorAngleSpread/2) angle = behaviorAngle-behaviorAngleSpread/2;
	if (angle > behaviorAngle+behaviorAngleSpread/2) angle = behaviorAngle+behaviorAngleSpread/2;
	scalar seed_speed = ((speed - behaviorSpeed) / behaviorSpeedSpread) + s(0.5);
	scalar seed_angle = ((angle - behaviorAngle) / behaviorAngleSpread) + s(0.5);

	return ((unsigned int)(seed_speed * d.seed_max_half)) +
	      (((unsigned int)(seed_angle * d.seed_max_half)) << (d.seedbits/2));
}

bool ZL_ParticleBehavior_LinearMove::ProvidesSpawnSeed()
{
	return behaviorSpeed != spawningSpeed || behaviorSpeedSpread != spawningSpeedSpread ||
	       behaviorAngle != spawningAngle || behaviorAngleSpread != spawningAngleSpread;
}

//--------------------------------------------------------------------
//                Behavior: LinearImageProperties
//--------------------------------------------------------------------
ZL_ParticleBehavior_LinearImageProperties::ZL_ParticleBehavior_LinearImageProperties(scalar alphaStart, scalar alphaEnd, scalar scaleStart, scalar scaleEnd)
	: ZL_ParticleBehavior(false,false,false), behaviorAlphaStart(alphaStart), behaviorAlphaEnd(alphaEnd), behaviorScaleStart(scaleStart), behaviorScaleEnd(scaleEnd)
{ }

void ZL_ParticleBehavior_LinearImageProperties::Calculate(ZL_ParticleEffect_CalcData& d)
{
	d.scalew = d.scaleh = behaviorScaleStart + (behaviorScaleEnd - behaviorScaleStart) * d.percentOfLifeTime;
	d.alpha = behaviorAlphaStart + (behaviorAlphaEnd - behaviorAlphaStart) * d.percentOfLifeTime;
}

//--------------------------------------------------------------------
//                Behavior: LinearColor
//--------------------------------------------------------------------
ZL_ParticleBehavior_LinearColor::ZL_ParticleBehavior_LinearColor()
	: ZL_ParticleBehavior(true,false,false), totalColorStartWeight(0), totalColorEndWeight(0)
{ }

ZL_ParticleBehavior_LinearColor* ZL_ParticleBehavior_LinearColor::AddColorStart(const ZL_Color &color, scalar weight)
{
	//add weighted entry and update weight indexes
	scalar newIndex = totalColorStartWeight/(totalColorStartWeight+weight);
	totalColorStartWeight += weight;
	for (std::vector<WeightedColor>::iterator it = colorStarts.begin(); it != colorStarts.end(); ++it)
		it->weightIndex *= newIndex;
	colorStarts.push_back(WeightedColor(color, newIndex));
	return this;
}

ZL_ParticleBehavior_LinearColor* ZL_ParticleBehavior_LinearColor::AddColorEnd(const ZL_Color &color, scalar weight)
{
	//add weighted entry and update weight indexes
	scalar newIndex = totalColorEndWeight/(totalColorEndWeight+weight);
	totalColorEndWeight += weight;
	for (std::vector<WeightedColor>::iterator it = colorEnds.begin(); it != colorEnds.end(); ++it)
		it->weightIndex *= newIndex;
	colorEnds.push_back(WeightedColor(color, newIndex));
	return this;
}

void ZL_ParticleBehavior_LinearColor::Calculate(ZL_ParticleEffect_CalcData& d)
{
	int iStart = d.getSeedIntFirstHalf(), iEnd = d.getSeedIntSecondHalf();
	ZL_Color &colStart = colorStarts[iStart], &colEnd = colorEnds[iEnd];
	d.col.r = colStart.r + d.percentOfLifeTime * (colEnd.r - colStart.r);
	d.col.g = colStart.g + d.percentOfLifeTime * (colEnd.g - colStart.g);
	d.col.b = colStart.b + d.percentOfLifeTime * (colEnd.b - colStart.b);
	d.col.a = colStart.a + d.percentOfLifeTime * (colEnd.a - colStart.a);
}

unsigned int ZL_ParticleBehavior_LinearColor::GetSpawnSeed(ZL_ParticleEffect_CalcData& d)
{
	scalar weightStart = RAND_FACTOR, weightEnd = RAND_FACTOR;
	unsigned int iStart, iEnd;
	for (iStart = 0; iStart < colorStarts.size() && colorStarts[iStart].weightIndex <= weightStart; iStart++);
	for (iEnd = 0; iEnd < colorEnds.size() && colorEnds[iEnd].weightIndex <= weightEnd; iEnd++);
	return (iStart-1) + ((iEnd-1) << (d.seedbits/2));
}

bool ZL_ParticleBehavior_LinearColor::ProvidesSpawnSeed()
{
	return true;
}

//--------------------------------------------------------------------
//                Behavior: Gravity
//--------------------------------------------------------------------
ZL_ParticleBehavior_Gravity::ZL_ParticleBehavior_Gravity(scalar Angle, scalar Strength)
	: ZL_ParticleBehavior(false,false,false), behaviorAngle(Angle), behaviorStrength(Strength)
{ }

void ZL_ParticleBehavior_Gravity::Calculate(ZL_ParticleEffect_CalcData& d)
{
	d.pos.y -= behaviorStrength*d.percentOfLifeTime;
	if (d.pos.y < 0) d.pos.y = 0;
}
