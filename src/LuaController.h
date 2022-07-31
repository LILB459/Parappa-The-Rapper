/*
 * [PaperPup]
 *   LuaController.h
 * Author(s): Regan Green
 * Date: 07/05/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "Platform/Platform.h"

#include "Platform/Filesystem.h"

#include <lua.h>
#include <lualib.h>

#include <string>
#include <iostream>

namespace PaperPup
{
	namespace Lua
	{
		// Lua helper functions
		static void DumpStack(lua_State *state)
		{
			#ifdef NDEBUG
				(void)state;
			#else
				// Dump marker
				std::cout << "DumpStack(" << (void*)state << ")" << std::endl;

				// Get stack element count
				int top = lua_gettop(state);
				for (int i = 1; i <= top; i++)
				{
					// Print index and type name
					std::cout << '\t' << i << '\t' << luaL_typename(state, i) << '\t';

					// Print value
					switch (lua_type(state, i))
					{
						case LUA_TNUMBER:
							std::cout << lua_tonumber(state, i) << std::endl;
							break;
						case LUA_TSTRING:
							std::cout << lua_tostring(state, i) << std::endl;
							break;
						case LUA_TBOOLEAN:
							if (lua_toboolean(state, i))
								std::cout << "true" << std::endl;
							else
								std::cout << "false" << std::endl;
							break;
						case LUA_TNIL:
							std::cout << "nil" << std::endl;
							break;
						default:
							std::cout << (void*)lua_topointer(state, i) << std::endl;
							break;
					}
				}
			#endif
		}

		static void ProtectedCall(lua_State *state, int nargs, int nresults)
		{
			int call_result = lua_pcall(state, nargs, nresults, 0);
			switch (call_result)
			{
				case LUA_OK:
					break;
				case LUA_ERRRUN:
				{
					const char *error_str = lua_tostring(state, -1);
					if (error_str != nullptr)
					{
						throw PaperPup::RuntimeError(error_str);
						break;
					}
				}
				default:
				{
					throw PaperPup::RuntimeError("Lua protected call failed");
					break;
				}
			}
		}

		template<typename T> static T *AllocUserdata(lua_State *state, const char *name)
		{
			T *userdata = (T*)lua_newuserdata(state, sizeof(T));
			luaL_getmetatable(state, name);
			lua_setmetatable(state, -2);
			return userdata;
		}

		template<typename T> static bool IsUserdata(lua_State *state, int index, const char *name)
		{
			// Get userdata
			T *p = (T *)lua_touserdata(state, index);
			if (p == nullptr)
				return false;

			// Get metatable
			if (!lua_getmetatable(state, index))
				return false;

			lua_getfield(state, LUA_REGISTRYINDEX, name);
			if (!lua_rawequal(state, -1, -2))
				return false;

			// Pop userdata and metatable off stack
			lua_pop(state, 2);
			return true;
		}

		template<typename T> static T *GetUserdata(lua_State *state, int index, const char *name)
		{
			#ifdef NDEBUG
				(void)name;

				// Return userdata
				return (T*)lua_touserdata(state, index);
			#else
				// Get userdata
				T *p = (T*)lua_touserdata(state, index);
				assert(p != nullptr);

				// Get metatable
				assert(lua_getmetatable(state, index));
			
				lua_getfield(state, LUA_REGISTRYINDEX, name);
				assert(lua_rawequal(state, -1, -2));

				// Pop userdata and metatable off stack
				lua_pop(state, 2);
				return p;
			#endif
		}

		template<typename T> static T *CheckUserdata(lua_State *state, int index, const char *name)
		{
			if (IsUserdata<T>(state, index, name))
				return GetUserdata<T>(state, index, name);
			luaL_typeerrorL(state, index, name);
			return nullptr;
		}

		// Lua access helper functions
		static std::string GetString(lua_State *state, int index, const char *name)
		{
			lua_getfield(state, index, name);
			if (!lua_isstring(state, -1))
				throw PaperPup::RuntimeError("Field " + std::string(name) + " is not a string");
			std::string result = lua_tostring(state, -1);
			lua_pop(state, 1);
			return result;
		}

		// Lua controller class
		class LuaController
		{
			public:
				// Lua objects
				lua_State *global_state;

			public:
				// Lua controller interface
				LuaController();
				~LuaController();

				void Require(std::string name);
				void RequireSource(std::string source, std::string name);
				void RequireFile(std::unique_ptr<Filesystem::File> &file, std::string name)
				{
					if (file == nullptr)
						throw PaperPup::RuntimeError("Failed to open source for module " + name);
					RequireSource(std::string(file->Dup().get(), file->Size()), name);
				}
				void RequireImageFile(std::unique_ptr<Filesystem::Image> &image, std::string name)
				{
					std::unique_ptr<Filesystem::File> file = image->OpenFile(name, false);
					if (file == nullptr)
						throw PaperPup::RuntimeError("Failed to open source for module " + name);
					RequireSource(std::string(file->Dup().get(), file->Size()), name);
				}

				void Register(const char *name, luaL_Reg *library, luaL_Reg *meta);
		};
	}
}