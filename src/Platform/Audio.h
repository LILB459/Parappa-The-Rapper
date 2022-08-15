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
		// Audio classes
		class Lock
		{
			public:
				// Locks the audio thread during lifetime
				Lock();
				~Lock();
		};

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

		template <class T>
		class SoundPtr
		{
			private:
				// Lock during pointer's lifetime
				Lock lock;
				
				// Held source
				T *source;

			public:
				// Sound pointer interface
				SoundPtr(T *_source) : lock(), source(_source) {}
				~SoundPtr() {}

				T &operator*()
				{
					return *source;
				}
				T *operator->()
				{
					return source;
				}
		};

		template <class T>
		class Sound
		{
			private:
				// Sound source and backend
				T *source;
				SoundBackend *backend;

			public:
				// Sound interface
				template <class... Args>
				static Sound<T> *New(Args&&... args)
				{
					return new Sound<T>(new T(std::forward<Args>(args)...));
				}
				~Sound()
				{
					// Delete backend sound
					delete backend;
				}

				void Play() { backend->Play(); }
				void Stop() { backend->Stop(); }

				SoundPtr<T> Source() { return SoundPtr<T>(source); }

			private:
				Sound(T *_source) : source(_source)
				{
					// Create backend for source
					backend = SoundBackend::New(source);
				}
		};

		// Sound interface
	}
}
