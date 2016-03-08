﻿// This file is part of LuaScript.
// 
// Copyright (C)2016 Justin Dailey <dail8859@yahoo.com>
// 
// LuaScript is free software; you can redistribute it and/or
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

#include <vector>
#include "PluginDefinition.h"
#include "AboutDialog.h"

#include "LuaConsole.h"
#include "LuaExtension.h"
#include "NppExtensionAPI.h"
#include "WcharMbcsConverter.h"

// --- Menu callbacks ---
static void showConsole();
static void editStartupScript();
static void runCurrentFile();
static void showAbout();

// --- Local variables ---
static NppData nppData;
static SciFnDirect pSciMsg;			// For direct scintilla call
static sptr_t pSciWndData;			// For direct scintilla call
static HANDLE _hModule;				// For dialog initialization
static LuaConsole *g_console;
static bool isReady = false;
static std::vector<FuncItem> funcItems;
static PFUNCPLUGINCMD LuaShortcutWrappers[] = {
	[](){ LuaExtension::Instance().CallShortcut(1); },
	[](){ LuaExtension::Instance().CallShortcut(2); },
	[](){ LuaExtension::Instance().CallShortcut(3); },
	[](){ LuaExtension::Instance().CallShortcut(4); },
	[](){ LuaExtension::Instance().CallShortcut(5); },
	[](){ LuaExtension::Instance().CallShortcut(6); },
	[](){ LuaExtension::Instance().CallShortcut(7); },
	[](){ LuaExtension::Instance().CallShortcut(8); },
	[](){ LuaExtension::Instance().CallShortcut(9); },
	[](){ LuaExtension::Instance().CallShortcut(10);},
};

static inline LRESULT SendScintilla(UINT Msg, WPARAM wParam = SCI_UNUSED, LPARAM lParam = SCI_UNUSED) {
	return pSciMsg(pSciWndData, Msg, wParam, lParam);
}

static inline LRESULT SendNpp(UINT Msg, WPARAM wParam = SCI_UNUSED, LPARAM lParam = SCI_UNUSED) {
	return SendMessage(nppData._nppHandle, Msg, wParam, lParam);
}

static bool updateScintilla() {
	HWND curScintilla;

	// Get the current scintilla
	int which = -1;
	SendNpp(NPPM_GETCURRENTSCINTILLA, SCI_UNUSED, (LPARAM)&which);
	if(which == -1) return false;
	curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	// Get the function and pointer to it for more efficient calls
	pSciMsg = (SciFnDirect) SendMessage(curScintilla, SCI_GETDIRECTFUNCTION, 0, 0);
	pSciWndData = (sptr_t) SendMessage(curScintilla, SCI_GETDIRECTPOINTER, 0, 0);

	return true;
}

std::shared_ptr<char> getStartupScriptFilePath(wchar_t *buff, size_t size) {
	SendNpp(NPPM_GETPLUGINSCONFIGDIR, size, (LPARAM)buff);
	wcscat_s(buff, size, TEXT("\\"));
	wcscat_s(buff, size, TEXT("startup"));
	wcscat_s(buff, size, TEXT(".lua"));
	return WcharMbcsConverter::wchar2char(buff);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  reasonForCall, LPVOID lpReserved)
{
	switch (reasonForCall)
	{
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

	// Initialize the dialogs
	g_console = new LuaConsole(nppData._nppHandle);
	g_console->init((HINSTANCE)_hModule, nppData);
	LuaExtension::Instance().Initialise(new NppExtensionAPI(g_console->mp_consoleDlg, &nppData));

	// Set up the short cuts that we know of
	funcItems.emplace_back(FuncItem{ TEXT("Show Console"), showConsole, 0, false, NULL });
	funcItems.emplace_back(FuncItem{ TEXT("Edit Startup Script"), editStartupScript, 0, false, NULL });
	funcItems.emplace_back(FuncItem{ TEXT("Run Current File"), runCurrentFile, 0, false, NULL });

	// Run the startup script
	wchar_t buff[MAX_PATH];
	SendNpp(NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)buff);
	wcscat_s(buff, MAX_PATH, TEXT("\\"));
	wcscat_s(buff, MAX_PATH, TEXT("startup"));
	wcscat_s(buff, MAX_PATH, TEXT(".lua"));
	if (LuaExtension::Instance().RunFile(WcharMbcsConverter::wchar2char(buff).get()) == true) {
		if (luaShortcuts.size() > 0) funcItems.emplace_back(FuncItem{ TEXT(""), NULL, 0, false, NULL });

		const int max_shortcuts = sizeof(LuaShortcutWrappers) / sizeof(LuaShortcutWrappers[0]);
		if (luaShortcuts.size() > max_shortcuts) {
			luaShortcuts.resize(max_shortcuts);
			MessageBox(NULL, TEXT("Too many Lua shortcuts have been registered.\r\nThis is an arbitrary limit and can be increased if you ask nicely on Github."), NPP_PLUGIN_NAME, MB_OK | MB_ICONERROR);
		}

		for (size_t i = 0; i < luaShortcuts.size(); ++i) {
			funcItems.emplace_back();
			_tcscpy_s(funcItems.back()._itemName, 64, WcharMbcsConverter::char2wchar(luaShortcuts[i]._itemName).get());
			funcItems.back()._pFunc = LuaShortcutWrappers[i];
			funcItems.back()._cmdID = 0;
			funcItems.back()._init2Check = false;
			funcItems.back()._pShKey = luaShortcuts[i]._pShKey;
		}

		luaShortcuts.clear(); // No longer need anything from it
	}
	else {
		// Something went wrong :(
		g_console->showDialog();
	}

	funcItems.emplace_back(FuncItem{ TEXT(""), NULL, 0, false, NULL });
	funcItems.emplace_back(FuncItem{ TEXT("About..."), showAbout, 0, false, NULL });
}

extern "C" __declspec(dllexport) const wchar_t *getName() {
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem *getFuncsArray(int *nbF) {
	*nbF = funcItems.size();
	return funcItems.data();
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode) {
	static TCHAR static_fname[MAX_PATH];
	TCHAR fname[MAX_PATH];
	NotifyHeader nh = notifyCode->nmhdr;

	switch (nh.code)
	{
	case SCN_CHARADDED:
		LuaExtension::Instance().OnChar(notifyCode->ch);
		break;
	case SCN_SAVEPOINTREACHED:
		LuaExtension::Instance().OnSavePointReached();
		break;
	case SCN_SAVEPOINTLEFT:
		LuaExtension::Instance().OnSavePointLeft();
		break;
	case NPPN_READY:
		isReady = true;
		break;
	case NPPN_FILEBEFOREOPEN:
		SendNpp(NPPM_GETFULLPATHFROMBUFFERID, nh.idFrom, (LPARAM)fname);
		LuaExtension::Instance().OnBeforeOpen(WcharMbcsConverter::wchar2char(fname).get());
		break;
	case NPPN_FILEOPENED:
		SendNpp(NPPM_GETFULLPATHFROMBUFFERID, nh.idFrom, (LPARAM)fname);
		LuaExtension::Instance().OnOpen(WcharMbcsConverter::wchar2char(fname).get());
		break;
	case NPPN_BUFFERACTIVATED:
		SendNpp(NPPM_GETFULLPATHFROMBUFFERID, nh.idFrom, (LPARAM)fname);
		LuaExtension::Instance().OnSwitchFile(WcharMbcsConverter::wchar2char(fname).get());
		break;
	case NPPN_FILEBEFORESAVE:
		SendNpp(NPPM_GETFULLPATHFROMBUFFERID, nh.idFrom, (LPARAM)fname);
		LuaExtension::Instance().OnBeforeSave(WcharMbcsConverter::wchar2char(fname).get());
		break;
	case NPPN_FILESAVED:
		SendNpp(NPPM_GETFULLPATHFROMBUFFERID, nh.idFrom, (LPARAM)fname);
		LuaExtension::Instance().OnSave(WcharMbcsConverter::wchar2char(fname).get());
		break;
	case NPPN_FILEBEFORECLOSE:
		SendNpp(NPPM_GETFULLPATHFROMBUFFERID, nh.idFrom, (LPARAM)static_fname);
		LuaExtension::Instance().OnBeforeClose(WcharMbcsConverter::wchar2char(static_fname).get());
		break;
	case NPPN_FILECLOSED:
		// NOTE: cannot use idFrom to get the path since it is no longer valid
		LuaExtension::Instance().OnClose(WcharMbcsConverter::wchar2char(static_fname).get());
		break;
	case NPPN_SHUTDOWN:
		LuaExtension::Instance().Finalise();
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

// --- Menu call backs ---

void showConsole() {
	g_console->showDialog();
	// If the console is shown automatically at startup, don't give it focus
	if (isReady) g_console->mp_consoleDlg->giveInputFocus();
}

void editStartupScript() {
	wchar_t buff[MAX_PATH];
	SendNpp(NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)buff);
	wcscat_s(buff, MAX_PATH, TEXT("\\"));
	wcscat_s(buff, MAX_PATH, TEXT("startup"));
	wcscat_s(buff, MAX_PATH, TEXT(".lua"));
	if (PathFileExists(buff) == 0) {
		const char *s = "-- Startup script\r\n-- Changes will take effect once Notepad++ is restarted\r\n\r\n";
		DWORD dwBytesWritten;
		HANDLE h = CreateFile(buff, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(h, s, strlen(s), &dwBytesWritten, NULL);
		CloseHandle(h);
	}
	SendNpp(NPPM_DOOPEN, 0, (LPARAM)buff);
}

static void runCurrentFile() {
	updateScintilla();
	const char *doc = (const char *)SendScintilla(SCI_GETCHARACTERPOINTER);
	if (LuaExtension::Instance().RunString(doc) == false) {
		g_console->showDialog();
	}
}

void showAbout() {
	CreateDialog((HINSTANCE) _hModule, MAKEINTRESOURCE(IDD_ABOUTDLG), nppData._nppHandle, abtDlgProc);
}
