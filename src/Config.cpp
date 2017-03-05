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

#include <stdio.h>
#include <functional>
#include <algorithm>
#include <cctype>
#include <sstream>
#include "Config.h"

const wchar_t *GetIniFilePath(const NppData *nppData) {
	static wchar_t iniPath[MAX_PATH];
	SendMessage(nppData->_nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)iniPath);
	wcscat_s(iniPath, MAX_PATH, L"\\Scintillua++.ini");
	return iniPath;
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

inline char MakeUpperCase(char ch) {
	if (ch < 'a' || ch > 'z')
		return ch;
	else
		return static_cast<char>(ch - 'a' + 'A');
}

static bool StringEqual(const char *a, const char *b, size_t len, bool caseSensitive) {
	if (caseSensitive) {
		for (size_t i = 0; i < len; i++) {
			if (a[i] != b[i])
				return false;
		}
	}
	else {
		for (size_t i = 0; i < len; i++) {
			if (MakeUpperCase(a[i]) != MakeUpperCase(b[i]))
				return false;
		}
	}
	return true;
}

// Match file names to patterns allowing for a '*' suffix or prefix.
bool MatchWild(const char *pattern, size_t lenPattern, const char *fileName, bool caseSensitive) {
	size_t lenFileName = strlen(fileName);
	if (lenFileName == lenPattern) {
		if (StringEqual(pattern, fileName, lenFileName, caseSensitive)) {
			return true;
		}
	}
	if (lenFileName >= lenPattern - 1) {
		if (pattern[0] == '*') {
			// Matching suffixes
			return StringEqual(pattern + 1, fileName + lenFileName - (lenPattern - 1), lenPattern - 1, caseSensitive);
		}
		else if (pattern[lenPattern - 1] == '*') {
			// Matching prefixes
			return StringEqual(pattern, fileName, lenPattern - 1, caseSensitive);
		}
	}
	return false;
}

void ConfigLoad(const NppData *nppData, Configuration *config) {
	const wchar_t *iniPath = GetIniFilePath(nppData);

	FILE *file = _wfopen(iniPath, L"r");

	if (file == nullptr) return;

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
