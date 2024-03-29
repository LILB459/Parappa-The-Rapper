/*
 * [PaperPup]
 *   Win32.h
 * Author(s): Regan Green
 * Date: 07/26/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl/client.h>

#include <memory>
#include <string>

namespace PaperPup
{
	namespace Win32
	{
		// Text conversion functions
		static std::string WideToUTF8(std::wstring string)
		{
			// Calculate char count
			int chars = WideCharToMultiByte(CP_UTF8, 0, string.data(), -1, nullptr, 0, nullptr, nullptr);
			if (chars <= 0)
				throw PaperPup::RuntimeError("Failed to get UTF-8 string length");

			// Convert string
			std::string result(chars, '\0');
			if (WideCharToMultiByte(CP_UTF8, 0, string.data(), -1, &result[0], chars, nullptr, nullptr) <= 0)
				throw PaperPup::RuntimeError("Failed to get UTF-8 string data");
			
			return result.substr(0, (std::string::size_type)chars - 1);
		}
		static std::wstring UTF8ToWide(std::string string)
		{
			// Calculate char count
			int chars = MultiByteToWideChar(CP_UTF8, 0, string.data(), -1, nullptr, 0);
			if (chars <= 0)
				throw PaperPup::RuntimeError("Failed to get UTF-16 string length");

			// Convert string
			std::wstring result(chars, '\0');
			if (MultiByteToWideChar(CP_UTF8, 0, string.data(), -1, &result[0], chars) <= 0)
				throw PaperPup::RuntimeError("Failed to get UTF-16 string data");
			
			return result.substr(0, (std::string::size_type)chars - 1);
		}
	}
}