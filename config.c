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

BOOL APIENTRY dlg_proc_engines(const HWND h_dlg, const UINT u_msg, const WPARAM w_param, LPARAM l_param)
{
	switch (u_msg)
	{
		// Init
	case WM_INITDIALOG:
		config = h_dlg;
		set_config_values();
		load_config();
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(w_param))
		{
		case IDOK:
			save_config();
			SendMessage(config, WM_CLOSE, 0, 0);
			return TRUE;

		case IDCANCEL:
			SendMessage(config, WM_CLOSE, 0, 0);
			return TRUE;

		case ID_HASH_CFG:
			// ENGINE Hash
			if (HIWORD(w_param) == CBN_SELCHANGE)
				get_engine_config();
			break;

		case ID_THREADS_CFG:
			// ENGINE Threads
			if (HIWORD(w_param) == CBN_SELCHANGE)
				get_engine_config();
			break;

		case ID_COMMAND_CFG:
			// ENGINE Threads
			if (HIWORD(w_param) == CBN_SELCHANGE)
				get_engine_config();
			break;

		case ID_LOAD_EVAL_CFG:
			// ENGINE Load Eval
			if (IsDlgButtonChecked(config, ID_LOAD_EVAL_CFG) == BST_CHECKED)
				engine_config.load_eval = 1;
			else
				engine_config.load_eval = 0;
			break;

		case ID_LOG_CFG:
			// ENGINE Log
			if (IsDlgButtonChecked(config, ID_LOG_CFG) == BST_CHECKED)
				engine_config.log = 1;
			else
				engine_config.log = 0;
			break;

		case ID_ENGINE_SELECT:
			// Select ENGINE 1
			select_engine(ID_ENGINE_PATH);
			break;

		case ID_ENGINE_TB_SELECT:
			// Select ENGINE 1 TB path
			select_engine_tb(ID_ENGINE_TB_PATH);
			break;

		default:
			return FALSE;
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(config);
		return TRUE;
	default:;
	}
	return FALSE;
}

void select_engine(const int dialog_id)
{
	OPENFILENAME ofn;
	char file_name[8192];
	int len;

	memset(file_name, 0, 8192);
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = config;
	ofn.lpstrFilter = "Executables files (*.exe)\0*.exe\0";
	ofn.lpstrFile = file_name;
	ofn.nMaxFile = 4095;
	ofn.lpstrTitle = "Load UCI engine...";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrInitialDir = NULL;

	if (GetOpenFileName(&ofn))
	{
		if ((len = strlen(file_name)) == 0 || file_exists(file_name) == RET_E)
			return;

		// Set Path
		SetDlgItemText(config, dialog_id, file_name);
		SendMessage(GetDlgItem(config, dialog_id), EM_SETSEL, (WPARAM)len, (LPARAM)len);
	}
}

void select_engine_tb(const int dialog_id)
{
	TCHAR path[MAX_FILE_PATH];
	int len = 0;
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = "Browse for folder...";
	const LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		// get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		SetCurrentDirectory(path);

		// free memory used
		IMalloc* imalloc = 0;

		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			//imalloc->Free(pidl);
			//imalloc->Release();
		}
	}
	// Set Path
	SetDlgItemText(config, dialog_id, path);
}

void get_engine_config()
{
	char tmp[8];
	char commandline[1024];

	// Get Hash Size
	memset(tmp, 0, 8);
	int i = (int)SendMessage(GetDlgItem(config, ID_HASH_CFG), CB_GETCURSEL, 0, 0);
	SendMessage(GetDlgItem(config, ID_HASH_CFG), CB_GETLBTEXT, (WPARAM)i, (LPARAM)tmp);

	if ((engine_config.hash = str_is_int(tmp)) == RET_E || engine_config.hash < MIN_HASH
		|| engine_config.hash > MAX_HASH)
	{
		engine_config.hash = 64;
		SetDlgItemText(config, ID_HASH_CFG, "64");
	}
	else
		engine_config.hash = str_is_int(tmp);

	// Get Threads
	memset(tmp, 0, 8);
	i = (int)SendMessage(GetDlgItem(config, ID_THREADS_CFG), CB_GETCURSEL, 0, 0);
	SendMessage(GetDlgItem(config, ID_THREADS_CFG), CB_GETLBTEXT, (WPARAM)i, (LPARAM)tmp);

	if ((engine_config.threads = str_is_int(tmp)) == RET_E || engine_config.threads < MIN_THREADS
		|| engine_config.threads > MAX_THREADS)
	{
		engine_config.threads = 1;
		SetDlgItemText(config, ID_THREADS_CFG, "1");
	}
	else
		engine_config.threads = str_is_int(tmp);

	// Get Command
	memset(commandline, 0, 1024);
	i = (int)SendMessage(GetDlgItem(config, ID_COMMAND_CFG), CB_GETCURSEL, 0, 0);
	SendMessage(GetDlgItem(config, ID_COMMAND_CFG), CB_GETLBTEXT, (WPARAM)i, (LPARAM)commandline);
	SetDlgItemText(config, ID_COMMAND_CFG, commandline);
}

void save_config()
{
	FILE* fp = NULL;
	char tmp[MAX_FILE_PATH];
	char engine_path[MAX_FILE_PATH];
	char engine_tb_path[MAX_FILE_PATH];
	char command_line[MAX_COMMAND];
	int i;

	memset(tmp, 0, MAX_FILE_PATH);

	sprintf(tmp, "%sengine.cfg", app_path);

	if ((fp = fopen(tmp, "w")) == NULL)
	{
		return;
	}
	fprintf(fp, "%d\n", engine_config.hash);
	fprintf(fp, "%d\n", engine_config.threads);
	fprintf(fp, "%d\n", engine_config.load_eval);
	fprintf(fp, "%d\n", engine_config.log);

	memset(engine_path, 0, MAX_FILE_PATH);
	GetDlgItemText(config, ID_ENGINE_PATH, engine_path, MAX_FILE_PATH - 1);

	if (strlen(engine_path) > 0)
		fprintf(fp, "%s\n", engine_path);
	else
		fprintf(fp, "\n");

	memset(engine_tb_path, 0, MAX_FILE_PATH);
	GetDlgItemText(config, ID_ENGINE_TB_PATH, engine_tb_path, MAX_FILE_PATH - 1);

	if (strlen(engine_tb_path) > 0)
		fprintf(fp, "%s\n", engine_tb_path);
	else
		fprintf(fp, "\n");

	memset(command_line, 0, MAX_COMMAND);
	GetDlgItemText(config, ID_COMMAND_CFG, command_line, MAX_COMMAND - 1);

	if (strlen(command_line) > 0)
		fprintf(fp, "%s", command_line);
	else
		fprintf(fp, "\n");

	fclose(fp);
}

void load_config()
{
	FILE* fp = NULL;
	char tmp[MAX_FILE_PATH];
	char line[MAX_COMMAND];
	char command[MAX_COMMAND];
	char val[8];
	int i = 0;
	int j;
	int len;

	memset(tmp, 0, MAX_FILE_PATH);
	sprintf(tmp, "%sengine.cfg", app_path);

	if ((fp = fopen(tmp, "r")) == NULL)
	{
		engine_config.hash = 16;
		engine_config.threads = 1;
		engine_config.load_eval = 1;
		engine_config.log = 1;
		return;
	}

	for (i = 0; i < 12; i++)
	{
		memset(line, 0, MAX_COMMAND);

		if (!feof(fp))
			read_file_line(fp, line, MAX_COMMAND - 1);

		switch (i)
		{
		case 0:
			// Hash Size
			if ((engine_config.hash = str_is_int(line)) == RET_E || engine_config.hash < MIN_HASH
				|| engine_config.hash > MAX_HASH)
				engine_config.hash = 64;

			for (j = 0; j < 11; j++)
			{
				memset(val, 0, 8);
				SendMessage(GetDlgItem(config, ID_HASH_CFG), CB_GETLBTEXT, (WPARAM)j, (LPARAM)val);

				if (atoi(val) == engine_config.hash)
				{
					SendDlgItemMessage(config, ID_HASH_CFG, CB_SETCURSEL, j, 0);
					break;
				}
			}
			break;

		case 1:
			// Threads
			if ((engine_config.threads = str_is_int(line)) == RET_E || engine_config.threads < MIN_THREADS
				|| engine_config.threads > MAX_THREADS)
				engine_config.threads = 1;

			for (j = 0; j < 15; j++)
			{
				memset(val, 0, 8);
				SendMessage(GetDlgItem(config, ID_THREADS_CFG), CB_GETLBTEXT, (WPARAM)j, (LPARAM)val);

				if (atoi(val) == engine_config.threads)
				{
					SendDlgItemMessage(config, ID_THREADS_CFG, CB_SETCURSEL, j, 0);
					break;
				}
			}
			break;
		case 2:
			// Load Eval
			if ((engine_config.load_eval = str_is_int(line)) == RET_E
				|| engine_config.load_eval != 0 && engine_config.load_eval != 1)
				engine_config.load_eval = 0;

			if (engine_config.load_eval == 1)
				CheckDlgButton(config, ID_LOAD_EVAL_CFG, BST_CHECKED);
			else
				CheckDlgButton(config, ID_LOAD_EVAL_CFG, BST_UNCHECKED);
			break;

		case 3:
			// Log
			if ((engine_config.log = str_is_int(line)) == RET_E
				|| engine_config.log != 0 && engine_config.log != 1)
				engine_config.log = 0;

			if (engine_config.log == 1)
				CheckDlgButton(config, ID_LOG_CFG, BST_CHECKED);
			else
				CheckDlgButton(config, ID_LOG_CFG, BST_UNCHECKED);
			break;

		case 4:
			// ENGINE
			if ((len = strlen(line)) > 0 && file_exists(line) == RET_O)
			{
				strcpy(engine.path, line);
				SetDlgItemText(config, ID_ENGINE_PATH, line);
				SendMessage(GetDlgItem(config, ID_ENGINE_PATH), EM_SETSEL, (WPARAM)len, (LPARAM)len);
				load_engine(0);
			}
			break;

		case 5:
			// ENGINE_TBPATH
			if ((len = strlen(line) > 0))
			{
				strcpy(engine.tb_path, line);
				SetDlgItemText(config, ID_ENGINE_TB_PATH, line);
				SendMessage(GetDlgItem(config, ID_ENGINE_TB_PATH), EM_SETSEL, (WPARAM)len, (LPARAM)len);
			}
			break;

		case 6:
			// ENGINE1_COMMAND

			if ((len = strlen(line)) > 0)
				strcpy(engine_config.command, line);
			else
				strcpy(engine_config.command, "gensfen depth 8 loop 10000000 output_file_name trainingdata\\generated_kifu.bin\n");

			strcat(engine_config.command, "\n");
			SetDlgItemText(config, ID_COMMAND_CFG, engine_config.command);

			for (j = 0; j < 3; j++)
			{
				memset(command, 0, 1024);
				SendMessage(GetDlgItem(config, ID_COMMAND_CFG), CB_GETLBTEXT, (WPARAM)j, (LPARAM)command);

				if (command == engine_config.command)
				{
					SendDlgItemMessage(config, ID_COMMAND_CFG, CB_SETCURSEL, j, 0);
					break;
				}
			}
			break;
		default:;
		}
	}
	fclose(fp);
}

void report_engine_config()
{
	char tmp[16];
	memset(tmp, 0, 16);

	char hash[16];
	char threads[16];
	char load_eval[16];
	char log[16];

	memset(hash, 0, 16);
	memset(threads, 0, 16);
	memset(load_eval, 0, 16);
	memset(log, 0, 16);

	itoa(engine_config.hash, tmp, 10);
	sprintf(hash, "%s", "Hash ");
	strcat(hash, tmp);
	strcat(hash, " MB");
	SetDlgItemText(h_main, ID_HASH, hash);

	itoa(engine_config.threads, tmp, 10);
	sprintf(threads, "%s", "Threads ");
	strcat(threads, tmp);
	SetDlgItemText(h_main, ID_THREADS, threads);

	itoa(engine_config.load_eval, tmp, 10);
	sprintf(load_eval, "%s", "Load Eval ");
	strcat(load_eval, tmp);
	SetDlgItemText(h_main, ID_LOAD_EVAL, load_eval);

	itoa(engine_config.log, tmp, 10);
	sprintf(log, "%s", "Log ");
	strcat(log, tmp);
	SetDlgItemText(h_main, ID_LOG, log);
}

void set_config_values()
{
	// Set ENGINE Config

	SendDlgItemMessage(config, ID_COMMAND_CFG, CB_ADDSTRING, 0, (LPARAM)TEXT("gensfen depth 8 loop 10000000 output_file_name trainingdata\\generated_kifu.bin"));
	SendDlgItemMessage(config, ID_COMMAND_CFG, CB_ADDSTRING, 1, (LPARAM)TEXT("gensfen depth 8 loop 1000000 output_file_name validationdata\\generated_kifu.bin"));
	SendDlgItemMessage(config, ID_COMMAND_CFG, CB_ADDSTRING, 2, (LPARAM)TEXT("learn targetdir trainingdata loop 100 batchsize 1000000 eta 1.0 lambda 0.5 eval_limit 32000 nn_batch_size 1000 newbob_decay 0.5 eval_save_interval 10000000 loss_output_interval 1000000 mirror_percentage 50 validation_set_file_name validationdata\\generated_kifu.bin"));
	SendDlgItemMessage(config, ID_COMMAND_CFG, CB_SETCURSEL, 0, 0);

	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 0, (LPARAM)TEXT("16"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 1, (LPARAM)TEXT("32"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 2, (LPARAM)TEXT("64"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 3, (LPARAM)TEXT("128"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 4, (LPARAM)TEXT("256"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 5, (LPARAM)TEXT("512"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 6, (LPARAM)TEXT("1024"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 7, (LPARAM)TEXT("2048"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 8, (LPARAM)TEXT("4096"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 9, (LPARAM)TEXT("8192"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 10, (LPARAM)TEXT("16384"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 11, (LPARAM)TEXT("32768"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_ADDSTRING, 12, (LPARAM)TEXT("65536"));
	SendDlgItemMessage(config, ID_HASH_CFG, CB_SETCURSEL, 0, 0);

	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 0, (LPARAM)TEXT("1"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 1, (LPARAM)TEXT("2"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 2, (LPARAM)TEXT("3"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 3, (LPARAM)TEXT("4"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 4, (LPARAM)TEXT("5"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 5, (LPARAM)TEXT("6"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 6, (LPARAM)TEXT("7"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 7, (LPARAM)TEXT("8"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 8, (LPARAM)TEXT("9"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 9, (LPARAM)TEXT("10"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 10, (LPARAM)TEXT("11"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 11, (LPARAM)TEXT("12"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 12, (LPARAM)TEXT("13"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 13, (LPARAM)TEXT("14"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 14, (LPARAM)TEXT("15"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_ADDSTRING, 15, (LPARAM)TEXT("16"));
	SendDlgItemMessage(config, ID_THREADS_CFG, CB_SETCURSEL, 0, 0);

	// Set ENGINE Limits
	SendDlgItemMessage(config, ID_ENGINE_PATH, EM_LIMITTEXT, (WPARAM)(MAX_FILE_PATH - 1), 0);
	SendDlgItemMessage(config, ID_ENGINE_TB_PATH, EM_LIMITTEXT, (WPARAM)(MAX_FILE_PATH - 1), 0);
}