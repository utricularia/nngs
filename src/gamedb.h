/* gamedb.h
 *
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

#ifndef _GAMEDB_H_
#define _GAMEDB_H_

	/* AvK Fixme: to avoid dependency on sys/types.h:ulong,
	** mink.h now supplies xulong. (typedef'd as unsigned long, anyway)
	** I don't think, the code relies on (ulong===xulong).
	*/
#include "mink.h"

#define GSTATUS_EMPTY 0
#define GSTATUS_NEW 1
#define GSTATUS_ACTIVE 2
#define GSTATUS_STORED 3
#define GSTATUS_ANALYSIS 4   /* for analysis mode */

#define MAX_GO_MOVES 1000

#define GAMETYPE_GO 1
#define GAMETYPE_GOEGO 2
#define GAMETYPE_NETGO 3

#define GAMETYPE_LADDER 4
#define GAMETYPE_TGO 5
#define GAMETYPE_TGOEGO 6 
#define GAMETYPE_TNETGO 7

#define END_RESIGN 0           /* A player typed "resign" */
#define END_FLAG 1             /* A player ran out of time */
#define END_ADJOURN 2          /* Players agreed to adjourn */
#define END_LOSTCONNECTION 3   /* A player lost connection */
#define END_NOTENDED 4         /* The game is in progress */
#define END_DONE 5             /* The game is finished, was scored */

#define WHITE 0x00
#define BLACK 0x80
#define NEITHER 10

#define NOTPAIRED	0
#define PAIR1		1
#define PAIR2		2

#define TIMETYPE_UNTIMED 0
#define TIMETYPE_TIMED 1

struct game {
	/* Key info */
  int gstatus;
  int white;
  int black;
#ifdef PAIR
  int pairwith;
  int pairstate;
#endif
  char *gtitle;
  char *gevent;
  time_t timeOfStart; 
  unsigned startTime;    /* The relative time the game started  */
  unsigned lastMoveTime; /* Last time a move was made */
  unsigned lastDecTime;  /* Last time a players clock was decremented */
  struct minkgame *GoGame;
  int nmvinfos;
  struct mvinfo *mvinfos;
	/* flags */
  int Teach;  /* Single player teaching game presentation */
  int Teach2; /* two players teaching */
  int rated;
  int Private;
  int Tourn;
  int Ladder9;
  int Ladder19;
  int Ladder_Possible;
	/* parameters */
  int size;
  int type;
  int gotype;
  int rules;         /* RULES_NET or RULES_ING */
  float komi;
  int time_type; /* If timed or untimed */
  int Byo;         /* Byo time per player */
  int ByoS;        /* Byo stones per player */
	/* move state */
  int onMove;
  int num_pass; 
  int wInByo;
  int bInByo;
  int wByoStones;  /* Stones left to play in byo period */
  int bByoStones;  /* Stones left to play in byo period */
  int wTime;
  int bTime;
  int clockStopped;
  int nocaps;
  /* GOE */
  int B_penalty;	 /* number of B penalty points */
  int W_penalty;	 /* number of W penalty points */
  int num_Bovertime;      /* number of B overtime periods entered */
  int num_Wovertime;      /* number of W overtime periods entered */
	/* Result */
  int result;
  int winner;
  float gresult;
	/* Misc :-) */
  int old_white; /* Contains the old game player number */
  int old_black; /* Contains the old game player number */
} ;

extern struct game *garray;
extern int garray_top;

extern int game_new(int, int);
extern int game_remove(int);
extern int game_finish(int);

extern char *game_time_str(int, int, int, int);
extern char *game_str(int, int, int, int, int, char *, char *);
extern char *ggame_str(int, int, int);
extern int game_isblitz(int, int, int, int, char *, char *);

extern void add_kib(struct game *g, int movecnt, char *s);

extern void send_go_board_to(int, int);
extern void send_go_boards(int, int);
extern void game_update_time(int);
extern void game_update_times(void);

	/* This is the number of games that is kept in
	** garray[], after they completed.
	** This should allow players to analyse or continue
	** after finishing or disconnect/reconnect
	*/
#define MAXOLDGAMES 50

extern int FindOldGameFor(int);
extern int RemoveOldGamesForPlayer(int);
extern int ReallyRemoveOldGamesForPlayer(int);
extern int NewOldGame(int);

extern void game_disconnect(int, int);

extern int game_read(struct game *, int, int);
extern int game_delete(int, int);
extern int game_save(int);

extern int pgames(int, char *);
extern void game_write_complete(int, int, twodstring);
extern int game_count(void);
extern int game_get_num_ob(int);
extern int write_g_out(int, FILE *, int, int, char *);
extern int game_save_complete(int, FILE *, twodstring);
#endif /* _GAMEDB_H_ */

#if WANT_NEWSERVER

typedef struct _game {

char *PlayerList;          /* colon delimited, NUL terminated list of players */
unsigned long game_id;     /* ID of the game.  Should be unique */
go_game_t go_game;         /* pointer to go game dependent information */
c4_game_t c4_game;         /* pointer to connect-4 game dependent information */
/* etc */
} game;

typedef struct _go_game_t {
  int wInByo;
  int wByoStones;  /* Stones left to play in byo period */
  int bInByo;
  int bByoStones;  /* Stones left to play in byo period */
  int Byo;         /* Byo time per player */
  int ByoS;        /* Byo stones per player */
  time_t timeOfStart; 
  float komi;
  int num_pass; 
  int Teach;
  int Ladder9;
  int Ladder19;
  int Ladder_Possible;
#ifdef PAIR
  int pairwith;
  int pairstate;
#endif
  int wTime;
  int bTime;
  int clockStopped;
  int rated;
  int nocaps;
  int Private;
  int type;
  int size;
  int gotype;
  int look;
  char *gtitle;
  minkgame *GoGame;
  int onMove;
  float gresult;
  struct mvinfo *mvinfos;
  int nmvinfos;

  /* Not saved in game file */
  int white;
  int black;
  int old_white; /* Contains the old game player number */
  int old_black; /* Contains the old game player number */
  int status;

  unsigned startTime;    /* The relative time the game started  */
  unsigned lastMoveTime; /* Last time a move was made */
  unsigned lastDecTime;  /* Last time a players clock was decremented */

  int result;
  int winner;
  int width, height;    /* board dimensions */
  move *moves;          /* move history */
  int mvsize,movenr;    /* size of moves and number of moves played */
  int *board;           /* current state of go-board */
  xulongpair *zob;       /* zobrist random numbers */
  int *uf;              /* union-find structure */
  uf_log *uflog;        /* log of changes to uf */
  int logsize,logid;    /* size of uflog & number of changes to uf */
#ifdef MINKKOMI
  float komi;
#endif
  int handicap;         /* handicap */
  int caps[4];          /* # stones captured */
  xulong hash;
  int root,kostat;      /* root is temporary variable used in group merging */
} go_game;

typedef struct _c4_game_t {
  int whatever;
} c4_game;

#endif /* NEWSERVER */
