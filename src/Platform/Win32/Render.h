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

#include "Platform/Render.h"

#include "Platform/Win32/Win32.h"

#include <d3d11.h>
#include <dxgi1_5.h>
#include <atlbase.h>

namespace PaperPup
{
	namespace Render
	{
		// Win32 implementation
		class Win32Impl
		{
			public:
				// Window handle
				HWND window = nullptr;

				// D3D11 objects
				template<typename T>
				using ComPtr = Microsoft::WRL::ComPtr<T>;

				ComPtr<ID3D11Device> device;
				ComPtr<ID3D11DeviceContext> device_context;

				ComPtr<IDXGIDevice> dxgi_device;
				ComPtr<IDXGIAdapter> dxgi_adapter;
				ComPtr<IDXGIFactory> dxgi_factory;

				ComPtr<IDXGISwapChain> swap_chain;
				ComPtr<ID3D11RenderTargetView> swap_chain_rtv;

				// D3D11 capabilities
				BOOL cap_allow_tearing = FALSE;

				// Sync state
				unsigned int sync_interval = 0;

			public:
				// Win32 implementation interface
				Win32Impl();
				~Win32Impl();

				void CreateSwapChain(const DXGI_MODE_DESC *output_mode);
				void CreateSwapChainRTV();

				void Resize();
				
				void SetWindow(unsigned int width, unsigned int height);

				bool IsFullscreen();
				void SetFullscreen(bool fullscreen);

				void SetSyncInterval(unsigned int interval);

				void StartFrame();
				void EndFrame();
		};
	}
}
