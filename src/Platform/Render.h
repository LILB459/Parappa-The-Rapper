/*
 * [PaperPup]
 *   Render.h
 * Author(s): Regan Green
 * Date: 08/01/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Platform.h"

namespace PaperPup
{
	namespace Render
	{
		// Render interface
		void SetWindow(unsigned int width, unsigned int height);
		void SetFullscreen(bool fullscreen);
	}
}
