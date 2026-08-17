## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src data

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS \
	TODO

## Desktop DIR
desktopdir = $(datadir)/applications
desktop_DATA = pacman_sdl.desktop

INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = \
	$(INTLTOOL_FILES) \
	pacman_sdl.desktop
  

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)

UPDATE_DESKTOP = update-desktop-database -q || :

install-data-hook:
	$(UPDATE_DESKTOP)
uninstall-hook:
	$(UPDATE_DESKTOP)
