/*
    NNGS - The No Name Go Server
    Copyright (C) 1995-1996 Erik Van Riper (geek@willent.com)
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

#include <math.h>
#include <stdlib.h>
#include <assert.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "nngsmain.h"
#include "nngsconfig.h"
#include "gameproc.h"
#include "common.h"
#include "servercodes.h"
#include "playerdb.h"
#include "gamedb.h"
#include "utils.h"
#include "ladder.h"

#ifdef USING_DMALLOC
#include <dmalloc.h>
#define DMALLOC_FUNC_CHECK 1
#endif

void game_ended(int g, int winner, int why)
{
  const struct player *Ladder_B, *Ladder_W;
  char outstr[180];
  char tmp[50];
  char statZ1[200];
  char statZ2[200];
  int p, p2, pb, pw,
  wterr,  /* W's territory */
  wocc,   /* W stones on the board */
  bterr,  /* B's territory */
  bocc,   /* B stones on the board */
  wcaps,  /* Number of captured W stones (add to B's score in Japanese score) */
  bcaps,  /* Number of captured B stones (add to W's score in Japanese score) */
  rate_change = 0,
  isDraw = 0,
  until, x;

  twodstring statstring;
  float wscore, bscore;
  FILE *fp;
  time_t now;

  now = globClock;
  pb=garray[g].black ;
  pw=garray[g].white ;

  sprintf(outstr, "{Game %d: %s vs %s :",
          g + 1,
          parray[pw].pname,
          parray[pb].pname);

  garray[g].result = why;
  garray[g].winner = winner;

  switch (why) {
  case END_DONE:
    completed_games++;
    getcaps(garray[g].GoGame, &wcaps, &bcaps);
    countscore(garray[g].GoGame, statstring, &wterr, &bterr, &wocc, &bocc);
    if(Debug)
      Logit("k=%.1f wtr=%d, btr=%d, wocc=%d, bocc=%d, wcaps=%d, bcaps=%d",
                 garray[g].komi, wterr, bterr,  wocc, bocc, wcaps, bcaps);
#ifdef CHINESESCORE
    if (garray[g].komi > 0) wscore = wterr + wocc + garray[g].komi;
    else wscore = wterr + wocc;
    if (garray[g].komi < 0) bscore = bterr + bocc + fabs(garray[g].komi);
    else bscore = bterr + bocc;
#endif /* CHINESESCORE */
    if (garray[g].komi > 0) wscore = wterr + bcaps + garray[g].komi;
    else wscore = wterr + bcaps;
    if (garray[g].komi < 0) bscore = bterr + wcaps + fabs(garray[g].komi);
    else bscore = bterr + wcaps;
    if (wscore > bscore) {
      garray[g].gresult = (float) wscore - bscore;
      winner = pw;
    }
    else {
      garray[g].gresult = (float) bscore - wscore;
      winner = pb;
    }
    sprintf(statZ1, "%s %3.3s%s %d %d %d T %.1f %d\n",
                parray[pw].pname,
                parray[pw].srank,
                parray[pw].rated ? "*" : " ",
                bcaps,
                garray[g].wTime,
                garray[g].wByoStones,
                garray[g].komi,
                garray[g].GoGame->handicap);
    sprintf(statZ2, "%s %3.3s%s %d %d %d T %.1f %d\n",
                parray[pb].pname,
                parray[pb].srank,
                parray[pb].rated ? "*" : " ",
                wcaps,
                garray[g].bTime,
                garray[g].bByoStones,
                garray[g].komi,
                garray[g].GoGame->handicap);

    if (parray[pb].client) {
      pcn_out(pb, CODE_NONE, FORMAT_n);
      pcn_out(pb, CODE_STATUS, FORMAT_s, statZ1);
      pcn_out(pb, CODE_STATUS, FORMAT_s, statZ2);

      until = garray[g].GoGame->height;
      for(x = 0; x < until; x++) {
        pcn_out(pb, CODE_STATUS, FORMAT_d_sn, x, statstring[x]);
      }
    }
    if (parray[pw].client) {
      pcn_out(pw, CODE_NONE, FORMAT_n);
      pcn_out(pw, CODE_STATUS, FORMAT_s, statZ1);
      pcn_out(pw, CODE_STATUS, FORMAT_s, statZ2);

      until = garray[g].GoGame->height;
      for(x = 0; x < until; x++) {
        pcn_out(pw, CODE_STATUS, FORMAT_d_sn, x, statstring[x]);
      }
    }
    pcn_out(pw, CODE_SCORE_M, FORMAT_s_W_O_f_TO_s_B_fn,
                          parray[pw].pname,
                          wscore,
                          parray[pb].pname,
                          bscore);

    pcn_out(pb, CODE_SCORE_M, FORMAT_s_W_O_f_TO_s_B_fn,
                          parray[pw].pname,
                          wscore,
                          parray[pb].pname,
                          bscore);

    pcn_out(pw, CODE_INFO, FORMAT_s_HAS_RESIGNED_THE_GAME_n,
           (winner == pw) ? \
           parray[pb].pname : parray[pw].pname);

    pcn_out(pb, CODE_INFO, FORMAT_s_HAS_RESIGNED_THE_GAME_n,
           (winner == pw) ? \
           parray[pb].pname : parray[pw].pname);

    sprintf(tmp, " %s resigns. W %.1f B %.1f}\n",
            (winner == pw) ? "Black" : "White",
            wscore, bscore);

    rate_change = 1;
    for (p = 0; p < parray_top; p++) {
      if (!parray[p].slotstat.online) continue;
      if (!parray[p].i_game && !player_is_observe(p, g)) continue;
/*      pcn_out_prompt(p, CODE_SHOUT, "%s%s", outstr, tmp); */
      if (player_is_observe(p, g)) player_remove_observe(p, g); 
      /*pcn_out_prompt(p, CODE_INFO, "%s%s", outstr, tmp);*/
      if (parray[p].client) {
        pcn_out(p, CODE_CR1|CODE_STATUS, FORMAT_s, statZ1);
        pcn_out(p, CODE_STATUS, FORMAT_s, statZ2);
        until = garray[g].GoGame->height;
        for(x = 0; x < until; x++) {
          pcn_out(p, CODE_STATUS, FORMAT_d_sn, x, statstring[x]);
        }
        pcn_out_prompt(p, CODE_STATUS, FORMAT_ss, outstr, tmp);
      }
    }
    garray[g].winner = winner;
    game_delete(pw, pb);
    break;

  case END_RESIGN:
    completed_games++;
    sprintf(tmp, " %s resigns.}\n",
            (winner == pw) ? "Black" : "White");
    rate_change = 1;
    garray[g].winner = winner;
    garray[g].gresult = 0.0;
    game_delete(pw, pb);

    pcn_out(pw, CODE_INFO, FORMAT_s_HAS_RESIGNED_THE_GAME_n,
           (winner == pw) ? \
           parray[pb].pname : parray[pw].pname);

    pcn_out(pb, CODE_INFO, FORMAT_s_HAS_RESIGNED_THE_GAME_n,
           (winner == pw) ? \
           parray[pb].pname : parray[pw].pname);

    break;

  case END_FLAG:
    completed_games++;
    sprintf(tmp, " %s forfeits on time.}\n",
            (winner == pw) ? "Black" : "White");
    rate_change = 1;
    garray[g].winner = winner;
    garray[g].gresult = -1.0;
    game_delete(pw, pb);
    break;

  case END_ADJOURN:
  case END_LOSTCONNECTION:
    sprintf(tmp, " has adjourned.}\n");
    rate_change = 0;
    game_save(g);
    pcn_out(pb, CODE_INFO, FORMAT_GAME_HAS_BEEN_ADJOURNED_n);
    pcn_out(pb, CODE_INFO, FORMAT_GAME_d_s_VS_s_HAS_ADJOURNED_n,
          g + 1,
          parray[pw].pname,
          parray[pb].pname);

    if (!garray[g].Teach) {
      pcn_out(pw, CODE_INFO, FORMAT_GAME_HAS_BEEN_ADJOURNED_n );
      pcn_out(pw, CODE_INFO, FORMAT_GAME_d_s_VS_s_HAS_ADJOURNED_n,
          g + 1,
          parray[pw].pname,
          parray[pb].pname);
    }
    break;

  default:
    sprintf(tmp, " Hmm, the game ended and I don't know why(%d).} *\n", why);
    break;
  }
  strcat(outstr, tmp);
  
  for (p = 0; p < parray_top; p++) {
    if (!parray[p].slotstat.online) continue;
    if (parray[p].i_game || player_is_observe(p, g))
    pcn_out_prompt(p, CODE_SHOUT, FORMAT_s, outstr);
    if ((player_is_observe(p, g)) || 
        (p == pb)   || 
        (p == pw)) {
      pcn_out_prompt(p, CODE_INFO, FORMAT_s, outstr);
    }
      player_remove_observe(p, g); 
  }

  if ((winner == pb) && (rate_change == 1)) {
    player_resort();
    if ((garray[g].Ladder9 == 1) || (garray[g].Ladder19 == 1)) {
      if (garray[g].Ladder9 == 1) {
        Ladder_W = PlayerNamed(Ladder9, parray[pw].pname);
        Ladder_B = PlayerNamed(Ladder9, parray[pb].pname);
        PlayerRotate(Ladder9, Ladder_W->idx, Ladder_B->idx);
        PlayerUpdTime(Ladder9, Ladder_W->idx, now);
        PlayerUpdTime(Ladder9, Ladder_B->idx, now);
        PlayerAddWin(Ladder9, Ladder_B->idx);
        PlayerAddLoss(Ladder9, Ladder_W->idx);
        fp = xyfopen(FILENAME_LADDER9, "w");
        if (fp == NULL) {
          pcn_out(p, CODE_ERROR,FORMAT_THERE_WAS_AN_INTERNAL_ERROR_PLEASE_NOTIFY_AN_ADMIN_n);
        } else {
          num_9 = PlayerSave(fp, Ladder9);
          fclose(fp);
          Ladder_W = PlayerNamed(Ladder9, parray[pw].pname);
          Ladder_B = PlayerNamed(Ladder9, parray[pb].pname);
          pcn_out(pb, CODE_INFO, FORMAT_YOU_ARE_NOW_AT_POSITION_d_IN_THE_9X9_LADDER_CONGRATS_n, (Ladder_B->idx) + 1);
          pcn_out(pw, CODE_INFO, FORMAT_YOU_ARE_NOW_AT_POSITION_d_IN_THE_9X9_LADDER_n, (Ladder_W->idx) + 1);
          for (p2 = 0; p2 < parray_top; p2++) {
            if (!parray[p2].slotstat.online) continue;
            if (!parray[p2].i_lshout) continue;
            pcn_out_prompt(p2, CODE_SHOUT,FORMAT_LADDER9_RESULT_s_TAKES_POSITION_d_FROM_s_n,
                  parray[pb].pname,
                  (Ladder_B->idx) + 1,
                  parray[pw].pname,
                  (Ladder_W->idx) + 1);
          }
        }
      }
      if (garray[g].Ladder19 == 1) {
        Ladder_W = PlayerNamed(Ladder19, parray[pw].pname);
        Ladder_B = PlayerNamed(Ladder19, parray[pb].pname);
        PlayerRotate(Ladder19, Ladder_W->idx, Ladder_B->idx);
        PlayerUpdTime(Ladder19, Ladder_W->idx, now);
        PlayerUpdTime(Ladder19, Ladder_B->idx, now);
        PlayerAddWin(Ladder19, Ladder_B->idx);
        PlayerAddLoss(Ladder19, Ladder_W->idx);
        fp = xyfopen(FILENAME_LADDER19, "w");
        if (fp == NULL) {
          pcn_out(p, CODE_ERROR,FORMAT_THERE_WAS_AN_INTERNAL_ERROR_PLEASE_NOTIFY_AN_ADMIN_n);
        } else {
          num_19 = PlayerSave(fp, Ladder19);
          fclose(fp);
          Ladder_W = PlayerNamed(Ladder19, parray[pw].pname);
          Ladder_B = PlayerNamed(Ladder19, parray[pb].pname);
          pcn_out(pb, CODE_INFO, FORMAT_YOU_ARE_NOW_AT_POSITION_d_IN_THE_19X19_LADDER_CONGRATS_n, (Ladder_B->idx) + 1);
          pcn_out(pw, CODE_INFO, FORMAT_YOU_ARE_NOW_AT_POSITION_d_IN_THE_19X19_LADDER_n, (Ladder_W->idx) + 1);
          for (p2 = 0; p2 < parray_top; p2++) {
            if (!parray[p2].slotstat.online) continue;
            if (!parray[p2].i_lshout) continue;
            pcn_out_prompt(p2, CODE_SHOUT,FORMAT_LADDER19_RESULT_s_TAKES_POSITION_d_FROM_s_n,
                  parray[pb].pname,
                  (Ladder_B->idx) + 1,
                  parray[pw].pname,
                  (Ladder_W->idx) + 1);
          }
        }
      }
    }
  }
  else if ((winner == pw) && (rate_change == 1)) {
    if ((garray[g].Ladder9 == 1) || (garray[g].Ladder19 == 1)) {
      if (garray[g].Ladder9 == 1) {
        Ladder_W = PlayerNamed(Ladder9, parray[pw].pname);
        Ladder_B = PlayerNamed(Ladder9, parray[pb].pname);
        PlayerUpdTime(Ladder9, Ladder_W->idx, now);
        PlayerUpdTime(Ladder9, Ladder_B->idx, now);
        PlayerAddWin(Ladder9, Ladder_W->idx);
        PlayerAddLoss(Ladder9, Ladder_B->idx);
        fp = xyfopen(FILENAME_LADDER9, "w");
        if (fp == NULL) {
          pcn_out(p, CODE_ERROR,FORMAT_THERE_WAS_AN_INTERNAL_ERROR_PLEASE_NOTIFY_AN_ADMIN_n);
        } else {
          num_9 = PlayerSave(fp, Ladder9);
          fclose(fp);
        }
      }
      if (garray[g].Ladder19 == 1) {
        Ladder_W = PlayerNamed(Ladder19, parray[pw].pname);
        Ladder_B = PlayerNamed(Ladder19, parray[pb].pname);
        PlayerUpdTime(Ladder19, Ladder_W->idx, now);
        PlayerUpdTime(Ladder19, Ladder_B->idx, now);
        PlayerAddWin(Ladder19, Ladder_W->idx);
        PlayerAddLoss(Ladder19, Ladder_B->idx);
        fp = xyfopen(FILENAME_LADDER19, "w");
        if (fp == NULL) {
          pcn_out(p, CODE_ERROR,FORMAT_THERE_WAS_AN_INTERNAL_ERROR_PLEASE_NOTIFY_AN_ADMIN_n);
        } else {
          num_19 = PlayerSave(fp, Ladder19);
          fclose(fp);
        }
      }
    }
  }
  if (rate_change)
    game_write_complete(g, isDraw, why == END_DONE ? statstring : NULL);
  parray[pw].protostate = STAT_WAITING;
  parray[pb].protostate = STAT_WAITING;
  pprintf_prompt(pw, "\n");
  if (!garray[g].Teach) pprintf_prompt(pb, "\n");
  if ((why == END_RESIGN) || (why == END_FLAG) || (why == END_DONE)) {
    if (!garray[g].Teach) {
      parray[pb].lastColor = BLACK;
      parray[pw].lastColor = WHITE;
      parray[pb].gonum_black++;
      parray[pw].gonum_white++;
      if (garray[g].winner == pw) {
        parray[pw].gowins++;
        parray[pw].water++;
        parray[pb].golose++;
      }
      else if (garray[g].winner == pb) {
        parray[pw].golose++;
        parray[pb].gowins++;
        parray[pb].water++;
      }
    }
  }
  parray[pw].game = -1;
  parray[pb].game = -1;
  parray[pw].opponent = -1;
  parray[pb].opponent = -1;
  parray[pw].last_opponent = pb;
  parray[pb].last_opponent = pw;
  parray[pw].match_type = 0;
  parray[pb].match_type = 0;
  game_finish(g);
}

#ifdef PAIR
int paired(int g)
{
  int i;

  return garray[g].pairstate != NOTPAIRED &&
         garray[i = garray[g].pairwith].pairstate != NOTPAIRED &&
         garray[i].pairwith == g && garray[i].gstatus == GSTATUS_ACTIVE;
}
#endif

#ifdef PAIR
void process_move(int p, char *command, int original)
#else
void process_move(int p, char *command)
#endif
{
  int g;
  int good;
  unsigned now;
  int gmove;
  int players_only = 0;


  if (parray[p].game < 0) {
    pcn_out_prompt(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_n);
    return;
  }

  player_decline_offers(p, -1, -1);

  g = parray[p].game;

  if (((garray[g].Teach == 0) && (garray[g].Teach2 == 0)) && 
      ((parray[p].side != garray[g].onMove) && (parray[p].protostate != STAT_SCORING))) {
    pcn_out_prompt(p, CODE_ERROR, FORMAT_IT_IS_NOT_YOUR_MOVE_n);
    return;
  }

  if (garray[g].clockStopped) {
    pcn_out_prompt(p, CODE_INFO, FORMAT_GAME_CLOCK_IS_PAUSED_USE_qUNPAUSEq_TO_RESUME_n);
    return;
  }

#ifdef PAIR
/* pair1 = (W1,B1), pair2 = (W2,B2)
   movenum % 4      0   1   2   3   ( == movenum & 3)
   player to move   B1  W1  B2  W2
*/
  if (paired(g) && original
      && parray[p].protostate != STAT_SCORING &&
      ((movenum(garray[g].GoGame) & 3) / 2) != (garray[g].pairstate == PAIR2)) {
    pcn_out_prompt(p, CODE_ERROR, FORMAT_IT_IS_YOUR_PARTNER_S_MOVE_n);
    return;
  }
#endif


  /* test if we are removing dead stones */
  if (parray[p].protostate == STAT_SCORING) {
    if (!strcmp(command, "pass") ) {    /* User passes */
      pcn_out(p, CODE_ERROR, FORMAT_PASS_IS_NOT_VALID_DURING_SCORING_n);
      assert(parray[garray[g].white].protostate == STAT_SCORING);
      assert(parray[garray[g].black].protostate == STAT_SCORING);
      return;
    }

    /* Remove all "done"'s, still removing stones..... */
    player_remove_requests(parray[p].opponent, p, PEND_DONE);
    player_remove_requests(p, parray[p].opponent, PEND_DONE);
    player_decline_offers(p, -1, PEND_DONE);
    player_decline_offers(parray[p].opponent, -1, PEND_DONE);
    player_withdraw_offers(p, -1, PEND_DONE);
    player_withdraw_offers(parray[p].opponent, -1, PEND_DONE);

/*    Logit("(Removing) g = %d, move = %d command = %s", g, go_move(garray[g].GoGame, command), command); */
    good = removedead(garray[g].GoGame, 
                go_move(garray[g].GoGame, command),
			  p == garray[g].white ? GO_WHITE : GO_BLACK);
  /* We are scoring, and removed a stone */
    if (good) {
      players_only = 1;
      pcn_out_prompt(garray[g].white, CODE_INFO, FORMAT_REMOVING_sn, stoupper(command));
      pcn_out_prompt(garray[g].black, CODE_INFO, FORMAT_REMOVING_sn, stoupper(command));
    }
  }

  /* Play the move, test if valid */
  else {
    if (!strcmp(command, "pass")) {    /* User passes */
      /* pass is valid */
      garray[g].num_pass = pass(garray[g].GoGame); 
      /* Check if we need to start scoring.... */
      if ((garray[g].Teach == 0) && (garray[g].num_pass >= 2)) {
        parray[garray[g].white].protostate = STAT_SCORING;
        parray[garray[g].black].protostate = STAT_SCORING;
        pcn_out_prompt(garray[g].white, CODE_INFO, FORMAT_YOU_CAN_CHECK_YOUR_SCORE_WITH_THE_SCORE_COMMAND_TYPE_DONE_WHEN_FINISHED_n);
        pcn_out_prompt(garray[g].black, CODE_INFO, FORMAT_YOU_CAN_CHECK_YOUR_SCORE_WITH_THE_SCORE_COMMAND_TYPE_DONE_WHEN_FINISHED_n);
      }
    }
    else if (!play(garray[g].GoGame, go_move(garray[g].GoGame, command),1)) { 
      pcn_out(p, CODE_ERROR, FORMAT_YOUR_MOVE_IS_NOT_VALID_n);
      pcn_out_prompt(p, CODE_ERROR, FORMAT_ILLEGAL_MOVE_n);
      return;
    }
    game_update_time(g);
    if(garray[g].gstatus != GSTATUS_ACTIVE) return;
    now = tenth_secs();
    garray[g].lastMoveTime = now;
    if (garray[g].onMove==WHITE) { 
      garray[g].onMove = BLACK;
      if (garray[g].wByoStones > 0) {
        if (garray[g].wByoStones == 1) {
          garray[g].wByoStones = garray[g].ByoS;
          garray[g].wTime = garray[g].Byo;
        }
        else garray[g].wByoStones--;
      }
    } else {
      garray[g].onMove = WHITE;
      if (garray[g].bByoStones > 0) {
        if (garray[g].bByoStones == 1) {
          garray[g].bByoStones = garray[g].ByoS;
          garray[g].bTime = garray[g].Byo;
        }
        else garray[g].bByoStones--;
      }
    }
  } 

  /* Check to see if we should be saving the game */
  gmove = movenum(garray[g].GoGame);
  if (gmove) if ((gmove % SAVEFREQ) == 0) game_save(g);

  /* send out the boards to everyone.... */
  send_go_boards(g, players_only);

#ifdef PAIR
  if (paired(g) && original) {
    process_move(garray[g].onMove == WHITE ? garray[garray[g].pairwith].black :
                                             garray[garray[g].pairwith].white,
                 stolower(command), 0);
  }
#endif
}

int com_title(int p, struct parameter* param)
{
  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  free(garray[parray[p].game].gtitle);
  garray[parray[p].game].gtitle = mystrdup(param[0].val.string); 
  return COM_OK;
}

int com_event(int p, struct parameter* param)
{
  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  free(garray[parray[p].game].gevent);
  garray[parray[p].game].gevent = mystrdup(param[0].val.string); 
  return COM_OK;
}

int com_resign(int p, struct parameter* param)
{
  UNUSED(param);
  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  player_decline_offers(p, -1, -1);
  game_ended(parray[p].game, (garray[parray[p].game].white == p)
     ? garray[parray[p].game].black
     : garray[parray[p].game].white, END_RESIGN);
  return COM_OK;
}

int com_pause(int p, struct parameter* param)
{
  int g;
  int now;
  struct pending *ptr;
  UNUSED(param);

  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  g = parray[p].game;
  if (garray[g].wTime == 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_CAN_T_PAUSE_UNTIMED_GAMES_);
    return COM_OK;
  }
  if (garray[g].clockStopped) {
    pcn_out(p, CODE_ERROR, FORMAT_GAME_IS_ALREADY_PAUSED_USE_qUNPAUSEq_TO_RESUME_);
    return COM_OK;
  }
  ptr=pending_find(parray[p].opponent, p, PEND_PAUSE);
  if (ptr) {
    player_remove_requests(parray[p].opponent, p, PEND_PAUSE);
    garray[g].clockStopped = 1;
    /* Roll back the time */
    if (garray[g].lastDecTime < garray[g].lastMoveTime) {
      if (garray[g].onMove == WHITE) {
	garray[g].wTime += (garray[g].lastDecTime - garray[g].lastMoveTime);
      } else {
	garray[g].bTime += (garray[g].lastDecTime - garray[g].lastMoveTime);
      }
    }
    now = tenth_secs();
    garray[g].lastMoveTime = now;
    garray[g].lastDecTime = now;
    pcn_out_prompt(parray[p].opponent, CODE_INFO, FORMAT_s_ACCEPTED_PAUSE_GAME_CLOCK_PAUSED_n,
		   parray[p].pname);
    pcn_out(p, CODE_INFO, FORMAT_GAME_CLOCK_PAUSED_n);
  return COM_OKN;

  player_add_request(p, parray[p].opponent, PEND_PAUSE, 0);
  pcn_out_prompt(parray[p].opponent, CODE_INFO, FORMAT_s_REQUESTS_TO_PAUSE_THE_GAME_n,
		   parray[p].pname);
  pcn_out(p, CODE_INFO, FORMAT_PAUSE_REQUEST_SENT_n);
  }
  return COM_OKN;
}

int com_unpause(int p, struct parameter * param)
{
  int now;
  int g;
  UNUSED(param);

  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  g = parray[p].game;
  if (!garray[g].clockStopped) {
    pcn_out(p, CODE_ERROR, FORMAT_GAME_IS_NOT_PAUSED_);
    return COM_OK;
  }
  garray[g].clockStopped = 0;
  now = tenth_secs();
  garray[g].lastMoveTime = now;
  garray[g].lastDecTime = now;
  pcn_out(p, CODE_INFO, FORMAT_GAME_CLOCK_RESUMED_);
  pcn_out_prompt(parray[p].opponent, CODE_INFO, FORMAT_GAME_CLOCK_RESUMED_n );
  return COM_OK;
}

int com_done(int p, struct parameter * param)
{
  UNUSED(param);

  if ((parray[p].game < 0) || (garray[parray[p].game].gotype < GAMETYPE_GO)) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  if (garray[parray[p].game].num_pass < 2) {
    pcn_out(p, CODE_ERROR, FORMAT_CANNOT_TYPE_DONE_UNTIL_BOTH_SIDES_HAVE_PASSED_);
    return COM_OK;
  }
  if (pending_find(parray[p].opponent, p, PEND_DONE)) {
    player_remove_requests(parray[p].opponent, p, PEND_DONE);
    player_decline_offers(p, -1, -1);
    game_ended(parray[p].game, NEITHER, END_DONE);
    return COM_OK;
  }

  player_add_request(p, parray[p].opponent, PEND_DONE, 0);
  pcn_out_prompt(parray[p].opponent, CODE_INFO, FORMAT_empty);
  return COM_OK;
}

int com_adjourn(int p, struct parameter * param)
{
  UNUSED(param);
  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  if (garray[parray[p].game].Teach == 1) {
    game_ended(parray[p].game, NEITHER, END_ADJOURN);
    return COM_OKN;
    }
  if (pending_find(parray[p].opponent, p, PEND_ADJOURN)) {
    player_remove_requests(parray[p].opponent, p, PEND_ADJOURN);
    player_decline_offers(p, -1, -1);
#ifdef PAIR
    if (paired(parray[p].game)) {
      game_ended(garray[parray[p].game].pairwith, NEITHER, END_ADJOURN);
    }
#endif /* PAIR */
    game_ended(parray[p].game, NEITHER, END_ADJOURN);
  return COM_OKN;
  }
  player_add_request(p, parray[p].opponent, PEND_ADJOURN, 0);
  pcn_out(parray[p].opponent, CODE_INFO,
          FORMAT_s_WOULD_LIKE_TO_ADJOURN_THE_GAME_n, parray[p].pname);
  pcn_out_prompt(parray[p].opponent, CODE_INFO,
                 FORMAT_USE_ADJOURN_TO_ADJOURN_THE_GAME_n);
  pcn_out(p, CODE_INFO, FORMAT_REQUEST_FOR_ADJOURNMENT_SENT_n);
 
  return COM_OKN;
}

int com_pteach(int p, struct parameter * param)
{
  int g, p1;
  UNUSED(param);

  g = parray[p].game;
  if ((g < 0) || (g >= garray_top) || (garray[g].gstatus != GSTATUS_ACTIVE)) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }

  if(garray[g].Teach2 == 1) {
    pcn_out(p, CODE_ERROR, FORMAT_SORRY_THIS_IS_ALREADY_A_TEACHING_GAME_);
    return COM_OK;
  }
  p1 = parray[p].opponent;
 
  if (pending_find(parray[p].opponent, p, PEND_TEACH)) {
    player_remove_requests(parray[p].opponent, p, PEND_TEACH);
    player_decline_offers(p, -1, -1);

    pcn_out(p, CODE_INFO, FORMAT_THIS_IS_NOW_A_FREE_UNRATED_TEACHING_GAME_n);
    pcn_out(p1, CODE_INFO, FORMAT_THIS_IS_NOW_A_FREE_UNRATED_TEACHING_GAME_n);
    garray[g].Teach2 = 1;
    garray[g].rated = 0;
    return COM_OK;
  }
 
  player_add_request(p, parray[p].opponent, PEND_TEACH, 0);
  return COM_OK;
}

int com_ladder(int p, struct parameter * param)
{
  int g, p1, p2;
  int size;
  const struct player *pl1, *pl2;
  UNUSED(param);

  g = parray[p].game;
  if ((g < 0) || (g >= garray_top) || (garray[g].gstatus != GSTATUS_ACTIVE)) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }

  if (garray[g].Ladder_Possible == 0) {
    pcn_out(p, CODE_ERROR, FORMAT_SORRY_THIS_CANNOT_BE_A_LADDER_RATED_GAME_);
    return COM_OK;
  }
  p1 = parray[p].opponent;
  
  if (pending_find(parray[p].opponent, p, PEND_LADDER)) {
    player_remove_requests(parray[p].opponent, p, PEND_LADDER);
    player_decline_offers(p, -1, -1);

    size = garray[g].GoGame->width;

    if (size == 9) {
      pl1 = PlayerNamed(Ladder9, parray[garray[g].white].pname);
      pl2 = PlayerNamed(Ladder9, parray[garray[g].black].pname);
    } else {
      pl1 = PlayerNamed(Ladder19, parray[garray[g].white].pname);
      pl2 = PlayerNamed(Ladder19, parray[garray[g].black].pname);
    }

    if (!pl1 || !pl2) {
      pcn_out(p, CODE_ERROR, FORMAT_YOU_MUST_FIRST_JOIN_THE_LADDER_WITH_qJOINq_);
      return COM_OK;
    }

    if (size == 9) { garray[g].Ladder9 = 1;  garray[g].komi = (float) 5.5; }
    if (size == 19) { garray[g].Ladder19 = 1; garray[g].komi = (float) 5.5; }
    pcn_out(p, CODE_INFO, FORMAT_THIS_IS_NOW_A_LADDER_RATED_GAME_n );
    pcn_out(p1, CODE_INFO, FORMAT_THIS_IS_NOW_A_LADDER_RATED_GAME_n);
    pcn_out_prompt(p1, CODE_INFO, FORMAT_KOMI_IS_NOW_SET_TO_fn, garray[g].komi);
    pcn_out(p, CODE_INFO, FORMAT_KOMI_IS_NOW_SET_TO_fn, garray[g].komi);
    for (p2 = 0; p2 < parray_top; p2++) {
      if (!parray[p2].slotstat.online) continue;
      if (!parray[p2].i_lshout) continue;
      pcn_out_prompt(p2, CODE_SHOUT,FORMAT_LADDERd_GAME_MATCH_d_s_d_VS_s_d_n, 
         size, g + 1,
         parray[garray[g].white].pname, pl1->idx + 1,
         parray[garray[g].black].pname, pl2->idx + 1);
    }
    return COM_OK;
  }

  player_add_request(p, parray[p].opponent, PEND_LADDER, 0);
  return COM_OK;
}

int com_komi(int p, struct parameter * param)
{
  int g, p1;
  float newkomi = 0.5;		/* Init to shut up warnings. */
  struct pending * hers, *mine;
  int pendstat=0;

  switch(param[0].type) {
  case TYPE_NULL:
      return COM_OK;
  case TYPE_INT:
    newkomi = (float) param[0].val.integer;
    break;
  case TYPE_FLOAT:
    newkomi = param[0].val.f;
    break;
  }

  g = parray[p].game;
  if ((g < 0) || (g >= garray_top) || (garray[g].gstatus != GSTATUS_ACTIVE)) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }

  if(garray[g].Teach == 1) {
    garray[g].komi = newkomi;
    pcn_out(p, CODE_INFO, FORMAT_SET_THE_KOMI_TO_f, newkomi);
    return COM_OK;
  }

  p1 = parray[p].opponent;
  
  mine = pending_find(p, p1, PEND_KOMI);
  hers = pending_find(p1, p, PEND_KOMI);
  if(mine) pendstat |= 1;
  else {
     mine = player_pending_new(p, p1, PEND_KOMI);
     if(!mine) { return COM_OK; }
     mine->float1 = newkomi;
  }
  if(hers) pendstat |= 2;

  if(hers && hers->float1 == mine->float1) {
    /* komi is agreed upon.  Alter game record */
    garray[g].komi = newkomi;
    pcn_out(p, CODE_INFO, FORMAT_SET_THE_KOMI_TO_f, newkomi);
    pcn_out_prompt(p1, CODE_INFO, FORMAT_KOMI_IS_NOW_SET_TO_fn, newkomi);
    for (p1 = 0; p1 < parray_top; p1++) {
      if (!parray[p1].slotstat.online) continue;
      if (player_is_observe(p1, g)) {
        pcn_out(p1, CODE_INFO, FORMAT_KOMI_SET_TO_f_IN_MATCH_dn, newkomi, g+1);
      }
    }
#ifdef PAIR
    if (paired(g)) {
      garray[garray[g].pairwith].komi = newkomi;
    }
#endif
    player_pending_delete(mine);
    player_pending_delete(hers);
    return COM_OK;
  }
  switch(pendstat) {
  case 0:
    pcn_out(p, CODE_INFO, FORMAT_OFFERING_A_KOMI_OF_f_TO_s, newkomi, parray[p1].pname);
    pcn_out(p1, CODE_INFO, FORMAT_s_OFFERS_A_NEW_KOMI_OF_f_n, parray[p].pname,
            newkomi);
    break;
  case 1:
    pcn_out(p, CODE_INFO, FORMAT_UPDATING_KOMI_OFFER_TO_);
    pcn_out_prompt(p1, CODE_INFO, FORMAT_s_UPDATES_THE_KOMI_OFFER_n, parray[p].pname);
    pcn_out(p1, CODE_INFO, FORMAT_s_OFFERS_A_NEW_KOMI_OF_f_n,newkomi, parray[p].pname);
    break;
  case 2:
  case 3:
    pcn_out(p, CODE_INFO, FORMAT_DECLINING_KOMI_OFFER_FROM_s_AND_OFFERING_NEW_KOMI_n,
                parray[p1].pname);
    pcn_out_prompt(p1, CODE_INFO,FORMAT_s_DECLINES_YOUR_KOMI_OFFER_AND_OFFERS_A_NEW_KOMI_n,
		parray[p].pname);
    pcn_out(p1, CODE_INFO, FORMAT_s_OFFERS_A_NEW_KOMI_OF_f_n, parray[p].pname,newkomi);
    break;
  }
      

  pcn_out_prompt(p1, CODE_INFO, FORMAT_USE_qKOMI_fq_TO_ACCEPT_OR_qDECLINE_sq_TO_RESPOND_n, newkomi, parray[p].pname);
  return COM_OK;
}

int com_status(int p, struct parameter * param)
{
  int g, p1, x, white_player, black_player, wc, bc, until;
  twodstring statstring;

  until = 0;
  if (param[0].type == TYPE_NULL) {
    if (parray[p].game >= 0) {
      g = parray[p].game;
    } else if (parray[p].num_observe > 0) {
      g = parray[p].observe_list[0];
    } else {
      pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NEITHER_PLAYING_NOR_OBSERVING_A_GAME_);
      return COM_OK;
    }
  } else if (param[0].type == TYPE_WORD) {
    stolower(param[0].val.word);
    p1 = player_find_part_login(param[0].val.word);
    if (p1 < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_NO_USER_NAMED_qsq_IS_LOGGED_IN_, param[0].val.word);
      return COM_OK;
    }
    g = parray[p1].game;
  } else {                      /* Must be an integer */
    g = param[0].val.integer - 1;
  }
  if (param[1].type == TYPE_INT) {
     until = param[1].val.integer;
  }
  if ((g < 0) || (g >= garray_top) || (garray[g].gstatus != GSTATUS_ACTIVE)) {
    return COM_NOSUCHGAME;
  }
  if ((garray[g].white != p) && (garray[g].black != p) && garray[g].Private) {
    pcn_out(p, CODE_ERROR, FORMAT_SORRY_THAT_IS_A_PRIVATE_GAME_);
    return COM_OK;
  }

  black_player = garray[g].black;
  white_player = garray[g].white;

  getcaps(garray[g].GoGame, &wc, &bc);
  boardstatus(garray[g].GoGame, statstring);

  pcn_out(p, CODE_STATUS, FORMAT_s_ss_d_d_d_T_f_dn,
		parray[white_player].pname,
		parray[white_player].srank,
		parray[white_player].rated ? "*" : " ",
		wc,
		garray[g].wTime,
   		garray[g].wByoStones,
		garray[g].komi,
                garray[g].GoGame->handicap);
  pcn_out(p, CODE_STATUS, FORMAT_s_ss_d_d_d_T_f_dn,
		parray[black_player].pname,
		parray[black_player].srank,
		parray[black_player].rated ? "*" : " ",
		bc,
		garray[g].bTime,
   		garray[g].bByoStones,
		garray[g].komi,
                garray[g].GoGame->handicap);
  if (!until) until = garray[g].GoGame->height;
  if((until - 1) > garray[g].GoGame->height) return COM_OKN;
  for(x = 0; x < until; x++) {
    pcn_out(p, CODE_STATUS, FORMAT_d_sn,
		x,
		statstring[x]);
  }
  return COM_OKN;
}

int com_undo(int p, struct parameter * param)
{
  int gmove, pw, pb;
  int g, p1, pairg;
  unsigned now;
  char buf[20];
  int num, x;

  if (param[0].type == TYPE_NULL) {
    num = 1;
  }
  else num = param[0].val.integer;

  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_ );
    return COM_OK;
  }
  player_decline_offers(p, -1, -1);
  g = parray[p].game;
  pw = garray[g].white;
  pb = garray[g].black;

  if ((parray[p].side != garray[g].onMove) && 
      (parray[p].protostate != STAT_SCORING)         &&
      ((garray[g].Teach2 == 0) && (garray[g].Teach == 0))) {
    pcn_out(p, CODE_ERROR, FORMAT_IT_IS_NOT_YOUR_MOVE_);
    return COM_OK;
  }
  gmove = movenum(garray[g].GoGame);
#ifdef PAIR
  pairg = garray[g].pairwith;

/* pair1 = (W1,B1), pair2 = (W2,B2)
   movenum % 4      0   1   2   3   ( == movenum & 3)
   player to move   B1  W1  B2  W2
*/

  if ((paired(g)) && (gmove == movenum(garray[pairg].GoGame))
      && ((gmove&3)/2) != (garray[g].pairstate==PAIR2)) {
    pcn_out(p, CODE_ERROR, FORMAT_IT_IS_YOUR_PARTNER_S_MOVE_n);
    return COM_OK;
  }
#endif

  if (parray[p].protostate == STAT_SCORING) {
    parray[pw].protostate = STAT_PLAYING_GO;
    parray[pb].protostate = STAT_PLAYING_GO;
    garray[g].num_pass = 1;
    replay(garray[g].GoGame);
  }
  player_remove_requests(parray[p].opponent, p, PEND_DONE);
  
  for(x = 0; x < num; x++) {
    if (gmove == 0) {  /* At beginning of game. */
      garray[g].onMove = BLACK; 
      return COM_OK; 
    }
    if(((garray[g].Teach == 0) && (garray[g].Teach2 == 0)) && x == 0) x = num;
    listmove(garray[g].GoGame, gmove, buf);
    pcn_out(garray[g].black, CODE_UNDO, FORMAT_s_UNDID_THE_LAST_MOVE_s_n,
              parray[p].pname,
              buf + 1);
    if (garray[g].Teach == 0) 
      pcn_out(garray[g].white, CODE_UNDO, FORMAT_s_UNDID_THE_LAST_MOVE_s_n,
              parray[p].pname,
              buf + 1);
    for (p1 = 0; p1 < parray_top; p1++) {
      if (!parray[p1].slotstat.online) continue;
      if (player_is_observe(p1, g)) {
        pcn_out_prompt(p1, CODE_UNDO, FORMAT_UNDO_IN_GAME_d_s_VS_s_sn,
            g + 1,
            parray[garray[g].white].pname,
            parray[garray[g].black].pname,
            buf + 1);
      }
    }
    back(garray[g].GoGame);
    now = tenth_secs();
    garray[g].lastMoveTime = now;
    garray[g].onMove = BLACK+WHITE - garray[g].onMove;
    send_go_boards(g, 0);
    gmove = movenum(garray[g].GoGame);
    if(gmove == 0) {
      garray[g].GoGame->handicap = 0;
    }
  }
#ifdef PAIR
  if (paired(g) && gmove == movenum(garray[pairg].GoGame)) {
    com_undo(garray[g].onMove == WHITE ? garray[garray[g].pairwith].black
				: garray[garray[g].pairwith].white,
                 param);
      Logit("DUPLICATING undo");
  }
#endif
  return COM_OKN;
}

int com_games(int p, struct parameter * param)
{
  int i;
  int wp, bp;

  int selected = 0;  int count = 0;  int totalcount = 0;
  char *s = NULL;  int slen = 0;
  int mnum = 0;

  if (param[0].type == TYPE_WORD) {
    s = param[0].val.word;
    slen = strlen(s);
    selected = atoi(s);
    if (selected < 0) selected = 0;
  }
  pcn_out(p, CODE_GAMES, FORMAT_WHITE_NAME_RK_BLACK_NAME_RK_MOVE_SIZE_H_KOMI_BY_FR_);
  for (i = 0; i < garray_top; i++) {
    if (garray[i].gstatus != GSTATUS_ACTIVE) continue;
    totalcount++;
    if ((selected) && (selected != i+1))
      continue;  /* not selected game number */
    wp = garray[i].white;
    bp = garray[i].black;
    if ((!selected) && s 
          && strncmp(s, parray[wp].login, slen)
          && strncmp(s, parray[bp].login, slen))
      continue;  /* player names did not match */
    count++;
    mnum = movenum(garray[i].GoGame);
    pcn_out(p, CODE_CR1|CODE_GAMES, FORMAT_d_s_ss_VS_s_ss_d_d_d_f_d_cc_d_,
            i + 1,
	    parray[wp].pname,
            parray[wp].srank,
            parray[wp].rated ? "*" : " ",
	    parray[bp].pname,
            parray[bp].srank,
            parray[bp].rated ? "*" : " ",
	    mnum,
            garray[i].GoGame->width,
            garray[i].GoGame->handicap,
            garray[i].komi,
            (garray[i].Byo) / 60,
	    (garray[i].rated) ? ' ' : ((garray[i].Teach) || 
                                      (garray[i].Teach2))? 'T' : 'F',
/*	    (garray[i].gotype) ? 'I' : '*', */
	    (garray[i].rules == RULES_NET) ? (parray[wp].match_type == GAMETYPE_TNETGO ? '*' : 'I') : 'G',
            game_get_num_ob(i));
  }
  return COM_OK;
}

int com_gomoves(int p, struct parameter * param)
{
  int i, p1;
  int wc, bc;

  int g = 0;  int count = 0;

  char buf[20], outStr[1024];

  if (param[0].type == TYPE_NULL) {
    if (parray[p].game >= 0) {
      g = parray[p].game;
    } else {
      pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NEITHER_PLAYING_NOR_OBSERVING_A_GAME_);
      return COM_OK;
    }
  } else if (param[0].type == TYPE_WORD) {
    stolower(param[0].val.word);
    p1 = player_find_part_login(param[0].val.word);
    if (p1 < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_NO_USER_NAMED_qsq_IS_LOGGED_IN_, 
                   param[0].val.word);
      return COM_OK;
    }
    g = parray[p1].game;
  } else {                      /* Must be an integer */
    g = param[0].val.integer - 1;
  }
  if ((g < 0) || (g >= garray_top) || (garray[g].gstatus != GSTATUS_ACTIVE)) {
    return COM_NOSUCHGAME;
  }
  if ((garray[g].white != p) && (garray[g].black != p) && garray[g].Private) {
    pcn_out(p, CODE_ERROR, FORMAT_SORRY_THAT_IS_A_PRIVATE_GAME_);
    return COM_OK;
  }

  getcaps(garray[g].GoGame, &wc, &bc);
  sprintf(outStr, "Game %d %s: %s (%d %d %d) vs %s (%d %d %d)",
        g + 1,
        "I",
        parray[garray[g].white].pname,
        bc,
        garray[g].wTime,
        garray[g].wByoStones,
        parray[garray[g].black].pname,
        wc,
        garray[g].bTime,
        garray[g].bByoStones);
  count = movenum(garray[g].GoGame);
  if (count == 0) {
    pcn_out_prompt(p, CODE_MOVE, FORMAT_sn, outStr);  
    return COM_OKN;
  }
  pcn_out(p, CODE_MOVE, FORMAT_sn, outStr);  
  for(i = 0; i < count; i++) {
    listmove(garray[g].GoGame, i + 1, buf);
    pcn_out(p, CODE_MOVE, FORMAT_d_c_sn, i, buf[0], buf + 1);
  }
  return COM_OKN;
}

static int do_observe(int p, int obgame)
{
  int wc, bc, g;
  int gmove = 0;
  char buf[200];

  if ((garray[obgame].Private) && (parray[p].adminLevel < ADMIN_ADMIN)) {
    pcn_out(p, CODE_INFO, FORMAT_SORRY_GAME_d_IS_A_PRIVATE_GAME_, 
                obgame + 1);
    return COM_OK;
  }
  if ((garray[obgame].white == p) || (garray[obgame].black == p)) {
    pcn_out(p, CODE_INFO, FORMAT_YOU_CANNOT_OBSERVE_A_GAME_THAT_YOU_ARE_PLAYING_);
    return COM_OK;
  }
  if (player_is_observe(p, obgame)) {
    pcn_out(p, CODE_INFO, FORMAT_REMOVING_GAME_d_FROM_OBSERVATION_LIST_n, 
                obgame + 1);
    player_remove_observe(p, obgame);
  } else {
    if (!player_add_observe(p, obgame)) {
      pcn_out(p, CODE_INFO, FORMAT_ADDING_GAME_TO_OBSERVATION_LIST_n, 
                  obgame + 1);
      g = obgame;
      sprintf(buf, " %s started observation.", parray[p].pname);
      add_kib(&garray[g], movenum(garray[g].GoGame), buf);
      if (parray[p].client) {
        getcaps(garray[g].GoGame, &wc, &bc);
        gmove = movenum(garray[obgame].GoGame);
        pcn_out(p, CODE_MOVE, FORMAT_GAME_d_s_s_d_d_d_VS_s_d_d_d_n,
                    g + 1,
                    "I",
                    parray[garray[g].white].pname,
                    bc,
                    garray[g].wTime,
                    garray[g].wByoStones,
                    parray[garray[g].black].pname,
                    wc,
                    garray[g].bTime,
                    garray[g].bByoStones);

        if (gmove) {
          listmove(garray[g].GoGame, gmove, buf);
          pcn_out(p, CODE_MOVE, FORMAT_d_c_s, 
                      gmove - 1, buf[0], buf + 1);
        }
      }
      else if (parray[p].i_verbose) {
        pcommand (p,"refresh %d", g + 1);
      }
      parray[p].protostate = STAT_OBSERVING;
    } else {
      pcn_out(p, CODE_INFO, FORMAT_YOU_ARE_ALREADY_OBSERVING_THE_MAXIMUM_NUMBER_OF_GAMES_n);
    }
  }
  return COM_OKN;
}

int com_observe(int p, struct parameter * param)
{
  int i;
  int p1 = -1, obgame;

  if (param[0].type == TYPE_NULL) {
    for (i = 0; i < parray[p].num_observe; i++) {
      pcn_out(p, CODE_INFO, FORMAT_REMOVING_GAME_d_FROM_OBSERVATION_LIST_, 
                parray[p].observe_list[i] + 1);
    }
    parray[p].num_observe = 0;
    parray[p].protostate = STAT_WAITING;
    return COM_OK;
  } else if (param[0].type == TYPE_WORD) {
    stolower(param[0].val.word);
    p1 = player_find_part_login(param[0].val.word);
    if (p1 < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_NO_USER_NAMED_qsq_IS_LOGGED_IN_, 
              param[0].val.word);
      return COM_OK;
    }
    obgame = parray[p1].game;
  } else {			/* Must be an integer */
    obgame = param[0].val.integer - 1;
  }
  if ((obgame < 0) || 
     (obgame >= garray_top) || 
     (garray[obgame].gstatus != GSTATUS_ACTIVE)) {
    return COM_NOSUCHGAME;
  }
  if ((garray[obgame].white == p) || (garray[obgame].black == p)) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_CANNOT_OBSERVE_A_GAME_THAT_YOU_ARE_PLAYING_);
    return COM_OK;
  }
  do_observe(p, obgame);
  return COM_OK;
}

#ifdef PAIR 
int com_pair(int p, struct parameter * param)
{
  int gmove, p2;
  int ourgame, theirgame;
  int btim, ttim;

  theirgame = param[0].val.integer - 1;
  ourgame = parray[p].game;

  if ((ourgame < 0) || 
      (ourgame >= garray_top) || 
      (garray[ourgame].gstatus != GSTATUS_ACTIVE)) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  if (garray[ourgame].pairstate != NOTPAIRED) {
    pcn_out(p, CODE_ERROR, FORMAT_YOUR_GAME_IS_PAIRED_ALREADY_);
    return COM_OK;
  }
  if ((theirgame < 0) || 
      (theirgame >= garray_top) || 
      (garray[theirgame].gstatus != GSTATUS_ACTIVE)) {
    pcn_out(p, CODE_ERROR, FORMAT_NO_SUCH_GAME_);
    return COM_OK;
  }

  p2 = garray[theirgame].white;

  if (p != garray[ourgame].white) {
    pcn_out(p, CODE_ERROR, FORMAT_ONLY_THE_WHITE_PLAYER_MAY_REQUEST_A_PAIR_MATCH);
    return COM_OK;
  }
  
  if((p == garray[ourgame].black)   ||
     (p == garray[theirgame].white) ||
     (p == garray[theirgame].black)) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_ONE_OF_THE_OTHER_PLAYERS_CANNOT_PAIR_);
    return COM_OK;
  }

  gmove = movenum(garray[ourgame].GoGame) - movenum(garray[theirgame].GoGame);
  if (gmove != 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_MUST_PAIR_BEFORE_YOUR_FIRST_MOVE_OR_AT_EQUAL_MOVES_IN_A_MATCH);
    return COM_OK;
  }

  if (garray[theirgame].GoGame->width != garray[ourgame].GoGame->width) {
    pcn_out(p, CODE_ERROR, FORMAT_THE_BOARDS_ARE_NOT_THE_SAME_SIZE_);
    return COM_OK;
  }
   
  /* Test if other side already requested pair with us */

  if (pending_find(p2, p, PEND_PAIR)) {
    player_remove_requests(p2, p, PEND_PAIR);
    player_decline_offers(p, -1, -1);
    
    /* Ok, in here we have a match for the pairs */

    /* First, get all the times the same */

    ttim = (garray[theirgame].wTime > garray[ourgame].wTime) ? 
            garray[theirgame].wTime : garray[ourgame].wTime;
    btim = (garray[theirgame].Byo > garray[ourgame].Byo)     ? 
            garray[theirgame].Byo : garray[ourgame].Byo;

    garray[theirgame].wTime = ttim;
    garray[ourgame].wTime   = ttim;
    garray[theirgame].bTime = ttim;
    garray[ourgame].bTime   = ttim;
    garray[theirgame].Byo   = btim;
    garray[ourgame].Byo     = btim;

    /* Match the games up in a pair fashion */

    garray[theirgame].pairstate = PAIR1;
    garray[ourgame].pairstate = PAIR2;
    garray[theirgame].pairwith = ourgame;
    garray[ourgame].pairwith = theirgame;

    /* Remove ladder problems (for now) */

    garray[theirgame].Ladder_Possible = 0; 
    garray[theirgame].Ladder9 = 0;
    garray[theirgame].Ladder19 = 0;
    garray[ourgame].Ladder_Possible = 0; 
    garray[ourgame].Ladder9 = 0;
    garray[ourgame].Ladder19 = 0;

    /* Make the game unrated by default */
    garray[theirgame].rated = 0;
    garray[ourgame].rated = 0;

    /* Tell the players */

    pcn_out_prompt(garray[theirgame].white, CODE_INFO, FORMAT_GAMES_d_AND_d_ARE_NOW_PAIRED_n,
      ourgame+1,theirgame+1);
    pcn_out_prompt(garray[theirgame].black, CODE_INFO, FORMAT_GAMES_d_AND_d_ARE_NOW_PAIRED_n,
      ourgame+1,theirgame+1);
    pcn_out(garray[ourgame].white, CODE_INFO, FORMAT_GAMES_d_AND_d_ARE_NOW_PAIRED_n,
      ourgame+1,theirgame+1);
    pcn_out_prompt(garray[ourgame].black, CODE_INFO, FORMAT_GAMES_d_AND_d_ARE_NOW_PAIRED_n,
      ourgame+1,theirgame+1);

    return COM_OK;
  }

    /* Send pair request to other game White player */

  player_add_request(p, p2, PEND_PAIR, ourgame);
  pcn_out(p2, CODE_INFO, FORMAT_s_AND_s_WOULD_LIKE_TO_PAIR_THEIR_GAME_WITH_YOURS_n,
          parray[p].pname, parray[parray[p].opponent].pname);
  pcn_out_prompt(p2, CODE_INFO, FORMAT_TO_DO_SO_PLEASE_TYPE_qPAIR_dqn,
                 ourgame + 1);
  return COM_OK;
}
#endif /* PAIR */

int com_ginfo(int p, struct parameter * param)
{
  int obgame;
  int g;

  if (param[0].type == TYPE_NULL) {
    if (parray[p].game >= 0) obgame = parray[p].game;
    else if (parray[p].num_observe > 0) obgame = parray[p].observe_list[0];
    else return COM_BADPARAMETERS;
  } else {			/* Must be an integer */
    obgame = param[0].val.integer - 1;
  }
  if (obgame >= garray_top)
    return COM_OK;
  if (obgame <= 0)
    obgame = 0;
  g = obgame;
  pcn_out(p, CODE_INFO, FORMAT_GAME_INFO_sn, 
            IFNULL(garray[g].gtitle, "No info available") );
  pcn_out(p, CODE_INFO, FORMAT_EVENT_INFO_sn,
            IFNULL(garray[g].gevent,"none") );
  return COM_OK;
}

int com_allob(int p, struct parameter * param)
{
  int obgame;
  int p1;
  int g;
  int count;

  count = 0;
  if (param[0].type == TYPE_NULL) {
    if (parray[p].game >= 0) obgame = parray[p].game;
    else if (parray[p].num_observe > 0) obgame = parray[p].observe_list[0];
    else {
      pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_OR_OBSERVING_A_GAME);
      return COM_OK;
    }
  } else if (param[0].type == TYPE_WORD) {
    stolower(param[0].val.word);
    p1 = player_find_part_login(param[0].val.word);
    if (p1 < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_NO_USER_NAMED_qsq_IS_LOGGED_IN_, 
                  param[0].val.word);
      return COM_OK;
    }
    obgame = parray[p1].game;
  } else {			/* Must be an integer */
    obgame = param[0].val.integer - 1;
  }
  if (obgame >= garray_top) return COM_OK;
  if(garray[obgame].gstatus != GSTATUS_ACTIVE) return COM_OK;
  if (obgame <= 0) obgame = 0;
  g = obgame;
  
  pcn_out(p, CODE_INFO, FORMAT_OBSERVING_GAME_d_s_VS_s_,
                g + 1,
	        parray[garray[g].white].pname,
	        parray[garray[g].black].pname);
  for (p1 = 0; p1 < parray_top; p1++) {
    if (!parray[p1].slotstat.online) continue;
    if (!player_is_observe(p1, g)) continue;
    if ((count % 3) == 0) pcn_out(p, CODE_CR1|CODE_INFO, FORMAT_empty); 
    pprintf(p, "%15s %3.3s%s ", 
                parray[p1].pname, 
                parray[p1].srank,
                parray[p1].rated ? "*" : " ");
    count++;
  }
  if ((count % 3) != 0) 
    pcn_out(p, CODE_CR1|CODE_INFO, FORMAT_FOUND_d_OBSERVERS_, count);
  else
    pcn_out(p, CODE_INFO, FORMAT_FOUND_d_OBSERVERS_, count);
  return COM_OK;
}

int com_moves(int p, struct parameter * param)
{
  int g;
  int p1;

  if (param[0].type == TYPE_NULL) {
    if (parray[p].game >= 0) {
      g = parray[p].game;
    } else if (parray[p].num_observe > 0) {
      g = parray[p].observe_list[0];
    } else {
      pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NEITHER_PLAYING_NOR_OBSERVING_A_GAME_);
      return COM_OK;
    }
  } else if (param[0].type == TYPE_WORD) {
    stolower(param[0].val.word);
    p1 = player_find_part_login(param[0].val.word);
    if (p1 < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_NO_USER_NAMED_qsq_IS_LOGGED_IN_, 
               param[0].val.word);
      return COM_OK;
    }
    g = parray[p1].game;
  } else {			/* Must be an integer */
    g = param[0].val.integer - 1;
  }
  if ((g < 0) || (g >= garray_top) || 
      (garray[g].gstatus != GSTATUS_ACTIVE) || (garray[g].gotype < GAMETYPE_GO)) {
    return COM_NOSUCHGAME;
  }
  if ((garray[g].white != p) && (garray[g].black != p) && garray[g].Private) {
    pcn_out(p, CODE_ERROR, FORMAT_SORRY_THAT_IS_A_PRIVATE_GAME_);
    return COM_OK;
  }
  return COM_OK;
}

int com_touch(int p, struct parameter * param)
{
  int wp, bp;
  const char *bname, *wname;
  
  bname = file_bplayer(param[0].val.string);
  wname = file_wplayer(param[0].val.string);
  wp = player_fetch(wname);
  if(wp < 0) return COM_OK;
  bp = player_fetch(bname);
  if(bp < 0) {
    player_forget(wp);
    return COM_OK;
    }
  
  if (wp != p && bp != p) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_CANNOT_TOUCH_SOMEONE_ELSES_STORED_GAME);
    player_forget(wp);
    player_forget(bp);
    return COM_OK;
  }

  xytouch(FILENAME_GAMES_cs_s, bname, wname);
  xytouch(FILENAME_GAMES_cs_s, wname, bname);
  pcn_out(p, CODE_INFO, FORMAT_THE_GAME_s_s_HAS_BEEN_TOUCHED_, wname, bname);
  player_forget(wp);
  player_forget(bp);
  return COM_OK;
}

#define LOAD_TIME_WARNING 30

int com_load(int p, struct parameter * param)
{
  int wp, bp, p1, p3;
  int g;
  struct stat statbuf;
  const struct player *LadderPlayer;
  int bpos = -1, wpos = -1;
  struct pending *ptr;
  const char *bname, *wname;

  bname = file_bplayer(param[0].val.string);
  wname = file_wplayer(param[0].val.string);

  bp = player_find_part_login(bname);
  wp = player_find_part_login(wname);

  if (p == bp) p1 = wp;
  else if (p == wp) p1 = bp;
  else {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_CANNOT_LOAD_SOMEONE_ELSE_S_GAME_TRY_qLOOKq_TO_SEE_THE_GAME_);
    return COM_OK;
  }

  if (wp < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_s_IS_NOT_LOGGED_IN_, 
               file_wplayer(param[0].val.string));
    return COM_OK;
  }
  if (parray[wp].game >= 0) {
    pcn_out(p, CODE_ERROR, FORMAT_s_IS_CURRENTLY_PLAYING_A_GAME_, 
                parray[wp].pname);
    return COM_OK;
  }
  if (bp < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_NO_PLAYER_NAMED_s_IS_LOGGED_IN_, 
            file_bplayer(param[0].val.string));
    return COM_OK;
  }
  if (parray[bp].game >= 0) {
    pcn_out(p, CODE_ERROR, FORMAT_s_IS_CURRENTLY_PLAYING_A_GAME_, 
                    parray[bp].pname);
    return COM_OK;
  }

  if (!parray[p1].open) {
      pcn_out(p, CODE_ERROR, FORMAT_YOUR_OPPONENT_IS_NOT_OPEN_FOR_MATCHES_n);
      pcn_out(p, CODE_ERROR, FORMAT_GAME_FAILED_TO_LOAD_);
      return COM_OK;
  }
  if (xystat(&statbuf, FILENAME_GAMES_cs_s,parray[wp].login,parray[bp].login)) {
    Logit("Failed stat %s", filename() );
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_STORED_GAME_s_VS_s, parray[wp].pname,parray[bp].pname);
    return COM_OK;
  }
  Logit("Successfull stat %s", filename() );
  if(statbuf.st_size == 0) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_A_SERIOUS_PROBLEM_WITH_YOUR_GAME_RECORD_THIS_IS_SOMETIMESn);
    pcn_out(p, CODE_ERROR, FORMAT_CAUSED_BY_AN_NNGS_CRASH_DURING_YOUR_GAME_);
    pcn_out(p, CODE_ERROR, FORMAT_WE_APOLOGIZE_BUT_THE_GAME_IS_LOST_);
    return COM_OK;
  }
  g = game_new(GAMETYPE_GO,19);
  if (game_read(&garray[g], wp, bp) < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_STORED_GAME_s_VS_s, parray[wp].pname,parray[bp].pname);
    return COM_OK;
  }
  while((ptr=pending_find(bp, -1, PEND_MATCH))) {
    p3=ptr->whoto;
    if (!parray[p3].slotstat.online) continue;
    player_pending_delete(ptr);
    if (p3 != wp) {
      pcn_out_prompt(p3, CODE_SHOUT, FORMAT_s_IS_CONTINUING_A_GAME_WITH_sn,
		     parray[wp].pname, parray[bp].pname);
    }
  }
  while((ptr=pending_find(wp, -1, PEND_MATCH))) {
    p3=ptr->whoto;
    if (!parray[p3].slotstat.online) continue;
    player_pending_delete(ptr);
    if (p3 != bp) {
        pcn_out_prompt(p3, CODE_SHOUT, FORMAT_s_IS_CONTINUING_A_GAME_WITH_sn,
		     parray[wp].pname, parray[bp].pname);
    }
  }
  garray[g].gstatus = GSTATUS_ACTIVE;
  garray[g].startTime = tenth_secs();
  garray[g].lastMoveTime = garray[g].startTime;
  garray[g].lastDecTime = garray[g].startTime;

  pcn_out(p1, CODE_INFO, FORMAT_s_HAS_RESTORED_YOUR_OLD_GAME_n, 
          parray[p].pname);
  pcn_out(p1, CODE_INFO, FORMAT_s_HAS_RESTARTED_YOUR_GAME_n, 
          parray[p].pname);
  

  for (p3 = 0; p3 < parray_top; p3++) {
    if (!parray[p3].slotstat.online) continue;
    if (p3 == wp || p3 == bp) continue;
    if (!parray[p3].i_game) continue;
    pcn_out_prompt(p3, CODE_SHOUT, FORMAT_GAME_d_s_VS_s_MOVE_d_n, 
                   g+1, parray[wp].pname, parray[bp].pname, 
                   movenum(garray[g].GoGame));
  } 
  parray[wp].game = g;
  parray[wp].opponent = bp;
  parray[wp].side = WHITE;
  parray[bp].game = g;
  parray[bp].opponent = wp;
  parray[bp].side = BLACK;
  parray[bp].protostate = STAT_PLAYING_GO;
  parray[wp].protostate = STAT_PLAYING_GO;
  if ((garray[g].Ladder9) || (garray[g].Ladder19)) {
    if (garray[g].size == 19) {
      if ((LadderPlayer=PlayerNamed(Ladder19,parray[bp].pname)) != NULL) {
        bpos = LadderPlayer->idx;
      }
      else bpos = -1;
      if ((LadderPlayer=PlayerNamed(Ladder19,parray[wp].pname)) != NULL) {
        wpos = LadderPlayer->idx;
      }
      else wpos = -1;
    }
    else if (garray[g].size == 9) {
      if ((LadderPlayer=PlayerNamed(Ladder9,parray[bp].pname)) != NULL) {
        bpos = LadderPlayer->idx;
      }
      else bpos = -1;
      if ((LadderPlayer=PlayerNamed(Ladder9,parray[wp].pname)) != NULL) {
        wpos = LadderPlayer->idx;
      }
      else wpos = -1;
    }
    if ((wpos < 0) || (bpos < 0) || (wpos > bpos)) {
      garray[g].Ladder9 = 0;
      garray[g].Ladder19 = 0;
    }
  }
  send_go_boards(g, 0);

  /* [PEM]: Added some time checks and automatic pausing. */
  if (garray[g].wTime < LOAD_TIME_WARNING &&
      (garray[g].Byo == 0 || garray[g].wInByo))
  {
    if (garray[g].bTime < LOAD_TIME_WARNING &&
	(garray[g].Byo == 0 || garray[g].bInByo))
    {				/* both */
      pcn_out(wp, CODE_INFO, FORMAT_BOTH_PLAYERS_HAVE_LESS_THAN_d_SECONDS_LEFT_n,
	      LOAD_TIME_WARNING);
      pcn_out(bp, CODE_INFO, FORMAT_BOTH_PLAYERS_HAVE_LESS_THAN_d_SECONDS_LEFT_n,
	      LOAD_TIME_WARNING);
    }
    else
    {				/* white */
      pcn_out(wp, CODE_INFO, FORMAT_YOU_HAVE_ONLY_d_SECONDS_LEFT_n,
	      garray[g].wTime);
      pcn_out(bp, CODE_INFO, FORMAT_WHITE_HAS_ONLY_d_SECONDS_LEFT_n,
	      garray[g].wTime);
    }
    garray[g].clockStopped = 1;
  }
  else if (garray[g].bTime < LOAD_TIME_WARNING &&
	   (garray[g].Byo == 0 || garray[g].bInByo))
  {				/* black */
    pcn_out(wp, CODE_INFO, FORMAT_BLACK_HAS_ONLY_d_SECONDS_LEFT_n,
	    garray[g].bTime);
    pcn_out(bp, CODE_INFO, FORMAT_YOU_HAVE_ONLY_d_SECONDS_LEFT_n,
	    garray[g].bTime);
    garray[g].clockStopped = 1;
  }
  if (garray[g].clockStopped)
  {
    pcn_out(wp, CODE_INFO, FORMAT_GAME_CLOCK_PAUSED_USE_qUNPAUSEq_TO_RESUME_n);
    pcn_out(bp, CODE_INFO, FORMAT_GAME_CLOCK_PAUSED_USE_qUNPAUSEq_TO_RESUME_n);
  }

  return COM_OK;
}

int com_stored(int p, struct parameter * param)
{
  DIR *dirp;
  struct dirent *dp;
  int p1;
  int count;

  if (param[0].type == TYPE_WORD) {
    p1 = player_fetch(param[0].val.word);
    if(p1 < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_PLAYER_BY_THAT_NAME_n);
      pcn_out(p, CODE_INFO, FORMAT_FOUND_0_STORED_GAMES_);
      return COM_OK;
      }
  } else {
    p1 = p;
    player_fix(p1);
  }

  pcn_out(p, CODE_INFO, FORMAT_STORED_GAMES_FOR_s_, parray[p1].pname);

  dirp = xyopendir(FILENAME_GAMES_c,parray[p1].login);
  if (!dirp) {
    pcn_out(p, CODE_CR1|CODE_INFO, FORMAT_FOUND_0_STORED_GAMES_);

    player_forget(p1);
    return COM_OK;
  }
  for (count=0; (dp = readdir(dirp)); ) {
    if (!file_has_pname(dp->d_name, parray[p1].login)) continue;
    if (count % 3 == 0) {
      pcn_out(p, CODE_CR1|CODE_STORED, FORMAT_empty);
    }
    pprintf(p, " %25s", dp->d_name);
    count++;
  }

  closedir(dirp);
  if (count % 3) pprintf(p, "\n");
  pcn_out(p, CODE_INFO, FORMAT_FOUND_d_STORED_GAMES_, count);

  player_forget(p1);
  return COM_OK;
}


int com_sgf(int p, struct parameter * param)
{
  DIR *dirp;
  struct dirent *dp;
  char pname[sizeof parray[p].pname];
  int count;

  if (param[0].type == TYPE_WORD) {
    do_copy(pname, param[0].val.word, sizeof pname);
  } else {
    do_copy(pname, parray[p].pname, sizeof pname);
  }
    /* Always Lowercase name */
  stolower(pname);

  count = 0;
  /* FIXME(AvK): something seems wrong here : no /%c/ in path */
  /* sprintf(dname, "%s/%s", stats_dir, STATS_CGAMES); */
  dirp = xyopendir(FILENAME_CGAMES);
  if (!dirp) {
    pcn_out(p, CODE_ERROR, FORMAT_PLAYER_s_HAS_NO_SGF_GAMES_, pname);
    return COM_OK;
  }
  pcn_out(p, CODE_INFO,FORMAT_COMPLETED_GAMES_FOR_s_n, pname);
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    if ((strstr(dp->d_name, pname)) == NULL) continue;
    if (count % 2 == 0) pcn_out(p, CODE_INFO, FORMAT_empty);
    pprintf(p, "%30s", dp->d_name);
    if (count % 2) pprintf(p, "\n");
    count++;
  }

  closedir(dirp);
  if (count)  {
    if (count % 2) pprintf(p, "\n");
    pcn_out(p, CODE_INFO,FORMAT_FOUND_d_COMPLETED_GAMES_, count);
  }

  return COM_OK;
}

int com_history(int p, struct parameter * param)
{
  int p1 ;
  char fname[MAX_FILENAME_SIZE];

  if (param[0].type == TYPE_WORD) {
    p1 = player_fetch(param[0].val.word);
    if (p1 < 0) {
      return COM_OK;
    }
  }
  else player_fix(p1=p);
  if (p1 != p) {
    if(!parray[p].slotstat.registered) {
      pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_PLAYER_BY_THAT_NAME_);
      player_forget(p1);
      return COM_OK;
    }
    sprintf(fname, "%s/player_data/%c/%s.%s", stats_dir, 
                    parray[p1].login[0], parray[p1].login, STATS_GAMES);
  } else {
    sprintf(fname, "%s/%s", stats_dir, STATS_GAMES);
  }
  pgames(p, fname);
  player_forget(p1);
  return COM_OKN;
}

int com_rhistory(int p, struct parameter * param)
{
  int p1 = p;
  char fname[MAX_FILENAME_SIZE];

  if(parray[p].slotstat.registered == 0) return COM_OK;
  if (param[0].type == TYPE_WORD) {
    p1 = player_fetch(param[0].val.word);
    if (p1 < 0) return COM_OK;
  } else {
    player_fix(p1=p);
  }
  if(parray[p1].slotstat.registered) {
    sprintf(fname, "%s/player_data/%c/%s.%s", stats_dir,
                    parray[p1].login[0], parray[p1].login, STATS_RGAMES);
    pgames(p, fname);
    }
  player_forget(p1);
  return COM_OKN;
}


int com_time(int p, struct parameter * param)
{
  int p1, g;

  switch (param[0].type) {
  case TYPE_NULL:
    if (parray[p].game < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
      return COM_OK;
    }
    g = parray[p].game;
    break;
  case TYPE_WORD:
    stolower(param[0].val.word);
    p1 = player_find_part_login(param[0].val.word);
    if (p1 < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_NO_USER_NAMED_qsq_IS_LOGGED_IN_, 
                     param[0].val.word);
      return COM_OK;
    }
    g = parray[p1].game;
    break;
  case TYPE_INT:
    g = param[0].val.integer - 1;
    break;
  default:
    return COM_BADPARAMETERS;
  }
  if ((g < 0) || (g >= garray_top) || (garray[g].gstatus != GSTATUS_ACTIVE)) {
    return COM_NOSUCHGAME;
  }
  game_update_time(g);

  pcn_out(p, CODE_TIME, FORMAT_GAME_dn, g + 1);
  pcn_out(p, CODE_TIME, FORMAT_WHITE_s_d_dn,
	  parray[garray[g].white].pname,
	  garray[g].wTime / 60,
	  (garray[g].wTime % 60));
  pcn_out(p, CODE_TIME, FORMAT_BLACK_s_d_d,
	  parray[garray[g].black].pname,
	  garray[g].bTime / 60,
	  (garray[g].bTime % 60 ));
  return COM_OK;
}

/*
** [PEM]: Changed this to a non-toggling command. See com_unfree() below.
*/

int com_free(int p, struct parameter * param)
{
  int gmove;
  int g;
  UNUSED(param);

  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  g = parray[p].game;

  gmove = movenum(garray[g].GoGame);
  if (gmove > 1) {
    pcn_out(p, CODE_ERROR, FORMAT_ONLY_VALID_AS_YOUR_FIRST_MOVE_);
    return COM_OK;
  }
  if (garray[g].rated == 0)
    pcn_out(p, CODE_INFO, FORMAT_GAME_IS_ALREADY_FREE_USE_qUNFREEq_TO_CHANGE_THIS_n);
  else
  {
    garray[g].rated = 0;
    pcn_out(p, CODE_INFO, FORMAT_GAME_WILL_NOT_COUNT_TOWARD_RATINGS_n);
    pcn_out(parray[p].opponent, CODE_INFO, FORMAT_GAME_WILL_NOT_COUNT_TOWARD_RATINGS_n);
  }
  return COM_OK;
}
 
/*
** [PEM]: Added this after several incidents where confusion about a game's
**        status arised.
*/
int com_unfree(int p, struct parameter * param)
{
  int gmove;
  int g;
  UNUSED(param);

  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  g = parray[p].game;

  gmove = movenum(garray[g].GoGame);
  if (gmove > 1) {
    pcn_out(p, CODE_ERROR, FORMAT_ONLY_VALID_AS_YOUR_FIRST_MOVE_);
    return COM_OK;
  }
  if (garray[g].rated == 1)
    pcn_out(p, CODE_INFO, FORMAT_GAME_ALREADY_COUNTS_TOWARD_RATINGS_n);
  else
  {
    garray[g].rated = 1;
    pcn_out(p, CODE_INFO, FORMAT_GAME_WILL_COUNT_TOWARD_RATINGS_n);
    pcn_out(parray[p].opponent, CODE_INFO, FORMAT_GAME_WILL_COUNT_TOWARD_RATINGS_n);
  }
  return COM_OK;
}

#ifdef NOUSED
int com_nocaps(int p, struct parameter * param)
{
  int gmove, nc;
  int g, nocap;

  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  g = parray[p].game;

  gmove = movenum(garray[g].GoGame);
  if (gmove > 1) {
    pcn_out(p, CODE_ERROR, FORMAT_ONLY_VALID_AS_YOUR_FIRST_MOVE);
    return COM_OK;
  }
  nc = garray[g].nocaps = !garray[g].nocaps;
 

  if (handicap < 1) {
    pcn_out(p, CODE_ERROR, FORMAT_USE_HANDICAP_1_9_PLEASE_ );
    return COM_OK;
  }
  
  test = sethcap(garray[g].GoGame, handicap);
  gmove = movenum(garray[g].GoGame);

  garray[g].onMove = WHITE;
  send_go_boards(g, 0);
#ifdef PAIR
  if (paired(g) && !movenum(garray[garray[g].pairwith].GoGame)) {
    com_handicap(garray[garray[g].pairwith].black, param);
    Logit("DUPLICATING handicap");
  }
#endif
  /* [PEM]: Point out that we change komi when setting handicap. */
  if (garray[g].komi != 0.5) {
    int p1 = parray[p].opponent;

    garray[g].komi = 0.5;
    pcn_out(p, CODE_INFO, FORMAT_KOMI_IS_NOW_SET_TO_fn, garray[g].komi);
    pcn_out(p1, CODE_INFO, FORMAT_KOMI_IS_NOW_SET_TO_fn, garray[g].komi);
    for (p1 = 0; p1 < parray_top; p1++) {
      if (!parray[p1].slotstat.online) continue;
      if (player_is_observe(p1, g))
	pcn_out(p1, CODE_INFO, FORMAT_KOMI_SET_TO_f_IN_MATCH_dn, garray[g].komi, g+1);
    }
  }
  return COM_OK;
}
#endif /* NOUSED */


int com_handicap(int p, struct parameter * param)
{
  int gmove;
  int test, g, handicap;

  handicap = param[0].val.integer;

  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  g = parray[p].game;

  gmove = movenum(garray[g].GoGame);
  if (gmove > 0) return COM_OK;
  
  if (handicap > 9) {
    pcn_out(p, CODE_ERROR, FORMAT_USE_HANDICAP_9_THEN_ASK_WHITE_TO_PASS_PLEASE_);
    return COM_OK;
  }

  if (handicap < 1) {
    pcn_out(p, CODE_ERROR, FORMAT_USE_HANDICAP_1_9_PLEASE_ );
    return COM_OK;
  }
  
  test = sethcap(garray[g].GoGame, handicap);
  gmove = movenum(garray[g].GoGame);

  garray[g].onMove = WHITE;
  send_go_boards(g, 0);
#ifdef PAIR
  if (paired(g) && !movenum(garray[garray[g].pairwith].GoGame)) {
    com_handicap(garray[garray[g].pairwith].black, param);
    Logit("DUPLICATING handicap");
  }
#endif
  /* [PEM]: Point out that we change komi when setting handicap. */
  if (garray[g].komi != 0.5)
  {
    int p1 = parray[p].opponent;

    garray[g].komi = 0.5;
    pcn_out(p, CODE_INFO, FORMAT_KOMI_IS_NOW_SET_TO_fn, garray[g].komi);
    pcn_out(p1, CODE_INFO, FORMAT_KOMI_IS_NOW_SET_TO_fn, garray[g].komi);
    for (p1 = 0; p1 < parray_top; p1++)
    {
      if (!parray[p1].slotstat.online) continue;
      if (player_is_observe(p1, g))
	pcn_out(p1, CODE_INFO, FORMAT_KOMI_SET_TO_f_IN_MATCH_dn, garray[g].komi, g+1);
    }
  }
  return COM_OK;
}


int com_save(int p, struct parameter * param)
{
  UNUSED(param);
  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  
  game_save(parray[p].game);
  pcn_out(p, CODE_INFO, FORMAT_GAME_SAVED_);

  return COM_OK;
}


int com_moretime(int p, struct parameter * param)
{
  int g, increment;

  increment = param[0].val.integer;
  if (increment <= 0) {
    pcn_out(p, CODE_ERROR, FORMAT_ADDTIME_REQUIRES_AN_INTEGER_VALUE_GREATER_THAN_ZERO_);
    return COM_OK;
  }
  if (parray[p].game < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_ARE_NOT_PLAYING_A_GAME_);
    return COM_OK;
  }
  g = parray[p].game;
  if (increment > 60000) {
    pcn_out(p, CODE_ERROR, FORMAT_ADDTIME_HAS_A_MAXIMUM_LIMIT_OF_60000_MINUTES_);
    increment = 60000;
  }
  if (garray[g].white == p) {
    garray[g].bTime += increment * 60;
#ifdef PAIR
    if(paired(g)) {
      garray[garray[g].pairwith].bTime += increment * 60;
    }
#endif
  }
  if (garray[g].black == p) {
    garray[g].wTime += increment * 60;
#ifdef PAIR
    if(paired(g)) {
      garray[garray[g].pairwith].wTime += increment * 60;
    }
#endif
  }
  pcn_out(p, CODE_INFO, FORMAT_d_MINUTES_WERE_ADDED_TO_YOUR_OPPONENTS_CLOCK,
	    increment);
#ifdef PAIR
  if(paired(g)) {
    if(p == garray[g].black) {
      pcn_out_prompt(garray[garray[g].pairwith].black, CODE_INFO, 
                     FORMAT_d_MINUTES_WERE_ADDED_TO_YOUR_OPPONENTS_CLOCKn,
                     increment);
    } else {
      pcn_out_prompt(garray[garray[g].pairwith].white, CODE_INFO, 
                     FORMAT_d_MINUTES_WERE_ADDED_TO_YOUR_OPPONENTS_CLOCKn,
                     increment);
    }
  }
#endif /* PAIR */

  pcn_out_prompt(parray[p].opponent, CODE_INFO,
	   FORMAT_YOUR_OPPONENT_HAS_ADDED_d_MINUTES_TO_YOUR_CLOCK_n,
	   increment);

#ifdef PAIR
  if(paired(g)) {
    if(p == garray[g].black) {
      pcn_out_prompt(garray[garray[g].pairwith].white, CODE_INFO, 
                     FORMAT_YOUR_OPPONENT_HAS_ADDED_d_MINUTES_TO_YOUR_CLOCK_n,
                     increment);
    } else {
      pcn_out_prompt(garray[garray[g].pairwith].black, CODE_INFO, 
                     FORMAT_YOUR_OPPONENT_HAS_ADDED_d_MINUTES_TO_YOUR_CLOCK_n,
                     increment);
    }
  }
#endif /* PAIR */

  return COM_OK;
}

void game_update_time(int g)
{
  unsigned now, timesince;
  int wp, bp, gmove;

  /* If players have "paused" the game */
  if (garray[g].clockStopped) return;

  /* If players have time controls of 0 0 (untimed) */
  if (garray[g].time_type == TIMETYPE_UNTIMED) {
    garray[g].bTime = 480;
    garray[g].wTime = 480;
    garray[g].bInByo = 0;
    garray[g].wInByo = 0;
    return;
  }

  /* If a teaching game */
  if ((garray[g].Teach == 1) || (garray[g].Teach2 == 1)) {
    garray[g].bTime = 600;
    garray[g].wTime = 600;
    garray[g].bInByo = 0;
    garray[g].wInByo = 0;
    return;
  }

  /* Courtesy to allow hcap setup, etc.... */
  gmove = movenum(garray[g].GoGame);
  if(gmove < 2) return;

  bp = garray[g].black;
  wp = garray[g].white;

  /* If players are scoring */
  if((parray[wp].protostate == STAT_SCORING) || (parray[bp].protostate == STAT_SCORING)) {
    return;
  }
 
  now = tenth_secs();
  timesince = now - garray[g].lastDecTime;

  /* Game over, ran out of time! */
  if (((garray[g].wTime < 1) && (garray[g].wByoStones > 1))) {
#ifdef PAIR
    if (paired(g)) {
      game_ended(garray[g].pairwith, garray[garray[g].pairwith].black, END_FLAG);
    }
#endif /* PAIR */
    game_ended(g, garray[g].black, END_FLAG);
  }
  else if (((garray[g].bTime < 1) && (garray[g].bByoStones > 1))) {
#ifdef PAIR
    if (paired(g)) {
      game_ended(garray[g].pairwith, garray[garray[g].pairwith].white, END_FLAG);
    }
#endif /* PAIR */
    game_ended(g, garray[g].white, END_FLAG);
  }
  if (garray[g].onMove == WHITE) {
    garray[g].wTime -= (timesince / 10);
#ifdef PAIR
    if(paired(g)) {
      garray[garray[g].pairwith].wTime = garray[g].wTime;
    }
#endif /* PAIR */
    if ((garray[g].wTime <= 0) && (garray[g].wInByo == 0)) {
      garray[g].wInByo = 1;
      garray[g].wByoStones = garray[g].ByoS - 1;
      garray[g].wTime = garray[g].Byo;
      pcn_out(garray[g].white, CODE_INFO,
                FORMAT_THE_PLAYER_s_IS_NOW_IN_BYO_YOMI_n,
                parray[garray[g].white].pname);
      pcn_out_prompt(garray[g].white, CODE_INFO,
                FORMAT_YOU_HAVE_d_STONES_AND_d_MINUTESn,
                garray[g].wByoStones,
                garray[g].wTime/60);
      pcn_out(garray[g].black, CODE_INFO,
               FORMAT_THE_PLAYER_s_IS_NOW_IN_BYO_YOMI_n,
                parray[garray[g].white].pname);
      pcn_out_prompt(garray[g].black, CODE_INFO,
               FORMAT_YOU_HAVE_d_STONES_AND_d_MINUTESn,
                garray[g].wByoStones,
                garray[g].wTime/60);
    }
    else if (garray[g].wByoStones > 0) {
      if (garray[g].wByoStones == 0) {
        garray[g].wByoStones = garray[g].ByoS - 1;
        garray[g].wTime = garray[g].Byo;
      }
    }
#ifdef PAIR
    if(paired(g)) {
    if ((garray[garray[g].pairwith].wTime <= 0) && (garray[garray[g].pairwith].wInByo == 0)) {
      garray[garray[g].pairwith].wInByo = 1;
      garray[garray[g].pairwith].wByoStones = garray[garray[g].pairwith].ByoS - 1;
      garray[garray[g].pairwith].wTime = garray[garray[g].pairwith].Byo;
	/* AvK split */
      pcn_out(garray[garray[g].pairwith].white, CODE_INFO, 
                  FORMAT_THE_PLAYER_s_IS_NOW_IN_BYO_YOMI_n,
                  parray[garray[garray[g].pairwith].white].pname);
      pcn_out_prompt(garray[garray[g].pairwith].white, CODE_INFO, 
                  FORMAT_YOU_HAVE_d_STONES_AND_d_MINUTESn,
                  garray[garray[g].pairwith].wByoStones,
                  garray[garray[g].pairwith].wTime/60);
      pcn_out(garray[garray[g].pairwith].black, CODE_INFO, 
                 FORMAT_sTHE_PLAYER_s_IS_NOW_IN_BYO_YOMI_n,
                  parray[garray[garray[g].pairwith].white].pname);
      pcn_out_prompt(garray[garray[g].pairwith].black, CODE_INFO, 
                 FORMAT_YOU_HAVE_d_STONES_AND_d_MINUTESn,
                  garray[garray[g].pairwith].wByoStones,
                  garray[garray[g].pairwith].wTime/60);
    }
    else if (garray[garray[g].pairwith].wByoStones > 0) {
      if (garray[garray[g].pairwith].wByoStones == 0) {
        garray[garray[g].pairwith].wByoStones = garray[garray[g].pairwith].ByoS - 1;
        garray[garray[g].pairwith].wTime = garray[garray[g].pairwith].Byo;
      }
    }
    }
#endif /* PAIR */

  } else {  /* onMove == BLACK */
    garray[g].bTime -= (timesince / 10);
#ifdef PAIR
    if(paired(g)) {
      garray[garray[g].pairwith].bTime = garray[g].bTime;
    }
#endif /* PAIR */
    if ((garray[g].bTime <= 0) && (garray[g].bInByo == 0)) {
      garray[g].bInByo = 1;
      garray[g].bByoStones = garray[g].ByoS - 1;
      garray[g].bTime = garray[g].Byo;
	/* AvK split */
      pcn_out(garray[g].white, CODE_INFO,
                FORMAT_THE_PLAYER_s_IS_NOW_IN_BYO_YOMI_n,
                parray[garray[g].black].pname);
      pcn_out_prompt(garray[g].white, CODE_INFO,
                FORMAT_YOU_HAVE_d_STONES_AND_d_MINUTESn,
                garray[g].bByoStones,
                garray[g].bTime/60);
      pcn_out(garray[g].black, CODE_INFO,
                FORMAT_THE_PLAYER_s_IS_NOW_IN_BYO_YOMI_n,
                parray[garray[g].black].pname);
      pcn_out_prompt(garray[g].black, CODE_INFO,
                FORMAT_YOU_HAVE_d_STONES_AND_d_MINUTESn,
                garray[g].bByoStones,
                garray[g].bTime/60);
    }
    else if (garray[g].bByoStones > 0) {
      if (garray[g].bByoStones == 0) {
        garray[g].bByoStones = garray[g].ByoS - 1;
        garray[g].bTime = garray[g].Byo;
      }
    }
#ifdef PAIR
    if(paired(g)) {
    if ((garray[garray[g].pairwith].bTime <= 0) && (garray[garray[g].pairwith].bInByo == 0)) {
      garray[garray[g].pairwith].bInByo = 1;
      garray[garray[g].pairwith].bByoStones = garray[garray[g].pairwith].ByoS - 1;
      garray[garray[g].pairwith].bTime = garray[garray[g].pairwith].Byo;
      pcn_out(garray[garray[g].pairwith].white, CODE_INFO, 
         FORMAT_THE_PLAYER_s_IS_NOW_IN_BYO_YOMI_n,
         parray[garray[garray[g].pairwith].black].pname);
      pcn_out_prompt(garray[garray[g].pairwith].white, CODE_INFO, 
         FORMAT_YOU_HAVE_d_STONES_AND_d_MINUTESn,
         garray[garray[g].pairwith].bByoStones,
         garray[garray[g].pairwith].bTime/60);
      pcn_out(garray[garray[g].pairwith].black, CODE_INFO, 
         FORMAT_THE_PLAYER_s_IS_NOW_IN_BYO_YOMI_n,
         parray[garray[garray[g].pairwith].black].pname);
      pcn_out_prompt(garray[garray[g].pairwith].black, CODE_INFO, 
         FORMAT_YOU_HAVE_d_STONES_AND_d_MINUTESn,
         garray[garray[g].pairwith].bByoStones,
         garray[garray[g].pairwith].bTime/60);
    }
    else if (garray[garray[g].pairwith].bByoStones > 0) {
      if (garray[garray[g].pairwith].bByoStones == 0) {
        garray[garray[g].pairwith].bByoStones = garray[garray[g].pairwith].ByoS - 1;
        garray[garray[g].pairwith].bTime = garray[garray[g].pairwith].Byo;
      }
    }
    }
#endif /* PAIR */
  }
  garray[g].lastDecTime = now;
#ifdef PAIR
  if(paired(g)) 
    garray[garray[g].pairwith].lastDecTime = now;
#endif /* PAIR */    
}

void game_update_times()
{
  int g;

  for (g = 0; g < garray_top; g++) {
    if (garray[g].gstatus != GSTATUS_ACTIVE) continue;
    if (garray[g].clockStopped) continue;
    game_update_time(g);
  }
}

int com_sresign(int p, struct parameter * param)
{
  int wp, bp, g, oldwstate, oldbstate;
  int old_w_game, old_b_game;
  struct stat statbuf;
  const char *wname, *bname;

  bname = file_bplayer(param[0].val.string);
  wname = file_wplayer(param[0].val.string);

  if (strlen(bname) < 2) return COM_BADPARAMETERS;
  if (strlen(wname) < 2) return COM_BADPARAMETERS;

  wp = player_fetch(wname);
  if(wp < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_PLAYER_BY_THAT_NAME_);
    return COM_OK;
    }
  bp = player_fetch(bname);
  if(bp < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_PLAYER_BY_THAT_NAME_);
    player_forget(wp);
    return COM_OK;
  }
  
  if((p != wp) && (p != bp)) {
    pcn_out(p, CODE_ERROR, FORMAT_YOU_MUST_BE_ONE_OF_THE_TWO_PLAYERS_TO_SRESIGN_);
    player_forget(wp);
    player_forget(bp);
    return COM_OK;
  }

  if (xystat(&statbuf,FILENAME_GAMES_cs_s,wname, bname) ) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_STORED_GAME_s_VS_s, 
               parray[wp].pname,parray[bp].pname);
    player_forget(wp);
    player_forget(bp);
    return COM_OK;
  }
  oldbstate = parray[bp].protostate;
  oldwstate = parray[wp].protostate;
  old_b_game = parray[bp].game ;
  old_w_game = parray[wp].game ;

  g = game_new(GAMETYPE_GO,19);
  if (game_read(&garray[g], wp, bp) < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_STORED_GAME_s_VS_s_HMMMMM_, 
           parray[wp].pname,parray[bp].pname);
    game_remove(g);
    player_forget(wp);
    player_forget(bp);
    return COM_OK;
  }
  parray[bp].game = g;
  parray[wp].game = g;
  game_ended(g, (p == wp) ? bp : wp, END_RESIGN);
  parray[wp].protostate = oldwstate;
  parray[bp].protostate = oldbstate;
  parray[wp].game = old_w_game;
  parray[bp].game = old_b_game;

  player_forget(bp);
  player_forget(wp);
  return COM_OKN;
}

int com_look(int p, struct parameter * param)
{
  int wp, bp, g, x, until, wc, bc, oldwstate, oldbstate;
  struct stat statbuf;
  twodstring statstring;
  const char *wname, *bname;

  until = 0;

  bname = file_bplayer(param[0].val.string);
  wname = file_wplayer(param[0].val.string);

  if (strlen(bname) < 2) return COM_BADPARAMETERS;
  if (strlen(wname) < 2) return COM_BADPARAMETERS;

  wp = player_fetch(wname);
  if (wp < 0) {
      pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_PLAYER_BY_THAT_NAME_);
      return COM_OK;
    }
  bp = player_fetch(bname);
  if (bp < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_PLAYER_BY_THAT_NAME_);
    player_forget(wp);
    return COM_OK;
  }

  if (xystat(&statbuf,FILENAME_GAMES_cs_s,wname, bname) ) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_STORED_GAME_s_VS_s, 
               parray[wp].pname,parray[bp].pname);
    player_forget(wp);
    player_forget(bp);
    return COM_OK;
  }
  oldwstate = parray[wp].protostate;
  oldbstate = parray[bp].protostate;

  g = game_new(GAMETYPE_GO,19);
  if (game_read(&garray[g], wp, bp) < 0) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_STORED_GAME_s_VS_s_HMMMMM_, 
           parray[wp].pname,parray[bp].pname);
    game_remove(g);
    player_forget(wp);
    player_forget(bp);
    return COM_OK;
  }
  if (parray[p].i_verbose) send_go_board_to(g, p);
  else {
    getcaps(garray[g].GoGame, &wc, &bc);
    boardstatus(garray[g].GoGame, statstring);
    pcn_out(p, CODE_STATUS, FORMAT_s_ss_d_d_d_T_f_dn,
                parray[wp].pname,
                parray[wp].srank,
                parray[wp].rated ? "*" : " ",
                wc,
                garray[g].wTime,
                garray[g].wByoStones,
                garray[g].komi,
                garray[g].GoGame->handicap);
    pcn_out(p, CODE_STATUS, FORMAT_s_ss_d_d_d_T_f_dn,
                parray[bp].pname,
                parray[bp].srank,
                parray[bp].rated ? "*" : " ",
                bc,
                garray[g].bTime,
                garray[g].bByoStones,
                garray[g].komi,
                garray[g].GoGame->handicap);
    if (!until) until = garray[g].GoGame->height;
    for(x = 0; x < until; x++) {
      pcn_out(p, CODE_STATUS, FORMAT_d_sn,
                  x,
                  statstring[x]);
    }
  }
  game_remove(g);
  parray[bp].protostate = oldbstate;
  parray[wp].protostate = oldwstate;
  player_forget(wp);
  player_forget(bp);
  return COM_OKN;
}

int com_problem(int p, struct parameter * param)
{
  int g, x, until, wc, bc;
  twodstring statstring;
  int problem = 0;
  FILE *fp;

  until = 0;

  if(param[0].type == TYPE_NULL) {
    problem = parray[p].last_problem + 1;
    parray[p].last_problem++;
  }
  else
    problem = param[0].val.integer;

  fp = xyfopen(FILENAME_PROBLEM_d, "r", problem);
  if (!fp) {
    pcn_out(p, CODE_ERROR, FORMAT_THERE_IS_NO_PROBLEM_NUMBER_d, problem);
    return COM_OK;
  }
  g = game_new(GAMETYPE_GO,19);
  garray[g].size = 19;
  garray[g].GoGame = initminkgame(19,19, RULES_NET);
  garray[g].wTime = 600;
  garray[g].bTime = 600;
  garray[g].gtitle = mystrdup("Black to play");
  garray[g].white = p;
  garray[g].black = p;
  loadpos(fp, garray[g].GoGame);
  fclose(fp);

  if (parray[p].i_verbose) send_go_board_to(g, p);
  else {
    getcaps(garray[g].GoGame, &wc, &bc);
    boardstatus(garray[g].GoGame, statstring);
    pcn_out(p, CODE_STATUS, FORMAT_s_ss_d_d_d_T_f_dn,
                parray[p].pname,
                parray[p].srank,
                parray[p].rated ? "*" : " ",
                wc,
                garray[g].wTime,
                garray[g].wByoStones,
                garray[g].komi,
                garray[g].GoGame->handicap);
    pcn_out(p, CODE_STATUS, FORMAT_s_ss_d_d_d_T_f_dn,
                parray[p].pname,
                parray[p].srank,
                parray[p].rated ? "*" : " ",
                bc,
                garray[g].bTime,
                garray[g].bByoStones,
                garray[g].komi,
                garray[g].GoGame->handicap);
    if (!until) until = garray[g].GoGame->height;
    for(x = 0; x < until; x++) {
      pcn_out(p, CODE_STATUS, FORMAT_d_sn,
                  x,
                  statstring[x]);
    }
  }
  game_remove(g);
  return COM_OKN;
}
