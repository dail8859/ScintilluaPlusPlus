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

#include "Config.h"
#include "Utilities.h"

#include <Shlwapi.h>

const wchar_t *GetIniFilePath(const NotepadPPGateway &npp) {
	static wchar_t iniPath[MAX_PATH] = { 0 };

	if (iniPath[0] == 0) {
		npp.GetPluginsConfigDir(MAX_PATH, iniPath);
		wcscat_s(iniPath, MAX_PATH, L"\\Scintillua++.ini");
	}

	return iniPath;
}

void EnsureConfigFileExists(const NotepadPPGateway &npp) {
	if (PathFileExists(GetIniFilePath(npp)) == FALSE) {
		wchar_t defaultPath[MAX_PATH] = { 0 };
		npp.GetPluginsConfigDir(MAX_PATH, defaultPath);
		wcscat_s(defaultPath, MAX_PATH, L"\\Scintillua++\\default.ini");
		CopyFile(defaultPath, GetIniFilePath(npp), TRUE);
	}
}

void ConfigLoad(const NotepadPPGateway &npp, Configuration *config) {
	EnsureConfigFileExists(npp);

	const wchar_t *iniPath = GetIniFilePath(npp);

	FILE *file = _wfopen(iniPath, L"r");

	if (file == nullptr) return;

	config->file_extensions.clear();

	char line[512];
	while (true) {
		if (fgets(line, 512, file) == NULL) break;

		// Ignore comments and blank lines
		if (line[0] == ';' || line[0] == '\r' || line[0] == '\n') continue;

		std::string s(line);

		auto key_value = split(s, '=');

		// Just skip lines that aren't what we expect
		if (key_value.size() != 2) continue;

		trim(key_value[0]);
		trim(key_value[1]);

		// Handle some specific settings
		if (key_value[0] == "theme") {
			config->theme = key_value[1];
			continue;
		}
		else if (key_value[0] == "override") {
			config->over_ride = key_value[1] == "true";
		}

		// Anything else is assumed to be a language/extentsion pattern
		config->file_extensions[key_value[0]] = split(key_value[1], ';');
	}

	fclose(file);
}

void ConfigSave(const NppData *nppData, const Configuration *config) {
	// not needed currently
}
