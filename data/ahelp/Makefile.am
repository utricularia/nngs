ahelpdatadir=${prefix}/share/${PACKAGE}/ahelp
ahelpdata_DATA=

LANGUAGES=en ch

install-data-hook:
	for lang in ${LANGUAGES}; do\
	  install -m 755 -d $(DESTDIR)$(ahelpdatadir)/$$lang ;\
	  install -m 644 $$lang/[a-z]* $(DESTDIR)$(ahelpdatadir)/$$lang ;\
	done

# Remove these files here... they are created locally
DISTCLEANFILES = *~

@NO_WARNINGS@
