/*
 * [PaperPup]
 *   Input.h
 * Author(s): Regan Green
 * Date: 08/01/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Input.h"

#include "Platform/Win32/Win32.h"

namespace PaperPup
{
	namespace Input
	{
		// Win32 implementation
		class Win32Impl
		{
			public:
			

			public:
				// Win32 implementation interface
				Win32Impl();
				~Win32Impl();

				static LRESULT WindowProc(HWND parent, UINT message, WPARAM wparam, LPARAM lparam);
		};
	}
}
