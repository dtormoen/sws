/******************************************************************************
/ SnM_Dlg.cpp
/
/ Copyright (c) 2009-2013 Jeffos
/ http://www.standingwaterstudios.com/reaper
/
/ Permission is hereby granted, free of charge, to any person obtaining a copy
/ of this software and associated documentation files (the "Software"), to deal
/ in the Software without restriction, including without limitation the rights to
/ use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/ of the Software, and to permit persons to whom the Software is furnished to
/ do so, subject to the following conditions:
/ 
/ The above copyright notice and this permission notice shall be included in all
/ copies or substantial portions of the Software.
/ 
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/ OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/ NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/ HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/ WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/ FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
/ OTHER DEALINGS IN THE SOFTWARE.
/
******************************************************************************/

#include "stdafx.h"
#include "SnM.h"
#include "../reaper/localize.h"
#include "../Prompt.h"


void SNM_UIInit() {}

void SNM_UIExit() {
	if (LICE_IBitmap* logo = SNM_GetThemeLogo())
		DELETE_NULL(logo);
}

void SNM_UIRefresh(COMMAND_T* _ct) 
{
	UpdateTimeline(); // ruler+arrange
	TrackList_AdjustWindows(false);
	DockWindowRefresh();
}

void SNM_SetUIRefresh(COMMAND_T* _ct) {
	if (PreventUIRefresh)
		PreventUIRefresh((int)_ct->user);
}


///////////////////////////////////////////////////////////////////////////////
// Theming
///////////////////////////////////////////////////////////////////////////////

// native font rendering: not configurable on osx, optional on win
bool g_SNMClearType =
#ifdef _WIN32
	false; // default value (value overrided by the s&m.ini one)
#else
	false; //JFB!!! disabled on OSX: font issue on x64 (ex: live config knobs)
#endif

ColorTheme* SNM_GetColorTheme(bool _checkForSize) {
	int sz; ColorTheme* ct = (ColorTheme*)GetColorThemeStruct(&sz);
	if (ct && (!_checkForSize || (_checkForSize && sz >= sizeof(ColorTheme)))) return ct;
	return NULL;
}

IconTheme* SNM_GetIconTheme(bool _checkForSize) {
	int sz; IconTheme* it = (IconTheme*)GetIconThemeStruct(&sz);
	if (it && (!_checkForSize || (_checkForSize && sz >= sizeof(IconTheme)))) return it;
	return NULL;
}

LICE_CachedFont* SNM_GetThemeFont()
{
	static LICE_CachedFont themeFont;
	if (!themeFont.GetHFont()) // single lazy init..
	{
		LOGFONT lf = {
			SNM_FONT_HEIGHT,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,SNM_FONT_NAME
		};
		themeFont.SetFromHFont(
			CreateFontIndirect(&lf),
			LICE_FONT_FLAG_OWNS_HFONT | (g_SNMClearType?LICE_FONT_FLAG_FORCE_NATIVE:0));
		// others props are set on demand (to support theme switches)
	}
	ColorTheme* ct = SNM_GetColorTheme();
	themeFont.SetBkMode(TRANSPARENT);
	themeFont.SetTextColor(ct ? LICE_RGBA_FROMNATIVE(ct->main_text,255) : LICE_RGBA(255,255,255,255));
	return &themeFont;
}

// non native version
// note: cannot really factorize the code with SNM_GetThemeFont() due to the static font declaration
LICE_CachedFont* SNM_GetFont()
{
	static LICE_CachedFont themeFont;
	if (!themeFont.GetHFont()) // single lazy init..
	{
		LOGFONT lf = {
			SNM_FONT_HEIGHT,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,SNM_FONT_NAME
		};
		themeFont.SetFromHFont(CreateFontIndirect(&lf),LICE_FONT_FLAG_OWNS_HFONT);
		// others props are set on demand (to support theme switches)
	}
	ColorTheme* ct = SNM_GetColorTheme();
	themeFont.SetBkMode(TRANSPARENT);
	themeFont.SetTextColor(ct ? LICE_RGBA_FROMNATIVE(ct->main_text,255) : LICE_RGBA(255,255,255,255));
	return &themeFont;
}

// note: cannot really factorize the code with SNM_GetThemeFont() & SNM_GetFont() due to the static font declaration
LICE_CachedFont* SNM_GetToolbarFont()
{
	static LICE_CachedFont themeFont;
	if (!themeFont.GetHFont()) // single lazy init..
	{
		LOGFONT lf = {
			SNM_FONT_HEIGHT,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,SNM_FONT_NAME
		};
		themeFont.SetFromHFont(CreateFontIndirect(&lf), LICE_FONT_FLAG_OWNS_HFONT | (g_SNMClearType?LICE_FONT_FLAG_FORCE_NATIVE:0));
		// others props are set on demand (support theme switches)
	}
	ColorTheme* ct = SNM_GetColorTheme();
	themeFont.SetBkMode(TRANSPARENT);
	themeFont.SetTextColor(ct ? LICE_RGBA_FROMNATIVE(ct->toolbar_button_text,255) : LICE_RGBA(255,255,255,255));
	return &themeFont;
}

void SNM_GetThemeListColors(int* _bg, int* _txt, int* _grid)
{
	int bgcol=-1, txtcol=-1, gridcol=-1;
	ColorTheme* ct = SNM_GetColorTheme(true); // true: list view colors are recent (v4.11)
	if (ct) {
		bgcol = ct->genlist_bg;
		txtcol = ct->genlist_fg;
		gridcol = ct->genlist_gridlines;
		// note: selection colors are not managed here
	}
	if (bgcol == txtcol) { // safety
		bgcol = GSC_mainwnd(COLOR_WINDOW);
		txtcol = GSC_mainwnd(COLOR_BTNTEXT);
	}
	if (_bg) *_bg = bgcol;
	if (_txt) *_txt = txtcol;
	if (_grid) *_grid = gridcol;
}

void SNM_GetThemeEditColors(int* _bg, int* _txt)
{
	int bgcol=-1, txtcol=-1;
	ColorTheme* ct = SNM_GetColorTheme();
	if (ct) {
		bgcol =  ct->main_editbk;
		txtcol = GSC_mainwnd(COLOR_BTNTEXT);
	}
	if (bgcol == txtcol) { // safety
		bgcol = GSC_mainwnd(COLOR_WINDOW);
		txtcol = GSC_mainwnd(COLOR_BTNTEXT);
	}
	if (_bg) *_bg = bgcol;
	if (_txt) *_txt = txtcol;
}

void SNM_ThemeListView(SWS_ListView* _lv)
{
	if (_lv && _lv->GetHWND())
	{
		int bgcol, txtcol, gridcol;
		SNM_GetThemeListColors(&bgcol, &txtcol, &gridcol);
		ListView_SetBkColor(_lv->GetHWND(), bgcol);
		ListView_SetTextColor(_lv->GetHWND(), txtcol);
		ListView_SetTextBkColor(_lv->GetHWND(), bgcol);
#ifndef _WIN32
		ListView_SetGridColor(_lv->GetHWND(), gridcol);
#endif
	}
}

LICE_IBitmap* SNM_GetThemeLogo()
{
	static LICE_IBitmap* snmLogo;
	if (!snmLogo)
	{
/*JFB commented: load from resources KO for OSX (it looks for REAPER's resources..)
#ifdef _WIN32
		snmLogo = LICE_LoadPNGFromResource(g_hInst,IDB_SNM,NULL);
#else
		// SWS doesn't work, sorry. :( 
		//snmLogo =  LICE_LoadPNGFromNamedResource("SnM.png",NULL);
		snmLogo = NULL;
#endif
*/
		// logo is now loaded from memory (for OSX support)
		if (WDL_HeapBuf* hb = TranscodeStr64ToHeapBuf(SNM_LOGO_PNG_FILE)) {
			snmLogo = LICE_LoadPNGFromMemory(hb->Get(), hb->GetSize());
			delete hb;
		}
	}
	return snmLogo;
}


#ifdef _WIN32

// calling RemoveXPStyle() straight in there would crash!
static BOOL CALLBACK EnumRemoveXPStyles(HWND _hwnd, LPARAM _childHwnds)
{
	int i=0;

	// do not deal with list views & list boxes
	char className[64] = "";
	if (GetClassName(_hwnd, className, sizeof(className)) && 
		strcmp(className, WC_LISTVIEW) && 
		strcmp(className, WC_LISTBOX))
	{
		LONG style = GetWindowLong(_hwnd, GWL_STYLE);
		if ((style & BS_AUTOCHECKBOX) == BS_AUTOCHECKBOX ||
			(style & BS_AUTORADIOBUTTON) == BS_AUTORADIOBUTTON ||
			(style & BS_GROUPBOX) == BS_GROUPBOX)
		{
			((WDL_PtrList<HWND__>*)_childHwnds)->Add(_hwnd);
		}
	}
	return TRUE;
}

#endif

WDL_DLGRET SNM_HookThemeColorsMessage(HWND _hwnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, bool _wantColorEdit)
{
	if (SWS_THEMING)
	{
		switch(_uMsg)
		{
#ifdef _WIN32
			case WM_INITDIALOG :
			{
				// remove XP style on some child ctrls (cannot be themed otherwise)
				WDL_PtrList<HWND__> childHwnds;
				EnumChildWindows(_hwnd, EnumRemoveXPStyles, (LPARAM)&childHwnds);
				for (int i=0; i<childHwnds.GetSize(); i++)
					RemoveXPStyle(childHwnds.Get(i), 1);
				return 0;
			}
#endif
			case WM_CTLCOLOREDIT:
				if (!_wantColorEdit) return 0;
			case WM_CTLCOLORSCROLLBAR: // not managed yet, just in case..
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORBTN:
			case WM_CTLCOLORDLG:
			case WM_CTLCOLORSTATIC:
/* commented (custom implementations)
			case WM_DRAWITEM:
*/
				return SendMessage(GetMainHwnd(), _uMsg,_wParam,_lParam);
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Messages, prompt, etc..
///////////////////////////////////////////////////////////////////////////////

void SNM_ShowMsg(const char* _msg, const char* _title, HWND _hParent)
{
	char msg[1024*8] = "";
	GetStringWithRN(_msg, msg, sizeof(msg)); // truncates if needed
	DisplayInfoBox(_hParent?_hParent:GetMainHwnd(), _title, msg, false, false); // modeless
}

// _min and _max: 1-based (i.e. as displayed)
// returns -1 on cancel, 0-based number otherwise
int PromptForInteger(const char* _title, const char* _what, int _min, int _max)
{
	WDL_String str;
	int nb = -1;
	while (nb == -1)
	{
		str.SetFormatted(128, "%s (%d-%d):", _what, _min, _max);
		char reply[32]= ""; // no default
		if (GetUserInputs(_title, 1, str.Get(), reply, sizeof(reply)))
		{
			nb = atoi(reply); // 0 on error
			if (nb >= _min && nb <= _max)
				return (nb-1);
			else {
				nb = -1;
				str.SetFormatted(128, "Invalid %s!\nPlease enter a value in [%d; %d].", _what, _min, _max);
				MessageBox(GetMainHwnd(), str.Get(), __LOCALIZE("S&M - Error","sws_mbox"), MB_OK);
			}
		}
		else return -1; // user has cancelled
	}
	return -1;
}
