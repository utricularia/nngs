ahelpdatadir=${prefix}/share/${PACKAGE}/ahelp
ahelpdata_DATA=

LANGUAGES=en ch

install-data-hook:
	for lang in ${LANGUAGES}; do\
	  install -d $(DESTDIR)$(ahelpdatadir)/$$lang ;\
	  for name in $$lang/*; do\
	    test -f $$name \
	    && install -m 644 $$name $(DESTDIR)$(ahelpdatadir)/$$lang ;\
	  done ;\
	done

# Remove these files here... they are created locally
DISTCLEANFILES = *~

@NO_WARNINGS@
