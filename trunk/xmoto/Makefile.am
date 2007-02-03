SUBDIRS = src bin extra po

man_MANS = xmoto.6.gz

EXTRA_DIST = m4/ChangeLog  $(man_MANS) bootstrap mingw_linux_to_windows.sh

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@

ACLOCAL_AMFLAGS = -I m4
