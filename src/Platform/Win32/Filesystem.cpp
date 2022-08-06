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

#include "Platform/Common/Mode2.h"
#include "Platform/Common/Binary.h"
#include "Platform/Common/IntArchive.h"

#include <algorithm>
#include <functional>

namespace PaperPup
{
	namespace Filesystem
	{
		// Win32 filesystem helpers
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

		static bool FileExists(std::wstring name)
		{
			DWORD attrib = GetFileAttributesW(name.c_str());
			return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
		}

		static bool DirectoryExists(std::wstring name)
		{
			DWORD attrib = GetFileAttributesW(name.c_str());
			return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
		}

		static void DirectoryIterate(std::wstring name, std::function<void(WIN32_FIND_DATAW &file_data)> iter)
		{
			WIN32_FIND_DATAW find_data;
			HANDLE handle_find = FindFirstFileW(name.c_str(), &find_data);
			if (handle_find != INVALID_HANDLE_VALUE)
			{
				do
				{
					iter(find_data);
				} while (FindNextFileW(handle_find, &find_data));
				FindClose(handle_find);
			}
		}

		// Win32 implementation interface
		Win32Impl::Win32Impl()
		{
			// Get module path
			module_path = GetModulePath();
		}

		Win32Impl::~Win32Impl()
		{

		}

		// Image interface
		class Binary_Win32Impl : public Binary
		{
			private:
				// Binary handle
				HANDLE handle_bin;

			public:
				// Binary interface
				Binary_Win32Impl(HANDLE _handle_bin): handle_bin(_handle_bin)
				{
					// Parse binary directory
					ParseDirectory();
				}

				~Binary_Win32Impl()
				{
					// Close binary file
					CloseHandle(handle_bin);
				}

				// Binary implementation
				void SeekLBA(uint32_t lba) override
				{
					// Seek to LBA in file
					DWORD result = SetFilePointer(handle_bin, lba * SECTOR_MODE2, nullptr, FILE_BEGIN);
					if (result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
						throw PaperPup::RuntimeError("Binary seek failed");
				}

				void ReadSector(char *data, uint32_t count) override
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
		};

		class Archive_Win32Impl : public Archive
		{
			private:
				// Archive path and file
				std::wstring path_archive;
				std::unique_ptr<IntArchive> archive;
				
			public:
				// Archive interface
				Archive_Win32Impl(Image *image, std::string name)
				{
					// Get path
					path_archive = g_win32_impl->filesystem->module_path + Win32::UTF8ToWide(name) + L"\\";

					// Open archive file
					File *file_archive = image->OpenFile(name + ".INT", false);
					if (file_archive != nullptr)
						archive = std::make_unique<IntArchive>(file_archive);
				}

				~Archive_Win32Impl() override
				{

				}

				File *OpenFile(std::string name) override
				{
					// Try to open from folder
					std::wstring path_file = path_archive + Win32::UTF8ToWide(name);
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
							return new File(data, file_size);
					}

					// Try to open from archive
					if (archive != nullptr)
					{
						File *file;
						if ((file = archive->OpenFile(name)) != nullptr)
							return file;
					}

					return nullptr;
				}
		};

		class Image_Win32Impl : public Image
		{
			public:
				// Image path and binary
				std::wstring path_image;
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

				Archive *OpenArchive(std::string name)
				{
					// Open archive
					return new Archive_Win32Impl(this, name);
				}

				File *OpenFile(std::string name, bool mode2) override
				{
					// Try to open from folder
					std::wstring path_file = path_image + Win32::UTF8ToWide(name);
					std::replace(path_file.begin(), path_file.end(), '/', '\\');

					HANDLE handle_file = CreateFileW(path_file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
					if (handle_file != INVALID_HANDLE_VALUE)
					{
						// Allocate file buffer
						DWORD file_size = GetFileSize(handle_file, nullptr);
						if (file_size <= 0)
							return nullptr;
						char *data = new char[file_size];

						// Read file contents
						DWORD result;
						BOOL read_result = ReadFile(handle_file, data, file_size, &result, nullptr);
						CloseHandle(handle_file);

						if (read_result == FALSE || result != file_size)
							return nullptr;

						// Ensure mode 2 data is mode 2
						size_t data_size = file_size;
						if (mode2)
							InsureMode2(&data, &data_size);

						// Return file
						return new File(data, file_size);
					}

					// Try to open file binary
					if (binary != nullptr)
					{
						File *file;
						if ((file = binary->OpenFile(name, mode2)) != nullptr)
							return file;
					}

					// Failed to open file
					return nullptr;
				}
		};

		Image *Image::Open(std::string name)
		{
			// Open image
			return new Image_Win32Impl(name);
		}

		// Filesystem functions
		std::vector<std::string> GetPackList()
		{
			// Get packs folder
			std::wstring path_packs = g_win32_impl->filesystem->module_path + L"Packs";
			if (!DirectoryExists(path_packs))
				throw PaperPup::RuntimeError("Packs folder doesn't exist");

			// Iterate packs folder
			std::vector<std::string> packs;
			DirectoryIterate(path_packs + L"\\*", [&](WIN32_FIND_DATAW &file_data)
			{
				// Check if file is a directory
				if ((file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(file_data.dwFileAttributes & FILE_ATTRIBUTE_PINNED))
				{
					// Check if folder is individual
					std::wstring pack_name = std::wstring(file_data.cFileName);
					if (pack_name == L"." || pack_name == L"..")
						return;

					// Add to list
					packs.push_back(Win32::WideToUTF8(pack_name));
				}
			});
			return packs;
		}
	}
}