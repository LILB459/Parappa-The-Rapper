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

#include "Platform/Common/ADPCM.h"

#include <fstream>

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

			ADPCM::SPU::Block *blocks = (ADPCM::SPU::Block*)wave.get();

			ADPCM::SPU::Channel channel;
			channel.SetMemory(blocks, wave_size);

			channel.SetSampleRate(0x0800);
			channel.SetADSR(0x9FC080FF);
			channel.SetVolume(0x3FFF, 0x3FFF);

			channel.KeyOn(0, 0);

			{
				std::ofstream test("test.bin", std::ios::binary);
				for (size_t i = 0; i < 5; i++)
				{
					static int16_t decoded[ADPCM::SAMPLE_RATE * 2] = {};
					channel.Decode(decoded, ADPCM::SAMPLE_RATE);
					test.write((char *)decoded, sizeof(decoded));
				}
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
