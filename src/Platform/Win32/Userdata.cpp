/*
 * [PaperPup]
 *   Userdata.h
 * Author(s): Regan Green
 * Date: 08/06/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Platform/Win32/Userdata.h"

#include "Platform/Win32/Filesystem.h"

namespace PaperPup
{
	namespace Userdata
	{
		// Win32 implementation interface
		Win32Impl::Win32Impl(PaperPup::Win32Impl &win32_impl)
		{
			// Open userdata file
			std::wstring path_userdata = win32_impl.filesystem->module_path + L"Userdata.bin";

			HANDLE handle_userdata = CreateFileW(path_userdata.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
			if (handle_userdata != INVALID_HANDLE_VALUE)
			{
				// Allocate file buffer
				DWORD userdata_size = GetFileSize(handle_userdata, nullptr);
				if (userdata_size <= 0)
					return;
				std::vector<char> userdata_data((size_t)userdata_size);

				// Read file contents
				DWORD result;
				BOOL read_result = ReadFile(handle_userdata, userdata_data.data(), (DWORD)userdata_data.size(), &result, nullptr);
				CloseHandle(handle_userdata);

				if (read_result == FALSE || result != userdata_data.size())
					return;

				// Deserialize userdata
				userdata.Deserialize(userdata_data);
			}
		}

		Win32Impl::~Win32Impl()
		{
			// Open userdata file
			std::wstring path_userdata = g_win32_impl->filesystem->module_path + L"Userdata.bin";

			HANDLE handle_userdata = CreateFileW(path_userdata.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
			if (handle_userdata != INVALID_HANDLE_VALUE)
			{
				// Serialize userdata
				std::vector<char> userdata_data = userdata.Serialize();

				// Write userdata to file
				WriteFile(handle_userdata, userdata_data.data(), (DWORD)userdata_data.size(), nullptr, nullptr);
				CloseHandle(handle_userdata);
			}
		}

		// Userdata interface
		void Set(std::string key, std::string value)
		{
			// Set in implementation
			g_win32_impl->userdata->userdata.Set(key, value);
		}

		std::string Get(std::string key)
		{
			// Get from implementation
			return g_win32_impl->userdata->userdata.Get(key);
		}

		bool Exists(std::string key)
		{
			// Get from implementation
			return g_win32_impl->userdata->userdata.Exists(key);
		}

		void Clear(std::string key)
		{
			// Clear in implementation
			g_win32_impl->userdata->userdata.Clear(key);
		}
	}
}
