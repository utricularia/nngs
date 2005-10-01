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

#ifndef IFNULL
#define IFNULL(_p,_d) (_p)?(_p):(_d)
#endif

	/* This is the mapping between names and variables.
	** The ->ptr points to the actual storage,
	** for ints, this is an (int*), for strings it is a (char **)
	** dflt is a STRING, containing the default value.
	*/
struct confmatch {
  int type;
  const char *name;
  void *ptr;
  const char *dflt;
  };

	/* Behaviour of configuration items differs:
	** MESSAGE is ignored, but written (as a comment) into configfile.
	** ZOMBIE is not read from file, even if present, so is
	** always filled with the initial value.
	** CHPATH is read from file, default wil be applied,
	** and chroot will trim it's prefix off.
	** NAME is read from file, default wil be applied,
	** but chroot will *not* alter it.
	** MESSAGE just puts a comment in the output-config file
	** NUMBER is a decimal integer. Not altered by program.
	** OCTAL is like number, but represented in octal.
	** BOOL is a character used as a boolean value.
	** -1 := False, > 0 := True; 0 := missing/NULL
	*/
#define ZOMBIE(_n,_p,_d) {'Z',(_n),(void*)((char**)(_p)),(_d)},
#define CHPATH(_n,_p,_d) {'p',(_n),(void*)((char**)(_p)),(_d)},
#define NAME(_n,_p,_d) {'P',(_n),(void*)((char**)(_p)),(_d)},
#define BOOL(_n,_b,_d) {'b',(_n),(void*)(_b),(_d)},
#define NUMBER(_n,_i,_d) {'i',(_n),(void*)(_i),(_d)},
#define OCTAL(_n,_o,_d) {'o',(_n),(void*)(_o),(_d)},
#define MESSAGE(_m) {'m', (_m),NULL, NULL},

static struct confmatch confmatchs[] = {
ZOMBIE("version_string", &conffile.version_string, VERSION)
ZOMBIE("compile_date", &conffile.compile_date, __DATE__ )
ZOMBIE("compile_time", &conffile.compile_time, __TIME__)
MESSAGE("")
MESSAGE("Note: Don't put any spaces after the '='. Everything between the '='")
MESSAGE("and the end-of-line is read into the variable.")
MESSAGE("")
MESSAGE("Leave chroot_dir empty if chroot() is not wanted.")
MESSAGE("Note: chroot() itself needs root permissions.")
MESSAGE("Note: chroot will cause the pathnames below to be")
MESSAGE("changed automatically (prefix is stripped)")
MESSAGE("Stripped filenames will appear in written.cnf for verification." )
MESSAGE("Note: chroot also :")
MESSAGE(" * needs chroot_user && chroot_group to be set.")
MESSAGE(" * needs mail to be sent by SMTP.")
MESSAGE(" * causes tempnam() to fail, even if /tmp/ directory is present in jail.")
MESSAGE("")
NAME("chroot_dir", &conffile.chroot_dir, NULL)
NAME("chroot_user", &conffile.chroot_user, NULL)
NAME("chroot_group", &conffile.chroot_group, NULL)
MESSAGE("")
MESSAGE("Directory- and file-names are all absolute in the config file.")
MESSAGE("They can be configured independently, but some combinations")
MESSAGE("make no sense. (and will probably not work)")
MESSAGE("")
CHPATH("ahelp_dir", &conffile.ahelp_dir, AHELP_DIR)
CHPATH("help_dir", &conffile.help_dir, HELP_DIR)
CHPATH("mess_dir", &conffile.mess_dir, MESSAGE_DIR)
CHPATH("info_dir", &conffile.info_dir, INFO_DIR)
CHPATH("stats_dir", &conffile.stats_dir, STATS_DIR)
CHPATH("player_dir", &conffile.player_dir, PLAYER_DIR)
CHPATH("game_dir", &conffile.game_dir, GAME_DIR)
CHPATH("cgame_dir", &conffile.cgame_dir, CGAME_DIR)
CHPATH("problem_dir", &conffile.problem_dir, PROBLEM_DIR)
CHPATH("lists_dir", &conffile.lists_dir, LIST_DIR)
CHPATH("news_dir", &conffile.news_dir, NEWS_DIR)
CHPATH("spool_dir", &conffile.spool_dir, SPOOL_DIR)
MESSAGE("")
CHPATH("ratings_file", &conffile.ratings_file, RATINGS_FILE)
CHPATH("nratings_file", &conffile.nratings_file, NRATINGS_FILE)
CHPATH("intergo_file", &conffile.intergo_file, INTERGO_FILE)
CHPATH("results_file", &conffile.results_file, RESULTS_FILE)
CHPATH("nresults_file", &conffile.nresults_file, NRESULTS_FILE)
CHPATH("emotes_file", &conffile.emotes_file, EMOTES_FILE)
CHPATH("note_file", &conffile.note_file, NOTE_FILE)
CHPATH("ladder9_file", &conffile.ladder9_file, LADDER9_FILE)
CHPATH("ladder19_file", &conffile.ladder19_file, LADDER19_FILE)
CHPATH("log_file", &conffile.log_file, LOG_FILE)
CHPATH("logons_file", &conffile.logons_file, LOGONS_FILE)

MESSAGE("")
MESSAGE("These entries define how the server will present itself.")
MESSAGE("")

NAME("server_name", &conffile.server_name, SERVER_NAME)
NAME("server_http", &conffile.server_http, SERVER_HTTP)
NAME("server_address", &conffile.server_address, SERVER_ADDRESS)
NAME("server_ports", &conffile.server_ports, SERVER_PORTS)
NAME("server_email", &conffile.server_email, SERVER_EMAIL)
NAME("geek_email", &conffile.geek_email, GEEK_EMAIL)
MESSAGE("")
MESSAGE("To send mail by SMTP: set mail_program to \"SMTP\", and fill in the smtp_* fields.")
MESSAGE("This is needed in chroot() installations, because /usr/bin/mail et.al.")
MESSAGE("are unavailabls in a chroot() jail.")
MESSAGE("The smtp_xxx - fields are used to fill the SMTP-request:")
MESSAGE("")
MESSAGE(" smtp_mta := mailer we want to use.")
MESSAGE(" smtp_helo := what we put in the HELO line (this host).")
MESSAGE(" smtp_from := what we put in the MAIL FROM: line (our address).")
MESSAGE(" smtp_reply_to := what we put in the Reply-to: header (reply address).")
MESSAGE("")
NAME("mail_program", &conffile.mail_program, MAILPROGRAM)
NAME("smtp_mta", &conffile.smtp_mta, "localhost")
NUMBER("smtp_portnum", &conffile.smtp_portnum, "25")
NAME("smtp_helo", &conffile.smtp_helo, "localhost")
NAME("smtp_from", &conffile.smtp_from, SERVER_EMAIL)
NAME("smtp_reply_to", &conffile.smtp_reply_to, SERVER_EMAIL)

MESSAGE("")
MESSAGE("Set mode_for_dir to nonzero to create non-existant directories silently.")
MESSAGE("Use with care...")
MESSAGE("")
OCTAL("mode_for_dir", &conffile.mode_for_dir, 0)
MESSAGE("")
NAME("def_prompt", &conffile.def_prompt, DEFAULT_PROMPT)
MESSAGE("")
MESSAGE("Boolean flags. Set to {1,Yes,True} or {-1,0,No,False}.")
MESSAGE("")
BOOL("allow_unregistered", &conffile.allow_unregistered, "Yes" )
BOOL("unregs_can_shout", &conffile.unregs_can_shout, "Yes" )
BOOL("want_udp_port", &conffile.want_udp_port, "No" )

MESSAGE("")
{ 0,  NULL,NULL,NULL } }; /* sentinel */
#undef NUL2
#undef ZOMBIE
#undef NAME
#undef CHPATH
#undef MESSAGE
#undef NUMBER
#undef OCTAL
#undef BOOL

static struct confmatch *conf_find(const char *name);
static int conf_set_pair(const char *name, const char *value);
static void config_fill_defaults(void);
static void trimtrail(char *str);
static int conf_file_fixup1(char * target, char *part, int len);

int conf_file_read(const char * fname)
{
  FILE *fp;
  char buff[1024];
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

  config_fill_defaults();
  return 0;
}

static void trimtrail(char *str)
{
  size_t len;
  len = strlen(str);
  while(len > 0) {
    switch (str[len-1]) {
    case ' ':
    case '\t':
    case '\n': str[--len] = 0 ; continue;
    default: return;
    }
  }
}

int conf_file_write(const char *fname)
{
  FILE *fp;
  struct confmatch *mp;

  fp = fopen( fname, "w" );
  if (!fp) return -1;

  fprintf(fp, "## %s, Generated %s UTC\n", fname, time2str_utc(&globclock.time));
  fprintf(fp, "# compile_date=%s %s (local)\n" , __DATE__ , __TIME__ );
  fprintf(fp, "#\n" );

  for (mp = confmatchs; mp->type; mp++) {
    switch (mp->type) {
    case 'm': fprintf(fp, "# %s\n", mp->name);
      break;
    case 'Z' :
    case 'P':
    case 'p': fprintf(fp, "%s=%s\n"
      , mp->name, IFNULL(*(char**)mp->ptr,"") );
      break;
    case 'i': fprintf(fp, "%s=%d\n", mp->name, (int) *((int*)mp->ptr) );
      break;
    case 'o': fprintf(fp, "%s=%o\n", mp->name, (int) *((int*)mp->ptr) );
      break;
    case 'b': fprintf(fp, "%s=%s\n"
      , mp->name, (mp->ptr && *((char*)mp->ptr)>0)?"Yes":"No" );
      break;
    default: break;
    }
  }
  fprintf(fp, "#\n" );
  fprintf(fp, "## Eof\n" );

  fclose (fp);
  return 0;
}


static void config_fill_defaults(void)
{
  struct confmatch *mp;

  for (mp = confmatchs; mp->type; mp++) {
    switch (mp->type) {
    case 'o':
    case 'i': if (!*(int*)(mp->ptr)) conf_set_pair(mp->name, mp->dflt);
      break;
    case 'b': if (!*(char*)(mp->ptr)) conf_set_pair(mp->name, mp->dflt);
      break;
    case 'Z':
    case 'P':
    case 'p': if (!*(char**)(mp->ptr) && mp->dflt) {
       *((char**)mp->ptr) = mystrdup (mp->dflt);
      break;
      }
    break;
    case 'm': default: continue;
    }
  }
}


int conf_file_fixup(void)
{
  int cnt=0, len;
  struct confmatch *mp;

  if (!conffile.chroot_dir) return 0;
  len = strlen(conffile.chroot_dir) ;
  if (len && conffile.chroot_dir[len-1] == '/') len--;

#define DO_ONE(_s) cnt += conf_file_fixup1((_s), conffile.chroot_dir, len)

  for (mp = confmatchs; mp->type; mp++) {
    switch (mp->type) {
    case 'p': if (*(char**)(mp->ptr)) DO_ONE(*(char**)(mp->ptr) );
      break;
    case 'Z':
    case 'P':
    case 'm':
    case 'b':
    case 'i':
    case 'o':
    default: continue;
    }
  }
#undef DO_ONE

  return cnt;
}

static int conf_file_fixup1(char * target, char *part, int len)
{
  if (strncmp(target, part, len)) return 1; /* fail */

  while (target[0] = target[len]) target++;
  return 0;

}


static int conf_set_pair(const char *name, const char *value)
{
  struct confmatch *mp;

  mp = conf_find(name);
  if (!mp) return -1; /* nametag Not found */

  switch (mp->type) {
  case 'p':
  case 'P':
    if (*(char**)(mp->ptr)) free (*(char**)(mp->ptr));
    *(char**)(mp->ptr) = (value) ? mystrdup(value): NULL;
    break;
  case 'o':
    if (!value || !*value) value = "0";
    sscanf(value, "%o", (int*)(mp->ptr) );
    break;
  case 'i':
    if (!value || !*value) value = "0";
    sscanf(value, "%d", (int*)(mp->ptr) );
    break;
  case 'b':
    if (!value || !*value) value = "F";
    switch (*value) {
      case 't': case 'T':
      case 'y': case 'Y':
      case '+': case '1':
       *((char*)(mp->ptr)) = 1; break;
      case 'f': case 'F':
      case 'n': case 'N':
      case '-':
      case '0':
       *((char*)(mp->ptr)) = -1; break;
    }
    break;
  case 'Z':
  case 'm':
  default: return -1;
  }

  return 0;
}


static struct confmatch *conf_find(const char *name)
{
  struct confmatch *mp;

  for (mp = confmatchs; mp->type; mp++) {
    if (mp->type== 'm') continue;
    if (!strcmp(mp->name, name) ) return mp;
  }
  return NULL;
}

