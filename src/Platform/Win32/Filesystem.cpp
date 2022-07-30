/*
 * [PaperPup]
 *   Filesystem.h
 * Author(s): Regan Green
 * Date: 07/26/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Platform/Win32/Filesystem.h"

#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace PaperPup
{
	namespace Filesystem
	{
		// Return directory to module as wide string
		static std::wstring GetModulePath()
		{
			// Fill wide string buffer with path to executable
			std::wstring buffer(0x100, '\0');
			DWORD len = 0;

			while (1)
			{
				if ((len = GetModuleFileNameW(nullptr, &buffer[0], (DWORD)buffer.size())) < (buffer.size() - 1))
					break;
				buffer.resize(buffer.size() << 1);
			}

			if (len == 0)
				return L"";

			// Cut executable filename
			size_t path_end = buffer.find_last_of(L"/\\");
			if (path_end == std::string::npos)
				return L"";

			// Return new cut string
			return L"\\\\?\\" + buffer.substr(0, path_end + 1);
		}

		// Filesystem interface
		Win32Impl::Win32Impl()
		{
			// Get module path
			module_path = GetModulePath();
		}

		Win32Impl::~Win32Impl()
		{

		}

		// File interface
		class File_Win32Impl : public File
		{
			private:
				// Data
				std::unique_ptr<char[]> data;
				uint32_t cursor = 0, size;

			public:
				// File interface
				File_Win32Impl(char *_data, uint32_t _size): data(_data), size(_size) {}
				~File_Win32Impl() override {}

				uint32_t Size() const override
				{
					return size;
				}

				bool Seek(uint32_t pos) override
				{
					if (pos > size)
						return false;
					cursor = pos;
					return true;
				}

				uint32_t Tell() const override
				{
					return cursor;
				}

				uint32_t Read(char *buffer, uint32_t length) override
				{
					// Check if length is in bounds
					if (length > size || cursor > (length - size))
					{
						if (cursor >= size)
							return 0;
						length = size - cursor;
					}

					// Copy to buffer
					std::memcpy(buffer, data.get() + cursor, length);
					return length;
				}
		};

		// Image interface
		struct Binary_Directory
		{
			uint32_t lba, size;
		};

		class Binary_Win32Impl
		{
			private:
				// Binary handle
				HANDLE handle_bin;

				// File directory
				std::unordered_set<uint32_t> directories;
				std::unordered_map<std::string, Binary_Directory> directory;

			public:
				// Binary interface
				Binary_Win32Impl(HANDLE _handle_bin): handle_bin(_handle_bin)
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
						ReadSector(sector);
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

				~Binary_Win32Impl()
				{
					// Close binary file
					CloseHandle(handle_bin);
				}

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

						return std::make_unique<File_Win32Impl>(data, sectors * SECTOR_MODE2);
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
							ReadSector(sector);
							std::memcpy(datap, sector + 0x018, SECTOR_MODE1);
						}
						
						return std::make_unique<File_Win32Impl>(data, dir->second.size);
					}
					return nullptr;
				}

				// File helpers
				static uint16_t Read16(char *data) { return (((uint16_t)((uint8_t)data[0])) << 0) | (((uint16_t)((uint8_t)data[1])) << 8); }
				static uint32_t Read32(char *data) { return (((uint32_t)((uint8_t)data[0])) << 0) | (((uint32_t)((uint8_t)data[1])) << 8) | (((uint32_t)((uint8_t)data[2])) << 16) | (((uint32_t)((uint8_t)data[3])) << 24); }

				void SeekLBA(uint32_t lba)
				{
					// Seek to LBA in file
					DWORD result = SetFilePointer(handle_bin, lba * SECTOR_MODE2, nullptr, FILE_BEGIN);
					if (result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
						throw PaperPup::RuntimeError("Binary seek failed");
				}

				void ReadSector(char *data, uint32_t count = 1)
				{
					// Read sector from file
					DWORD request = SECTOR_MODE2 * count;
					DWORD result;
					if (ReadFile(handle_bin, data, request, &result, nullptr) == FALSE || result != request)
						throw PaperPup::RuntimeError("Binary read failed");
					
					// Check for sync bytes
					char *datap = data;
					for (uint32_t i = 0; i < count; i++)
					{
						static const unsigned char sync_bytes[12] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
						if (std::memcmp(sync_bytes, datap, 12))
							throw PaperPup::RuntimeError("Binary invalid read");
						datap += SECTOR_MODE2;
					}
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
					ReadSector(sector);
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
		};

		class Image_Win32Impl : public Image
		{
			private:
				// Folder path
				std::wstring path_image;

				// Image binary
				std::unique_ptr<Binary_Win32Impl> binary;

			public:
				// Image interface
				Image_Win32Impl(std::string name)
				{
					// Get path
					std::wstring path_name = Win32::UTF8ToWide(name);
					std::wstring path_bin = g_win32_impl->filesystem->module_path + path_name + L".bin";
					path_image = g_win32_impl->filesystem->module_path + path_name + L"\\";

					// Open binary file
					HANDLE handle_bin = CreateFileW(path_bin.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
					if (handle_bin != INVALID_HANDLE_VALUE)
						binary = std::make_unique<Binary_Win32Impl>(handle_bin);
				}

				~Image_Win32Impl() override
				{
					
				}

				std::unique_ptr<File> OpenFile(std::string name, bool mode2) override
				{
					// Try to open from folder
					std::wstring path_file = path_image + Win32::UTF8ToWide(name);
					std::replace(path_file.begin(), path_file.end(), '/', '\\');

					HANDLE handle_file = CreateFileW(path_file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
					if (handle_file != INVALID_HANDLE_VALUE)
					{
						// Allocate file buffer
						DWORD file_size = GetFileSize(handle_file, nullptr);
						char *data = new char[file_size];

						// Read file contents
						DWORD result;
						BOOL read_result = ReadFile(handle_file, data, file_size, &result, nullptr);
						CloseHandle(handle_file);

						// Return file
						if (read_result == FALSE || result != file_size)
							return nullptr;
						else
							return std::make_unique<File_Win32Impl>(data, file_size);
					}

					// Try to open file binary
					std::unique_ptr<File> file;
					if ((file = binary->OpenFile(name, mode2)) != nullptr)
						return file;

					// Failed to open file
					return nullptr;
				}
		};

		std::unique_ptr<Image> Image::Open(std::string name)
		{
			return std::make_unique<Image_Win32Impl>(name);
		}
	}
}