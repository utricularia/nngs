helpdatadir=${prefix}/share/${PACKAGE}/help
helpdata_DATA=

LANGUAGES=en ch fr pl

install-data-hook:
	for lang in ${LANGUAGES}; do\
	  install -m 755 -d $(DESTDIR)$(helpdatadir)/$$lang ;\
	  install -m 644 $$lang/* $(DESTDIR)$(helpdatadir)/$$lang ;\
	done

# Remove these files here... they are created locally
DISTCLEANFILES = *~

@NO_WARNINGS@
