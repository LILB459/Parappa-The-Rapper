/*
 * [PaperPup]
 *   Mode2.h
 * Author(s): Regan Green
 * Date: 07/30/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Filesystem.h"
#include "Platform/Common/Binary.h"

namespace PaperPup
{
	namespace Filesystem
	{
		// Mode 2 insurance function
		static void InsureMode2(char **data, size_t *size)
		{
			// Check file size
			bool is2336 = (*size % SECTOR_MODE2_PARTIAL) == 0;
			bool is2352 = (*size % SECTOR_MODE2) == 0;
			if (!(is2336 || is2352))
				throw PaperPup::RuntimeError("Data size cannot be a mode 2 file");

			// Check if mode 2
			bool is_mode2;
			if (is2352)
			{
				if (is2336)
				{
					// Sync bytes determine a mode 2 file
					static const unsigned char sync_bytes[12] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
					if (std::memcmp(sync_bytes, *data, 12))
						is_mode2 = false;
					else
						is_mode2 = true;
				}
				else
				{
					// Size can only be a mode 2 file
					is_mode2 = true;
				}
			}
			else
			{
				// Size can not be a mode 2 file
				is_mode2 = false;
			}

			if (!is_mode2)
			{
				// Insert mode 2 header for each sector
				char *datap = *data;

				size_t sectors = *size / SECTOR_MODE2_PARTIAL;
				char *mode2_data = new char[sectors * SECTOR_MODE2];
				char *mode2_datap = mode2_data;

				for (size_t i = 0; i < sectors; i++)
				{
					static const unsigned char mode2_head[16] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x02 }; // Sync bytes, address, then mode (2)
					std::memcpy(mode2_datap, mode2_head, 16);
					std::memcpy(mode2_datap + 16, datap, SECTOR_MODE2_PARTIAL);
					datap += SECTOR_MODE2_PARTIAL;
					mode2_datap += SECTOR_MODE2;
				}

				// Return new mode 2 file
				delete[] *data;
				*data = mode2_data;
				*size = sectors * SECTOR_MODE2;
			}
		}
	}
}
