/*
  nnue-gui is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  nnue-gui is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

int WINAPI WinMain(const HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	inst = hInstance;

	if (init_app() == RET_O)
	{
		DialogBox(hInstance, "MAIN", NULL, (DLGPROC)dlg_proc);
	}
	return RET_O;
}

short int init_app()
{
	h_main = NULL;
	config = NULL;

	memset(app_path, 0, MAX_FILE_PATH);

	if (GetModuleFileName(NULL, app_path, MAX_FILE_PATH - 384) == 0)
	{
		MessageBox(HWND_DESKTOP, "Can't get application path", "Error", MB_OK | MB_ICONERROR);
		return RET_E;
	}
	int i = strlen(app_path) - 1;

	while (i > 0 && app_path[i] != '\\')
	{
		i--;
	}

	if (i == 0)
	{
		MessageBox(HWND_DESKTOP, "Can't get application path", "Error", MB_OK | MB_ICONERROR);
		return RET_E;
	}
	app_path[i + 1] = '\0';
	return RET_O;
}

BOOL APIENTRY dlg_proc(const HWND h_dlg, const UINT u_msg, const WPARAM w_param, LPARAM l_param)
{
	HBITMAP hBMP = 0;
	HWND hBitmap = 0;

	switch (u_msg)
	{
	case WM_INITDIALOG:
		if (h_main == NULL)
			h_main = h_dlg;

		hBitmap = GetDlgItem(h_dlg, ID_LOGO);
		hBMP = LoadBitmap(inst, MAKEINTRESOURCE(ID_BMP));
		SendMessage(hBitmap, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBMP);
		ShowWindow(hBitmap, SW_SHOW);

		load_config();
		report_engine_config();
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(w_param))
		{
		case ID_EXIT:
			stop_engine_running();
			EndDialog(h_dlg, 0);
			break;

		case ID_CONFIG:
			DialogBox(inst, "CONFIG", NULL, (DLGPROC)dlg_proc_engines);
			load_config();
			report_engine_config();
			break;

		case ID_START:
			start_thinking();
			break;

		case ID_STOP:
			stop_engine();
			break;
		default:;
		}
		return TRUE;

	case WM_LBUTTONUP:
		return TRUE;

	case WM_CLOSE:
		stop_engine_running();
		EndDialog(h_dlg, 0);
		return TRUE;

	default:
		return FALSE;
	}
}