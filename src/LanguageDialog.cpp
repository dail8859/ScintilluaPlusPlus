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

#include <WindowsX.h>
#include "Config.h"
#include "resource.h"
#include "Utilities.h"

static const Configuration *config = nullptr;
static std::string language;

INT_PTR CALLBACK lngDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG: {
			RECT rc, rcDlg, rcOwner;
			HWND hwndOwner;
			HWND comboLanguage;

			if ((hwndOwner = GetParent(hwndDlg)) == NULL)
			{
				hwndOwner = GetDesktopWindow();
			}

			GetWindowRect(hwndOwner, &rcOwner);
			GetWindowRect(hwndDlg, &rcDlg);
			CopyRect(&rc, &rcOwner);

			// Offset the owner and dialog box rectangles so that right and bottom 
			// values represent the width and height, and then offset the owner again 
			// to discard space taken up by the dialog box. 

			OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
			OffsetRect(&rc, -rc.left, -rc.top);
			OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

			// The new position is the sum of half the remaining space and the owner's 
			// original position. 

			SetWindowPos(hwndDlg,
				HWND_TOP,
				rcOwner.left + (rc.right / 2),
				rcOwner.top + (rc.bottom / 2),
				0, 0,          // Ignores size arguments. 
				SWP_NOSIZE);

			comboLanguage = GetDlgItem(hwndDlg, IDC_COMBOLANGUAGE);
			for (const auto &kv : config->file_extensions) {
				ComboBox_AddString(comboLanguage, reinterpret_cast<sptr_t>(StringFromUTF8(kv.first).c_str()));
			}
			ComboBox_SetCurSel(comboLanguage, 0);
			return true;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					int curSelection;
					int selSize;
					wchar_t lang[MAX_PATH] = { 0 };

					curSelection = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBOLANGUAGE));
					selSize = ComboBox_GetLBTextLen(GetDlgItem(hwndDlg, IDC_COMBOLANGUAGE), curSelection);
					ComboBox_GetLBText(GetDlgItem(hwndDlg, IDC_COMBOLANGUAGE), curSelection, lang);
					language = UTF8FromString(lang);
				}
				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
					return true;
			}
	}
	return false;
}

std::string ShowLanguageDialog(HINSTANCE hInstance, const wchar_t *lpTemplateName, HWND hWndParent, const Configuration *cfg) {
	config = cfg;
	language = "";

	DialogBox((HINSTANCE)hInstance, lpTemplateName, hWndParent, (DLGPROC)lngDlgProc);
	return language;
}