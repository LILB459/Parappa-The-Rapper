/*
 * [PaperPup]
 *   Render.h
 * Author(s): Regan Green
 * Date: 08/01/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Platform.h"

namespace PaperPup
{
	namespace Render
	{
		// Texture class
		enum TextureBind
		{
			None = 0,
			Resource = (1 << 0),
			Target = (1 << 1),
			Dynamic = (1 << 2)
		};

		class Texture
		{
			public:
				// Texture interface
				static Texture *New();
				static Texture *New(unsigned int bind, unsigned int w, unsigned int h, const void *data);

				virtual ~Texture() {}

				virtual void Image(unsigned int bind, unsigned int w, unsigned int h, const void *data) = 0;
				virtual void SubImage(unsigned int x, unsigned int y, unsigned int w, unsigned int h, const void *data) = 0;
		};

		// Render interface
		void SetWindow(unsigned int width, unsigned int height);

		bool IsFullscreen();
		void SetFullscreen(bool fullscreen);

		void SetSync(bool limiter_enabled, unsigned int limiter, bool tearing_enabled, bool vsync_enabled);

		void StartFrame();
		void EndFrame();
	}
}
