/*
 * [PaperPup]
 *   PaperPup.cpp
 * Author(s): Regan Green
 * Date: 07/26/2022

 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "PaperPup.h"

#include <iostream>
#include <fstream>

namespace PaperPup
{
	// Main image
	std::unique_ptr<Filesystem::Image> g_image_main;
	std::unique_ptr<Filesystem::Image> g_image_song;

	// Entry point
	int Main(std::vector<std::string> args)
	{
		// Open main image
		if ((g_image_main = Filesystem::Image::Open("Image")) == nullptr)
			throw PaperPup::RuntimeError("Failed to open main image");
		
		return 0;
	}
}
