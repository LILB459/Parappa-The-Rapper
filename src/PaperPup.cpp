/*
 * [PaperPup]
 *   PaperPup.cpp
 * Author(s): Regan Green
 * Date: 07/26/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "PaperPup.h"

#include "Engine.h"

namespace PaperPup
{
	// Entry point
	int Main(std::vector<std::string> args)
	{
		try
		{
			// Create engine
			g_engine = new Engine();

			// Start engine
			g_engine->Start();

			// Delete engine
			delete g_engine;
		}
		catch (std::exception & exception)
		{
			// Delete engine and pass exception to exception handler
			if (g_engine != nullptr)
			{
				delete g_engine;
				g_engine = nullptr;
			}
			throw PaperPup::RuntimeError(exception.what());
		}
		
		return 0;
	}
}
