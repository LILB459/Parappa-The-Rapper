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

		static constexpr DXGI_FORMAT TEXTURE_FORMAT = DXGI_FORMAT_R16_UINT;
		static constexpr size_t TEXTURE_PIXELWIDTH = 2;

		// Texture class
		class Texture_Impl : public Texture
		{
			private:
				// D3D11 objects
				template<typename T>
				using ComPtr = Microsoft::WRL::ComPtr<T>;

				ComPtr<ID3D11Texture2D> texture;
				ComPtr<ID3D11ShaderResourceView> texture_srv;
				ComPtr<ID3D11RenderTargetView> texture_rtv;
				

			public:
				// Texture interface
				Texture_Impl()
				{

				}

				Texture_Impl(unsigned int bind, unsigned int w, unsigned int h, const void *data)
				{
					// Image texture
					Image(bind, w, h, data);
				}

				~Texture_Impl()
				{

				}

				void Image(unsigned int bind, unsigned int w, unsigned int h, const void *data)
				{
					// Reset pointers
					texture_srv.Reset();
					texture_rtv.Reset();
					texture.Reset();

					// Create image description
					CD3D11_TEXTURE2D_DESC desc(TEXTURE_FORMAT, w, h, 1, 1,
						((bind & TextureBind::Resource) ? D3D11_BIND_SHADER_RESOURCE : 0) | ((bind & TextureBind::Target) ? D3D11_BIND_RENDER_TARGET : 0),
						(bind & TextureBind::Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT, (bind & TextureBind::Dynamic) ? D3D11_CPU_ACCESS_WRITE : 0,
						1, 0, 0
					);

					// Create subresource
					D3D11_SUBRESOURCE_DATA srd;
					srd.pSysMem = data;
					srd.SysMemSlicePitch = (srd.SysMemPitch = (w * TEXTURE_PIXELWIDTH)) * h;

					// Create texture
					if (FAILED(g_impl->render->device->CreateTexture2D(&desc, data ? &srd : nullptr, texture.GetAddressOf())))
						throw PaperPup::RuntimeError("Failed to create D3D11 texture");

					// Create shader render view
					if (bind & TextureBind::Resource)
					{
						CD3D11_SHADER_RESOURCE_VIEW_DESC srv_desc(D3D11_SRV_DIMENSION_TEXTURE2D, TEXTURE_FORMAT, 0, 1, 0, 1);
						if (FAILED(g_impl->render->device->CreateShaderResourceView(texture.Get(), &srv_desc, texture_srv.GetAddressOf())))
							throw PaperPup::RuntimeError("Failed to create texture shader render view");
					}

					// Create texture render view
					if (bind & TextureBind::Target)
					{
						CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc(D3D11_RTV_DIMENSION_TEXTURE2D, TEXTURE_FORMAT, 0, 0, 1);
						if (FAILED(g_impl->render->device->CreateRenderTargetView(texture.Get(), &rtv_desc, texture_rtv.GetAddressOf())))
							throw PaperPup::RuntimeError("Failed to create texture render view");
					}
				}

				void SubImage(unsigned int x, unsigned int y, unsigned int w, unsigned int h, const void *data)
				{
					// Map texture subresource
					D3D11_MAPPED_SUBRESOURCE texture_subresource;
					if (FAILED(g_impl->render->device_context->Map(texture.Get(), 0, D3D11_MAP_WRITE, 0, &texture_subresource)))
						throw PaperPup::RuntimeError("Failed to map texture subresource");

					// Copy into texture area
					const char *inp = (char*)data;
					char *outp = (char*)texture_subresource.pData + (y * texture_subresource.RowPitch) + (x * TEXTURE_PIXELWIDTH);

					for (unsigned int i = 0; i < h; i++)
					{
						std::memcpy(outp, inp, w * TEXTURE_PIXELWIDTH);
						outp += texture_subresource.RowPitch;
						inp += (w * TEXTURE_PIXELWIDTH);
					}

					// Unmap texture subresource
					g_impl->render->device_context->Unmap(texture.Get(), 0);
				}
		};

		Texture *Texture::New()
		{
			// Create new implementation texture
			return new Texture_Impl();
		}

		Texture *Texture::New(unsigned int bind, unsigned int w, unsigned int h, const void *data)
		{
			// Create new implementation texture
			return new Texture_Impl(bind, w, h, data);
		}

		// Win32 implementation interface
		Impl::Impl(PaperPup::Impl &impl)
		{
			// Define window class
			WNDCLASSEXW window_class = {};
			window_class.cbSize = sizeof(WNDCLASSEX);

			window_class.style = CS_HREDRAW | CS_VREDRAW;
			window_class.lpfnWndProc = Input::Impl::WindowProc;
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

		Impl::~Impl()
		{
			// Ensure swap chain is windowed before we exit
			if (swap_chain != nullptr)
				swap_chain->SetFullscreenState(FALSE, nullptr);

			// Destroy window
			if (window != nullptr)
				DestroyWindow(window);
		}

		void Impl::CreateSwapChain(const DXGI_MODE_DESC *output_mode)
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
			if (sync_tearing_enabled)
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

		void Impl::CreateSwapChainRTV()
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

		void Impl::Resize()
		{
			// Check swap chain
			if (swap_chain == nullptr)
				return;

			// Resize swap chain and render target view to window
			swap_chain_rtv.Reset();
			if (FAILED(swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, (sync_tearing_enabled) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0)))
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

		void Impl::SetWindow(unsigned int width, unsigned int height)
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

		bool Impl::IsFullscreen()
		{
			// Get fullscreen state from swap chain
			BOOL result;
			if (swap_chain == nullptr || FAILED(swap_chain->GetFullscreenState(&result, nullptr)))
				return false;
			return result;
		}

		void Impl::SetFullscreen(bool fullscreen)
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

		void Impl::SetSync(bool limiter_enabled, unsigned int limiter, bool tearing_enabled, bool vsync_enabled)
		{
			// Set sync state
			sync_limiter_enabled = limiter_enabled && (limiter != 0) && !vsync_enabled;
			sync_limiter = limiter;

			sync_limiter_tick = (double)GetTickCount64();

			sync_tearing_enabled = tearing_enabled && !vsync_enabled;
			sync_vsync_enabled = vsync_enabled;

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

		void Impl::StartFrame()
		{
			// Prepare swap chain
			
		}

		void Impl::EndFrame()
		{
			// Present swap chain
			if (sync_vsync_enabled)
			{
				// Present with vsync
				swap_chain->Present(1, 0);
			}
			else
			{
				// Present without vsync
				swap_chain->Present(0, sync_tearing_enabled ? DXGI_PRESENT_ALLOW_TEARING : 0);

				// Perform frame limiting
				if (sync_limiter_enabled)
				{
					// Get time of a limiter tick
					const double limiter_time = (1000.0 / (double)sync_limiter);

					// Correct out of sync ticker
					if ((double)GetTickCount64() > (sync_limiter_tick + (limiter_time * 2.0)))
						sync_limiter_tick = (double)GetTickCount64();

					// Sleep until next tick
					sync_limiter_tick += limiter_time;
					while ((double)GetTickCount64() < sync_limiter_tick)
						Sleep(1);
				}
			}
		}

		// Render interface
		void SetWindow(unsigned int width, unsigned int height)
		{
			// Set implementation window
			g_impl->render->SetWindow(width, height);
		}

		bool IsFullscreen()
		{
			// Return implementation fullscreen
			return g_impl->render->IsFullscreen();
		}

		void SetFullscreen(bool fullscreen)
		{
			// Set implementation fullscreen
			g_impl->render->SetFullscreen(fullscreen);
		}

		void SetSync(bool limiter_enabled, unsigned int limiter, bool tearing_enabled, bool vsync_enabled)
		{
			// Set implementation sync
			g_impl->render->SetSync(limiter_enabled, limiter, tearing_enabled, vsync_enabled);
		}

		void StartFrame()
		{
			// Start implementation frame
			g_impl->render->StartFrame();
		}

		void EndFrame()
		{
			// End implementation frame
			g_impl->render->EndFrame();
		}
	}
}
