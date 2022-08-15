/*
 * [PaperPup]
 *   ADPCM.h
 * Author(s): Regan Green
 * Date: 08/07/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace PaperPup
{
	namespace ADPCM
	{
		// ADPCM constants
		static const struct
		{
			char f0, f1;
		} FILTERS[8] = {
			{   0,   0 },
			{  60,   0 },
			{ 115, -52 },
			{  98, -55 },
			{ 122, -60 }, // Filter 4 is only available to SPU ADPCM
			{   0,   0 },
			{   0,   0 },
			{   0,   0 }
		};

		namespace SPU
		{
			// SPU block structure
			enum Flags
			{
				Loop = (1 << 0),
				Sustain = (1 << 1),
				SetLoop = (1 << 2)
			};

			/*
				SPU Block Structure:
				  0 - Filter Shift (nibbles)
				  1 - Flags
				2-F - Samples (nibbles)
			*/
			typedef char Block[16];

			static constexpr size_t SamplesToBlocks(size_t samples) { return (samples + 27) / 28; }
			static constexpr size_t BlocksToSamples(size_t blocks) { return blocks * 28; }

			// SPU decoding
			class Decode
			{
				public:
					// Decode state
					Block *block_s = nullptr;
					Block *block_e = nullptr;

					Block *block_p = nullptr;
					Block *block_loop = nullptr;

					long filter_old = 0, filter_older = 0;
					
				public:
					// Decode interface
					Decode() {}
					~Decode() {}

					void SetMemory(Block *s, Block *e)
					{
						assert(e > s);

						block_s = s;
						block_e = e;
					}

					void SetPointer(Block *p, Block *loop)
					{
						assert(p >= block_s && p < block_e);
						assert(loop >= block_s && loop < block_e);

						block_p = p;
						block_loop = loop;
					}

					void DecodeBlock(int16_t *out)
					{
						// Read block header
						unsigned char shift_filter = (*block_p)[0];
						unsigned char flags = (*block_p)[1];

						long shift = 12 - (shift_filter & 0x0F);
						long filter = (shift_filter & 0x70) >> 4;

						long f0 = FILTERS[filter].f0;
						long f1 = FILTERS[filter].f1;

						// Process flags
						if (flags & Flags::SetLoop)
						{
							block_loop = block_p;
						}

						// Decode bytes
						auto DecodeNibble = [&](unsigned char nibble)
						{
							// Sign extend nibble
							long t = nibble;
							if (t & 0x8)
								t -= 0x10;

							// Decode and clip sample
							long s = (t << shift) + ((filter_old * f0 + filter_older * f1 + 32) / 64);
							if (s < -0x7FFF)
								s = -0x7FFF;
							if (s > 0x7FFF)
								s = 0x7FFF;

							// Write sample
							*out++ = (int16_t)s;
							filter_older = filter_old;
							filter_old = s;
						};

						unsigned char *blockp = (unsigned char*)(*block_p) + 2;
						for (int i = 0; i < 14; i++)
						{
							DecodeNibble((*blockp >> 0) & 0xF);
							DecodeNibble((*blockp >> 4) & 0xF);
							blockp++;
						}

						// Update block pointer
						if (flags & Flags::Loop)
						{
							// Set block pointer to loop point
							block_p = block_loop;
						}
						else
						{
							// Increment block pointer
							if (++block_p >= block_e)
								block_p = block_loop;
						}
					}

					operator bool()
					{
						return block_s != nullptr && block_e != nullptr && block_p != nullptr && block_loop != nullptr;
					}
			};

			// SPU channel
			class Channel
			{
				public:
					// Decode state
					Decode decode;

					// Channel state


				public:
					// Channel interface
					
			};
		}
	}
}
