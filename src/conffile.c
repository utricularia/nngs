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
struct confmatch {
	int type;
	const char *name;
	void *ptr;
	char *dflt;
	};
#define NUL2 NULL,NULL
#define NUL3 NULL,NULL,NULL
/* Behaviour of configuration items is differs:
** ZOmbie is not read from file, even if present, so always is filled with the initial value
** CHPATH is read from file, default wil be applied, and chroot will thim it.
** Name is read from file, default wil be applied, but chroot will *not* alter it
** Message just puts a comment in the output-config file
*/
#define ZOMBIE(_n,_p,_d) {'Z',(const char*)(_n),(void*)((char**)(_p)),(const char*)(_d)},
#define CHPATH(_n,_p,_d) {'p',(const char*)(_n),(void*)((char**)(_p)),(const char*)(_d)},
#define NAME(_n,_p,_d) {'P',(const char*)(_n),(void*)((char**)(_p)),(const char*)(_d)},
#define MESSAGE(_m) {'m', (const char*)(_m),NUL2},

static struct confmatch confmatchs[] = {
ZOMBIE("version_string", &conffile.version_string, VERSION)
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

NAME("def_prompt", &conffile.def_prompt, DEFAULT_PROMPT)

NAME("server_name", &conffile.server_name, SERVER_NAME)
NAME("server_address", &conffile.server_address, SERVER_ADDRESS)
NAME("server_ports", &conffile.server_ports, SERVER_PORTS)
NAME("server_http", &conffile.server_http, SERVER_HTTP)
NAME("server_email", &conffile.server_email, SERVER_EMAIL)
NAME("geek_email", &conffile.geek_email, GEEK_EMAIL)
MESSAGE("")
MESSAGE("To send mail by SMTP: set mail_program to \"SMTP\", and fill in the smtp_* fields.")
MESSAGE("This is needed in chroot() installations, because /usr/bin/mail et.al.")
MESSAGE("are unavailabls in a chroot() jail.")
MESSAGE("The smtp_xxx - fields are used to fill the SMTP-request:")
MESSAGE(" smtp_mta := mailer we want to use.")
MESSAGE(" smtp_helo := what we put in the HELO line (this host).")
MESSAGE(" smtp_from := what we put in the MAIL FROM: line (our address).")
MESSAGE(" smtp_reply_to := what we put in the Reply-to: header (reply address).")
MESSAGE("")
NAME("mail_program", &conffile.mail_program, MAILPROGRAM)
NAME("smtp_mta", &conffile.smtp_mta, "localhost")
NAME("smtp_helo", &conffile.smtp_helo, "localhost")
NAME("smtp_from", &conffile.smtp_from, SERVER_EMAIL)
NAME("smtp_reply_to", &conffile.server_email, SERVER_EMAIL)
{ 0,  NUL3 } }; /* sentinel */
#undef NUL2
#undef NUL3
#undef ZOMBIE
#undef NAME
#undef CHPATH
#undef MESSAGE
static struct confmatch *conf_find(char *name);
static int conf_set_pair(char *name, char *value);
static void config_fill_defaults(void);
static void trimtrail(char *str);
static int conf_file_fixup1(char * target, char *part, int len);

int conf_file_read(char * fname)
{
FILE *fp;
char buff[1024];
char *name, *value;
size_t len;
int rc;
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#define __USE_XOPEN 1
#include <unistd.h>
#undef __USE_XOPEN
#include <errno.h>

struct passwd *pp = NULL;
struct group *gp = NULL;
uid_t uid = -1, euid =0;
gid_t gid = -1;

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
if (conffile.chroot_user) pp = getpwnam(conffile.chroot_user);
if (conffile.chroot_group) gp = getgrnam(conffile.chroot_group);

if (conffile.chroot_user && !pp) fprintf(stderr, "Could not find user %s\n", conffile.chroot_user );
else { uid = pp->pw_uid; gid = pp->pw_gid; }
if (conffile.chroot_group && !gp) fprintf(stderr, "Could not find group %s\n", conffile.chroot_group);
else gid = gp->gr_gid;

if (conffile.chroot_dir) {
	rc = chdir(conffile.chroot_dir);
	if (rc==-1) { rc=errno;
		fprintf(stderr, "Failed chdir(%s): %d (%s)\n"
		, conffile.chroot_dir, rc, strerror(rc) );
		}
	rc = chroot(conffile.chroot_dir);
	if (rc==-1) { rc=errno;
		fprintf(stderr, "Failed chroot(%s): %d (%s)\n"
		, conffile.chroot_dir, rc, strerror(rc) );
		}
	if (rc) return rc;
	}

if (gid) rc = setgid(gid);
if (rc==-1) { rc=errno;
	fprintf(stderr, "Failed setgid(%d:%s): %d (%s)\n"
	, gid, conffile.chroot_group, rc, strerror(rc) );
	}
if (uid) rc = setuid(uid);
if (rc==-1) { rc=errno;
	fprintf(stderr, "Failed setuid(%d:%s): %d (%s)\n"
	, uid, conffile.chroot_user, rc, strerror(rc) );
	}
#if 1
if (rc=fork()) { fprintf(stderr, "Fork1 = %d\n", rc); _exit(0); }
if (rc=fork()) { fprintf(stderr, "Fork2 = %d\n", rc);  _exit(0); }
#endif

if (uid && (euid = geteuid()) !=uid) {
	fprintf(stderr, "Failed setuid(%d:%s): euid=%d\n"
	, uid, conffile.chroot_user, euid );
	main_exit(1);
	}

if (conffile.chroot_dir) {
	conf_file_fixup();
	}
conf_file_write("written.cnf");
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

int conf_file_write(char *fname)
{
FILE *fp;
struct confmatch *cp;

fp = fopen( fname, "w" );
if (!fp) return -1;

fprintf(fp, "## nngs.cnf Config generated Date %s UTC\n", strgtime(&globclock.time));
fprintf(fp, "# compile_date=%s %s\n" , __DATE__ , __TIME__ );
fprintf(fp, "#\n" );

for (cp = confmatchs; cp->type; cp++) {
	switch (cp->type) {
	case 'm': fprintf(fp, "# %s\n", cp->name); break;
	case 'Z' :
	case 'P':
	case 'p': fprintf(fp, "%s=%s\n"
		, cp->name, IFNULL(*(char**)cp->ptr,"") ); break;
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
struct confmatch *cp;

for (cp = confmatchs; cp->type; cp++) {
	switch (cp->type) {
	case 'Z':
	case 'P':
	case 'p': if (!*(char**)(cp->ptr) && cp->dflt) {
		 *((char**)cp->ptr) = mystrdup (cp->dflt);
		}
	break;
	case 'm': default: continue;
		}
	}
}

int conf_file_fixup(void)
{
int cnt=0, len;
struct confmatch *cp;

if (!conffile.chroot_dir) return 0;
len = strlen(conffile.chroot_dir) ;
if (len && conffile.chroot_dir[len-1] == '/') len--;

#define DO_ONE(_s) cnt += conf_file_fixup1((_s), conffile.chroot_dir, len)

for (cp = confmatchs; cp->type; cp++) {
	switch (cp->type) {
	case 'p': if (*(char**)(cp->ptr)) DO_ONE(*(char**)(cp->ptr) );
	break;
	case 'Z':
	case 'P':
	case 'm':
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


static int conf_set_pair(char *name, char *value)
{
struct confmatch *cp;

cp = conf_find(name);
if (!cp) { /* nametag Not found */
	return -1;
	}
switch (cp->type) {
case 'p':
case 'P':
	if (*(char**)(cp->ptr)) free (*(char**)(cp->ptr));
	*(char**)(cp->ptr) = (value) ? mystrdup(value): NULL;
	break;
case 'Z':
case 'm':
default: return -1;
	}

return 0;
}

static struct confmatch *conf_find(char *name)
{
struct confmatch *cp;

for (cp = confmatchs; cp->type; cp++) {
	if (cp->type== 'm') continue;
	if (!strcmp(cp->name, name) ) return cp;
	}
return NULL;
}
