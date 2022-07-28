/*
 * [PaperPup]
 *   LuaController.cpp
 * Author(s): Regan Green
 * Date: 07/05/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "LuaController.h"

#include "Platform/Data.h"

// Lua libraries

namespace PaperPup
{
	namespace Lua
	{
		// Lua functions
		static int Lua_Require(lua_State *state, std::string name)
		{
			/*
			// This function is called from C++ as well, so we should leave the stack clean
			
			// Look for module in cache
			luaL_findtable(state, LUA_REGISTRYINDEX, "_MODULES", 1);
			lua_getfield(state, -1, name.c_str());
			if (!lua_isnil(state, -1))
			{
				// Return the found module
				lua_remove(state, -2);
				return 1;
			}
			lua_pop(state, 1);

			// Load bytecode for module
			std::unique_ptr<char[]> bytecode;
			size_t bytecode_size;
			{
				// Open bytecode data stream
				std::unique_ptr<PaperPup::Data::DataStream> bytecode_stream = PaperPup::Data::DataStream::Open("Bytecode/" + name);
				if (bytecode_stream == nullptr)
					throw PaperPup::RuntimeError("Failed to find bytecode for module " + name);

				// Allocate and fill bytecode array
				bytecode_size = bytecode_stream->Size();
				bytecode = std::make_unique<char[]>(bytecode_size);
				if (bytecode_stream->Read(bytecode.get(), bytecode_size) != bytecode_size)
					throw PaperPup::RuntimeError("Failed to read bytecode for module " + name);
			}

			// Create new thread for module
			lua_State *main_thread = lua_mainthread(state);
			lua_State *module_thread = lua_newthread(main_thread);
			lua_xmove(main_thread, state, 1);
			
			luaL_sandboxthread(module_thread);

			// Load and execute bytecode
			std::string chunkname = "=" + name;
			if (luau_load(module_thread, chunkname.c_str(), bytecode.get(), bytecode_size, 0) == 0)
			{
				int status = lua_resume(module_thread, state, 0);
				if (status == 0)
				{
					if (lua_gettop(module_thread) == 0)
						throw PaperPup::RuntimeError("No return value from module " + name);
				}
				else if (status == LUA_YIELD)
				{
					throw PaperPup::RuntimeError("Yield requiring module " + name);
				}
				else
				{
					if (lua_isstring(module_thread, -1))
						throw PaperPup::RuntimeError("Error requiring " + name + ": " + lua_tostring(module_thread, -1));
					else
						throw PaperPup::RuntimeError("Error requiring " + name);
				}
			}

			// Module thread stack contains our module, so we need to transfer the module to our main state
			lua_xmove(module_thread, state, 1);
			lua_remove(state, -2);

			// The bottom of our stack is still _MODULES, so we'll take the chance here to add our module to _MODULES
			lua_pushvalue(state, -1);
			lua_setfield(state, -3, name.c_str());
			lua_remove(state, -2);
			*/
		
			// Return the loaded module
			return 1;
		}

		// Lua controller interface
		LuaController::LuaController()
		{
			// Open global state
			if ((global_state = luaL_newstate()) == nullptr)
				throw PaperPup::RuntimeError("Failed to open Luau global state");
			luaL_openlibs(global_state);

			// Register global methods
			static const luaL_Reg lib_global[] = {
				{"require", [](lua_State *state)
				{
					// Get module name
					std::string name = luaL_checkstring(state, 1);
					return Lua_Require(state, name);
				}},
				{nullptr, nullptr}
			};

			lua_pushvalue(global_state, LUA_GLOBALSINDEX);
			luaL_register(global_state, nullptr, lib_global);
			lua_pop(global_state, 1);

			// Register libraries

			// Sandbox global state
			luaL_sandbox(global_state);
		}

		LuaController::~LuaController()
		{
			// Close global state
			if (global_state != nullptr)
				lua_close(global_state);
		}

		void LuaController::Require(std::string name)
		{
			// Require module
			Lua_Require(global_state, name);
			// Our stack contains the module
		}

		void LuaController::Register(const char *name, luaL_Reg *library, luaL_Reg *meta)
		{
			// Create metatable
			luaL_newmetatable(global_state, name);

			lua_pushstring(global_state, "__type");
			lua_pushstring(global_state, name);
			lua_settable(global_state, -3);

			for (; meta->func != nullptr; meta++)
			{
				lua_pushstring(global_state, meta->name);
				lua_pushcfunction(global_state, meta->func, meta->name);
				lua_settable(global_state, -3);
			}

			lua_setreadonly(global_state, -1, true);
			lua_pop(global_state, 1);

			// Register library in globals index
			lua_pushvalue(global_state, LUA_GLOBALSINDEX);

			luaL_register(global_state, name, library);
			lua_setreadonly(global_state, -1, true);

			lua_pop(global_state, 2);
		}
	}
}
