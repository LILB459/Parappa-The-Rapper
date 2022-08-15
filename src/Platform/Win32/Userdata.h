/*
 * [PaperPup]
 *   Userdata.h
 * Author(s): Regan Green
 * Date: 08/06/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Userdata.h"

#include "Platform/Win32/Win32.h"

#include "Platform/Common/Userdata.h"

namespace PaperPup
{
	namespace Userdata
	{
		// Win32 implementation
		class Impl
		{
			public:
				// Userdata
				Userdata userdata;

			public:
				// Win32 implementation interface
				Impl(PaperPup::Impl &impl);
				~Impl();
		};
	}
}
