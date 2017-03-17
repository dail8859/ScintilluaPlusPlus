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

#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <sstream>

#include "Utilities.h"

enum { SURROGATE_LEAD_FIRST = 0xD800 };
enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
enum { SURROGATE_TRAIL_LAST = 0xDFFF };
static unsigned int UTF8Length(const wchar_t *uptr, size_t tlen) {
	unsigned int len = 0;
	for (size_t i = 0; i < tlen && uptr[i];) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			len++;
		}
		else if (uch < 0x800) {
			len += 2;
		}
		else if ((uch >= SURROGATE_LEAD_FIRST) &&
			(uch <= SURROGATE_TRAIL_LAST)) {
			len += 4;
			i++;
		}
		else {
			len += 3;
		}
		i++;
	}
	return len;
}

static size_t UTF16Length(const char *s, unsigned int len) {
	size_t ulen = 0;
	size_t charLen;
	for (size_t i = 0; i<len;) {
		unsigned char ch = static_cast<unsigned char>(s[i]);
		if (ch < 0x80) {
			charLen = 1;
		}
		else if (ch < 0x80 + 0x40 + 0x20) {
			charLen = 2;
		}
		else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
			charLen = 3;
		}
		else {
			charLen = 4;
			ulen++;
		}
		i += charLen;
		ulen++;
	}
	return ulen;
}

static void UTF8FromUTF16(const wchar_t *uptr, size_t tlen, char *putf, size_t len) {
	int k = 0;
	for (size_t i = 0; i < tlen && uptr[i];) {
		unsigned int uch = uptr[i];
		if (uch < 0x80) {
			putf[k++] = static_cast<char>(uch);
		}
		else if (uch < 0x800) {
			putf[k++] = static_cast<char>(0xC0 | (uch >> 6));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		}
		else if ((uch >= SURROGATE_LEAD_FIRST) &&
			(uch <= SURROGATE_TRAIL_LAST)) {
			// Half a surrogate pair
			i++;
			unsigned int xch = 0x10000 + ((uch & 0x3ff) << 10) + (uptr[i] & 0x3ff);
			putf[k++] = static_cast<char>(0xF0 | (xch >> 18));
			putf[k++] = static_cast<char>(0x80 | ((xch >> 12) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | ((xch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (xch & 0x3f));
		}
		else {
			putf[k++] = static_cast<char>(0xE0 | (uch >> 12));
			putf[k++] = static_cast<char>(0x80 | ((uch >> 6) & 0x3f));
			putf[k++] = static_cast<char>(0x80 | (uch & 0x3f));
		}
		i++;
	}
	putf[len] = '\0';
}

static size_t UTF16FromUTF8(const char *s, size_t len, wchar_t *tbuf, size_t tlen) {
	size_t ui = 0;
	const unsigned char *us = reinterpret_cast<const unsigned char *>(s);
	size_t i = 0;
	while ((i<len) && (ui<tlen)) {
		unsigned char ch = us[i++];
		if (ch < 0x80) {
			tbuf[ui] = ch;
		}
		else if (ch < 0x80 + 0x40 + 0x20) {
			tbuf[ui] = static_cast<wchar_t>((ch & 0x1F) << 6);
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + (ch & 0x7F));
		}
		else if (ch < 0x80 + 0x40 + 0x20 + 0x10) {
			tbuf[ui] = static_cast<wchar_t>((ch & 0xF) << 12);
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + ((ch & 0x7F) << 6));
			ch = us[i++];
			tbuf[ui] = static_cast<wchar_t>(tbuf[ui] + (ch & 0x7F));
		}
		else {
			// Outside the BMP so need two surrogates
			int val = (ch & 0x7) << 18;
			ch = us[i++];
			val += (ch & 0x3F) << 12;
			ch = us[i++];
			val += (ch & 0x3F) << 6;
			ch = us[i++];
			val += (ch & 0x3F);
			tbuf[ui] = static_cast<wchar_t>(((val - 0x10000) >> 10) + SURROGATE_LEAD_FIRST);
			ui++;
			tbuf[ui] = static_cast<wchar_t>((val & 0x3ff) + SURROGATE_TRAIL_FIRST);
		}
		ui++;
	}
	return ui;
}

std::string UTF8FromString(const std::wstring &s) {
	size_t sLen = s.size();
	size_t narrowLen = UTF8Length(s.c_str(), sLen);
	std::vector<char> vc(narrowLen + 1);
	UTF8FromUTF16(s.c_str(), sLen, &vc[0], narrowLen);
	return std::string(&vc[0], narrowLen);
}

std::wstring StringFromUTF8(const std::string &s) {
	size_t sLen = s.length();
	size_t wideLen = UTF16Length(s.c_str(), static_cast<int>(sLen));
	std::vector<wchar_t> vgc(wideLen + 1);
	size_t outLen = UTF16FromUTF8(s.c_str(), sLen, &vgc[0], wideLen);
	vgc[outLen] = 0;
	return std::wstring(&vgc[0], outLen);
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
static void inline ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
void trim(std::string &s) {
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