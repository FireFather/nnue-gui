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

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlobj.h>

#pragma warning(disable : 4311)
// 'type cast': pointer truncation from 'char [3]' to 'LONG'
#pragma warning(disable : 4312)
// 'type cast': conversion from 'int' to 'HMENU' of greater size

// main window
#define ID_UCI_NAME 500
#define ID_STATUS 501
#define ID_OUTPUT 502
#define ID_CONFIG 503
#define ID_START 504
#define ID_STOP 505
#define ID_EXIT 506
#define ID_BMP 507
#define ID_LOGO 508

// report config
#define ID_HASH 600
#define ID_THREADS 601
#define ID_LOAD_NN 602
#define ID_LOG 603

// config
#define ID_HASH_CFG 700
#define ID_THREADS_CFG 701
#define ID_LOAD_NN_CFG 702
#define ID_COMMAND_CFG 703
#define ID_LOG_CFG 704

#define ID_ENGINE 1000
#define ID_ENGINE_PATH 1002
#define ID_ENGINE_SELECT 1003
#define ID_ENGINE_TB_PATH 1004
#define ID_ENGINE_TB_SELECT 1005

// Defines
#define RET_O 0
#define RET_E (-1) // Must Be < 0 Or > 64
#define MAX_FILE_PATH 1024
#define MAX_COMMAND 1024
#define MAX_BUFFER 16384
#define MIN_HASH 16
#define MAX_HASH 8192
#define MIN_THREADS 1
#define MAX_THREADS 64

// Enums
enum status
{
	status_none = 0,
	status_check = 1,
	status_won = 2,
	status_checkmate = 3,
	status_draw = 4,
	status_draw_material = 5,
	status_draw_50_moves = 6,
	status_draw_3_fold_rep = 7,
	status_resign = 8
};

// Structures

struct engine_config
{
	int hash;
	int threads;
	int load_nn;
	int log;
	char command[MAX_COMMAND];
	short int infinite;
};

struct engine
{
	short int is_running;
	short int is_ready;
	short int is_thinking;
	short int send; // -1=Nothing, 0=uci, 2=isready ...
	int id;
	char name[2048];
	char path[MAX_FILE_PATH];
	char tb_path[MAX_FILE_PATH];
	char buffer[16384];
	char* send_buf;
	int send_buf_len;

	PROCESS_INFORMATION proc_info;
	HANDLE thread;
	HANDLE thread_think;
	HANDLE write_out;
	HANDLE read_out;
	HANDLE write_in;
	HANDLE read_in;
};

// GLOBALS
HINSTANCE inst;
HWND h_main;
HWND config;
struct engine_config engine_config;
struct engine engine;
char app_path[MAX_FILE_PATH];

/*** config.c ***/
BOOL APIENTRY dlg_proc_engines(HWND h_dlg, UINT u_msg, WPARAM w_param, LPARAM l_param);
void select_engine(int dialog_id);
void select_engine_tb(int dialog_id);
void get_engine_config();
short int load_engine();
void save_config();
void load_config();
void report_engine_config();
void set_config_values();

/*** engine.c ***/
DWORD WINAPI start_engine(LPVOID arg);
void stop_engine_running();
void set_ui_engine_info(char* msg);
DWORD WINAPI start_engine_thinking(LPVOID arg);
void start_thinking();
void stop_engine();

/*** main.c ***/
BOOL APIENTRY dlg_proc(HWND h_dlg, UINT u_msg, WPARAM w_param, LPARAM l_param);
short int init_app();

/*** misc.c ***/
short int file_exists(char* path);
int read_file_line(FILE* fp, char* buf, int buf_len);
int str_is_int(char* str);

/*** uci.c ***/
short int send_uci_engine(char* cmd, short int send);
short int send_uci(struct engine* engine);
short int send_is_ready(struct engine* engine);

