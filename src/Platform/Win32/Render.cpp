/*
 * [PaperPup]
 *   Render.cpp
 * Author(s): Regan Green
 * Date: 08/01/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Platform/Win32/Render.h"

#include "Platform/Win32/Input.h"

namespace PaperPup
{
	namespace Render
	{
		// Render constants
		static constexpr LPWSTR WINDOW_CLASS = L"PaperPup::window";
		static constexpr LPWSTR WINDOW_NAME = L"PaperPup";
		static constexpr DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW;

		static constexpr DXGI_FORMAT OUTPUT_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Win32 implementation interface
		Win32Impl::Win32Impl()
		{
			// Define window class
			WNDCLASSEXW window_class = {};
			window_class.cbSize = sizeof(WNDCLASSEX);

			window_class.style = CS_HREDRAW | CS_VREDRAW;
			window_class.lpfnWndProc = Input::Win32Impl::WindowProc;
			window_class.hInstance = GetModuleHandleW(nullptr);
			window_class.hIcon = LoadIconW(window_class.hInstance, L"PAPERPUP_ICON");
			window_class.hIconSm = LoadIconW(window_class.hInstance, L"PAPERPUP_ICONSM");
			window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
			window_class.lpszClassName = WINDOW_CLASS;

			// Register window class
			if (RegisterClassExW(&window_class) == 0)
				throw PaperPup::RuntimeError("Failed to register window class");

			// Create window
			if ((window = CreateWindowExW(
				0,
				WINDOW_CLASS,
				WINDOW_NAME,
				WINDOW_STYLE,
				0, 0, 0, 0,
				nullptr, nullptr, window_class.hInstance,
				nullptr)) == nullptr)
				throw PaperPup::RuntimeError("Failed to create window");

			// Find first available device
			static D3D_DRIVER_TYPE driver_types[] =
			{
				D3D_DRIVER_TYPE_HARDWARE,
				D3D_DRIVER_TYPE_WARP,
				D3D_DRIVER_TYPE_REFERENCE,
			};
			static D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

			HRESULT hresult;
			for (size_t i = 0; i < ARRAYSIZE(driver_types); i++)
			{
				if (SUCCEEDED(hresult = D3D11CreateDevice(
					nullptr,
					driver_types[i],
					nullptr,
					0,
					feature_levels, ARRAYSIZE(feature_levels),
					D3D11_SDK_VERSION,
					&device, nullptr, &device_context)))
					break;
			}
			if (FAILED(hresult))
				throw PaperPup::RuntimeError("Failed to find DirectX 11.0 compatible device");

			// Get DXGI objects
			if (FAILED(device->QueryInterface(IID_PPV_ARGS(dxgi_device.GetAddressOf()))))
				throw PaperPup::RuntimeError("Failed to get DXGI device");
			if (FAILED(dxgi_device->GetAdapter(&dxgi_adapter)))
				throw PaperPup::RuntimeError("Failed to get DXGI adapter");
			if (FAILED(dxgi_adapter->GetParent(IID_PPV_ARGS(dxgi_factory.GetAddressOf()))))
				throw PaperPup::RuntimeError("Failed to get DXGI factory");

			// Get DXGI 1.5 factory
			ComPtr<IDXGIFactory5> dxgi_factory5;
			if (SUCCEEDED(dxgi_factory.As(&dxgi_factory5)))
			{
				// Check hardware capabilities
				dxgi_factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &cap_allow_tearing, sizeof(cap_allow_tearing));
			}

			// Create swap chain
			CreateSwapChain(nullptr);
		}

		Win32Impl::~Win32Impl()
		{
			// Ensure swap chain is windowed before we exit
			if (swap_chain != nullptr)
				swap_chain->SetFullscreenState(FALSE, nullptr);

			// Destroy window
			if (window != nullptr)
				DestroyWindow(window);
		}

		void Win32Impl::CreateSwapChain(const DXGI_MODE_DESC *output_mode)
		{
			// Get swap chain description
			DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
			if (output_mode != nullptr)
			{
				// Use output description
				swap_chain_desc.BufferDesc = *output_mode;
			}
			else
			{
				// Ensure our current swap chain exits fullscreen
				if (swap_chain != nullptr)
					swap_chain->SetFullscreenState(FALSE, nullptr);

				// Get window client area
				RECT client_rect = {};
				GetClientRect(window, &client_rect);
				const unsigned int width = client_rect.right - client_rect.left;
				const unsigned int height = client_rect.bottom - client_rect.top;

				// Use window area for buffer
				swap_chain_desc.BufferDesc.Width = width;
				swap_chain_desc.BufferDesc.Height = height;
				swap_chain_desc.BufferDesc.Format = OUTPUT_FORMAT;
			}
			swap_chain_desc.BufferCount = 3;
			swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

			swap_chain_desc.SampleDesc.Count = 1;

			swap_chain_desc.OutputWindow = window;
			swap_chain_desc.Windowed = (output_mode == nullptr);
			swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

			swap_chain_desc.Flags = 0;
			if (sync_interval == 0)
				swap_chain_desc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

			// Create swap chain
			swap_chain_rtv.Reset();
			if (FAILED(dxgi_factory->CreateSwapChain(device.Get(), &swap_chain_desc, swap_chain.ReleaseAndGetAddressOf())))
				throw PaperPup::RuntimeError("Failed to create swap chain");

			// Create new render target view for swap chain
			CreateSwapChainRTV();
			
			// Set swap chain window association
			if (FAILED(dxgi_factory->MakeWindowAssociation(window, DXGI_MWA_NO_WINDOW_CHANGES)))
				throw PaperPup::RuntimeError("Failed to set swap chain window association");
		}

		void Win32Impl::CreateSwapChainRTV()
		{
			// Get backbuffer
			ComPtr<ID3D11Texture2D> backbuffer;
			if (FAILED(swap_chain->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()))))
				throw PaperPup::RuntimeError("Failed to get backbuffer");

			// Get backbuffer and render target view desc
			D3D11_TEXTURE2D_DESC backbuffer_desc;
			backbuffer->GetDesc(&backbuffer_desc);

			CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc(D3D11_RTV_DIMENSION_TEXTURE2D, backbuffer_desc.Format, 0, 0, backbuffer_desc.ArraySize);

			// Create render target view
			if (FAILED(device->CreateRenderTargetView(backbuffer.Get(), &rtv_desc, swap_chain_rtv.ReleaseAndGetAddressOf())))
				throw PaperPup::RuntimeError("Failed to create render target view");
		}

		void Win32Impl::Resize()
		{
			// Check swap chain
			if (swap_chain == nullptr)
				return;

			// Resize swap chain and render target view to window
			swap_chain_rtv.Reset();
			if (FAILED(swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, (sync_interval == 0) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0)))
				throw PaperPup::RuntimeError("Failed to set resize swap chain");
			CreateSwapChainRTV();

			// Get new swap chain description
			DXGI_SWAP_CHAIN_DESC swap_chain_desc;
			if (FAILED(swap_chain->GetDesc(&swap_chain_desc)))
				throw PaperPup::RuntimeError("Failed to get swap chain description");

			// Set viewport
			D3D11_VIEWPORT viewport = {};

			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = (FLOAT)swap_chain_desc.BufferDesc.Width;
			viewport.Height = (FLOAT)swap_chain_desc.BufferDesc.Height;

			device_context->RSSetViewports(1, &viewport);
		}

		void Win32Impl::SetWindow(unsigned int width, unsigned int height)
		{
			// Get containing output
			ComPtr<IDXGIOutput> dxgi_output;
			if (FAILED(swap_chain->GetContainingOutput(dxgi_output.GetAddressOf())))
				throw PaperPup::RuntimeError("Failed to get containing output");

			// Get output monitor
			DXGI_OUTPUT_DESC output_desc;
			if (FAILED(dxgi_output->GetDesc(&output_desc)))
				throw PaperPup::RuntimeError("Failed to get output description");

			// Get monitor info
			MONITORINFO monitor_info;
			monitor_info.cbSize = sizeof(MONITORINFO);
			if (!GetMonitorInfo(output_desc.Monitor, &monitor_info))
				throw PaperPup::RuntimeError("Failed to get monitor info");

			// Get new window rect
			RECT window_rect;
			window_rect.left = monitor_info.rcWork.left + (monitor_info.rcWork.right - monitor_info.rcWork.left - (LONG)width) / 2;
			window_rect.top = monitor_info.rcWork.top + (monitor_info.rcWork.bottom - monitor_info.rcWork.top - (LONG)height) / 2;
			window_rect.right = window_rect.left + width;
			window_rect.bottom = window_rect.top + height;
			AdjustWindowRect(&window_rect, GetWindowLongW(window, GWL_STYLE), FALSE);

			// Update window position and show
			SetWindowPos(window, nullptr, window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, 0);
			ShowWindow(window, SW_NORMAL);
		}

		bool Win32Impl::IsFullscreen()
		{
			// Get fullscreen state from swap chain
			BOOL result;
			if (swap_chain == nullptr || FAILED(swap_chain->GetFullscreenState(&result, nullptr)))
				return false;
			return result;
		}

		void Win32Impl::SetFullscreen(bool fullscreen)
		{
			if (fullscreen)
			{
				// Get containing output
				ComPtr<IDXGIOutput> dxgi_output;
				if (FAILED(swap_chain->GetContainingOutput(dxgi_output.GetAddressOf())))
					throw PaperPup::RuntimeError("Failed to get containing output");

				// Get current swap chain description
				DXGI_SWAP_CHAIN_DESC current_desc;
				if (FAILED(swap_chain->GetDesc(&current_desc)))
					throw PaperPup::RuntimeError("Failed to get swap chain description");

				// Get output monitor
				DXGI_OUTPUT_DESC output_desc;
				if (FAILED(dxgi_output->GetDesc(&output_desc)))
					throw PaperPup::RuntimeError("Failed to get output description");

				// Get monitor info
				MONITORINFO monitor_info;
				monitor_info.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(output_desc.Monitor, &monitor_info);

				// Get closest display mode to our requested resolution
				DXGI_MODE_DESC new_mode = current_desc.BufferDesc;
				new_mode.Width = monitor_info.rcWork.right - monitor_info.rcWork.left;
				new_mode.Height = monitor_info.rcWork.bottom - monitor_info.rcWork.top;
				new_mode.RefreshRate.Numerator = 0;
				new_mode.RefreshRate.Denominator = 0;

				DXGI_MODE_DESC closest_mode;
				if (FAILED(dxgi_output->FindClosestMatchingMode(&new_mode, &closest_mode, nullptr)) || new_mode.Format != current_desc.BufferDesc.Format)
					throw PaperPup::RuntimeError("Failed to get closest display mode");

				// Recreate swap chain
				CreateSwapChain(&closest_mode);
			}
			else
			{
				// Recreate swap chain
				CreateSwapChain(nullptr);
			}
		}

		void Win32Impl::SetSyncInterval(unsigned int interval)
		{
			// Set sync state
			sync_interval = interval;

			if (IsFullscreen())
			{
				// Get current swap chain description
				DXGI_SWAP_CHAIN_DESC current_desc;
				if (FAILED(swap_chain->GetDesc(&current_desc)))
					throw PaperPup::RuntimeError("Failed to get swap chain description");

				// Recreate swap chain
				CreateSwapChain(&current_desc.BufferDesc);
			}
			else
			{
				// Recreate swap chain
				CreateSwapChain(nullptr);
			}
		}

		void Win32Impl::StartFrame()
		{
			// Prepare swap chain
			
		}

		void Win32Impl::EndFrame()
		{
			// Present swap chain
			if (sync_interval == 0)
				swap_chain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
			else
				swap_chain->Present(sync_interval, 0);
		}

		// Render interface
		void SetWindow(unsigned int width, unsigned int height)
		{
			g_win32_impl->render->SetWindow(width, height);
		}

		bool IsFullscreen()
		{
			return g_win32_impl->render->IsFullscreen();
		}

		void SetFullscreen(bool fullscreen)
		{
			g_win32_impl->render->SetFullscreen(fullscreen);
		}

		void SetSyncInterval(unsigned int interval)
		{
			g_win32_impl->render->SetSyncInterval(interval);
		}

		void StartFrame()
		{
			g_win32_impl->render->StartFrame();
		}

		void EndFrame()
		{
			g_win32_impl->render->EndFrame();
		}
	}
}
