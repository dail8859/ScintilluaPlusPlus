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

#include "PluginDefinition.h"
#include "Version.h"
#include "AboutDialog.h"
#include "resource.h"
#include "Config.h"
#include "ScintillaGateway.h"
#include <vector>

static HANDLE _hModule;
static NppData nppData;
static Configuration config;

// Helper functions
static HWND getCurrentScintilla();
static bool DetermineLanguageFromFileName();

// Menu callbacks
static void editSettings();
static void showAbout();

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

std::string UTF8FromString(const std::wstring &s) {
	size_t sLen = s.size();
	size_t narrowLen = UTF8Length(s.c_str(), sLen);
	std::vector<char> vc(narrowLen + 1);
	UTF8FromUTF16(s.c_str(), sLen, &vc[0], narrowLen);
	return std::string(&vc[0], narrowLen);
}

FuncItem funcItem[] = {
	{ TEXT("Edit Settings..."), editSettings, 0, false, nullptr },
	{ TEXT(""), nullptr, 0, false, nullptr }, // separator
	{ TEXT("About..."), showAbout, 0, false, nullptr }
};

static HWND getCurrentScintilla() {
	int id;
	SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&id);
	if (id == 0) return nppData._scintillaMainHandle;
	else return nppData._scintillaSecondHandle;
}

static std::string DetermineLanguageFromFileName(const std::string &fileName) {
	for (const auto &kv : config.file_extensions) {
		for (const auto &ext : kv.second) {
			if (MatchWild(ext.c_str(), ext.size(), fileName.c_str(), true)) {
				return kv.first;
			}
		}
	}

	return std::string("");
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  reasonForCall, LPVOID lpReserved) {
	switch (reasonForCall) {
		case DLL_PROCESS_ATTACH:
			_hModule = hModule;
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData) {
	nppData = notepadPlusData;
	ConfigLoad(&nppData, &config);
}

extern "C" __declspec(dllexport) const wchar_t * getName() {
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF) {
	*nbF = sizeof(funcItem) / sizeof(funcItem[0]);
	return funcItem;
}

extern "C" __declspec(dllexport) void beNotified(const SCNotification *notify) {
	static bool isReady = false;

	// Somehow we are getting notifications from other scintilla handles at times
	if (notify->nmhdr.hwndFrom != nppData._nppHandle &&
		notify->nmhdr.hwndFrom != nppData._scintillaMainHandle &&
		notify->nmhdr.hwndFrom != nppData._scintillaSecondHandle)
		return;

	switch (notify->nmhdr.code) {
		case NPPN_READY: {
			isReady = true;
			ConfigLoad(&nppData, &config);

			// Get the path to the external lexer
			wchar_t config_dir[MAX_PATH] = { 0 };
			SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)config_dir);
			wcscat_s(config_dir, MAX_PATH, L"\\Scintillua++\\LexLPeg.dll");

			ScintillaGateway editor1(nppData._scintillaMainHandle);
			ScintillaGateway editor2(nppData._scintillaSecondHandle);

			editor1.LoadLexerLibrary(UTF8FromString(config_dir).c_str());
			editor2.LoadLexerLibrary(UTF8FromString(config_dir).c_str());

			// Fall through - when launching N++, NPPN_BUFFERACTIVATED is received before
			// NPPN_READY. Thus the first file can get ignored so now we can check now...
		}
		case NPPN_BUFFERACTIVATED: {
			if (!isReady) break;

			ScintillaGateway editor(getCurrentScintilla());

			if (config.over_ride || editor.GetLexer() == 1 /*SCLEX_NULL*/) {
				wchar_t ext[MAX_PATH] = { 0 };
				SendMessage(nppData._nppHandle, NPPM_GETFILENAME, MAX_PATH, (LPARAM)ext);

				auto language = DetermineLanguageFromFileName(UTF8FromString(ext).c_str());
				if (!language.empty()) {
					wchar_t config_dir[MAX_PATH] = { 0 };
					SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)config_dir);
					wcscat_s(config_dir, MAX_PATH, L"\\Scintillua++");

					editor.SetLexerLanguage("lpeg");
					editor.SetProperty("lexer.lpeg.home", UTF8FromString(config_dir).c_str());
					editor.SetProperty("lexer.lpeg.color.theme", config.theme.c_str());

					editor.PrivateLexerCall(SCI_GETDIRECTFUNCTION, editor.GetDirectFunction());
					editor.PrivateLexerCall(SCI_SETDOCPOINTER, editor.GetDirectPointer());
					editor.PrivateLexerCall(SCI_SETLEXERLANGUAGE, reinterpret_cast<int>(language.c_str()));
					editor.Colourise(0, -1);

					SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(TEXT("lpeg")));
				}
			}
			break;
		}
		case NPPN_FILESAVED: {
			// If the ini file was edited, reload it
			wchar_t fname[MAX_PATH] = { 0 };
			SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, notify->nmhdr.idFrom, (LPARAM)fname);
			if (wcscmp(fname, GetIniFilePath(&nppData)) == 0) {
				ConfigLoad(&nppData, &config);
			}
			break;
		}
	}
	return;
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) {
	return TRUE;
}

extern "C" __declspec(dllexport) BOOL isUnicode() {
	return TRUE;
}

static void editSettings() {
	//ConfigSave(&nppData, &config);
	SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)GetIniFilePath(&nppData));
}

static void showAbout() {
	ShowAboutDialog((HINSTANCE)_hModule, MAKEINTRESOURCE(IDD_ABOUTDLG), nppData._nppHandle);
}
