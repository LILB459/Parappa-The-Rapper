/*
 * [PaperPup]
 *   Engine.h
 * Author(s): Regan Green
 * Date: 07/31/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "PaperPup.h"

#include "Platform/Filesystem.h"

#include <memory>

namespace PaperPup
{
	// Engine class
	class Engine
	{
		private:
			// Images
			std::unique_ptr<Filesystem::Image> image_main;
			std::unique_ptr<Filesystem::Image> image_pack;

		public:
			// Engine interface
			Engine();
			~Engine();

			void Start();

			std::unique_ptr<Filesystem::Archive> OpenArchive(std::string name)
			{
				std::unique_ptr<Filesystem::Archive> archive;
				if (image_pack != nullptr)
				{
					if ((archive = image_pack->OpenArchive(name)) != nullptr)
						return archive;
				}
				if (image_main != nullptr)
				{
					if ((archive = image_main->OpenArchive(name)) != nullptr)
						return archive;
				}
				return nullptr;
			}

			std::unique_ptr<Filesystem::File> OpenFile(std::string name, bool mode2)
			{
				std::unique_ptr<Filesystem::File> file;
				if (image_pack != nullptr)
				{
					if ((file = image_pack->OpenFile(name, mode2)) != nullptr)
						return file;
				}
				if (image_main != nullptr)
				{
					if ((file = image_main->OpenFile(name, mode2)) != nullptr)
						return file;
				}
				return nullptr;
			}
	};

	// Engine global
	extern Engine *g_engine;
}
