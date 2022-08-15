/*
 * [PaperPup]
 *   Audio.h
 * Author(s): Regan Green
 * Date: 08/14/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Audio.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION

#include <miniaudio/miniaudio.h>

#include <memory>
#include <unordered_set>
#include <mutex>

namespace PaperPup
{
	namespace Audio
	{
		// Miniaudio implementation
		class Impl
		{
			public:
				// Miniaudio objects
				std::recursive_mutex mutex;

				ma_context context{};
				ma_device device{};

				// Sound set
				std::unordered_set<SoundSource*> sounds;

			public:
				// Miniaudio implementation interface
				Impl();
				~Impl();
		};
	}
}
