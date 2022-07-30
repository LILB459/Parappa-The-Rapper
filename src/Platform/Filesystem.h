/*
 * [PaperPup]
 *   Filesystem.h
 * Author(s): Regan Green
 * Date: 07/26/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Platform.h"

#include <memory>

namespace PaperPup
{
	namespace Filesystem
	{
		// Filesystem constants
		static constexpr unsigned int SECTOR_MODE1 = 2048;
		static constexpr unsigned int SECTOR_MODE2 = 2352;

		// Image and file classes
		class File;

		class Image
		{
			public:
				// Image interface
				static std::unique_ptr<Image> Open(std::string name);
				virtual ~Image() {}

				virtual std::unique_ptr<File> OpenFile(std::string name, bool mode2) = 0;
		};

		class File
		{
			public:
				// File interface
				virtual ~File() {}

				virtual uint32_t Size() const = 0;

				virtual bool Seek(uint32_t pos) = 0;
				virtual uint32_t Tell() const = 0;

				virtual uint32_t Read(char *buffer, uint32_t length) = 0;
		};
	}
}
