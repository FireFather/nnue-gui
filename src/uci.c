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

short int send_uci_engine(char* cmd, const short int send)
{
	DWORD dw_written;
	const int len = strlen(cmd);

	if (engine.is_running == 0 || len < 3)
		return RET_E;

	engine.send = send;

	if (!WriteFile(engine.write_in, cmd, len, &dw_written, NULL) || dw_written != len)
	{
		MessageBox(h_main, "Can't write to engine process", "Error", MB_OK | MB_ICONERROR);
		return RET_E;
	}
	return RET_O;
}

short int send_uci(struct engine* engine)
{
	char* info = NULL;
	char tmp[128];

	if (engine->is_running == 0)
		return RET_E;

	// Check Command Result
	if (strstr(engine->send_buf, "uciok") == NULL)
		return RET_O;

	info = strstr(engine->send_buf, "id name ");

	if (info)
	{
		for (int i = 0; i < 8; i++)
			info++;

		char* pos = strstr(info, "id author ");
		char engine_name[64];

		memset(engine_name, 0, 64);
		memcpy(engine_name, info, pos - info);

		strcpy(engine->name, engine_name);
		SetDlgItemText(h_main, ID_UCI_NAME, engine->name);
	}

	// Re-init Buffer
	free(engine->send_buf);
	engine->send_buf = (char*)calloc(1, sizeof(char));
	engine->send_buf_len = 0;

	// Send All Options

	// Log
	memset(tmp, 0, 128);
	if (engine_config.log == 0)
		sprintf(tmp, "setoption name Debug Log File value ""\r\n");
	else
		sprintf(tmp, "setoption name Debug Log File value nnue-gui.log\r\n");
	if (send_uci_engine(tmp, -1) == RET_E)
		return RET_E;

	// Threads
	memset(tmp, 0, 128);
	sprintf(tmp, "setoption name Threads value %d\r\n", engine_config.threads);
	if (send_uci_engine(tmp, -1) == RET_E)
		return RET_E;

	// Hash Size
	memset(tmp, 0, 128);
	sprintf(tmp, "setoption name Hash value %d\r\n", engine_config.hash);
	if (send_uci_engine(tmp, -1) == RET_E)
		return RET_E;

	// Load Eval
	memset(tmp, 0, 128);
	if (engine_config.skip_loading_eval == 1)
		sprintf(tmp, "setoption name SkipLoadingEval value true\r\n");
	else
		sprintf(tmp, "setoption name SkipLoadingEval value false\r\n");
	if (send_uci_engine(tmp, -1) == RET_E)
		return RET_E;

	// Syzygy Paths
	memset(tmp, 0, 128);
	sprintf(tmp, "setoption name SyzygyPath value %s\r\n", engine->tb_path);
	if (send_uci_engine(tmp, -1) == RET_E)
		return RET_E;

	// Send isready
	if (send_uci_engine("isready\r\n", 1) == RET_E)
		return RET_E;

	return
		RET_O;
}

short int send_is_ready(struct engine* engine)
{
	if (engine->is_running == 0)
		return RET_E;

	// Check Command Result
	if (strstr(engine->send_buf, "readyok") == NULL)
		return RET_O;

	// Set UI
	engine->is_ready = 1;

	return RET_O;
}
