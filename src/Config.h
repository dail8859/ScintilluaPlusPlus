// This file is part of Scintillua++.
// 
// Copyright (C)2017 Justin Dailey <dail8859@yahoo.com>
// 
// Scintillua++ is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

#include "PluginDefinition.h"
#include "NotepadPPGateway.h"

#include <map>
#include <string>
#include <vector>

typedef struct Configuration {
	bool over_ride;
	std::string theme;
	std::map<std::string, std::vector<std::string>> file_extensions;
} Configuration;

bool MatchWild(const char *pattern, size_t lenPattern, const char *fileName, bool caseSensitive);
const wchar_t *GetIniFilePath(const NotepadPPGateway &npp);
void ConfigLoad(const NotepadPPGateway &npp, Configuration *config);
void ConfigSave(const NppData *nppData, const Configuration *config);
