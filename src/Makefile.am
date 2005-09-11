bin_PROGRAMS = nngssrv

noinst_LIBRARIES = libladder.a
libladder_a_SOURCES = ladder.c #avk emote2.c

EXTRA_DIST =

INCLUDES = \
        -I$(top_srcdir)/memory \
        -I$(top_srcdir)/mlrate/src

LDADD = \
        $(top_srcdir)/memory/libmemory.a \
        $(top_srcdir)/src/libladder.a \
        $(top_srcdir)/mlrate/src/libmlr.a

nngssrv_SOURCES = \
	network.c \
	bm.c \
	channel.c \
	utils.c \
	language.c \
	variable.c \
	legal.c \
	gameproc.c \
	adminproc.c \
	nngsmain.c \
	multicol.c \
	mink.c \
	alias.c \
	censor.c \
	plan.c \
	ip_ban.c \
	chkaddr.c \
	command.c \
	comproc.c \
	gamedb.c \
	playerdb.c \
	emote2.c \
	pending.c \
	udp_commands.c \
	conffile.c \
	mailer.c

noinst_HEADERS =

# Remove these files here... they are created locally
DISTCLEANFILES = *~ *%

@NNGS_ALL_WARNINGS@
