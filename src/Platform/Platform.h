/*
 * [PaperPup]
 *   Platform.h
 * Author(s): Regan Green
 * Date: 07/26/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>
#include <memory>

namespace PaperPup
{
	// System implementations
	namespace Filesystem { class Impl; }
	namespace Userdata { class Impl; }
	namespace Render { class Impl; }
	namespace Audio { class Impl; }
	namespace Input { class Impl; }

	class Impl
	{
		public:
			// Systems
			std::unique_ptr<Filesystem::Impl> filesystem;
			std::unique_ptr<Userdata::Impl> userdata;
			std::unique_ptr<Render::Impl> render;
			std::unique_ptr<Audio::Impl> audio;
			std::unique_ptr<Input::Impl> input;

		public:
			// Implementation interface
			Impl();
			~Impl();
	};
	extern Impl *g_impl;

	// Application facing entry point
	int Main(std::vector<std::string> args);

	// Exception types
	class RuntimeError : public std::runtime_error
	{
		public:
			RuntimeError(std::string what_arg = "") : std::runtime_error(what_arg) {}
	};
}
