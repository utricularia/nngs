/* conffile.h
 * Copyright (C) 2005 Adriaan W.D. van Kessel
 */

/*
    NNGS - The No Name Go Server
    Copyright (C) 1995-1997  Erik Van Riper (geek@midway.com)
    and John Tromp (tromp@daisy.uwaterloo.ca/tromp@cwi.nl)

    Adapted from:
    fics - An internet chess server.
    Copyright (C) 1993  Richard V. Nash

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef CONFFILE_H
#define CONFFILE_H

struct conffile {
  char *mess_dir;
  char *help_dir;
  char *comhelp_dir;
  char *info_dir;
  char *ahelp_dir;
  char *stats_dir;
  char *config_dir;
  char *player_dir;
  char *game_dir;
  char *cgame_dir;
  char *problem_dir;
  char *board_dir;
  char *def_prompt;
  char *source_dir;
  char *lists_dir;
  char *news_dir;

  char *server_name;
  char *server_address;
  char *server_email;
  char *server_http;
  char *geek_email;

  char *intergo_file;
  char *ratings_file;
  char *results_file;
  char *nresults_file;
  char *emotes_file;
  char *note_file;
  char *log_file;
  char *ladder9_file;
  char *ladder19_file;

  char *stats_logons;
  char *stats_messages;
  char *stats_games;
  char *stats_rgames;
  char *stats_cgames;

  char *version_string;
  };

#if CONFFILE_C
struct conffile conffile;
#else
extern struct conffile conffile;
#endif

int config_file_read(char * fname);
int config_file_write(char * fname);

#endif /* CONFFILE_H */
