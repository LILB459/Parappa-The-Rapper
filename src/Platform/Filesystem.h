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
		// Image helpers
		static uint16_t Read16(char *data) { return (((uint16_t)((uint8_t)data[0])) << 0) | (((uint16_t)((uint8_t)data[1])) << 8); }
		static uint32_t Read32(char *data) { return (((uint32_t)((uint8_t)data[0])) << 0) | (((uint32_t)((uint8_t)data[1])) << 8) | (((uint32_t)((uint8_t)data[2])) << 16) | (((uint32_t)((uint8_t)data[3])) << 24); }

		// Image class
		class Archive;
		class File;

		class Image
		{
			public:
				// Image interface
				static std::unique_ptr<Image> Open(std::string name);
				virtual ~Image() {}

				virtual std::unique_ptr<Archive> OpenArchive(std::string name) = 0;
				virtual std::unique_ptr<File> OpenFile(std::string name, bool mode2) = 0;
		};

		// Archive class
		class Archive
		{
			public:
				// Archive interface
				virtual ~Archive() {}

				virtual std::unique_ptr<File> OpenFile(std::string name) = 0;
		};
		
		// File class
		class File
		{
			private:
				// Data
				std::unique_ptr<char[]> data;
				size_t cursor = 0, size;

			public:
				// File interface
				File(char *_data, size_t _size) : data(_data), size(_size) {}
				~File() {}

				size_t Size() const
				{
					return size;
				}

				bool Seek(size_t pos)
				{
					if (pos > size)
						return false;
					cursor = pos;
					return true;
				}

				size_t Tell() const
				{
					return cursor;
				}

				size_t Read(char *buffer, size_t length)
				{
					// Check if length is in bounds
					if (length > size || cursor > (size - length))
					{
						if (cursor >= size)
							return 0;
						length = size - cursor;
					}

					// Copy to buffer
					std::memcpy(buffer, data.get() + cursor, length);
					cursor += length;
					return length;
				}

				std::unique_ptr<char[]> Dup() const
				{
					// Create new buffer with file contents
					std::unique_ptr<char[]> dup = std::make_unique<char[]>(size);
					std::memcpy(dup.get(), data.get(), size);
					return dup;
				}
		};

		// Filesystem functions
		std::vector<std::string> GetPackList();
	}
}
