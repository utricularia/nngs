/* conffile.c
 * Copyright (C) 2005 Adriaan W.D. van Kessel
 */

/*
    NNGS - The No Name Go Server
    Copyright (C) 1995-1996 Erik Van Riper (geek@nngs.cosmic.org)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef USING_DMALLOC
#include <dmalloc.h>
#define DMALLOC_FUNC_CHECK 1
#endif

#include "nngsconfig.h"
#include "common.h"

#define CONFFILE_C 1
#include "conffile.h"

#include "nngsmain.h"
#include "utils.h"

static int conf_set_pair(char *name, char *value);
static void config_fill_defaults(void);
static void trimtrail(char *str);

int conf_file_read(char * fname)
{
FILE *fp;
char buff[100];
char *name, *value;
size_t len;

fp = fopen( fname, "r" );
if (!fp) {
	config_fill_defaults();
	return -1;
	}

while (fgets(buff, sizeof buff, fp) ) {
	len = strspn(buff, " \t\n" );
	name = buff + len;
	if (!*name) continue;
	if (*name == '#' ) continue;
	len = strcspn(name, "=");
	if (name[len] != '=') continue;
	name[len] = 0; value = name + len + 1;
	if (!*value) continue;
	trimtrail(name);
	trimtrail(value);
	conf_set_pair(name, value);
	}
fclose (fp);
return 0;
}

static void trimtrail(char *str)
{
size_t len;
len = strlen(str);
while(len > 0) {
	if (str[len-1] == ' '
	|| str[len-1] == '\t'
	|| str[len-1] == '\n') str[--len] = 0 ;
	else break;
	}
}

static int conf_set_pair(char *name, char *value)
{
char **target = NULL;

if (!strcmp(name, "ahelp_dir") ) target = &conffile.ahelp_dir;
else if (!strcmp(name, "help_dir") ) target = &conffile.help_dir;
else if (!strcmp(name, "mess_dir") ) target = &conffile.mess_dir;
else if (!strcmp(name, "info_dir") ) target = &conffile.info_dir;
else if (!strcmp(name, "stats_dir") ) target = &conffile.stats_dir;
else if (!strcmp(name, "player_dir") ) target = &conffile.player_dir;
else if (!strcmp(name, "game_dir") ) target = &conffile.game_dir;
else if (!strcmp(name, "cgame_dir") ) target = &conffile.cgame_dir;
else if (!strcmp(name, "problem_dir") ) target = &conffile.problem_dir;
else if (!strcmp(name, "lists_dir") ) target = &conffile.lists_dir;
else if (!strcmp(name, "news_dir") ) target = &conffile.news_dir;

else if (!strcmp(name, "ratings_file") ) target = &conffile.ratings_file;
else if (!strcmp(name, "nratings_file") ) target = &conffile.nratings_file;
else if (!strcmp(name, "intergo_file") ) target = &conffile.intergo_file;
else if (!strcmp(name, "results_file") ) target = &conffile.results_file;
else if (!strcmp(name, "nresults_file") ) target = &conffile.nresults_file;
else if (!strcmp(name, "emotes_file") ) target = &conffile.emotes_file;
else if (!strcmp(name, "note_file") ) target = &conffile.note_file;
else if (!strcmp(name, "log_file") ) target = &conffile.log_file;
else if (!strcmp(name, "logons_file") ) target = &conffile.logons_file;
else if (!strcmp(name, "ladder9_file") ) target = &conffile.ladder9_file;
else if (!strcmp(name, "ladder19_file") ) target = &conffile.ladder19_file;

else if (!strcmp(name, "def_prompt") ) target = &conffile.def_prompt;

else if (!strcmp(name, "server_name") ) target = &conffile.server_name;
else if (!strcmp(name, "server_address") ) target = &conffile.server_address;
else if (!strcmp(name, "server_http") ) target = &conffile.server_http;
else if (!strcmp(name, "server_email") ) target = &conffile.server_email;
else if (!strcmp(name, "geek_email") ) target = &conffile.geek_email;
else if (!strcmp(name, "mail_program") ) target = &conffile.mail_program;

else if (!strcmp(name, "version_string") ) target = &conffile.version_string;

if (!target) { /* nametag Not found */
	return -1;
	}
if (*target) free (*target);
*target = mystrdup(value);

return 0;
}

int conf_file_write(char *fname)
{
FILE *fp;

fp = fopen( fname, "w" );
if (!fp) return -1;

fprintf(fp, "## Config generated Date %s UTC\n", strgtime(&globclock.time));
fprintf(fp, "ahelp_dir=%s\n" , conffile.ahelp_dir);
fprintf(fp, "help_dir=%s\n" , conffile.help_dir);
fprintf(fp, "mess_dir=%s\n" , conffile.mess_dir);
fprintf(fp, "info_dir=%s\n" , conffile.info_dir);
fprintf(fp, "stats_dir=%s\n" , conffile.stats_dir);
fprintf(fp, "player_dir=%s\n" , conffile.player_dir);
fprintf(fp, "game_dir=%s\n" , conffile.game_dir);
fprintf(fp, "cgame_dir=%s\n" , conffile.cgame_dir);
fprintf(fp, "problem_dir=%s\n" , conffile.problem_dir);
fprintf(fp, "lists_dir=%s\n" , conffile.lists_dir);
fprintf(fp, "news_dir=%s\n" , conffile.news_dir);

fprintf(fp, "ratings_file=%s\n" , conffile.ratings_file);
fprintf(fp, "nratings_file=%s\n" , conffile.nratings_file);
fprintf(fp, "intergo_file=%s\n" , conffile.intergo_file);
fprintf(fp, "results_file=%s\n" , conffile.results_file);
fprintf(fp, "nresults_file=%s\n" , conffile.nresults_file);
fprintf(fp, "emotes_file=%s\n" , conffile.emotes_file);
fprintf(fp, "note_file=%s\n" , conffile.note_file);
fprintf(fp, "log_file=%s\n" , conffile.log_file);
fprintf(fp, "logons_file=%s\n" , conffile.logons_file);
fprintf(fp, "ladder9_file=%s\n" , conffile.ladder9_file);
fprintf(fp, "ladder19_file=%s\n" , conffile.ladder19_file);

fprintf(fp, "def_prompt=%s\n" , conffile.def_prompt);

fprintf(fp, "server_name=%s\n" , conffile.server_name);
fprintf(fp, "server_address=%s\n" , conffile.server_address);
fprintf(fp, "server_email=%s\n" , conffile.server_email);
fprintf(fp, "server_http=%s\n" , conffile.server_http);
fprintf(fp, "geek_email=%s\n" , IFNULL(conffile.geek_email,"") );
fprintf(fp, "mail_program=%s\n" , conffile.mail_program);

fprintf(fp, "#version_string=%s\n" , conffile.version_string);
fprintf(fp, "#compile_date=%s %s\n" , __DATE__ , __TIME__ );
fprintf(fp, "## Eof\n" );

fclose (fp);
return 0;
}


static void config_fill_defaults(void)
{
if (!conffile.ahelp_dir) conffile.ahelp_dir = mystrdup(AHELP_DIR);
if (!conffile.help_dir) conffile.help_dir = mystrdup(HELP_DIR);
if (!conffile.mess_dir) conffile.mess_dir = mystrdup(MESSAGE_DIR);
if (!conffile.info_dir) conffile.info_dir = mystrdup(INFO_DIR);
if (!conffile.stats_dir) conffile.stats_dir = mystrdup(STATS_DIR);
if (!conffile.player_dir) conffile.player_dir = mystrdup(PLAYER_DIR);
if (!conffile.game_dir) conffile.game_dir = mystrdup(GAME_DIR);
if (!conffile.cgame_dir) conffile.cgame_dir = mystrdup(CGAME_DIR);
if (!conffile.problem_dir) conffile.problem_dir = mystrdup(PROBLEM_DIR);
if (!conffile.lists_dir) conffile.lists_dir = mystrdup(LIST_DIR);
if (!conffile.news_dir) conffile.news_dir = mystrdup(NEWS_DIR);

if (!conffile.ratings_file) conffile.ratings_file = mystrdup(RATINGS_FILE);
if (!conffile.nratings_file) conffile.nratings_file = mystrdup(NRATINGS_FILE);
if (!conffile.intergo_file) conffile.intergo_file = mystrdup(INTERGO_FILE);
if (!conffile.results_file) conffile.results_file = mystrdup(RESULTS_FILE);
if (!conffile.nresults_file) conffile.nresults_file = mystrdup(NRESULTS_FILE);
if (!conffile.emotes_file) conffile.emotes_file = mystrdup(EMOTES_FILE);
if (!conffile.note_file) conffile.note_file = mystrdup(NOTE_FILE);

if (!conffile.log_file) conffile.log_file = mystrdup(LOG_FILE);
if (!conffile.logons_file) conffile.logons_file = mystrdup(LOGONS_FILE);
if (!conffile.ladder9_file) conffile.ladder9_file = mystrdup(LADDER9_FILE);
if (!conffile.ladder19_file) conffile.ladder19_file = mystrdup(LADDER19_FILE);

if (!conffile.def_prompt) conffile.def_prompt = mystrdup(DEFAULT_PROMPT);

if (!conffile.server_name) conffile.server_name = mystrdup(SERVER_NAME);
if (!conffile.server_address) conffile.server_address = mystrdup(SERVER_ADDRESS);
if (!conffile.server_email) conffile.server_email = mystrdup(SERVER_EMAIL);
if (!conffile.server_http) conffile.server_http = mystrdup(SERVER_HTTP);
if (!conffile.geek_email) conffile.geek_email = mystrdup(GEEK_EMAIL);
if (!conffile.mail_program) conffile.mail_program = mystrdup(MAILPROGRAM);

if (!conffile.version_string) conffile.version_string = mystrdup(VERSION);
return;
}

