helpdatadir=${prefix}/share/${PACKAGE}/help
helpdata_DATA=

LANGUAGES=en ch fr pl

install-data-hook:
	for lang in ${LANGUAGES}; do\
	  install -d $(DESTDIR)$(helpdatadir)/$$lang ;\
	  for name in $$lang/* ; do\
	    test -f $$name \
	    && install -m 644 $$name $(DESTDIR)$(helpdatadir)/$$lang ;\
	  done ;\
	done

# Remove these files here... they are created locally
DISTCLEANFILES = *~

@NO_WARNINGS@
