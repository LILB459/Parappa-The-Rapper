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
		// SPU constants
		static constexpr unsigned int SAMPLE_RATE = 44100;

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
			// SPU gaussian interpolation
			static const short gaussian[0x200] = {
				-0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001,
				-0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001, -0x001,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001,
				0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0003, 0x0003,
				0x0003, 0x0004, 0x0004, 0x0005, 0x0005, 0x0006, 0x0007, 0x0007,
				0x0008, 0x0009, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
				0x000F, 0x0010, 0x0011, 0x0012, 0x0013, 0x0015, 0x0016, 0x0018,
				0x0019, 0x001B, 0x001C, 0x001E, 0x0020, 0x0021, 0x0023, 0x0025,
				0x0027, 0x0029, 0x002C, 0x002E, 0x0030, 0x0033, 0x0035, 0x0038,
				0x003A, 0x003D, 0x0040, 0x0043, 0x0046, 0x0049, 0x004D, 0x0050,
				0x0054, 0x0057, 0x005B, 0x005F, 0x0063, 0x0067, 0x006B, 0x006F,
				0x0074, 0x0078, 0x007D, 0x0082, 0x0087, 0x008C, 0x0091, 0x0096,
				0x009C, 0x00A1, 0x00A7, 0x00AD, 0x00B3, 0x00BA, 0x00C0, 0x00C7,
				0x00CD, 0x00D4, 0x00DB, 0x00E3, 0x00EA, 0x00F2, 0x00FA, 0x0101,
				0x010A, 0x0112, 0x011B, 0x0123, 0x012C, 0x0135, 0x013F, 0x0148,
				0x0152, 0x015C, 0x0166, 0x0171, 0x017B, 0x0186, 0x0191, 0x019C,

				0x01A8, 0x01B4, 0x01C0, 0x01CC, 0x01D9, 0x01E5, 0x01F2, 0x0200,
				0x020D, 0x021B, 0x0229, 0x0237, 0x0246, 0x0255, 0x0264, 0x0273,
				0x0283, 0x0293, 0x02A3, 0x02B4, 0x02C4, 0x02D6, 0x02E7, 0x02F9,
				0x030B, 0x031D, 0x0330, 0x0343, 0x0356, 0x036A, 0x037E, 0x0392,
				0x03A7, 0x03BC, 0x03D1, 0x03E7, 0x03FC, 0x0413, 0x042A, 0x0441,
				0x0458, 0x0470, 0x0488, 0x04A0, 0x04B9, 0x04D2, 0x04EC, 0x0506,
				0x0520, 0x053B, 0x0556, 0x0572, 0x058E, 0x05AA, 0x05C7, 0x05E4,
				0x0601, 0x061F, 0x063E, 0x065C, 0x067C, 0x069B, 0x06BB, 0x06DC,
				0x06FD, 0x071E, 0x0740, 0x0762, 0x0784, 0x07A7, 0x07CB, 0x07EF,
				0x0813, 0x0838, 0x085D, 0x0883, 0x08A9, 0x08D0, 0x08F7, 0x091E,
				0x0946, 0x096F, 0x0998, 0x09C1, 0x09EB, 0x0A16, 0x0A40, 0x0A6C,
				0x0A98, 0x0AC4, 0x0AF1, 0x0B1E, 0x0B4C, 0x0B7A, 0x0BA9, 0x0BD8,
				0x0C07, 0x0C38, 0x0C68, 0x0C99, 0x0CCB, 0x0CFD, 0x0D30, 0x0D63,
				0x0D97, 0x0DCB, 0x0E00, 0x0E35, 0x0E6B, 0x0EA1, 0x0ED7, 0x0F0F,
				0x0F46, 0x0F7F, 0x0FB7, 0x0FF1, 0x102A, 0x1065, 0x109F, 0x10DB,
				0x1116, 0x1153, 0x118F, 0x11CD, 0x120B, 0x1249, 0x1288, 0x12C7,

				0x1307, 0x1347, 0x1388, 0x13C9, 0x140B, 0x144D, 0x1490, 0x14D4,
				0x1517, 0x155C, 0x15A0, 0x15E6, 0x162C, 0x1672, 0x16B9, 0x1700,
				0x1747, 0x1790, 0x17D8, 0x1821, 0x186B, 0x18B5, 0x1900, 0x194B,
				0x1996, 0x19E2, 0x1A2E, 0x1A7B, 0x1AC8, 0x1B16, 0x1B64, 0x1BB3,
				0x1C02, 0x1C51, 0x1CA1, 0x1CF1, 0x1D42, 0x1D93, 0x1DE5, 0x1E37,
				0x1E89, 0x1EDC, 0x1F2F, 0x1F82, 0x1FD6, 0x202A, 0x207F, 0x20D4,
				0x2129, 0x217F, 0x21D5, 0x222C, 0x2282, 0x22DA, 0x2331, 0x2389,
				0x23E1, 0x2439, 0x2492, 0x24EB, 0x2545, 0x259E, 0x25F8, 0x2653,
				0x26AD, 0x2708, 0x2763, 0x27BE, 0x281A, 0x2876, 0x28D2, 0x292E,
				0x298B, 0x29E7, 0x2A44, 0x2AA1, 0x2AFF, 0x2B5C, 0x2BBA, 0x2C18,
				0x2C76, 0x2CD4, 0x2D33, 0x2D91, 0x2DF0, 0x2E4F, 0x2EAE, 0x2F0D,
				0x2F6C, 0x2FCC, 0x302B, 0x308B, 0x30EA, 0x314A, 0x31AA, 0x3209,
				0x3269, 0x32C9, 0x3329, 0x3389, 0x33E9, 0x3449, 0x34A9, 0x3509,
				0x3569, 0x35C9, 0x3629, 0x3689, 0x36E8, 0x3748, 0x37A8, 0x3807,
				0x3867, 0x38C6, 0x3926, 0x3985, 0x39E4, 0x3A43, 0x3AA2, 0x3B00,
				0x3B5F, 0x3BBD, 0x3C1B, 0x3C79, 0x3CD7, 0x3D35, 0x3D92, 0x3DEF,

				0x3E4C, 0x3EA9, 0x3F05, 0x3F62, 0x3FBD, 0x4019, 0x4074, 0x40D0,
				0x412A, 0x4185, 0x41DF, 0x4239, 0x4292, 0x42EB, 0x4344, 0x439C,
				0x43F4, 0x444C, 0x44A3, 0x44FA, 0x4550, 0x45A6, 0x45FC, 0x4651,
				0x46A6, 0x46FA, 0x474E, 0x47A1, 0x47F4, 0x4846, 0x4898, 0x48E9,
				0x493A, 0x498A, 0x49D9, 0x4A29, 0x4A77, 0x4AC5, 0x4B13, 0x4B5F,
				0x4BAC, 0x4BF7, 0x4C42, 0x4C8D, 0x4CD7, 0x4D20, 0x4D68, 0x4DB0,
				0x4DF7, 0x4E3E, 0x4E84, 0x4EC9, 0x4F0E, 0x4F52, 0x4F95, 0x4FD7,
				0x5019, 0x505A, 0x509A, 0x50DA, 0x5118, 0x5156, 0x5194, 0x51D0,
				0x520C, 0x5247, 0x5281, 0x52BA, 0x52F3, 0x532A, 0x5361, 0x5397,
				0x53CC, 0x5401, 0x5434, 0x5467, 0x5499, 0x54CA, 0x54FA, 0x5529,
				0x5558, 0x5585, 0x55B2, 0x55DE, 0x5609, 0x5632, 0x565B, 0x5684,
				0x56AB, 0x56D1, 0x56F6, 0x571B, 0x573E, 0x5761, 0x5782, 0x57A3,
				0x57C3, 0x57E2, 0x57FF, 0x581C, 0x5838, 0x5853, 0x586D, 0x5886,
				0x589E, 0x58B5, 0x58CB, 0x58E0, 0x58F4, 0x5907, 0x5919, 0x592A,
				0x593A, 0x5949, 0x5958, 0x5965, 0x5971, 0x597C, 0x5986, 0x598F,
				0x5997, 0x599E, 0x59A4, 0x59A9, 0x59AD, 0x59B0, 0x59B2, 0x59B3,
			};

			// SPU block structure
			enum Flags
			{
				Loop = (1 << 0),
				LoopADSR = (1 << 1),
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
				private:
					// Decode state
					Block *memory_p = nullptr;
					size_t memory_length = 0;

					size_t block_p = 0, block_loop = 0;

					long filter_old = 0, filter_older = 0;
					
				public:
					// Decode interface
					Decode() {}
					~Decode() {}

					void SetMemory(Block *p, size_t length)
					{
						memory_p = p;
						memory_length = length;
					}

					void SetPointer(size_t p, size_t loop)
					{
						assert(p < memory_length);
						assert(loop < memory_length);

						block_p = p;
						block_loop = loop;
					}

					void DecodeBlock(short *out)
					{
						// Get block pointer
						Block *blockp = memory_p + block_p;

						// Read block header
						unsigned char shift_filter = (*blockp)[0];
						unsigned char flags = (*blockp)[1];

						long shift = 12 - (shift_filter & 0x0F);
						if (shift < 0)
							shift = 0;
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
							*out++ = (short)s;
							filter_older = filter_old;
							filter_old = s;
						};

						unsigned char *bytep = (unsigned char*)(*blockp) + 2;
						for (int i = 0; i < 14; i++)
						{
							DecodeNibble((*bytep >> 0) & 0xF);
							DecodeNibble((*bytep >> 4) & 0xF);
							bytep++;
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
							if (++block_p >= memory_length)
								block_p = block_loop;
						}
					}

					unsigned char GetFlags()
					{
						// Get block pointer
						Block *blockp = memory_p + block_p;

						// Return block flags
						return (*blockp)[1];
					}

					operator bool()
					{
						return memory_p != nullptr;
					}
			};

			// SPU channel
			enum ADSRState
			{
				Off,
				Attack,
				Decay,
				Sustain,
				Release
			};

			class Channel
			{
				private:
					// Decode state
					Decode decode;
					short decode_wave[28] = {};

					// Channel state
					ADSRState adsr_state = ADSRState::Off;

					unsigned long subposition = 0;
					unsigned short sample_rate = 0;
					short vol_l = 0, vol_r = 0;

					// Resample state
					short resample_old = 0, resample_older = 0, resample_oldest = 0;

				public:
					// Channel interface
					Channel() {}
					~Channel() {}

					void SetMemory(Block *p, size_t length)
					{
						// Set decode memory
						decode.SetMemory(p, length);
					}

					void SetSampleRate(unsigned short _sample_rate)
					{
						// Set channel sample rate
						sample_rate = _sample_rate;
					}

					void SetVolume(short _vol_l, short _vol_r)
					{
						// Set channel volume
						vol_l = _vol_l;
						vol_r = _vol_r;
					}

					void SetADSR(unsigned long adsr)
					{
						// TODO: ADSR
					}

					void KeyOn(size_t p, size_t loop)
					{
						// Reset ADSR
						adsr_state = ADSRState::Attack;
						
						// Initialize decoding
						decode.SetPointer(p, loop);
						DecodeBlock();

						subposition = 0;
						resample_old = resample_older = resample_oldest = 0;
					}

					void KeyOff()
					{
						// Stop ADSR
						if (adsr_state != ADSRState::Off)
							adsr_state = ADSRState::Off; // ADSRState::Release
					}

					void Decode(int16_t *out, size_t frames)
					{
						// Decode samples
						auto OutSample = [&](long s)
						{
							if (s < -0x7FFF)
								*out++ = -0x7FFF;
							else if (s > 0x7FFF)
								*out++ = 0x7FFF;
							else
								*out++ = (int16_t)s;
						};

						size_t i = 0;
						for (; adsr_state != ADSRState::Off && i < frames; i++)
						{
							// Get current sample
							int16_t s = decode_wave[subposition >> 12];

							// Perform resample
							size_t ri = (subposition & 0xFF0) >> 4;
							
							long rs = 0;
							rs += (((long)gaussian[0x0FF - ri]) * (long)resample_oldest) >> 15;
							rs += (((long)gaussian[0x1FF - ri]) * (long)resample_older) >> 15;
							rs += (((long)gaussian[0x100 + ri]) * (long)resample_old) >> 15;
							rs += (((long)gaussian[0x000 + ri]) * (long)s) >> 15;

							// Output sample
							OutSample((rs * vol_l) >> 14);
							OutSample((rs * vol_r) >> 14);

							// Increment subposition
							unsigned long old_subposition = subposition;
							subposition += sample_rate;

							if ((subposition >> 12) != (old_subposition >> 12))
							{
								// Push resampler samples
								resample_oldest = resample_older;
								resample_older = resample_old;
								resample_old = s;
							}

							if (subposition >= (28 << 12))
							{
								// Decode block
								DecodeBlock();
								subposition -= (28 << 12);
							}
						}

						// Clear remaining output
						for (; i < frames; i++)
						{
							*out++ = 0;
							*out++ = 0;
						}
					}

				private:
					void DecodeBlock()
					{
						// Handle flags
						unsigned char flags = decode.GetFlags();
						if ((flags & (Flags::Loop | Flags::LoopADSR)) == Flags::Loop)
						{
							// Kill ADSR
							adsr_state = ADSRState::Off;
						}

						// Decode next block
						decode.DecodeBlock(decode_wave);
					}
			};
		}
	}
}
