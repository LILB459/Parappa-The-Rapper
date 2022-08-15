/*
 * [PaperPup]
 *   Win32.cpp
 * Author(s): Regan Green
 * Date: 07/26/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Platform/Win32/Win32.h"

#include "Platform/Win32/Filesystem.h"
#include "Platform/Win32/Userdata.h"
#include "Platform/Win32/Render.h"
#include "Platform/Win32/Input.h"

#include "Platform/Miniaudio/Audio.h"

#include <shellapi.h>

// Prioritize High-Performance Adapters
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
extern "C" { _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
// https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
extern "C" { _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; }

namespace PaperPup
{
	// Win32 implementation
	Impl *g_impl = nullptr;

	Impl::Impl()
	{
		// Initialize systems
		filesystem = std::make_unique<Filesystem::Impl>(*this);
		userdata = std::make_unique<Userdata::Impl>(*this);
		render = std::make_unique<Render::Impl>(*this);
		audio = std::make_unique<Audio::Impl>();
		input = std::make_unique<Input::Impl>(*this);
	}

	Impl::~Impl()
	{
		
	}

	// Error display
	void DisplayError(std::string _error)
	{
		// Show error message box
		HWND window = nullptr;
		if (g_impl != nullptr)
			if (g_impl->render != nullptr)
				window = g_impl->render->window;
		MessageBoxW(window, (L"PaperPup Runtime Error:\n" + Win32::UTF8ToWide(_error)).c_str(), L"PaperPup", MB_ICONERROR);
	}
}

// Program entry point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	// Initialize COM
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
		return 1;

	// Get argument list
	int argc;
	LPWSTR *wargv = CommandLineToArgvW(lpCmdLine, &argc);
	if (wargv == nullptr)
		return 1;

	std::vector<std::string> args;
	for (int argi = 0; argi < argc; argi++)
		args.push_back(PaperPup::Win32::WideToUTF8(std::wstring(wargv[argi])));
	
	LocalFree(wargv);

	// Run PaperPup engine
	int result = -1;

	try
	{
		// Initialize PaperPup implementation
		PaperPup::g_impl = new PaperPup::Impl();

		// Enter app facing entry point
		result = PaperPup::Main(args);

		// Delete PaperPup implementation
		delete PaperPup::g_impl;
		PaperPup::g_impl = nullptr;
	}
	catch (PaperPup::RuntimeError &exception)
	{
		// Display error
		PaperPup::DisplayError(exception.what());

		// Delete PaperPup implementation
		if (PaperPup::g_impl != nullptr)
		{
			delete PaperPup::g_impl;
			PaperPup::g_impl = nullptr;
		}
	}

	// Uninitialize COM
	CoUninitialize();

	return 0;
}
