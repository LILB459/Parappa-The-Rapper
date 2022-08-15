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

#include "Platform/Platform.h"

#include <cstdint>
#include <cstddef>

namespace PaperPup
{
	namespace Audio
	{
		// Sound classes
		class SoundSource
		{
			public:
				// Sound source interface
				virtual ~SoundSource() {}

				virtual void Play() = 0;
				virtual void Stop() = 0;

				virtual void Decode(unsigned long out_sample_rate, int16_t *out, size_t frames) = 0;
		};

		class SoundBackend
		{
			public:
				// Sound backend interface
				static SoundBackend *New(SoundSource *_source); // Ownership of source is taken
				virtual ~SoundBackend() {}

				virtual void Play() = 0;
				virtual void Stop() = 0;
		};

		template <typename T>
		class Sound
		{
			private:
				// Sound source and backend
				T *source;
				SoundBackend *backend;

			public:
				// Sound interface
				Sound(T *_source) // Ownership of source is taken
				{
					// Create backend for source
					source = _source;
					backend = SoundBackend::New(source);
				}
				~Sound()
				{
					// Delete backend sound
					delete backend;
				}

				void Play() { backend->Play(); }
				void Stop() { backend->Stop(); }

				T *Source() { return source; }
		};

		class Mutex
		{
			public:
				Mutex();
				~Mutex();
		};

		// Sound interface
	}
}
