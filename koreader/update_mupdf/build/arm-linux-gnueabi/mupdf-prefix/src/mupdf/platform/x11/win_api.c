#include "pdfapp.h"

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

#define MIN(x,y) ((x) < (y) ? (x) : (y))

#define ID_ABOUT	0x1000
#define ID_DOCINFO	0x1001

HWND hwndframe = NULL;
HWND hwndview = NULL;
HDC hdc;
HBRUSH bgbrush;
HBRUSH shbrush;
BITMAPINFO *dibinf = NULL;
HCURSOR arrowcurs, handcurs, waitcurs, caretcurs;
int timer_pending = 0;

int justcopied = 0;

static pdfapp_t gapp;


#define PATH_MAX (1024)

//static wchar_t wbuf[PATH_MAX];
wchar_t wbuf[PATH_MAX];

//static char filename[PATH_MAX];
char filename[PATH_MAX];


/*
 * Create registry keys to associate MuPDF with PDF and XPS files.
 */

#define OPEN_KEY(parent, name, ptr) \
	RegCreateKeyExA(parent, name, 0, 0, 0, KEY_WRITE, 0, &ptr, 0)

#define SET_KEY(parent, name, value) \
	RegSetValueExA(parent, name, 0, REG_SZ, (const BYTE *)(value), strlen(value) + 1)

extern __declspec(dllexport) void install_app(char *argv0)
{
	char buf[512];
	HKEY software, classes, mupdf, dotpdf, dotxps, dotepub;
	HKEY shell, open, command, supported_types;
	HKEY pdf_progids, xps_progids, epub_progids;

	OPEN_KEY(HKEY_CURRENT_USER, "Software", software);
	OPEN_KEY(software, "Classes", classes);
	OPEN_KEY(classes, ".pdf", dotpdf);
	OPEN_KEY(dotpdf, "OpenWithProgids", pdf_progids);
	OPEN_KEY(classes, ".xps", dotxps);
	OPEN_KEY(dotxps, "OpenWithProgids", xps_progids);
	OPEN_KEY(classes, ".epub", dotepub);
	OPEN_KEY(dotepub, "OpenWithProgids", epub_progids);
	OPEN_KEY(classes, "MuPDF", mupdf);
	OPEN_KEY(mupdf, "SupportedTypes", supported_types);
	OPEN_KEY(mupdf, "shell", shell);
	OPEN_KEY(shell, "open", open);
	OPEN_KEY(open, "command", command);

	sprintf(buf, "\"%s\" \"%%1\"", argv0);

	SET_KEY(open, "FriendlyAppName", "MuPDF");
	SET_KEY(command, "", buf);
	SET_KEY(supported_types, ".pdf", "");
	SET_KEY(supported_types, ".xps", "");
	SET_KEY(supported_types, ".epub", "");
	SET_KEY(pdf_progids, "MuPDF", "");
	SET_KEY(xps_progids, "MuPDF", "");
	SET_KEY(epub_progids, "MuPDF", "");

	RegCloseKey(dotepub);
	RegCloseKey(dotxps);
	RegCloseKey(dotpdf);
	RegCloseKey(mupdf);
	RegCloseKey(classes);
	RegCloseKey(software);
}

/*
 * Dialog boxes
 */

void winwarn(pdfapp_t *app, char *msg)
{
	MessageBoxA(hwndframe, msg, "MuPDF: Warning", MB_ICONWARNING);
}

void winerror(pdfapp_t *app, char *msg)
{
	MessageBoxA(hwndframe, msg, "MuPDF: Error", MB_ICONERROR);
	exit(1);
}

void winalert(pdfapp_t *app, pdf_alert_event *alert)
{
	int buttons = MB_OK;
	int icon = MB_ICONWARNING;
	int pressed = PDF_ALERT_BUTTON_NONE;

	switch (alert->icon_type)
	{
	case PDF_ALERT_ICON_ERROR:
		icon = MB_ICONERROR;
		break;
	case PDF_ALERT_ICON_WARNING:
		icon = MB_ICONWARNING;
		break;
	case PDF_ALERT_ICON_QUESTION:
		icon = MB_ICONQUESTION;
		break;
	case PDF_ALERT_ICON_STATUS:
		icon = MB_ICONINFORMATION;
		break;
	}

	switch (alert->button_group_type)
	{
	case PDF_ALERT_BUTTON_GROUP_OK:
		buttons = MB_OK;
		break;
	case PDF_ALERT_BUTTON_GROUP_OK_CANCEL:
		buttons = MB_OKCANCEL;
		break;
	case PDF_ALERT_BUTTON_GROUP_YES_NO:
		buttons = MB_YESNO;
		break;
	case PDF_ALERT_BUTTON_GROUP_YES_NO_CANCEL:
		buttons = MB_YESNOCANCEL;
		break;
	}

	pressed = MessageBoxA(hwndframe, alert->message, alert->title, icon|buttons);

	switch (pressed)
	{
	case IDOK:
		alert->button_pressed = PDF_ALERT_BUTTON_OK;
		break;
	case IDCANCEL:
		alert->button_pressed = PDF_ALERT_BUTTON_CANCEL;
		break;
	case IDNO:
		alert->button_pressed = PDF_ALERT_BUTTON_NO;
		break;
	case IDYES:
		alert->button_pressed = PDF_ALERT_BUTTON_YES;
	}
}

void winprint(pdfapp_t *app)
{
	MessageBoxA(hwndframe, "The MuPDF library supports printing, but this application currently does not", "Print document", MB_ICONWARNING);
}

int winsavequery(pdfapp_t *app)
{
	switch(MessageBoxA(hwndframe, "File has unsaved changes. Do you want to save", "MuPDF", MB_YESNOCANCEL))
	{
	case IDYES: return SAVE;
	case IDNO: return DISCARD;
	default: return CANCEL;
	}
}

extern __declspec(dllexport)int winfilename(wchar_t *buf, int len)
{
	OPENFILENAME ofn;
	buf[0] = 0;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndframe;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = len;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"MuPDF: Open PDF file";
	ofn.lpstrFilter = L"Documents (*.pdf;*.xps;*.cbz;*.epub;*.zip;*.png;*.jpeg;*.tiff)\0*.zip;*.cbz;*.xps;*.epub;*.pdf;*.jpe;*.jpg;*.jpeg;*.jfif;*.tif;*.tiff\0PDF Files (*.pdf)\0*.pdf\0XPS Files (*.xps)\0*.xps\0CBZ Files (*.cbz;*.zip)\0*.zip;*.cbz\0EPUB Files (*.epub)\0*.epub\0Image Files (*.png;*.jpeg;*.tiff)\0*.png;*.jpg;*.jpe;*.jpeg;*.jfif;*.tif;*.tiff\0All Files\0*\0\0";
	ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	return GetOpenFileNameW(&ofn);
}

int wingetsavepath(pdfapp_t *app, char *buf, int len)
{
	wchar_t twbuf[PATH_MAX];
	OPENFILENAME ofn;

	wcscpy(twbuf, wbuf);
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwndframe;
	ofn.lpstrFile = twbuf;
	ofn.nMaxFile = PATH_MAX;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"MuPDF: Save PDF file";
	ofn.lpstrFilter = L"PDF Documents (*.pdf)\0*.pdf\0All Files\0*\0\0";
	ofn.Flags = OFN_HIDEREADONLY;
	if (GetSaveFileName(&ofn))
	{
		int code = WideCharToMultiByte(CP_UTF8, 0, twbuf, -1, buf, MIN(PATH_MAX, len), NULL, NULL);
		if (code == 0)
		{
			winerror(&gapp, "cannot convert filename to utf-8");
			return 0;
		}

		wcscpy(wbuf, twbuf);
		strcpy(filename, buf);
		return 1;
	}
	else
	{
		return 0;
	}
}

void winreplacefile(char *source, char *target)
{
	wchar_t wsource[PATH_MAX];
	wchar_t wtarget[PATH_MAX];

	int sz = MultiByteToWideChar(CP_UTF8, 0, source, -1, wsource, PATH_MAX);
	if (sz == 0)
	{
		winerror(&gapp, "cannot convert filename to Unicode");
		return;
	}

	sz = MultiByteToWideChar(CP_UTF8, 0, target, -1, wtarget, PATH_MAX);
	if (sz == 0)
	{
		winerror(&gapp, "cannot convert filename to Unicode");
		return;
	}

#if (_WIN32_WINNT >= 0x0500)
	ReplaceFile(wtarget, wsource, NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL);
#else
	DeleteFile(wtarget);
	MoveFile(wsource, wtarget);
#endif
}

void wincopyfile(char *source, char *target)
{
	wchar_t wsource[PATH_MAX];
	wchar_t wtarget[PATH_MAX];

	int sz = MultiByteToWideChar(CP_UTF8, 0, source, -1, wsource, PATH_MAX);
	if (sz == 0)
	{
		winerror(&gapp, "cannot convert filename to Unicode");
		return;
	}

	sz = MultiByteToWideChar(CP_UTF8, 0, target, -1, wtarget, PATH_MAX);
	if (sz == 0)
	{
		winerror(&gapp, "cannot convert filename to Unicode");
		return;
	}

	CopyFile(wsource, wtarget, FALSE);
}

static char pd_filename[256] = "The file is encrypted.";
static char pd_password[256] = "";
static wchar_t pd_passwordw[256] = {0};
static char td_textinput[1024] = "";
static int td_retry = 0;
static int cd_nopts;
static int *cd_nvals;
static char **cd_opts;
static char **cd_vals;
static int pd_okay = 0;

INT CALLBACK
dlogpassproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		SetDlgItemTextA(hwnd, 4, pd_filename);
		return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case 1:
			pd_okay = 1;
			GetDlgItemTextW(hwnd, 3, pd_passwordw, nelem(pd_passwordw));
			EndDialog(hwnd, 1);
			WideCharToMultiByte(CP_UTF8, 0, pd_passwordw, -1, pd_password, sizeof pd_password, NULL, NULL);
			return TRUE;
		case 2:
			pd_okay = 0;
			EndDialog(hwnd, 1);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT CALLBACK
dlogtextproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		SetDlgItemTextA(hwnd, 3, td_textinput);
		if (!td_retry)
			ShowWindow(GetDlgItem(hwnd, 4), SW_HIDE);
		return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case 1:
			pd_okay = 1;
			GetDlgItemTextA(hwnd, 3, td_textinput, sizeof td_textinput);
			EndDialog(hwnd, 1);
			return TRUE;
		case 2:
			pd_okay = 0;
			EndDialog(hwnd, 1);
			return TRUE;
		}
		break;
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hwnd, 4))
		{
			SetTextColor((HDC)wParam, RGB(255,0,0));
			SetBkMode((HDC)wParam, TRANSPARENT);

			return (INT)GetStockObject(NULL_BRUSH);
		}
		break;
	}
	return FALSE;
}

INT CALLBACK
dlogchoiceproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND listbox;
	int i;
	int item;
	int sel;
	switch(message)
	{
	case WM_INITDIALOG:
		listbox = GetDlgItem(hwnd, 3);
		for (i = 0; i < cd_nopts; i++)
			SendMessageA(listbox, LB_ADDSTRING, 0, (LPARAM)cd_opts[i]);

		/* FIXME: handle multiple select */
		if (*cd_nvals > 0)
		{
			item = SendMessageA(listbox, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)cd_vals[0]);
			if (item != LB_ERR)
				SendMessageA(listbox, LB_SETCURSEL, item, 0);
		}
		return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case 1:
			listbox = GetDlgItem(hwnd, 3);
			*cd_nvals = 0;
			for (i = 0; i < cd_nopts; i++)
			{
				item = SendMessageA(listbox, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)cd_opts[i]);
				sel = SendMessageA(listbox, LB_GETSEL, item, 0);
				if (sel && sel != LB_ERR)
					cd_vals[(*cd_nvals)++] = cd_opts[i];
			}
			pd_okay = 1;
			EndDialog(hwnd, 1);
			return TRUE;
		case 2:
			pd_okay = 0;
			EndDialog(hwnd, 1);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

char *winpassword(pdfapp_t *app, char *filename)
{
	char buf[1024], *s;
	int code;
	strcpy(buf, filename);
	s = buf;
	if (strrchr(s, '\\')) s = strrchr(s, '\\') + 1;
	if (strrchr(s, '/')) s = strrchr(s, '/') + 1;
	if (strlen(s) > 32)
		strcpy(s + 30, "...");
	sprintf(pd_filename, "The file \"%s\" is encrypted.", s);
	code = DialogBoxW(NULL, L"IDD_DLOGPASS", hwndframe, dlogpassproc);
	if (code <= 0)
		winerror(app, "cannot create password dialog");
	if (pd_okay)
		return pd_password;
	return NULL;
}

char *wintextinput(pdfapp_t *app, char *inittext, int retry)
{
	int code;
	td_retry = retry;
	fz_strlcpy(td_textinput, inittext ? inittext : "", sizeof td_textinput);
	code = DialogBoxW(NULL, L"IDD_DLOGTEXT", hwndframe, dlogtextproc);
	if (code <= 0)
		winerror(app, "cannot create text input dialog");
	if (pd_okay)
		return td_textinput;
	return NULL;
}

int winchoiceinput(pdfapp_t *app, int nopts, char *opts[], int *nvals, char *vals[])
{
	int code;
	cd_nopts = nopts;
	cd_nvals = nvals;
	cd_opts = opts;
	cd_vals = vals;
	code = DialogBoxW(NULL, L"IDD_DLOGLIST", hwndframe, dlogchoiceproc);
	if (code <= 0)
		winerror(app, "cannot create text input dialog");
	return pd_okay;
}

INT CALLBACK
dloginfoproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char buf[256];
	wchar_t bufx[256];
	fz_context *ctx = gapp.ctx;
	fz_document *doc = gapp.doc;

	switch(message)
	{
	case WM_INITDIALOG:

		SetDlgItemTextW(hwnd, 0x10, wbuf);

		if (fz_lookup_metadata(ctx, doc, FZ_META_FORMAT, buf, sizeof buf) >= 0)
		{
			SetDlgItemTextA(hwnd, 0x11, buf);
		}
		else
		{
			SetDlgItemTextA(hwnd, 0x11, "Unknown");
			SetDlgItemTextA(hwnd, 0x12, "None");
			SetDlgItemTextA(hwnd, 0x13, "n/a");
			return TRUE;
		}

		if (fz_lookup_metadata(ctx, doc, FZ_META_ENCRYPTION, buf, sizeof buf) >= 0)
		{
			SetDlgItemTextA(hwnd, 0x12, buf);
		}
		else
		{
			SetDlgItemTextA(hwnd, 0x12, "None");
		}

		buf[0] = 0;
		if (fz_has_permission(ctx, doc, FZ_PERMISSION_PRINT))
			strcat(buf, "print, ");
		if (fz_has_permission(ctx, doc, FZ_PERMISSION_COPY))
			strcat(buf, "copy, ");
		if (fz_has_permission(ctx, doc, FZ_PERMISSION_EDIT))
			strcat(buf, "edit, ");
		if (fz_has_permission(ctx, doc, FZ_PERMISSION_ANNOTATE))
			strcat(buf, "annotate, ");
		if (strlen(buf) > 2)
			buf[strlen(buf)-2] = 0;
		else
			strcpy(buf, "none");
		SetDlgItemTextA(hwnd, 0x13, buf);

#define SETUTF8(ID, STRING) \
		if (fz_lookup_metadata(ctx, doc, "info:" STRING, buf, sizeof buf) >= 0) \
		{ \
			MultiByteToWideChar(CP_UTF8, 0, buf, -1, bufx, nelem(bufx)); \
			SetDlgItemTextW(hwnd, ID, bufx); \
		}

		SETUTF8(0x20, "Title");
		SETUTF8(0x21, "Author");
		SETUTF8(0x22, "Subject");
		SETUTF8(0x23, "Keywords");
		SETUTF8(0x24, "Creator");
		SETUTF8(0x25, "Producer");
		SETUTF8(0x26, "CreationDate");
		SETUTF8(0x27, "ModDate");
		return TRUE;

	case WM_COMMAND:
		EndDialog(hwnd, 1);
		return TRUE;
	}
	return FALSE;
}

void info()
{
	int code = DialogBoxW(NULL, L"IDD_DLOGINFO", hwndframe, dloginfoproc);
	if (code <= 0)
		winerror(&gapp, "cannot create info dialog");
}

INT CALLBACK
dlogaboutproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		SetDlgItemTextA(hwnd, 2, pdfapp_version(&gapp));
		SetDlgItemTextA(hwnd, 3, pdfapp_usage(&gapp));
		return TRUE;
	case WM_COMMAND:
		EndDialog(hwnd, 1);
		return TRUE;
	}
	return FALSE;
}

void winhelp(pdfapp_t *app)
{
	int code = DialogBoxW(NULL, L"IDD_DLOGABOUT", hwndframe, dlogaboutproc);
	if (code <= 0)
		winerror(&gapp, "cannot create help dialog");
}

/*
 * Main window
 */


extern __declspec(dllexport) void winopen(HWND handle, char *classname)
{
	WNDCLASS wc;
	HMENU menu;
	RECT r;
	ATOM a;
	int ret = 0;


	/* Get screen size */
	SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
	pdfapp_ext_set_screenwh(&gapp, r.right - r.left,  r.bottom - r.top);
	PDF_LOGI("[winopen]===>right=%d, left=%d, top=%d, bottom=%d\n", r.right, r.left, r.top, r.bottom);

	/* Create cursors */
	arrowcurs = LoadCursor(NULL, IDC_ARROW);
	handcurs = LoadCursor(NULL, IDC_HAND);
	waitcurs = LoadCursor(NULL, IDC_WAIT);
	caretcurs = LoadCursor(NULL, IDC_IBEAM);

	/* And a background color */
	bgbrush = CreateSolidBrush(RGB(0x70,0x70,0x70));
	shbrush = CreateSolidBrush(RGB(0x40,0x40,0x40));

	/* Init DIB info for buffer */
	dibinf = (BITMAPINFO*)malloc(sizeof(BITMAPINFO) + 12);
	//assert(dibinf);
	dibinf->bmiHeader.biSize = sizeof(dibinf->bmiHeader);
	dibinf->bmiHeader.biPlanes = 1;
	dibinf->bmiHeader.biBitCount = 32;
	dibinf->bmiHeader.biCompression = BI_RGB;
	dibinf->bmiHeader.biXPelsPerMeter = 2834;
	dibinf->bmiHeader.biYPelsPerMeter = 2834;
	dibinf->bmiHeader.biClrUsed = 0;
	dibinf->bmiHeader.biClrImportant = 0;
	dibinf->bmiHeader.biClrUsed = 0;
	
	hdc = NULL;
	
//	memset(classname, 0, 100);

//	ret = GetClassName(hwndframe, classname, 100);
//	PDF_LOGE("[%s] GetClassName ret = %d\n", __FUNCTION__, ret);
//	PDF_LOGE("[%s] classname :%s\n", __FUNCTION__, classname);

	hwndframe = handle;

	SetWindowTextW(hwndframe, L"MuPDF");

	menu = GetSystemMenu(hwndframe, 0);
	AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(menu, MF_STRING, ID_ABOUT, L"About MuPDF...");
	AppendMenuW(menu, MF_STRING, ID_DOCINFO, L"Document Properties...");

	SetCursor(arrowcurs);

	
}



//static void
extern __declspec(dllexport)void
do_close(pdfapp_t *app)
{
	fz_context *ctx = app->ctx;
	pdfapp_close(app);
	free(dibinf);
	fz_drop_context(ctx);
}

void winclose(pdfapp_t *app)
{
	if (pdfapp_preclose(app))
	{
		do_close(app);
		exit(0);
	}
}

void wincursor(pdfapp_t *app, int curs)
{
	if (curs == ARROW)
		SetCursor(arrowcurs);
	if (curs == HAND)
		SetCursor(handcurs);
	if (curs == WAIT)
		SetCursor(waitcurs);
	if (curs == CARET)
		SetCursor(caretcurs);
}

void wintitle(pdfapp_t *app, char *title)
{
	wchar_t wide[256], *dp;
	char *sp;
	int rune;

	dp = wide;
	sp = title;
	while (*sp && dp < wide + 255)
	{
		sp += fz_chartorune(&rune, sp);
		*dp++ = rune;
	}
	*dp = 0;

	SetWindowTextW(hwndframe, wide);
}

void windrawrect(pdfapp_t *app, int x0, int y0, int x1, int y1)
{
	RECT r;
	r.left = x0;
	r.top = y0;
	r.right = x1;
	r.bottom = y1;
	FillRect(hdc, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

void windrawstring(pdfapp_t *app, int x, int y, char *s)
{
	HFONT font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	SelectObject(hdc, font);
	TextOutA(hdc, x, y - 12, s, strlen(s));
}

void winblitsearch()
{
	if (gapp.issearching)
	{
		char buf[sizeof(gapp.search) + 50];
		sprintf(buf, "Search: %s", gapp.search);
		windrawrect(&gapp, 0, 0, gapp.winw, 30);
		windrawstring(&gapp, 10, 20, buf);
	}
}

void winblit()
{
	int image_w = fz_pixmap_width(gapp.ctx, gapp.image);
	int image_h = fz_pixmap_height(gapp.ctx, gapp.image);
	int image_n = fz_pixmap_components(gapp.ctx, gapp.image);
	unsigned char *samples = fz_pixmap_samples(gapp.ctx, gapp.image);
	int x0 = gapp.panx;
	int y0 = gapp.pany;
	int x1 = gapp.panx + image_w;
	int y1 = gapp.pany + image_h;
	RECT r;

	if (gapp.image)
	{
		if (gapp.iscopying || justcopied)
		{
			pdfapp_invert(&gapp, &gapp.selr);
			justcopied = 1;
		}

		pdfapp_inverthit(&gapp);

		dibinf->bmiHeader.biWidth = image_w;
		dibinf->bmiHeader.biHeight = -image_h;
		dibinf->bmiHeader.biSizeImage = image_h * 4;

		if (image_n == 2)
		{
			int i = image_w * image_h;
			unsigned char *color = (unsigned char*)malloc(i*4);
			unsigned char *s = samples;
			unsigned char *d = color;
			for (; i > 0 ; i--)
			{
				d[2] = d[1] = d[0] = *s++;
				d[3] = *s++;
				d += 4;
			}
			SetDIBitsToDevice(hdc,
				gapp.panx, gapp.pany, image_w, image_h,
				0, 0, 0, image_h, color,
				dibinf, DIB_RGB_COLORS);
			free(color);
		}
		if (image_n == 4)
		{
			SetDIBitsToDevice(hdc,
				gapp.panx, gapp.pany, image_w, image_h,
				0, 0, 0, image_h, samples,
				dibinf, DIB_RGB_COLORS);
		}

		pdfapp_inverthit(&gapp);

		if (gapp.iscopying || justcopied)
		{
			pdfapp_invert(&gapp, &gapp.selr);
			justcopied = 1;
		}
	}

	/* Grey background */
	r.top = 0; r.bottom = gapp.winh;
	r.left = 0; r.right = x0;
	FillRect(hdc, &r, bgbrush);
	r.left = x1; r.right = gapp.winw;
	FillRect(hdc, &r, bgbrush);
	r.left = 0; r.right = gapp.winw;
	r.top = 0; r.bottom = y0;
	FillRect(hdc, &r, bgbrush);
	r.top = y1; r.bottom = gapp.winh;
	FillRect(hdc, &r, bgbrush);

	/* Drop shadow */
	r.left = x0 + 2;
	r.right = x1 + 2;
	r.top = y1;
	r.bottom = y1 + 2;
	FillRect(hdc, &r, shbrush);
	r.left = x1;
	r.right = x1 + 2;
	r.top = y0 + 2;
	r.bottom = y1;
	FillRect(hdc, &r, shbrush);

	winblitsearch();
}

void winresize(pdfapp_t *app, int w, int h)
{
	ShowWindow(hwndframe, SW_SHOWDEFAULT);
	w += GetSystemMetrics(SM_CXFRAME) * 2;
	h += GetSystemMetrics(SM_CYFRAME) * 2;
	h += GetSystemMetrics(SM_CYCAPTION);
	SetWindowPos(hwndframe, 0, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
}

void winrepaint(pdfapp_t *app)
{
//	InvalidateRect(hwndview, NULL, 0);
	InvalidateRect(hwndframe, NULL, 0);


}

void winrepaintsearch(pdfapp_t *app)
{
	// TODO: invalidate only search area and
	// call only search redraw routine.
//	InvalidateRect(hwndview, NULL, 0);
	InvalidateRect(hwndframe, NULL, 0);
}

void winfullscreen(pdfapp_t *app, int state)
{
	static WINDOWPLACEMENT savedplace;
	static int isfullscreen = 0;
	if (state && !isfullscreen)
	{
		GetWindowPlacement(hwndframe, &savedplace);
		SetWindowLong(hwndframe, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(hwndframe, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowWindow(hwndframe, SW_SHOWMAXIMIZED);
		isfullscreen = 1;
	}
	if (!state && isfullscreen)
	{
		SetWindowLong(hwndframe, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(hwndframe, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		SetWindowPlacement(hwndframe, &savedplace);
		isfullscreen = 0;
	}
}

/*
 * Event handling
 */

void windocopy(pdfapp_t *app)
{
	HGLOBAL handle;
	unsigned short *ucsbuf;

	if (!OpenClipboard(hwndframe))
		return;
	EmptyClipboard();

	handle = GlobalAlloc(GMEM_MOVEABLE, 4096 * sizeof(unsigned short));
	if (!handle)
	{
		CloseClipboard();
		return;
	}

	ucsbuf = GlobalLock(handle);
	pdfapp_oncopy(&gapp, ucsbuf, 4096);
	GlobalUnlock(handle);

	SetClipboardData(CF_UNICODETEXT, handle);
	CloseClipboard();

	justcopied = 1;	/* keep inversion around for a while... */
}

void winreloadpage(pdfapp_t *app)
{
//	SendMessage(hwndview, WM_APP, 0, 0);
	SendMessage(hwndframe, WM_APP, 0, 0);
}

void winopenuri(pdfapp_t *app, char *buf)
{
	ShellExecuteA(hwndframe, "open", buf, 0, 0, SW_SHOWNORMAL);
}

#define OUR_TIMER_ID 1

void winadvancetimer(pdfapp_t *app, float delay)
{
	timer_pending = 1;
//	SetTimer(hwndview, OUR_TIMER_ID, (unsigned int)(1000*delay), NULL);
	SetTimer(hwndframe, OUR_TIMER_ID, (unsigned int)(1000*delay), NULL);

}



static void killtimer(pdfapp_t *app)
{
	timer_pending = 0;
}


void handlekey(int c)
{
	int modifier = (GetAsyncKeyState(VK_SHIFT) < 0);
	modifier |= ((GetAsyncKeyState(VK_CONTROL) < 0)<<2);

	if (timer_pending)
		killtimer(&gapp);

//	if (GetCapture() == hwndview)
//		return;

	if (GetCapture() == hwndframe)
		return;


	if (justcopied)
	{
		justcopied = 0;
		winrepaint(&gapp);
	}

	/* translate VK into ASCII equivalents */
	if (c > 256)
	{
		switch (c - 256)
		{
		case VK_F1: c = '?'; break;
		case VK_ESCAPE: c = '\033'; break;
		case VK_DOWN: c = 'j'; break;
		case VK_UP: c = 'k'; break;
		case VK_LEFT: c = 'b'; break;
		case VK_RIGHT: c = ' '; break;
		case VK_PRIOR: c = ','; break;
		case VK_NEXT: c = '.'; break;
		}
	}

	pdfapp_onkey(&gapp, c, modifier);
	winrepaint(&gapp);
}

void handlemouse(int x, int y, int btn, int state)
{
	int modifier = (GetAsyncKeyState(VK_SHIFT) < 0);
	modifier |= ((GetAsyncKeyState(VK_CONTROL) < 0)<<2);

	if (state != 0 && timer_pending)
		killtimer(&gapp);

	if (state != 0 && justcopied)
	{
		justcopied = 0;
		winrepaint(&gapp);
	}

//	if (state == 1)
//		SetCapture(hwndview);
	if (state == 1)
		SetCapture(hwndframe);
	if (state == -1)
		ReleaseCapture();

	pdfapp_onmouse(&gapp, x, y, btn, modifier, state);
}


static int oldx = 0;
static int oldy = 0;

extern __declspec(dllexport) int \
wm_size(HWND hwnd, int w, int h)
{
	
//	int x = (signed short) LOWORD(lParam);
//	int y = (signed short) HIWORD(lParam);

//	if (wParam == SIZE_MINIMIZED)
//		return 0;
//	if (wParam == SIZE_MAXIMIZED)
//		pdfapp_ext_set_shrinkwrap(&gapp,0);


	PDF_LOGI("[wm_size]===>w=%d, h=%d\n", w, h);
	pdfapp_onresize(&gapp, w, h);

	return 0;
}


extern __declspec(dllexport) int \
wm_paint(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	

	PAINTSTRUCT ps;
	hdc = BeginPaint(hwnd, &ps);
	winblit();
	hdc = NULL;
	EndPaint(hwnd, &ps);
	pdfapp_postblit(&gapp);
	return 0;
}


extern __declspec(dllexport) int \
wm_lbuttondown(HWND hwnd, int x, int y)
{

	SetFocus(hwndframe);
	oldx = x; oldy = y;
	handlemouse(x, y, 1, 1);

	return 0;
}

extern __declspec(dllexport) int \
wm_mbuttondown(HWND hwnd, int x, int y)
{

	SetFocus(hwndframe);
	oldx = x; oldy = y;
	handlemouse(x, y, 2, 1);
	return 0;

}


extern __declspec(dllexport) int \
wm_rbuttondown(HWND hwnd, int x, int y)
{

	SetFocus(hwndframe);
	oldx = x; oldy = y;
	handlemouse(x, y, 3, 1);
	return 0;
}


extern __declspec(dllexport) int \
wm_lbuttonup(HWND hwnd, int x, int y)
{

	oldx = x; oldy = y;
	handlemouse(x, y, 1, -1);

	return 0;
}

	

extern __declspec(dllexport) int \
wm_mbuttonup(HWND hwnd, int x, int y)
{

	oldx = x; oldy = y;
	handlemouse(x, y, 2, -1);
	return 0;

	return 0;
}

extern __declspec(dllexport) int \
wm_rbuttonup(HWND hwnd, int x, int y)
{

	oldx = x; oldy = y;
	handlemouse(x, y, 3, -1);
	return 0;
}

extern __declspec(dllexport) int \
wm_mousemove(HWND hwnd, int x, int y)
{

	oldx = x; oldy = y;
	handlemouse(x, y, 0, 0);
	return 0;
}


extern __declspec(dllexport) int \
wm_mousewheel(HWND hwnd, int dir)
{
	
	if (dir > 0)
	{
		handlemouse(oldx, oldy, 4, 1);
		handlemouse(oldx, oldy, 4, -1);
	}
	else
	{
		handlemouse(oldx, oldy, 5, 1);
		handlemouse(oldx, oldy, 5, -1);
	}
	return 0;
	
}


extern __declspec(dllexport) int \
wm_timer(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	int x = (signed short) LOWORD(lParam);
	int y = (signed short) HIWORD(lParam);
	
	if (wParam == OUR_TIMER_ID && timer_pending && pdfapp_ext_get_presentation_mode(&gapp))
	{
		timer_pending = 0;
		handlekey(VK_RIGHT + 256);
		handlemouse(oldx, oldy, 0, 0); /* update cursor */
		return 0;
	}
	return 0;
}


extern __declspec(dllexport) int \
wm_keydown(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	


	switch (wParam)
	{
	case VK_F1:
	case VK_LEFT:
	case VK_UP:
	case VK_PRIOR:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_NEXT:
	case VK_ESCAPE:
		handlekey(wParam + 256);
		handlemouse(oldx, oldy, 0, 0);	/* update cursor */
		return 0;
	}
	return 0;

}



extern __declspec(dllexport) int \
wm_char(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	int x = (signed short) LOWORD(lParam);
	int y = (signed short) HIWORD(lParam);
	
	if (wParam < 256)
	{
		handlekey(wParam);
		handlemouse(oldx, oldy, 0, 0);	/* update cursor */
	}
	return 0;

}

extern __declspec(dllexport) int \
wm_app(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	int x = (signed short) LOWORD(lParam);
	int y = (signed short) HIWORD(lParam);
	pdfapp_reloadpage(&gapp);
	return 0;

}
	
extern pdfapp_t  *obtain_gapp(void)
{
	return &gapp;

}




extern __declspec(dllexport) void  win_pdfapp_init(void)
{
	fz_context *ctx;
	
	ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
	if (!ctx)
	{
		fprintf(stderr, "cannot initialise context\n");
		exit(1);
	}

	app_print_open("mupdf.log", 2);
	pdfapp_init(ctx, &gapp);
	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	
}

extern __declspec(dllexport) void  win_pdfapp_open(wchar_t *wfilename,int bps)
{
	int code;
	char filename[1024];
	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	code = WideCharToMultiByte(CP_ACP, 0, wfilename, -1, filename, sizeof filename, NULL, NULL);
	if (code == 0){
			return;
	}

	if (bps)
		pdfapp_open_progressive(&gapp, filename, 0, bps);
	else
		pdfapp_open(&gapp, filename, 0);
	
}

extern __declspec(dllexport) void  win_do_close(void)
{
	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	do_close(&gapp);
}

extern __declspec(dllexport) void  win_pdfapp_fini(void)
{
	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	pdfapp_fini(&gapp);
	app_print_close();
}


extern __declspec(dllexport) int  win_doc_pagecount(void)
{
	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	return gapp.pagecount;
	

}

extern __declspec(dllexport) int  win_doc_pageno(void)
{	
	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	return gapp.pageno;

}

extern __declspec(dllexport) int  win_gotopage(int number)
{
	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	pdfapp_gotopage(&gapp, number);
	return 0;
}

/*extern __declspec(dllexport) int  win_oncopy(MyRectangle* myrect, char* outbuf, unsigned int* olen)
{
	//MyRectangle testrect;
	int size  = 16 * 1024;
	int copyucs2[16 * 1024];

	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	PDF_LOGI("[%s]left=%d, right=%d, top=%d, bottom=%d, ucslen = %d\n", __FUNCTION__, myrect->Left, myrect->Right, myrect->Top, myrect->Bottom, *olen);

	//testrect.Bottom = 515;
	//testrect.Right = 652;
	//testrect.Left = 0;
	//testrect.Top= 0;
	pdfapp_copytext(&gapp, *myrect, copyucs2, &size);
	PDF_LOGI("the content size is %d, the foure is %d, %d, %d, %d\n", size, copyucs2[0], 
		copyucs2[1], copyucs2[2], copyucs2[3]);
	PDF_LOGI("[%s] here 1 ~~\n", __FUNCTION__);
	memcpy(outbuf, (char*)copyucs2, size*4);
	PDF_LOGI("[%s] here 2 ~~\n", __FUNCTION__);
	*olen  = size*4;
	PDF_LOGI("[%s] here 3 ~~\n", __FUNCTION__);
	return 0;
}*/

extern __declspec(dllexport) int  win_oncopy(MyRectangle* myrect, int* outbuf, unsigned int* olen)
{
	//MyRectangle testrect;
	int size  = 16 * 1024;
	int copyucs2[16 * 1024];
	MyRectangle hijack;

	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	PDF_LOGI("[%s]left=%d, right=%d, top=%d, bottom=%d, ucslen = %d\n", __FUNCTION__, myrect->Left, myrect->Right, myrect->Top, myrect->Bottom, *olen);

	//testrect.Bottom = 515;
	//testrect.Right = 652;
	//testrect.Left = 0;
	//testrect.Top= 0;
	pdfapp_copytext(&gapp, *myrect, copyucs2, &size, &hijack);
	PDF_LOGI("the content size is %d, the four is %d, %d, %d, %d\n", size, copyucs2[0], 
		copyucs2[1], copyucs2[2], copyucs2[3]);
	memcpy(outbuf, copyucs2, size*4);
	PDF_LOGI("after copy, the four is %d, %d, %d, %d\n", outbuf[0], 
		outbuf[1], outbuf[2], outbuf[3]);
	*olen  = size;
	PDF_LOGI("[%s] here 3 ~~\n", __FUNCTION__);
	return 0;
}

extern __declspec(dllexport) int  win_oncopy_utf8(MyRectangle* myrect, char* outbuf, unsigned int* olen, MyRectangle* orect)
{
	//MyRectangle testrect;
	int size  = 16 * 1024;
	int copyucs2[16 * 1024];
	char *utf8 = outbuf;
	int *ucs2;
	int ucs;
	int realsize = 0;

	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	PDF_LOGI("[%s]left=%d, right=%d, top=%d, bottom=%d, ucslen = %d\n", __FUNCTION__, myrect->Left, myrect->Right, myrect->Top, myrect->Bottom, *olen);

	//testrect.Bottom = 515;
	//testrect.Right = 652;
	//testrect.Left = 0;
	//testrect.Top= 0;
	pdfapp_copytext(&gapp, *myrect, copyucs2, &size, orect);
	PDF_LOGI("[%s]the content size is %d, the four is %d, %d, %d, %d\n", __FUNCTION__, size, copyucs2[0], 
		copyucs2[1], copyucs2[2], copyucs2[3]);

	//convert to utf8
	for (ucs2 = copyucs2; ucs2[0] != 0; ucs2++)
	{
		ucs = ucs2[0];

		realsize = fz_runetochar(utf8, ucs);
		utf8 += realsize;
		*olen += realsize;
	}
	*utf8 = 0;

	return 0;
}

extern __declspec(dllexport) int  win_search_in_directorn(char* content, int dir)
{
#if 0
	int n = strlen(content);

	PDF_LOGI("in function %s ~~\n", __FUNCTION__);
	if (n + 2 < sizeof( gapp.search))
	{	
		snprintf(gapp.search, sizeof(gapp.search), content);
		gapp.searchdir = 1;
		
		if (dir < 0)
		{
			if (gapp.pageno == 1)
				gapp.pageno = gapp.pagecount;
			else
				gapp.pageno--;
			pdfapp_showpage(&gapp, 1, 1, 0, 0, 1);
		}

		if(dir > 0)
			pdfapp_onkey(&gapp, 'n', 0);
		else
			pdfapp_onkey(&gapp, 'N', 0);

	}
	else
	{
		winwarn(&gapp, "Searching content is too long.\n");
		return -1;
	}
#else
	return 0;
#endif
}

extern __declspec(dllexport) int win_autozoom(void)
{
	//pdfapp_autozoom_vertical(&gapp);
	//pdfapp_autozoom_horizontal(&gapp);

	pdfapp_autozoom(&gapp);
	
	return 0;
}

extern __declspec(dllexport) int win_getlinepos(int start_x, int start_y, MyRectangle* startrect, MyRectangle* orect)
{
	pdfapp_getlinepos(&gapp, start_x, start_y, startrect, orect);
	return 0;
}

