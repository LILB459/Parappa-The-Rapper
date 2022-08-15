/*
 * [PaperPup]
 *   Input.cpp
 * Author(s): Regan Green
 * Date: 08/01/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Platform/Win32/Input.h"

#include "Platform/Win32/Render.h"

namespace PaperPup
{
	namespace Input
	{
		// Win32 implementation
		Impl::Impl(PaperPup::Impl &impl)
		{

		}

		Impl::~Impl()
		{

		}

		LRESULT Impl::WindowProc(HWND parent, UINT message, WPARAM wparam, LPARAM lparam)
		{
			// Handle message
			switch (message)
			{
				case WM_DESTROY:
				case WM_CLOSE:
				{
					// Post quit message for program
					PostQuitMessage(0);
					return 0;
				}

				case WM_SIZE:
				{
					// Update render window
					g_impl->render->Resize();
					break;
				}
			}

			// Handle default process
			return DefWindowProc(parent, message, wparam, lparam);
		}

		// Input interface
		bool HandleEvents()
		{
			// Process window messages
			MSG window_msg = {};
			while (PeekMessageW(&window_msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&window_msg);
				DispatchMessageW(&window_msg);
				if (window_msg.message == WM_QUIT)
					return true;
			}
			return false;
		}
	}
}
