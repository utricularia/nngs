#
# autotools subdirs
#
SUBDIRS=help ahelp

#
# local variables
#
MESSAGEDIRS=info ladder lists messages news
NUMHASH=0 1 2 3 4 5 6 7 8 9
LETTERHASH=a b c d e f g h i j k l m n o p q r s t u v w x y z
HASHDIRS=0 1 2 3 4 5 6 7 8 9 a b c d e f g h i j k l m n o p q r s t u v w x y z

mydatadir=${prefix}/share/${PACKAGE}

# data files only can go into this category
mydata_DATA=emotestr

install-data-hook:
	for mess in ${MESSAGEDIRS}; do\
	  install -d $(DESTDIR)$(mydatadir)/$$mess ;\
	  install -m 644 $$mess/* $(DESTDIR)$(mydatadir)/$$mess ;\
	done
	install -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/stats
	install -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/players
	for dir in $(HASHDIRS); do\
	  install -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/players/$$dir ; done
	install -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/games
	for dir in $(HASHDIRS); do\
	  install -d $(DESTDIR)$(prefix)/share/$(PACKAGE)/games/$$dir ; done

# Remove these files here... they are created locally
DISTCLEANFILES = *~ *%

@NO_WARNINGS@
