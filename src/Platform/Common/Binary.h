/*
 * [PaperPup]
 *   Binary.h
 * Author(s): Regan Green
 * Date: 07/30/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Filesystem.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace PaperPup
{
	namespace Filesystem
	{
		// Binary constants
		static constexpr unsigned int SECTOR_MODE1 = 2048;
		static constexpr unsigned int SECTOR_MODE2 = 2352;

		// Binary class
		struct Binary_Directory
		{
			uint32_t lba, size;
		};
		
		class Binary
		{
			private:
				// File directory
				std::unordered_set<uint32_t> directories;
				std::unordered_map<std::string, Binary_Directory> directory;

			public:
				// Binary interface
				virtual ~Binary() {}

				std::unique_ptr<File> OpenFile(std::string name, bool mode2)
				{
					// Get directory
					auto dir = directory.find(name);
					if (dir == directory.end())
						return nullptr;

					// Read sectors
					if (mode2)
					{
						// Use raw output
						uint32_t sectors = (dir->second.size + 0x7FF) / SECTOR_MODE1;
						char *data = new char[sectors * SECTOR_MODE2];

						SeekLBA(dir->second.lba);
						ReadSector(data, sectors);

						return std::make_unique<File>(data, sectors * SECTOR_MODE2);
					}
					else
					{
						// Use only data part
						uint32_t sectors = (dir->second.size + 0x7FF) / SECTOR_MODE1;
						char *data = new char[sectors * SECTOR_MODE1];
						char *datap = data;

						SeekLBA(dir->second.lba);
						for (uint32_t i = 0; i < sectors; i++)
						{
							char sector[SECTOR_MODE2];
							ReadSector(sector, 1);
							std::memcpy(datap, sector + 0x018, SECTOR_MODE1);
							datap += SECTOR_MODE1;
						}

						return std::make_unique<File>(data, dir->second.size);
					}
					return nullptr;
				}

				void ParseDirectory()
				{
					// Primary volume descriptor data
					bool found_primary_volume = false;
					uint32_t directory_lba;

					// Read volume descriptors
					SeekLBA(0x10); // First 16 sectors is the system area, unused by our images

					while (1)
					{
						// Read sector
						char sector[SECTOR_MODE2];
						ReadSector(sector, 1);
						char *sector_data = sector + 0x018;

						// Check volume descriptor type
						if (std::memcmp("CD001", sector_data + 0x001, 5))
							throw PaperPup::RuntimeError("Binary invalid volume descriptor");

						uint8_t volume_type = sector_data[0x000];
						if (volume_type == 0xFF) // Terminator
							break;

						switch (volume_type)
						{
							case 1: // Primary Volume Descriptor
								// Read volume descriptor data
								found_primary_volume = true;
								directory_lba = Read32(sector_data + 0x09E);
								break;
						}
					}
					if (!found_primary_volume)
						throw PaperPup::RuntimeError("Binary missing primary volume descriptor");

					// Read directories
					ReadDirectory(directory_lba, "");
				}

				void ReadDirectory(uint32_t lba, std::string name)
				{
					// Only iterate through each directory once
					if (directories.find(lba) != directories.end())
						return;
					directories.emplace(lba);

					// Read sector
					char sector[SECTOR_MODE2];
					SeekLBA(lba);
					ReadSector(sector, 1);
					char *sector_data = sector + 0x018;

					// Read directories
					char *directory_data = sector_data;
					while (1)
					{
						// Read directory
						uint8_t dir_length = directory_data[0x000];
						if (dir_length == 0 || ((directory_data - sector_data) + dir_length) >= SECTOR_MODE1)
							break;
						uint32_t dir_lba = Read32(directory_data + 0x002);
						uint32_t dir_size = Read32(directory_data + 0x00A);
						uint8_t dir_flags = directory_data[0x019];

						std::string dir_name(directory_data + 0x21, directory_data[0x20]);

						if (dir_flags & (1 << 1)) // Is directory
						{
							// Read directory
							ReadDirectory(dir_lba, name + dir_name + "/");
						}
						else
						{
							// Emplace file
							size_t dir_colon = dir_name.find_last_of(";");
							if (dir_colon != std::string::npos)
								directory.emplace(std::make_pair<std::string, Binary_Directory>(name + dir_name.substr(0, dir_colon), { dir_lba, dir_size }));
							else
								directory.emplace(std::make_pair<std::string, Binary_Directory>(name + dir_name, { dir_lba, dir_size }));
						}

						// Go to next directory and make sure we have enough space
						directory_data += dir_length;
						if (((directory_data - sector_data) + 1) >= SECTOR_MODE1)
							break;
					}
				}

				// Binary implementation
				virtual void SeekLBA(uint32_t lba) = 0;
				virtual void ReadSector(char *data, uint32_t count) = 0;
		};
	}
}