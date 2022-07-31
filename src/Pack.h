/*
 * [PaperPup]
 *   Pack.h
 * Author(s): Regan Green
 * Date: 07/31/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "PaperPup.h"

#include "Platform/Filesystem.h"

#include "LuaController.h"

namespace PaperPup
{
	// Pack class
	struct Song
	{
		// Song information
		std::string song_name, song_description;
	};

	class Pack
	{
		public:
			// Pack information
			std::string pack_name, pack_description, pack_version;
			std::vector<Song> pack_songs;
			
		public:
			// Pack interface
			Pack(std::string name);
			~Pack();
	};
}
