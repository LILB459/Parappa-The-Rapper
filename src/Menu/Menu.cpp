/*
 * [PaperPup]
 *   Menu.cpp
 * Author(s): Regan Green
 * Date: 08/01/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Menu/Menu.h"

namespace PaperPup
{
	namespace Menu
	{
		// Menu class interface
		Menu::Menu()
		{

		}

		Menu::~Menu()
		{

		}

		std::unique_ptr<State> Menu::Start()
		{
			return nullptr;
		}
	}
}
