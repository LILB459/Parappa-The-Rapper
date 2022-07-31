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

#include <Windows.h>

#include <fstream>

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
	}

	Engine::~Engine()
	{

	}

	void Engine::Start()
	{
		
	}
}
