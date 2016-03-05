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

#include "ZL_Signal.h"

void ZL_Signal::disconnect_class(void *cb_inst)
{
	for (std::vector<ZL_Slot*>::iterator it = slots.begin(); it != slots.end();)
		if (cb_inst == ((ZL_MethodSlot_v0<ZL_Signal>*)(*it))->cb_inst) { delete (*it); it = slots.erase(it); } else ++it;
}

void ZL_Signal::disconnect_all()
{
	for (std::vector<ZL_Slot*>::iterator it = slots.begin(); it != slots.end(); ++it)
		delete (*it);
	slots.clear();
}
