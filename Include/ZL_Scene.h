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

#ifndef __ZL_SCENE__
#define __ZL_SCENE__

#include "ZL_Math.h"

typedef short ZL_SceneType;
#define SCN_NOSCENE ((ZL_SceneType)0)

class ZL_Scene
{
	bool bInitedGlobal;
protected:
	ZL_Scene(ZL_SceneType SceneType);

public:
	virtual void InitGlobal() { }
	virtual void InitEnter(ZL_SceneType SceneTypeFrom, void* data) { }
	virtual int InitTransitionEnter(ZL_SceneType SceneTypeFrom, void* data) { return 0; }
	virtual void InitAfterTransition() { }

	virtual void DeInitGlobal() { }
	virtual void DeInitLeave(ZL_SceneType SceneTypeTo) { }
	virtual int DeInitTransitionLeave(ZL_SceneType SceneTypeTo) { return 0; }
	virtual void DeInitAfterTransition() { }

	virtual void Calculate() { }
	virtual void DrawTransition(scalar f, bool IsLeaveTransition);
	virtual void DrawCrossfade(scalar f, bool IsLeaveTransition, ZL_Scene* pOtherScene) { DrawTransition(f, IsLeaveTransition); }
	virtual void Draw() { }

	void DoInitGlobal();
	bool IsActive();

	ZL_SceneType SceneType;
};

class ZL_SceneManager
{
public:
	static bool Init(ZL_SceneType FirstScene = SCN_NOSCENE);
	static void DeInit();
	static ZL_Scene* Get(ZL_SceneType SceneType);
	static ZL_Scene* GetCurrent();
	static bool GoToScene(ZL_SceneType SceneType, void* data = NULL, bool SwitchImmediately = false);
};

#endif //__ZL_SCENE__
