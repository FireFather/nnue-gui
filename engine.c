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

short int init_engines()
{
	// Init ENGINE

	engine.is_running = 0;
	engine.is_ready = 0;
	engine.is_thinking = 0;
	engine.send = -1;
	engine.id = 0;
	memset(engine.path, 0, MAX_FILE_PATH);
	memset(engine.tb_path, 0, MAX_FILE_PATH);
	memset(engine.buffer, 0, MAX_BUFFER);
	engine.send_buf = NULL;
	engine.send_buf_len = 0;
	ZeroMemory(&engine.proc_info, sizeof(PROCESS_INFORMATION));
	engine.thread = NULL;
	engine.thread_think = NULL;
	engine.write_out = NULL;
	engine.read_out = NULL;
	engine.write_in = NULL;
	engine.read_in = NULL;
	return RET_O;

}

short int load_engine(const int id)
{
	// Stop ENGINE
	if (engine.is_running == 1)
		stop_engine_running(id);

	// Start ENGINE Thread
	if (strlen(engine.path) == 0 || file_exists(engine.path) == RET_E)
		return RET_E;

	if ((engine.thread = CreateThread(NULL, 0, start_engine, (LPVOID)&engine, 0, NULL)) == NULL)
	{
		MessageBox(h_main, "Can't create engine thread", "Error", MB_OK | MB_ICONERROR);
		return RET_E;
	}
	return RET_O;
}

DWORD WINAPI start_engine(const LPVOID arg)
{
	struct engine* engine = (struct engine*)arg;
	STARTUPINFO start_info;
	SECURITY_ATTRIBUTES attr;
	DWORD read;

	engine->is_running = 1;
	SetDlgItemText(h_main, ID_STATUS, "ready");

	// Check ENGINE Path
	if (strlen(engine->path) == 0 || file_exists(engine->path) == RET_E)
	{
		MessageBox(HWND_DESKTOP, "Invalid engine path", "Error", MB_OK | MB_ICONERROR);
		goto stopEngineThread;
	}
	// Set Security For Pipe
	attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	attr.bInheritHandle = TRUE;
	attr.lpSecurityDescriptor = NULL;

	// Create OUT Pipe
	if (!CreatePipe(&engine->read_out, &engine->write_out, &attr, 0))
	{
		MessageBox(h_main, "Can't create output pipe", "Error", MB_OK | MB_ICONERROR);
		goto stopEngineThread;
	}

	if (!SetHandleInformation(engine->read_out, HANDLE_FLAG_INHERIT, 0))
	{
		MessageBox(h_main, "Can't set output pipe handle information", "Error", MB_OK | MB_ICONERROR);
		goto stopEngineThread;
	}

	// Create IN Pipe
	if (!CreatePipe(&engine->read_in, &engine->write_in, &attr, 0))
	{
		MessageBox(h_main, "Can't create input pipe", "Error", MB_OK | MB_ICONERROR);
		goto stopEngineThread;
	}

	if (!SetHandleInformation(engine->write_in, HANDLE_FLAG_INHERIT, 0))
	{
		MessageBox(h_main, "Can't set input pipe handle information", "Error", MB_OK | MB_ICONERROR);
		goto stopEngineThread;
	}

	// Init Proc
	ZeroMemory(&engine->proc_info, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&start_info, sizeof(STARTUPINFO));

	// Set Proc Infos
	start_info.cb = sizeof(STARTUPINFO);
	start_info.hStdError = engine->write_out;
	start_info.hStdOutput = engine->write_out;
	start_info.hStdInput = engine->read_in;
	start_info.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	start_info.wShowWindow = SW_HIDE;

	// Create Proc
	if (!CreateProcess(NULL, engine->path, NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &engine->proc_info))
	{
		MessageBox(h_main, "Can't create engine process", "Error", MB_OK | MB_ICONERROR);
		goto stopEngineThread;
	}

	// Start UCI
	if (send_uci_engine("uci\r\n", 0) == RET_E)
		goto stopEngineThread;

	// Start Read Process Loop
	engine->send_buf = (char*)calloc(1, sizeof(char));
	engine->send_buf_len = 0;

	while (engine->is_running == 1)
	{
		// Read
		memset(engine->buffer, 0, MAX_BUFFER);

		if (!ReadFile(engine->read_out, engine->buffer, MAX_BUFFER - 1, &read, NULL) || read == 0)
		{
			if (engine->is_running == 1)
				MessageBox(h_main, "Can't read engine process", "Error", MB_OK | MB_ICONERROR);
			goto stopEngineThread;
		}

		// Add To Temp Buffer
		engine->send_buf_len += read + 1;
		engine->send_buf = (char*)realloc((char*)engine->send_buf, sizeof(char) * engine->send_buf_len);
		strcat(engine->send_buf, engine->buffer);

		set_ui_engine_info(engine->buffer);

		// Check From Results
		switch (engine->send)
		{
		case 0:
			// uci (uciok)
			if (send_uci(engine) == RET_E)
			{
				MessageBox(h_main, "Invalid uciok parsing", "Error", MB_OK | MB_ICONERROR);
				goto stopEngineThread;
			}
			break;

		case 1:
			// isready (readyok)
			if (send_is_ready(engine) == RET_E)
			{
				MessageBox(h_main, "Invalid isready parsing", "Error", MB_OK | MB_ICONERROR);
				goto stopEngineThread;
			}
			break;

		case 2:
			// unused
			break;
		default:;
		}
	}
stopEngineThread:
	// End Thread
	if (engine->is_running == 1)
		stop_engine_running();
	return 0;
}

void stop_engine_running()
{
	engine.is_running = 0;
	engine.is_ready = 0;
	engine.is_thinking = 0;
	SetDlgItemText(h_main, ID_STATUS, "stopped");

	// Stop ENGINE Thread Think
	if (engine.thread_think != NULL)
	{
		TerminateThread(engine.thread_think, 0);
		CloseHandle(engine.thread_think);
		engine.thread_think = NULL;
	}

	// Stop ENGINE Thread
	if (engine.thread != NULL)
	{
		TerminateThread(engine.thread, 0);
		CloseHandle(engine.thread);
		engine.thread = NULL;
	}

	// Stop Process
	if (engine.proc_info.hProcess)
	{
		TerminateProcess(engine.proc_info.hProcess, 0);
		// Close ENGINE Proc Handles
		CloseHandle(engine.proc_info.hProcess);
		CloseHandle(engine.proc_info.hThread);
	}
	ZeroMemory(&engine.proc_info, sizeof(PROCESS_INFORMATION));

	// Close Handles
	if (engine.write_out != NULL)
	{
		CloseHandle(engine.write_out);
		engine.write_out = NULL;
	}

	if (engine.read_out != NULL)
	{
		CloseHandle(engine.read_out);
		engine.read_out = NULL;
	}

	if (engine.write_in != NULL)
	{
		CloseHandle(engine.write_out);
		engine.write_out = NULL;
	}

	if (engine.read_in != NULL)
	{
		CloseHandle(engine.read_in);
		engine.read_in = NULL;
	}
	// Clean
	memset(engine.path, 0, MAX_FILE_PATH);
	memset(engine.tb_path, 0, MAX_FILE_PATH);
	memset(engine.buffer, 0, MAX_BUFFER);

	if (engine.send_buf != NULL)
	{
		free(engine.send_buf);
		engine.send_buf = NULL;
	}
	engine.send_buf_len = 0;
	engine.send = -1;
}

void set_ui_engine_info(char* msg)
{
	SetDlgItemText(h_main, ID_OUTPUT, msg);
}

DWORD WINAPI start_engine_thinking(const LPVOID arg)
{
	struct engine* engine = (struct engine*)arg;
	char cmd[1024];

	if (engine->is_running == 0 || engine->is_ready == 0 || engine->is_thinking == 1)
		goto stopThreadThink;

	// Prepare Commands
	memset(cmd, 0, 1024);
	sprintf(cmd, engine_config.command);

	// Thinking
	if (send_uci_engine(cmd, -1) == RET_E)
		stop_engine_running(engine->id);
	else
	{
		engine->is_thinking = 1;
		SetDlgItemText(h_main, ID_STATUS, "running");
		if (send_uci_engine(cmd, 2) == RET_E)
			stop_engine_running(engine->id);
	}
stopThreadThink:
	if (engine->thread_think != NULL)
	{
		CloseHandle(engine->thread_think);
		engine->thread_think = NULL;
	}
	return 0;
}

void start_thinking()
{
	if (engine.is_running == 1 && engine.is_ready == 1 && engine.is_thinking == 0)
	{
		if ((engine.thread_think = CreateThread(NULL, 0, start_engine_thinking, (LPVOID)&engine, 0, NULL)) == NULL)
			MessageBox(h_main, "Can't create engine thread", "Error", MB_OK | MB_ICONERROR);
	}
}

void stop_engine()
{
	// Stop Engine Thinking
	if (engine.is_running == 1)
		stop_engine_running();
}
