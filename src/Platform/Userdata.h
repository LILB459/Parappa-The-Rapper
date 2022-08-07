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

#include "Platform/Platform.h"

namespace PaperPup
{
	namespace Userdata
	{
		// Userdata interface
		void Set(std::string key, std::string value);

		std::string Get(std::string key);
		bool Exists(std::string key);

		void Clear(std::string key);

		// Userdata helpers
		static std::string GetString(std::string key, std::string def)
		{
			if (Exists(key))
			{
				// Return string value
				return Get(key);
			}

			// Set and return default
			Set(key, def);
			return def;
		}

		static double GetNumber(std::string key, double def)
		{
			if (Exists(key))
			{
				// Return double value
				std::string value = Get(key);
				try
				{
					return std::stod(value);
				}
				catch (std::invalid_argument &exception) { (void)exception; }
				catch (std::out_of_range &exception) { (void)exception; }
			}

			// Set and return default
			Set(key, std::to_string(def));
			return def;
		}

		static int GetInteger(std::string key, int def)
		{
			if (Exists(key))
			{
				// Return integer value
				std::string value = Get(key);
				try
				{
					return std::stoi(value);
				}
				catch (std::invalid_argument &exception) { (void)exception; }
				catch (std::out_of_range &exception) { (void)exception; }
			}

			// Set and return default
			Set(key, std::to_string(def));
			return def;
		}

		static bool GetBool(std::string key, bool def)
		{
			return GetInteger(key, def);
		}

		static void SetNumber(std::string key, double value)
		{
			Set(key, std::to_string(value));
		}

		static void SetInteger(std::string key, int value)
		{
			Set(key, std::to_string(value));
		}

		static void SetBool(std::string key, bool value)
		{
			SetInteger(key, value);
		}
	}
}
