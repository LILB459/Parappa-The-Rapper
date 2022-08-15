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

#include "Platform/Audio.h"
#include "Platform/Common/ADPCM.h"

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

		State *Menu::Start()
		{
			std::unique_ptr<char[]> wave;
			size_t wave_size;

			{
				std::unique_ptr<Filesystem::File> file(g_engine->OpenFile("TEST.BIN", false));
				wave_size = file->Size() / 16;
				wave.reset(file->Dup());
			}

			ADPCM::SPU::Block *blocks = (ADPCM::SPU::Block *)wave.get();
			
			std::unique_ptr<Audio::Sound<ADPCM::SPU::Channel>> sound;
			{
				Audio::Mutex mutex;

				sound = std::make_unique<Audio::Sound<ADPCM::SPU::Channel>>(new ADPCM::SPU::Channel(blocks, wave_size, 0, 0));
				sound->Source()->SetSampleRate(0x1000);
				sound->Source()->SetVolume(0x3FFF, 0x3FFF);
				sound->Play();
			}

			while (!g_engine->StartFrame())
			{
				// End frame
				g_engine->EndFrame();
			}

			return nullptr;
		}
	}
}
