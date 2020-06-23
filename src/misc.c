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

short int file_exists(char* path)
{
	FILE* fp;

	if ((fp = fopen(path, "r")) == NULL)
		return RET_E;

	fclose(fp);
	return RET_O;
}

int read_file_line(FILE* fp, char* buf, const int buf_len)
{
	int len = 0;

	if (fp == NULL || buf_len <= 0)
		return 0;

	while (!feof(fp))
	{
		int c = fgetc(fp);

		if (c == EOF || c == '\n' || c == '\r')
		{
			if (c == '\r') // Windows Case
				c = fgetc(fp);

			break;
		}
		buf[len] = c;
		len++;

		if (len == buf_len)
			break;
	}
	return len;
}

int str_is_int(char* str)
{
	const int len = strlen(str);

	if (len < 1 || len > 5)
		return RET_E;

	for (int i = 0; i < len; i++)
	{
		if (str[i] < '0' || str[i] > '9')
			return RET_E;
	}
	return atoi(str);
}
