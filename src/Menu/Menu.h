/*
 * [PaperPup]
 *   Menu.h
 * Author(s): Regan Green
 * Date: 08/01/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "PaperPup.h"

#include "Engine.h"
#include "Pack.h"

namespace PaperPup
{
	namespace Menu
	{
		// Menu state class
		class Menu : public State
		{
			private:
				
			public:
				// Menu state interface
				Menu();
				~Menu() override;

				std::unique_ptr<State> Start() override;

				std::unique_ptr<Filesystem::Archive> OpenArchive(std::string name) override { return nullptr; }
				std::unique_ptr<Filesystem::File> OpenFile(std::string name, bool mode2) override { return nullptr; }
		};
	}
}
