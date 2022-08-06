/*
 * [PaperPup]
 *   Pack.cpp
 * Author(s): Regan Green
 * Date: 07/31/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Pack.h"

namespace PaperPup
{
	// Pack interface
	Pack::Pack(std::string name)
	{
		Lua::LuaController lua;

		// Open pack image
		std::unique_ptr<Filesystem::Image> pack_image(Filesystem::Image::Open("Packs/" + name + "/Image"));
		if (pack_image == nullptr)
			throw PaperPup::RuntimeError(name + " pack has no image");

		// Open pack module
		lua.RequireImageFile(pack_image.get(), "PACK.LUA");

		// Get pack information
		pack_name = Lua::GetString(lua.global_state, -1, "Name");
		pack_description = Lua::GetString(lua.global_state, -1, "Description");
		pack_version = Lua::GetString(lua.global_state, -1, "Version");

		// Get pack songs table
		lua_getfield(lua.global_state, -1, "Songs");
		if (!lua_istable(lua.global_state, -1))
			throw PaperPup::RuntimeError("Field Songs is not a table");

		lua_pushnil(lua.global_state);
		while (lua_next(lua.global_state, -2))
		{
			// Get song name
			std::string song_name = lua_tostring(lua.global_state, -1);

			// Open song module
			lua.RequireImageFile(pack_image.get(), song_name + "/SONG.LUA");

			// Get song information
			Song song;
			song.song_name = Lua::GetString(lua.global_state, -1, "Name");
			song.song_description = Lua::GetString(lua.global_state, -1, "Description");
			pack_songs.push_back(song);

			// Pop song module
			lua_pop(lua.global_state, 1);

			// Pop song name
			lua_pop(lua.global_state, 1);
		}

		// Pop pack songs table
		lua_pop(lua.global_state, 1);

		// Pop pack module
		lua_pop(lua.global_state, 1);
	}

	Pack::~Pack()
	{

	}
}
