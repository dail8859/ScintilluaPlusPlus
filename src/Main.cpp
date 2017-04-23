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

#include <string>
#include <fstream>
#include <streambuf>

#include "PluginDefinition.h"
#include "Version.h"
#include "AboutDialog.h"
#include "LanguageDialog.h"
#include "resource.h"
#include "Config.h"
#include "ScintillaGateway.h"
#include "Utilities.h"
#include "menuCmdID.h"
#include "NotepadPPGateway.h"

static HANDLE _hModule;
static NppData nppData;
static Configuration config;
static ScintillaGateway editor;
static NotepadPPGateway npp;
static std::map<uptr_t, std::string> bufferLanguages;

// Helper functions
static std::string DetermineLanguageFromFileName(const std::string &fileName);

// Menu callbacks
static void editSettings();
static void showAbout();
static void setLanguage();
static void editLanguageDefinition();
static void createNewLanguageDefinition();

FuncItem funcItem[] = {
	{ TEXT("Set Language..."), setLanguage, 0, false, nullptr },
	{ TEXT(""), nullptr, 0, false, nullptr }, // separator
	{ TEXT("Create New Language Definition..."), createNewLanguageDefinition, 0, false, nullptr },
	{ TEXT("Edit Language Definition..."), editLanguageDefinition, 0, false, nullptr },
	{ TEXT("Edit Settings..."), editSettings, 0, false, nullptr },
	{ TEXT(""), nullptr, 0, false, nullptr }, // separator
	{ TEXT("About..."), showAbout, 0, false, nullptr }
};

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

static void SetLexer(const std::string &language) {
	if (language.empty())
		return;

	auto config_dir = npp.GetPluginsConfigDir();
	config_dir += L"\\Scintillua++";

	editor.SetLexerLanguage("lpeg");

	if (editor.GetLexer() == 1 /*SCLEX_NULL*/) {
		MessageBox(NULL, L"Failed to set LPeg lexer", NPP_PLUGIN_NAME, MB_OK | MB_ICONERROR);
		return;
	}

	editor.SetProperty("lexer.lpeg.home", UTF8FromString(config_dir));
	editor.SetProperty("lexer.lpeg.color.theme", config.theme);
	editor.SetProperty("fold", "1");

	editor.PrivateLexerCall(SCI_GETDIRECTFUNCTION, editor.GetDirectFunction());
	editor.PrivateLexerCall(SCI_SETDOCPOINTER, editor.GetDirectPointer());
	editor.PrivateLexerCall(SCI_SETLEXERLANGUAGE, reinterpret_cast<sptr_t>(language.c_str()));

	// Always show the folding margin. Since N++ doesn't recognize the file it won't have the margin showing.
	editor.SetMarginWidthN(2, 14);

	editor.Colourise(0, -1);

	// Check for errors
	char buffer[512] = { 0 };
	editor.PrivateLexerCall(SCI_GETSTATUS, reinterpret_cast<sptr_t>(buffer));
	if (strlen(buffer) > 0) {
		MessageBox(nppData._nppHandle, StringFromUTF8(buffer).c_str(), NPP_PLUGIN_NAME, MB_OK | MB_ICONERROR);
		return;
	}

	std::wstring ws = StringFromUTF8(language);
	ws += L" (lpeg)";
	npp.SetStatusBar(STATUSBAR_DOC_TYPE, ws);
}

static void CheckFileForNewLexer() {
	auto bufferid = npp.GetCurrentBufferID();
	const auto search = bufferLanguages.find(bufferid);

	if (search != bufferLanguages.end()) {
		SetLexer(search->second);
	}
	else if (config.over_ride || editor.GetLexer() == 1 /*SCLEX_NULL*/) {
		auto ext = npp.GetFileName();
		SetLexer(DetermineLanguageFromFileName(UTF8FromString(ext)));
	}
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID lpReserved) {
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
	editor.SetScintillaInstance(nppData._scintillaMainHandle);
	npp.SetNppData(nppData);
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
	static std::wstring fileBeingSaved;

	// Somehow we are getting notifications from other scintilla handles at times
	if (notify->nmhdr.hwndFrom != nppData._nppHandle &&
		notify->nmhdr.hwndFrom != nppData._scintillaMainHandle &&
		notify->nmhdr.hwndFrom != nppData._scintillaSecondHandle)
		return;

	switch (notify->nmhdr.code) {
#if _DEBUG
		case SCN_UPDATEUI:
			if (notify->updated & SC_UPDATE_SELECTION) {
				if (editor.GetLexerLanguage() == "lpeg") {
					// Make sure no errors occured
					if (editor.PrivateLexerCall(SCI_GETSTATUS, NULL) == 0) {
						char buffer[512] = { 0 };
						std::string text;

						editor.PrivateLexerCall(SCI_GETLEXERLANGUAGE, reinterpret_cast<sptr_t>(buffer));
						text = buffer;
						text += " (lpeg): ";
						editor.PrivateLexerCall(editor.GetStyleAt(editor.GetCurrentPos()), reinterpret_cast<sptr_t>(buffer));
						text += buffer;
						text += ' ';
						text += std::to_string(editor.GetStyleAt(editor.GetCurrentPos()));

						npp.SetStatusBar(STATUSBAR_DOC_TYPE, StringFromUTF8(text));
					}
				}
			}
			break;
#endif
		case NPPN_READY: {
			isReady = true;
			ConfigLoad(npp, &config);

			// Get the path to the external lexer
			auto config_dir = npp.GetPluginsConfigDir();
#ifdef _WIN64
			config_dir += L"\\Scintillua++\\LexLPeg_64.dll";
#else
			config_dir += L"\\Scintillua++\\LexLPeg.dll";
#endif
			std::string wconfig_dir = UTF8FromString(config_dir);

			ScintillaGateway editor1(nppData._scintillaMainHandle);
			ScintillaGateway editor2(nppData._scintillaSecondHandle);

			editor1.LoadLexerLibrary(wconfig_dir);
			editor2.LoadLexerLibrary(wconfig_dir);

			// Fall through - when launching N++, NPPN_BUFFERACTIVATED is received before
			// NPPN_READY. Thus the first file can get ignored so now we can check now...
		}
		case NPPN_FILERENAMED:
		case NPPN_BUFFERACTIVATED:
			editor.SetScintillaInstance(npp.GetCurrentScintillaHwnd());

			if (!isReady)
				break;
			CheckFileForNewLexer();
			break;
		case NPPN_FILEBEFORESAVE: {
			// Notepad++ does not notify when a file has been renamed using the normal
			// "save as" dialog. So store the file name before the save then compare it
			// to the file name immediately after the save occurs. If they are different then
			// file has been renamed.
			// NOTE: this is different from the user doing "File > Rename..." which sends the
			// NPPN_FILERENAMED notification

			fileBeingSaved = npp.GetFullPathFromBufferID(notify->nmhdr.idFrom);
			break;
		}
		case NPPN_FILESAVED: {
			std::wstring fileSaved = npp.GetFullPathFromBufferID(notify->nmhdr.idFrom);

			if (fileSaved != fileBeingSaved) {
				// The file was saved as a different file name
				CheckFileForNewLexer();
			}
			else if (fileSaved.compare(GetIniFilePath(npp)) == 0) {
				// If the ini file was edited, reload it
				ConfigLoad(npp, &config);
			}
			else
			{
				if (config.over_ride || editor.GetLexer() == 1 /*SCLEX_NULL*/) {
					auto ext = npp.GetFileName();
					SetLexer(DetermineLanguageFromFileName(UTF8FromString(ext)));
				}
			}

			fileBeingSaved.clear();

			break;
		}
		case NPPN_LANGCHANGED:
		case NPPN_FILECLOSED:
			// Try to remove it
			bufferLanguages.erase(notify->nmhdr.idFrom);
			break;
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
	npp.DoOpen(GetIniFilePath(npp));
}

static void showAbout() {
	ShowAboutDialog((HINSTANCE)_hModule, MAKEINTRESOURCE(IDD_ABOUTDLG), nppData._nppHandle);
}

static void setLanguage() {
	std::string language = ShowLanguageDialog((HINSTANCE)_hModule, MAKEINTRESOURCE(IDD_LANGUAGEDLG), nppData._nppHandle, &config);

	if (!language.empty()) {
		npp.SetCurrentLangType(L_TEXT);

		uptr_t bufferid = npp.GetCurrentBufferID();
		bufferLanguages[bufferid] = language;
		SetLexer(language);
	}
}

static void editLanguageDefinition() {
	std::string language = ShowLanguageDialog((HINSTANCE)_hModule, MAKEINTRESOURCE(IDD_LANGUAGEDLG), nppData._nppHandle, &config);

	if (!language.empty()) {
		auto config_dir = npp.GetPluginsConfigDir();
		config_dir += L"\\Scintillua++\\";
		config_dir += StringFromUTF8(language);
		config_dir += L".lua";
		npp.DoOpen(config_dir);

		MessageBox(nppData._nppHandle, L"Note that any changes to built-in languages may get lost when updating this plugin.", NPP_PLUGIN_NAME, MB_OK | MB_ICONINFORMATION);
	}
}

static void createNewLanguageDefinition() {
	auto config_dir = npp.GetPluginsConfigDir();
	config_dir += L"\\Scintillua++\\template.txt";

	std::ifstream t(config_dir);
	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(static_cast<size_t>(t.tellg()));
	t.seekg(0, std::ios::beg);
	str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	npp.MenuCommand(IDM_FILE_NEW);

	editor.SetText(str);
	if (config.over_ride) {
		SetLexer("lua");
	}
	else {
		npp.SetCurrentLangType(L_LUA);
	}
}
