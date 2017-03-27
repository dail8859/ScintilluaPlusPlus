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
#include "Utilities.h"

static HANDLE _hModule;
static NppData nppData;
static Configuration config;

// Helper functions
static HWND GetCurrentScintilla();
static bool DetermineLanguageFromFileName();

// Menu callbacks
static void editSettings();
static void showAbout();

FuncItem funcItem[] = {
	{ TEXT("Edit Settings..."), editSettings, 0, false, nullptr },
	{ TEXT(""), nullptr, 0, false, nullptr }, // separator
	{ TEXT("About..."), showAbout, 0, false, nullptr }
};

static HWND GetCurrentScintilla() {
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

static void SetLexer(const ScintillaGateway &editor, const std::string &language) {
	if (language.empty())
		return;

	wchar_t config_dir[MAX_PATH] = { 0 };
	SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)config_dir);
	wcscat_s(config_dir, MAX_PATH, L"\\Scintillua++");

	editor.SetLexerLanguage("lpeg");

	if (editor.GetLexer() == 1 /*SCLEX_NULL*/) {
		MessageBox(NULL, L"Failed to set LPeg lexer", NPP_PLUGIN_NAME, MB_OK | MB_ICONERROR);
		return;
	}

	editor.SetProperty("lexer.lpeg.home", UTF8FromString(config_dir).c_str());
	editor.SetProperty("lexer.lpeg.color.theme", config.theme.c_str());
	editor.SetProperty("fold", "1");

	editor.PrivateLexerCall(SCI_GETDIRECTFUNCTION, editor.GetDirectFunction());
	editor.PrivateLexerCall(SCI_SETDOCPOINTER, editor.GetDirectPointer());
	editor.PrivateLexerCall(SCI_SETLEXERLANGUAGE, reinterpret_cast<sptr_t>(language.c_str()));

	// Always show the folding margin. Since N++ doesn't recognize the file it won't have the margin showing.
	editor.SetMarginWidthN(2, 14);

	editor.Colourise(0, -1);

	std::wstring ws = StringFromUTF8(language);
	ws += L" (lpeg)";
	SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(ws.c_str()));
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
#if _DEBUG
		case SCN_UPDATEUI:
			if (notify->updated & SC_UPDATE_SELECTION) {
				ScintillaGateway editor(GetCurrentScintilla());

				if (editor.GetLexerLanguage() == "lpeg") {
					std::string text;

					char buffer[256];
					editor.PrivateLexerCall(SCI_GETLEXERLANGUAGE, reinterpret_cast<sptr_t>(buffer));
					text = buffer;
					text += " (lpeg): ";
					editor.PrivateLexerCall(editor.GetStyleAt(editor.GetCurrentPos()), reinterpret_cast<sptr_t>(buffer));
					text += buffer;
					text += ' ';
					text += std::to_string(editor.GetStyleAt(editor.GetCurrentPos()));

					SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, reinterpret_cast<LPARAM>(StringFromUTF8(text).c_str()));
				}
			}
			break;
#endif
		case NPPN_READY: {
			isReady = true;
			ConfigLoad(&nppData, &config);

			// Get the path to the external lexer
			wchar_t config_dir[MAX_PATH] = { 0 };
			SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)config_dir);
#ifdef _WIN64
			wcscat_s(config_dir, MAX_PATH, L"\\Scintillua++\\LexLPeg_64.dll");
#else
			wcscat_s(config_dir, MAX_PATH, L"\\Scintillua++\\LexLPeg.dll");
#endif
			std::string wconfig_dir = UTF8FromString(config_dir);

			ScintillaGateway editor1(nppData._scintillaMainHandle);
			ScintillaGateway editor2(nppData._scintillaSecondHandle);

			editor1.LoadLexerLibrary(wconfig_dir);
			editor2.LoadLexerLibrary(wconfig_dir);

			// Fall through - when launching N++, NPPN_BUFFERACTIVATED is received before
			// NPPN_READY. Thus the first file can get ignored so now we can check now...
		}
		case NPPN_BUFFERACTIVATED: {
			if (!isReady) break;

			ScintillaGateway editor(GetCurrentScintilla());

			if (config.over_ride || editor.GetLexer() == 1 /*SCLEX_NULL*/) {
				wchar_t ext[MAX_PATH] = { 0 };
				SendMessage(nppData._nppHandle, NPPM_GETFILENAME, MAX_PATH, (LPARAM)ext);

				SetLexer(editor, DetermineLanguageFromFileName(UTF8FromString(ext).c_str()));
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
