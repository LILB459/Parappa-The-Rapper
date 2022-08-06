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

#include "Menu/Menu.h"

#include "Platform/Render.h"
#include "Platform/Input.h"

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

		// Set display mode
		Render::SetWindow(1920, 1080);
		Render::SetSync(false, 144, false, true);
	}

	Engine::~Engine()
	{
		
	}

	bool Engine::StartFrame()
	{
		// Start render frame
		Render::StartFrame();

		// Handle input events
		if (Input::HandleEvents())
			return true;

		return false;
	}

	void Engine::EndFrame()
	{
		// End render frame
		Render::EndFrame();
	}

	void Engine::Start()
	{
		// Start menu state
		state = std::make_unique<Menu::Menu>();

		// Engine loop
		while (state != nullptr)
			state = state->Start();
	}
}
