/* nngsmain.c
 *
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <stdlib.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef USING_DMALLOC
#include <dmalloc.h>
#define DMALLOC_FUNC_CHECK 1
#endif

#include "nngsconfig.h"
#include "conffile.h"
#include "nngsmain.h"
#include "common.h"
#include "network.h"
#include "command.h"
#include "channel.h"
#include "playerdb.h"
#include "utils.h"
#include "ladder.h"
#include "emote2.h"
#include "mink.h"
#include "ip_ban.h"

/* Arguments */
int /* port, */ Ladder9, Ladder19, num_19, num_9, completed_games,
       num_logins, num_logouts, new_players, Debug;
#if WANT_BYTE_COUNT
unsigned long byte_count = 0L;
#endif
static char confname[1024] = "nngs.cnf";

void player_array_init(void);
void player_init(void);
static void usage(char *);

static void usage(char *progname);
static void GetArgs(int argc, char *argv[]);
static void main_event_loop(void);
static void TerminateServer(int sig);
static void BrokenPipe(int sig);
static void read_ban_ip_list(void);
static int all_the_internets(void);


static void usage(char *progname) {
  fprintf(stderr, "Usage: %s [-c <conffilename>] [-h]\n", progname);
  fprintf(stderr, "\t\t-h\t\tDisplay this information.\n");
  main_exit(1);
}

static void GetArgs(int argc, char *argv[])
{
  int i;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
/*
      case 'p':
	if (i == argc - 1) usage(argv[0]);
	i++;
	if (sscanf(argv[i], "%d", &port) != 1)
	  usage(argv[0]);
	break;
*/
      case 'c':
	if (i == argc - 1) usage(argv[0]);
	i++;
	if (sscanf(argv[i], "%s", &confname) != 1)
	  usage(argv[0]);
	break;
      case 'h':
	usage(argv[0]);
	break;
      }
    } else {
      usage(argv[0]);
    }
  }
}


static void main_event_loop(void)
{
  net_select(1);
}


static void TerminateServer(int sig)
{
  fprintf(stderr, "Got signal %d\n", sig);
  Logit("Got signal %d", sig);
  TerminateCleanup();
  net_closeAll();
  main_exit(1);
}


static void BrokenPipe(int sig)
{
  static time_t lasttime=0;
  static unsigned count=0;

	/* Obsolete: reinstall signal handler. Won't harm */
  signal(SIGPIPE, BrokenPipe);
  count++;
  if (globclock.time - lasttime > 10) {
    Logit("Got %u Broken Pipes in %d seconds: sig %d\n"
	, count, globclock.time - lasttime, sig);
    lasttime=globclock.time;
    count=0;
    }
}


int main(int argc, char *argv[])
{
  FILE *fp;

#ifdef USING_DMALLOC
  dmalloc_debug(1);
#endif

	/* start the clock (which is used by the Logit fnc) */
  (void) refetch_ticker();
  GetArgs(argc, argv);
  if (conf_file_read(confname)) {
    Logit("Failed to read config file \"%s\"", confname);
    strcpy(confname, "./nngs.cnf");
    conf_file_write(confname);
    Logit("Created \"%s\"", confname);
  }
  signal(SIGTERM, TerminateServer);
  signal(SIGINT, TerminateServer);
#if 0
  signal(SIGPIPE, SIG_IGN);
#else
  signal(SIGPIPE, BrokenPipe);
#endif
  read_ban_ip_list();
  if (!all_the_internets() ) {
    fprintf(stderr, "Network initialize failed on ports %s.\n"
    , conffile.server_ports);
    main_exit(1);
  }
  startuptime = time(NULL);
  srand(startuptime);
  player_high = 0;
  game_high = 0;
#if WANT_BYTE_COUNT
  byte_count = 0;
#endif

#ifdef SGI
  /*mallopt(100, 1);*/  /* Turn on malloc(3X) debugging (Irix only) */
#endif
  command_init();
  EmoteInit(conffile.emotes_file);
  help_init();
  /*Logit("commands_init()");*/
  commands_init();
  /*Logit("channel_init()");*/
  channel_init();
  /*Logit("player_array_init()");*/
  player_array_init();
  player_init();
  LadderInit(NUM_LADDERS);
  Ladder9 = LadderNew(LADDERSIZE);
  Ladder19 = LadderNew(LADDERSIZE);

  Debug = 0;
  completed_games = 0;
  num_logins = num_logouts = new_players = 0;

  num_9 = 0;
  fp = xyfopen(FILENAME_LADDER9, "r");
  if (fp) {
    num_9 = PlayerLoad(fp, Ladder9);
    Logit("%d players loaded from file %s", num_9, filename() );
    fclose(fp);
  }

  num_19 = 0;
  fp = xyfopen(FILENAME_LADDER19, "r");
  if (fp) {
    num_19 = PlayerLoad(fp, Ladder19);
    Logit("%d players loaded from file %s", num_19, filename() );
    fclose(fp);
  }

  initmink();
  Logit("Server up and running at");
  main_event_loop();
  Logit("Closing down.");
  net_closeAll();
  main_exit(0);
  return 0;
}

static int all_the_internets(void)
{
int port;
char *cp;
int pos, len, rc, cnt;

  for(pos=cnt=0;conffile.server_ports[pos];pos += len	) {
    rc = sscanf(conffile.server_ports+pos, "%d%n", &port, &len );
    if (rc < 0) break;
    if (rc < 1) { len=1; continue; }
    if (net_init(port)) {
      Logit("Init failed on port %d.", port);
      continue;
      }
    Logit("Initialized on port %d.", port);
    cnt++;
  }
return cnt;
}

void main_exit(int code)
{

#ifdef USING_DMALLOC
  dmalloc_log_unfreed();
  dmalloc_shutdown();
#endif
  exit(code);
}


static void read_ban_ip_list(void)
{
  FILE *fp;
  int rc,cnt=0;
  char buff[100];
  unsigned bot, top;

  fp = xyfopen(FILENAME_LIST_BAN, "r");
  if (!fp) return;
  while(fgets(buff, sizeof buff, fp)) {
    rc = sscanf(buff, "%x %x", &bot, &top);
    if (rc < 2) continue;
    cnt += range_add(bot,top);
  }
  fclose(fp);
  Logit("Ipban: Read %d ranges from %s", cnt, filename() );
}
