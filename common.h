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
#define ID_OUTPUT 501
#define ID_CONFIG 502
#define ID_START 503
#define ID_STOP 504
#define ID_EXIT 505

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

#define ID_ENGINE1 1000
#define ID_ENGINE1_PATH 1002
#define ID_ENGINE1_SELECT 1003
#define ID_ENGINE1_TB_PATH 1004
#define ID_ENGINE1_TB_SELECT 1005

#define ID_ENGINE2 2000
#define ID_ENGINE2_PATH 2002
#define ID_ENGINE2_SELECT 2003
#define ID_ENGINE2_TB_PATH 2004
#define ID_ENGINE2_TB_SELECT 2005

#define ID_ENGINE3 3000
#define ID_ENGINE3_PATH 3002
#define ID_ENGINE3_SELECT 3003
#define ID_ENGINE3_TB_PATH 3004
#define ID_ENGINE3_TB_SELECT 3005

// Defines
#define RET_O 0
#define RET_E (-1) // Must Be < 0 Or > 64
#define MAX_FILE_PATH 1024
#define MAX_COMMAND 1024
#define MAX_BUFFER 16384
#define MAX_ENGINES 3
#define MIN_HASH 16
#define MAX_HASH 8192
#define MIN_THREADS 1
#define MAX_THREADS 64

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" // Start FEN Notation

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
	short int auto_play; // 1=ENGINE1, 2=ENGINE2, 3=ENGINE3
};

struct engine
{
	short int is_running;
	short int is_ready;
	short int send; // -1=Nothing, 0=uci, 1=id name, 2=bestmove, 3=isready ...
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
struct engine engine[MAX_ENGINES];
char app_path[MAX_FILE_PATH];

/*** config.c ***/
BOOL APIENTRY dlg_proc_engines(HWND h_dlg, UINT u_msg, WPARAM w_param, LPARAM l_param);
void select_engine(int dialog_id, int id);
void select_engine_tb(int dialog_id, int id);
void get_engine_config();
short int load_engine(int id);
void get_engine_auto_play();
void set_engine_auto_play(short int id);
void save_config();
void load_config();
void report_engine_config();
void set_config_values();

/*** engine.c ***/
DWORD WINAPI start_engine(LPVOID arg);
void stop_engine_running(int id);
void stop_all_engines_running();
void set_ui_engine_info(int id, char* msg);
DWORD WINAPI start_engine_thinking(LPVOID arg);
void start_thinking();
void stop_thinking();
void stop_engine_thinking(int id);

/*** main.c ***/
BOOL APIENTRY dlg_proc(HWND h_dlg, UINT u_msg, WPARAM w_param, LPARAM l_param);
short int init_app();

/*** misc.c ***/
short int file_exists(char* path);
int read_file_line(FILE* fp, char* buf, int buf_len);
int str_is_int(char* str);

/*** uci.c ***/
short int send_uci_engine(int id, char* cmd, short int send);
short int send_uci(struct engine* engine, int id);
short int send_is_ready(struct engine* engine);

