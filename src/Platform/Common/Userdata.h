/*
 * [PaperPup]
 *   Userdata.h
 * Author(s): Regan Green
 * Date: 08/06/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Platform/Filesystem.h"

namespace PaperPup
{
	namespace Userdata
	{
		// Userdata class
		class Userdata
		{
			private:
				// Userdata
				std::unordered_map<std::string, std::string> userdata;

			public:
				// Userdata interface
				void Set(std::string key, std::string value)
				{
					// Set userdata value
					if (key.size() != 0 && key.back() != '/')
						userdata[key] = value;
				}

				std::string Get(std::string key)
				{
					if (key.size() != 0 && key.back() != '/')
						return userdata[key];
					return "";
				}

				bool Exists(std::string key)
				{
					if (key.size() != 0 && key.back() != '/')
						return userdata.find(key) != userdata.end();
					return false;
				}

				void Clear(std::string key)
				{
					if (key.size() == 0)
					{
						// Clear userdata
						userdata.clear();
					}
					else if (key.back() != '/')
					{
						// Remove single value
						userdata.erase(key);
					}
					else
					{
						// Clear all values that are below our key
						for (auto i = userdata.begin(); i != userdata.end();)
						{
							if (i->first.rfind(key) == 0)
								i = userdata.erase(i);
							else
								i++;
						}
					}
				}

				std::vector<char> Serialize()
				{
					// Write userdata values to buffer
					std::vector<char> userdata_data;
					for (auto &i : userdata)
					{
						std::string key = i.first;
						uint32_t key_size = (uint32_t)key.size();
						userdata_data.push_back(key_size >> 0); userdata_data.push_back(key_size >> 8); userdata_data.push_back(key_size >> 16); userdata_data.push_back(key_size >> 24);

						std::string value = i.second;
						uint32_t value_size = (uint32_t)value.size();
						userdata_data.push_back(value_size >> 0); userdata_data.push_back(value_size >> 8); userdata_data.push_back(value_size >> 16); userdata_data.push_back(value_size >> 24);

						userdata_data.insert(userdata_data.end(), key.begin(), key.end());
						userdata_data.insert(userdata_data.end(), value.begin(), value.end());
					}
					return userdata_data;
				}

				void Deserialize(std::vector<char> &data)
				{
					// Deserialize data
					char *userdatap = data.data();
					char *userdata_end = userdatap + data.size();

					while ((userdata_end - userdatap) >= 8)
					{
						// Read userdata key and value
						uint32_t key_length = Filesystem::Read32(userdatap + 0);
						uint32_t value_length = Filesystem::Read32(userdatap + 4);

						char *userdata_next = userdatap + 8 + key_length + value_length;
						if (userdata_next > userdata_end)
							break;

						// Set userdata
						std::string key(userdatap + 8, key_length);
						std::string value(userdatap + 8 + key_length, value_length);
						Set(key, value);

						// Read next userdata
						userdatap = userdata_next;
					}
				}
		};
	}
}
