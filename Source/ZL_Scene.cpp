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

#include "ZL_Scene.h"
#include "ZL_Application.h"
#include "ZL_Display_Impl.h"
#include "ZL_Display.h"
#include <assert.h>
#include <map>

static ZL_Scene *pCurrentScene, *pSceneTransitionFrom, *pSceneTransitionTo;
static int TransitionTicksTotal, TransitionTicksLeft;
static void *pSceneTransitionData;
static unsigned char TransitionStepCount;
static std::map<ZL_SceneType, ZL_Scene*> *Scenes = NULL;

ZL_Scene::ZL_Scene(ZL_SceneType SceneType) : bInitedGlobal(false), SceneType(SceneType)
{
	if (SceneType == SCN_NOSCENE) return;
	if (!Scenes) Scenes = new std::map<ZL_SceneType, ZL_Scene*>();
	assert(!(Scenes->operator[](SceneType)));
	Scenes->operator[](SceneType) = this;
}

void ZL_Scene::DoInitGlobal()
{
	if (!bInitedGlobal)
	{
		InitGlobal();
		bInitedGlobal = true;
	}
}

bool ZL_Scene::IsActive()
{
	return (pCurrentScene == this && (!TransitionStepCount || TransitionStepCount > 3));
}

void ZL_Scene::DrawTransition(scalar f, bool IsLeaveTransition)
{
	Draw();
	ZL_Display::FillRect(0, 0, ZLWIDTH, ZLHEIGHT, ZLLUMA(0, f));
}

enum
{
	TRANSITION_INACTIVE = 0,
	TRANSITION_BEGIN,
	TRANSITION_WAIT_FADEOUT_LEAVE,
	TRANSITION_WAIT_FADEIN_ENTER,
	TRANSITION_WAIT_CROSSFADE_LEAVE,
	TRANSITION_WAIT_CROSSFADE_ENTER,
};

static void CalculateSceneTransitionEnd()
{
	if (pSceneTransitionTo) pSceneTransitionTo->InitAfterTransition();
	pSceneTransitionFrom = pSceneTransitionTo = NULL;
	TransitionStepCount = 0;
}

static void CalculateSceneTransitionSwitchScene()
{
	if (pSceneTransitionFrom) pSceneTransitionFrom->DeInitAfterTransition();
	pCurrentScene = pSceneTransitionTo;
	if (!pSceneTransitionTo) { CalculateSceneTransitionEnd(); return; }
	pSceneTransitionTo->InitEnter((pSceneTransitionFrom ? pSceneTransitionFrom->SceneType : SCN_NOSCENE), pSceneTransitionData);
	TransitionTicksTotal = pSceneTransitionTo->InitTransitionEnter((pSceneTransitionFrom ? pSceneTransitionFrom->SceneType : SCN_NOSCENE), pSceneTransitionData);
	if (!TransitionTicksTotal)
	{
		CalculateSceneTransitionEnd();
	}
	else if (TransitionTicksTotal < 0)
	{
		TransitionTicksLeft = TransitionTicksTotal = -TransitionTicksTotal;
		TransitionStepCount = TRANSITION_WAIT_CROSSFADE_ENTER;
	}
	else
	{
		TransitionTicksLeft = TransitionTicksTotal;
		TransitionStepCount = TRANSITION_WAIT_FADEIN_ENTER;
	}
}

static void CalculateSceneTransition()
{
	if (TransitionStepCount == TRANSITION_BEGIN)
	{
		if (!pSceneTransitionFrom) { CalculateSceneTransitionSwitchScene(); return; }
		pSceneTransitionFrom->DeInitLeave(pSceneTransitionTo ? pSceneTransitionTo->SceneType : SCN_NOSCENE);
		TransitionTicksTotal = pSceneTransitionFrom->DeInitTransitionLeave(pSceneTransitionTo ? pSceneTransitionTo->SceneType : SCN_NOSCENE);
		if (!TransitionTicksTotal)
		{
			CalculateSceneTransitionSwitchScene();
		}
		else if (TransitionTicksTotal < 0)
		{
			pSceneTransitionTo->InitEnter((pSceneTransitionFrom ? pSceneTransitionFrom->SceneType : SCN_NOSCENE), pSceneTransitionData);
			TransitionTicksLeft = TransitionTicksTotal = -TransitionTicksTotal;
			TransitionStepCount = TRANSITION_WAIT_CROSSFADE_LEAVE;
		}
		else
		{
			TransitionTicksLeft = TransitionTicksTotal;
			TransitionStepCount = TRANSITION_WAIT_FADEOUT_LEAVE;
		}
	}
	else if (TransitionTicksLeft)
	{
		TransitionTicksLeft -= ZLELAPSEDTICKS;
		if (TransitionTicksLeft <= 0)
		{
			TransitionTicksLeft = 0;
			if (TransitionStepCount == TRANSITION_WAIT_CROSSFADE_LEAVE || TransitionStepCount == TRANSITION_WAIT_FADEOUT_LEAVE)
				CalculateSceneTransitionSwitchScene();
			else
				CalculateSceneTransitionEnd();
		}
	}
}

static void SceneManagerCalculate()
{
	if (TransitionStepCount) CalculateSceneTransition();
	if (pCurrentScene) pCurrentScene->Calculate();
}

static void SceneManagerDraw()
{
	if (native_aspectcorrection) { glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT); }
	ZL_MainApplication->BeforeFrame();
	switch (TransitionStepCount)
	{
		case TRANSITION_WAIT_FADEOUT_LEAVE:   pSceneTransitionFrom->DrawTransition(s(1) - s(TransitionTicksLeft) / s(TransitionTicksTotal), true); break;
		case TRANSITION_WAIT_FADEIN_ENTER:    pSceneTransitionTo->DrawTransition(s(TransitionTicksLeft) / s(TransitionTicksTotal), false); break;
		case TRANSITION_WAIT_CROSSFADE_LEAVE: pSceneTransitionFrom->DrawCrossfade(s(1) - s(TransitionTicksLeft) / s(TransitionTicksTotal), true, pSceneTransitionTo); break;
		case TRANSITION_WAIT_CROSSFADE_ENTER: pSceneTransitionTo->DrawCrossfade(s(TransitionTicksLeft) / s(TransitionTicksTotal), true, pSceneTransitionFrom); break;
		default: if (pCurrentScene) pCurrentScene->Draw();
	}
	ZL_MainApplication->AfterFrame();
}

bool ZL_SceneManager::GoToScene(ZL_SceneType SceneType, void* data, bool SwitchImmediately)
{
	ZL_Scene *pScene = Get(SceneType);
	if (!pScene) { ZL_LOG1("SceneManager", "Unknown scene type: %d", SceneType); return false; }
	if (pScene == pCurrentScene || TransitionStepCount) return false;
	pScene->DoInitGlobal();
	pSceneTransitionFrom = pCurrentScene;
	pSceneTransitionTo = pScene;
	pSceneTransitionData = data;
	TransitionStepCount = 1;
	if (SwitchImmediately) CalculateSceneTransition();
	return true;
}

bool ZL_SceneManager::Init(ZL_SceneType FirstScene)
{
	if (!Scenes) Scenes = new std::map<ZL_SceneType, ZL_Scene*>();
	for (std::map<ZL_SceneType, ZL_Scene*>::iterator it = Scenes->begin(); it != Scenes->end(); ++it)
		(*it).second->DoInitGlobal();
	funcSceneManagerCalculate = &SceneManagerCalculate;
	funcSceneManagerDraw = &SceneManagerDraw;
	if (FirstScene != SCN_NOSCENE) { GoToScene(FirstScene); CalculateSceneTransition(); }
	return true;
}

void ZL_SceneManager::DeInit()
{
	funcSceneManagerCalculate = NULL;
	funcSceneManagerDraw = NULL;
}

ZL_Scene* ZL_SceneManager::Get(ZL_SceneType SceneType)
{
	if (!Scenes) Scenes = new std::map<ZL_SceneType, ZL_Scene*>();
	std::map<ZL_SceneType, ZL_Scene*>::iterator it = Scenes->find(SceneType);
	return (it != Scenes->end() ? it->second : NULL);
}

ZL_Scene* ZL_SceneManager::GetCurrent()
{
	return pCurrentScene;
}
