/*
 * udp_commands.c
*/

/*
    NNGS - The No Name Go Server
    Copyright (C) 2005 Adriaan W.D. van Kessel

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
#include <ctype.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "nngsmain.h"
#include "conffile.h"
#include "mink.h"
#include "utils.h"
#include "playerdb.h"
#include "gamedb.h"
#include "command.h"
#include "network.h"
#include "common.h"

#include "udp_commands.h"

static int udp_uptime(char *dst, size_t dstlen);
static int udp_shutdown(char *dst, size_t dstlen, char *opt);
static int udp_slots(char *dst, size_t dstlen);
static int udp_net(char *dst, size_t dstlen);
static int udp_players(char *dst, size_t dstlen);
static int udp_games(char *dst, size_t dstlen);
static int udp_board(char *dst, size_t dstlen, int num);

extern int snprintf(char *dst, size_t dstlen, const char *fmt, ...);

#define snprintf my_snprintf

/***********************************************************************/
int udp_command(char *dst, size_t dstlen, char * src )
{
int len=0, opt=0;

if (!strcmp(src,"uptime")) len = udp_uptime(dst, dstlen);
else if (!strcmp(src,"players")) len = udp_players(dst, dstlen);
else if (!strcmp(src,"slots")) len = udp_slots(dst, dstlen);
else if (!strcmp(src,"net")) len = udp_net(dst, dstlen);
else if (!strcmp(src,"games")) len = udp_games(dst, dstlen);
else if (!strncmp(src,"shutdown ", 9)) len = udp_shutdown(dst, dstlen, src+9);
else if (1 == sscanf(src,"board %d", &opt)) len = udp_board(dst, dstlen, opt);
else len = snprintf(dst ,dstlen, "Kuttje!:%s\n", src);

return len;
}

static int udp_net(char *dst, size_t dstlen)
{
int ii, len;
size_t pos=0;
char *cp;

  len = snprintf(dst, dstlen, "Net\n");
  if (len < 0) return pos; else pos = len;
  for (ii = 0; ii < 200; ii++) {
    if (pos >= dstlen-80) break; /* assume well-behaved */
    cp = net_dumpslot(ii);
    if (!cp) break;
/* fprintf(stderr, "[%u]%s\n", pos, cp ); */
    len = snprintf(dst+pos, (size_t)(dstlen-pos), "%s\n", cp); /* assume well-behaved */
    if (len < 0) break; else pos += len;
  }

return pos;
}

static int udp_games(char *dst, size_t dstlen)
{
int g0, len;
int pb, pw, mnum;
size_t pos=0;

  len = snprintf(dst, dstlen, "Games (%u)\n", (unsigned) garray_top);
  if (len < 0) return len; else pos = len;
  for (g0 = 0; g0 < garray_top; g0++) {
    if (pos+80 >= dstlen) break;
    /* if (!garray[g0].gstatus != GSTATUS_ACTIVE) continue; */
    /* if (!garray[g0].slotstat.in_use) continue; */
    pb = garray[g0].black.pnum;
    pw = garray[g0].white.pnum;
    len = snprintf(dst+pos, dstlen-pos, "%d:%d:%p:%d:%d\n"
      , g0+1, (int) garray[g0].gstatus, (void*) garray[g0].minkg, pb+1, pw+1);
    if (len < 0) break; else pos += len;
    if (pb < 0 || pw < 0) continue;
    if (pb >= parray_top || pw >= parray_top) continue;
    pos--;
    len = snprintf(dst+pos, dstlen-pos, ":%s:%s:%s:%s\n"
      , parray[pb].pname, parray[pb].srank
      , parray[pw].pname, parray[pw].srank
      );
    if (len < 0) return pos; else pos += len;
    if (!garray[g0].minkg) continue;
    pos--;
    mnum = movenum(garray[g0].minkg);
    len = snprintf(dst+pos, dstlen-pos , ":%d\n" , (int) mnum);
    if (len < 0) return pos; else pos += len;
#if 0
    pos--;
    len = snprintf(dst+pos, dstlen-pos
    , ":%d:%d:%d:%f:%d:%c:%c\n"
    , (int) mnum
    , (int) garray[g0].minkg->width, (int) garray[g0].minkg->handicap
    , (float) garray[g0].komi
    , (int) TICS2SECS(garray[g0].ts.byoticks) / 60
    , (garray[g0].rated) ? ' ' : garray[g0].teach ? 'T' : 'F'
    , (garray[g0].rules == RULES_NET) 
      ? (parray[pw].match_type == GAMETYPE_TNETGO) ? '*' : 'I'
      : 'G'
    );
    if (len < 0) break; else pos += len;
#endif
  }

return pos;
}

static int udp_board(char *dst, size_t dstlen, int gnum)
{
int len;
size_t pos=0;

len = snprintf(dst, dstlen, "Board %d\n", gnum);
gnum -= 1;

if (len < 0) return len; pos = len;
if (pos >= dstlen) {
	len = snprintf(dst, dstlen, "%d/%u\n", pos, dstlen);
	return len; }

if (gnum < 0 || gnum >= garray_top) {
	len = snprintf(dst, dstlen, "[invalid: %d >= %u]\n", gnum, garray_top);
	return len;
	}

len = printboard_raw(dst+pos, dstlen-pos, garray[gnum].minkg);
if (len < 0) return pos; pos += len;

return pos;
}

static int udp_slots(char *dst, size_t dstlen)
{
int len, idx;
char *cp;
size_t pos=0;

len = snprintf(dst, dstlen, "Slots\n");
if (len < 0) return pos; pos = len;
for (idx=0; idx < parray_top; idx++) {
	cp = player_dumpslot(idx) ;
	len = strlen(cp);
	if (len + pos +2 >= dstlen) break;
	memcpy(dst+pos, cp, len);
	pos += len;
	len = snprintf(dst+pos, dstlen-pos, ":%d\n" , parray[idx].session.gnum);
	if (len < 0) break;
	pos += len;
	}
for (cp = dst; *cp; cp++) switch (*cp) {
	case ' ': case '/': case '[': case ']': *cp = ':';
	default: break;
	}
return pos;
}

static int udp_players(char *dst, size_t dstlen)
{
int len,idx;
size_t pos=0;

len = snprintf(dst, dstlen, "Players\n");
if (len < 0) return pos; pos = len;
for (idx=0; idx < parray_top; idx++) {
	if (pos +40 > dstlen) break;
	if (!parray[idx].slotstat.is_inuse) continue;
	if (!parray[idx].slotstat.is_online) continue;
	len = snprintf(dst+pos, dstlen-pos, "%d:%s", idx+1, parray[idx].pname);
	if (len < 0) return pos; pos += len;
	len = snprintf(dst+pos, dstlen-pos, ":%s:%s:%s:%d"
	, parray[idx].srank
	, parray[idx].flags.is_looking? "!" : "-"
	, parray[idx].flags.is_open? "O" : "-"
	, parray[idx].session.gnum);
	if (len < 0) return pos; pos += len;
	memcpy(dst+pos, "\n" , 2); pos += 1;
	}
return pos;
}

static int udp_shutdown(char *dst, size_t dstlen, char *opt)
{
int len, slot;
size_t pos=0;
char *name="*shutdown*";

  slot = player_find_login(name);
  if (slot < 0) {
    slot = player_new(); if (slot <0) slot = 0; /*!!*/
    parray[slot].session.socket = open("/dev/null", 0, 0);
    do_copy(parray[slot].login, name, sizeof parray[slot].login);
    do_copy(parray[slot].pname, name, sizeof parray[slot].pname);
  }
  len = snprintf(dst, dstlen, "shutdown %s\n", opt);
  if (len < 0) return pos; pos = len;
  pcommand(slot, dst);

return pos;
}

static int udp_uptime(char *dst, size_t dstlen)
{
int len;
time_t uptime, now;
int count;
size_t pos=0;

  len = snprintf(dst, dstlen, "Uptime\n");
  if (len < 0) return pos; pos = len;
  now = globclock.time;
  uptime = now - startuptime;

  len = snprintf(dst+pos, dstlen-pos, "Servername: %s\n", conffile.server_name);
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Serveraddress: %s\n", conffile.server_address);
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Version: %s\n", conffile.version_string);
  if (len < 0) return pos; pos += len;

  len = snprintf(dst+pos, dstlen-pos, "Date(UTC): %s\n", strgtime(&now));
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Date(local): %s\n", strltime(&now));
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Startuptime(local): %s\n", strltime(&startuptime));
  if (len < 0) return pos; pos += len;

  /* Does this break any clients? */
  if (uptime > 86400)
    len = snprintf(dst+pos, dstlen-pos, "Uptime: %d days,%s\n", 
	    uptime/86400, strhms(uptime%86400));
  else
    len = snprintf(dst+pos, dstlen-pos, "Uptime: %s\n", strhms(uptime));
  if (len < 0) return pos; pos += len;

  len = snprintf(dst+pos, dstlen-pos, "Fd_count: %d\n", net_fd_count);
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Completed_games: %d\n", completed_games);
  if (len < 0) return pos; pos += len;
  count = player_count();
  len = snprintf(dst+pos, dstlen-pos, "Player_count: %d\n", count);
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Player_high: %d\n", player_high);
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Logins: %d\n", num_logins);
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Logouts: %d\n", num_logouts);
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "New_players: %d\n", new_players);
  if (len < 0) return pos; pos += len;
  count = game_count();
  len = snprintf(dst+pos, dstlen-pos, "Game_count: %d\n", count);
  if (len < 0) return pos; pos += len;
  len = snprintf(dst+pos, dstlen-pos, "Game_high: %d\n", game_high);
  if (len < 0) return pos; pos += len;

  len = snprintf(dst+pos, dstlen-pos, "Byte_count: %d\n", byte_count);
  if (len < 0) return pos; pos += len;

return pos;
}

