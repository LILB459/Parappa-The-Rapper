/*
 * [PaperPup]
 *   Audio.cpp
 * Author(s): Regan Green
 * Date: 08/14/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Miniaudio/Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace PaperPup
{
	namespace Audio
	{
		// Miniaudio implementation
		Impl::Impl()
		{
			// Initialize miniaudio mutex
			if (ma_mutex_init(&mutex) != MA_SUCCESS)
				throw PaperPup::RuntimeError("Failed to initialize miniaudio mutex");

			// Initialize miniaudio context
			if (ma_context_init(nullptr, 0, nullptr, &context) != MA_SUCCESS)
				throw PaperPup::RuntimeError("Failed to initialize miniaudio context");

			// Initialize miniaudio device
			ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
			device_config.performanceProfile = ma_performance_profile_low_latency;
			device_config.playback.channels = 2;
			device_config.playback.format = ma_format_s16;

			device_config.pUserData = (void*)this;
			device_config.dataCallback = [](ma_device *device, void *output_buffer_void, const void *input_buffer, ma_uint32 frames_to_do)
			{
				// Get implementation and output buffer
				Impl *impl = (Impl *)device->pUserData;
				int16_t *output_buffer = (int16_t *)output_buffer_void;

				// Mix sounds
				std::unique_ptr<int32_t[]> mix_buffer = std::make_unique<int32_t[]>(frames_to_do * 2);

				ma_mutex_lock(&impl->mutex);
				for (auto &i : impl->sounds)
				{
					// Add to mix buffer
					i->Decode(device->sampleRate, output_buffer, frames_to_do);
					for (size_t j = 0; j < frames_to_do * 2; j++)
						mix_buffer[j] += output_buffer[j];
				}
				ma_mutex_unlock(&impl->mutex);

				// Clip mix buffer to output
				for (size_t i = 0; i < frames_to_do * 2; i++)
				{
					if (mix_buffer[i] < -0x7FFF)
						output_buffer[i] = -0x7FFF;
					else if (mix_buffer[i] > 0x7FFF)
						output_buffer[i] = 0x7FFF;
					else
						output_buffer[i] = mix_buffer[i];
				}
			};

			if (ma_device_init(&context, &device_config, &device))
				throw PaperPup::RuntimeError("Failed to initialize miniaudio device");

			// Start device
			ma_device_start(&device);
		}

		Impl::~Impl()
		{
			// Deinitialize miniaudio objects
			ma_device_stop(&device);
			
			ma_device_uninit(&device);
			ma_context_uninit(&context);
			ma_mutex_uninit(&mutex);
		}

		// Sound backend implementation
		class SoundBackend_Impl : public SoundBackend
		{
			private:
				// Sound data source
				std::unique_ptr<SoundSource> source;

			public:
				// Sound backend interface
				SoundBackend_Impl(SoundSource *_source): source(_source)
				{
					
				}
				~SoundBackend_Impl()
				{
					// Stop sound
					g_impl->audio->sounds.erase(source.get());
				}

				void Play()
				{
					// Play sound
					source->Play();
					g_impl->audio->sounds.insert(source.get());
				}

				void Stop()
				{
					// Stop sound
					g_impl->audio->sounds.erase(source.get());
					source->Stop();
				}
		};

		SoundBackend *SoundBackend::New(SoundSource *_source)
		{
			return new SoundBackend_Impl(_source);
		}

		// Mutex implementation
		Mutex::Mutex() { ma_mutex_lock(&g_impl->audio->mutex); }
		Mutex::~Mutex() { ma_mutex_unlock(&g_impl->audio->mutex); }
	}
}
