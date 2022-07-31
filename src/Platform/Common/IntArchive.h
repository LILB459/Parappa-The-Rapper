/*
 * [PaperPup]
 *   IntArchive.h
 * Author(s): Regan Green
 * Date: 07/30/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Filesystem.h"

#include <unordered_map>
#include <memory>

namespace PaperPup
{
	namespace Filesystem
	{
		// Int archive constants
		static constexpr unsigned int INT_BLOCK_SIZE = 4 << 11;

		// Int archive class
		struct IntArchive_Directory
		{
			size_t offset, size;
		};

		class IntArchive
		{
			private:
				// Archive file
				std::unique_ptr<File> file;

				// File directory
				std::unordered_map<std::string, IntArchive_Directory> directory;

			public:
				// Int archive interface
				IntArchive(std::unique_ptr<File> &_file)
				{
					// Obtain file pointer
					file = std::move(_file);

					// Read blocks
					while (1)
					{
						// Read block header
						char block[INT_BLOCK_SIZE];
						if ((file->Read(block, INT_BLOCK_SIZE)) != INT_BLOCK_SIZE)
							throw PaperPup::RuntimeError("Archive failed to read block");

						uint32_t block_type = Read32(block + 0);
						uint32_t block_files = Read32(block + 4);
						uint32_t block_datasectors = Read32(block + 8);
						uint32_t block_datasize = block_datasectors << 11;

						// Check block type
						if (block_type == 0xFFFFFFFF)
							break;
						switch (block_type)
						{
							case 1: // TIM
							case 2: // VAB
							case 3: // MEM
								break;
							default:
								throw PaperPup::RuntimeError("Archive unrecognized block type");
						}

						// Index directory
						if ((0x10 + block_files * 0x14) > INT_BLOCK_SIZE)
							throw PaperPup::RuntimeError("Archive block directory too large");

						char *dirp = block + 0x10;
						size_t file_offset = 0;

						for (uint32_t i = 0; i < block_files; i++)
						{
							// Read directory header
							size_t dir_size = Read32(dirp + 0);

							char dir_name[0x11];
							std::memcpy(dir_name, dirp + 4, 0x10);
							dir_name[0x10] = '\0';

							if (dir_size > block_datasize || file_offset > (block_datasize - dir_size))
								throw PaperPup::RuntimeError("Archive block data doesn't fit in allocated size");
							
							// Emplace directory
							directory.emplace(std::make_pair<std::string, IntArchive_Directory>(dir_name, { file->Tell() + file_offset, dir_size }));

							// Index next file
							dirp += 0x14;

							file_offset += dir_size;
							if (dir_size & 3)
								file_offset += (4 - (dir_size & 3));
						}
						
						// Seek past data
						file->Seek(file->Tell() + block_datasize);
					}
				}

				~IntArchive()
				{

				}

				std::unique_ptr<File> OpenFile(std::string name)
				{
					// Get directory
					auto dir = directory.find(name);
					if (dir == directory.end())
						return nullptr;

					// Read file data
					char *data = new char[dir->second.size];
					if (file->Seek(dir->second.offset) == false || file->Read(data, dir->second.size) != dir->second.size)
						throw PaperPup::RuntimeError("Archive failed to read file data");

					return std::make_unique<File>(data, dir->second.size);
				}
		};
	}
}
