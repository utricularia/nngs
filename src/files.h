/* files.h
 * constants used by the xyfopen() function
 */
#ifndef FILES_H_ 

/*
    NNGS - The No Name Go Server
    Copyright (C) 1995  Erik Van Riper (geek@imageek.york.cuny.edu)
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

#define FILENAME_CMDS 1
#define FILENAME_ACMDS 2
#define FILENAME_INFO 3
	/*
	** Convention used for filename #defines:
	** The _q_ versions take *no* arg and insert a two-letter language dir
	** The _p_ versions take a language arg and insert the two-letter subdir
	** The _l_ versions take an int arg and insert the two-letter subdir
	** The _s_ versions take an char* arg and use it
        ** The filename construction relies on 
	** _q=(_p+0) and _l==(_p +1) and _s=(_l + 1)
	*/
#define FILENAME_HELP 4
#define FILENAME_HELP_q 5
#define FILENAME_HELP_p 5
#define FILENAME_HELP_l (FILENAME_HELP+2)
#define FILENAME_HELP_s 7
#define FILENAME_HELP_q_s 8
#define FILENAME_HELP_p_s 8
#define FILENAME_HELP_l_s 9
#define FILENAME_HELP_s_s 10
#define FILENAME_HELP_l_index (FILENAME_HELP_l+5)
#define FILENAME_HELP_s_index (FILENAME_HELP_s+5)

#define FILENAME_AHELP 14
#define FILENAME_AHELP_q 15
#define FILENAME_AHELP_p 15
#define FILENAME_AHELP_l (FILENAME_AHELP+2)
#define FILENAME_AHELP_s 17
#define FILENAME_AHELP_q_s 18
#define FILENAME_AHELP_p_s 18
#define FILENAME_AHELP_l_s 19
#define FILENAME_AHELP_s_s 20
#define FILENAME_AHELP_l_index (FILENAME_AHELP_l+5)
#define FILENAME_AHELP_s_index (FILENAME_AHELP_s+5)

#define FILENAME_PLAYER 24
#define FILENAME_PLAYER_s 25
#define FILENAME_PLAYER_s_DELETE 26
#define FILENAME_PLAYER_GAMES_s 27
#define FILENAME_PLAYER_LOGONS_s 28
#define FILENAME_PLAYER_MESSAGES_s 30

#define FILENAME_GAMES_s 31
#define FILENAME_GAMES_c 32
#define FILENAME_GAMES_cs_s 33

#define FILENAME_CGAMES 34
#define FILENAME_CGAMES_c 35
#define FILENAME_CGAMES_cs_s_s 36

#define FILENAME_RATINGS 37
#define FILENAME_RESULTS 38
#define FILENAME_NRESULTS 39

#define FILENAME_LADDER9 40
#define FILENAME_LADDER19 41

#define FILENAME_NEWS_s 42
#define FILENAME_NEWSINDEX 43
#define FILENAME_ADMINNEWS_s 44
#define FILENAME_ADMINNEWSINDEX 45
#define FILENAME_NOTEFILE 46
#define FILENAME_LOGONS 47

#define FILENAME_MESS_WELCOME 48
#define FILENAME_MESS_UNREGISTERED 49
#define FILENAME_MESS_MOTD 50
#define FILENAME_MESS_MOTDs 51
#define FILENAME_MESS_AMOTD 52
#define FILENAME_MESS_LOGIN 53
#define FILENAME_MESS_LOGOUT 54
#define FILENAME_EMOTE 55

#define FILENAME_FIND 56
#define FILENAME_LISTINDEX 57
#define FILENAME_LIST_s 58
#define FILENAME_LIST_s_OLD 59

#define FILENAME_PROBLEM_d 60

#endif /* FILES_H_ */
