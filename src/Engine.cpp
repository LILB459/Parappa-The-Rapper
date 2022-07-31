/*
 * [PaperPup]
 *   Engine.cpp
 * Author(s): Regan Green
 * Date: 07/31/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Engine.h"

#include "Pack.h"

#include <Windows.h>

namespace PaperPup
{
	// Engine global
	Engine *g_engine = nullptr;

	// Engine interface
	Engine::Engine()
	{
		// Open main image
		image_main = Filesystem::Image::Open("Image");
		if (image_main == nullptr)
			throw PaperPup::RuntimeError("Failed to open main image");
		
		// Read packs
		std::vector<std::string> packs = Filesystem::GetPackList();
		for (auto &i : packs)
		{
			Pack pack(i);
			MessageBoxA(nullptr, (pack.pack_name + "\n" + pack.pack_description + "\n" + pack.pack_version).c_str(), i.c_str(), 0);
			for (auto &j : pack.pack_songs)
			{
				MessageBoxA(nullptr, (j.song_name + "\n" + j.song_description).c_str(), i.c_str(), 0);
			}
		}
	}

	Engine::~Engine()
	{

	}

	void Engine::Start()
	{
		
	}
}
