/*
 * [PaperPup]
 *   Filesystem.h
 * Author(s): Regan Green
 * Date: 07/26/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Filesystem.h"

#include "Platform/Win32/Win32.h"

namespace PaperPup
{
	namespace Filesystem
	{
		// Win32 implementation
		class Win32Impl
		{
			public:
				// Module path
				std::wstring module_path;

			public:
				// Filesystem interface
				Win32Impl();
				~Win32Impl();
		};
	}
}