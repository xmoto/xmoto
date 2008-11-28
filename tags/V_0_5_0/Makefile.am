SUBDIRS = intl src bin extra po

man_MANS = xmoto.6

EXTRA_DIST = m4/ChangeLog  $(man_MANS) bootstrap configure_mingw_from_linux.sh make_windows_package.sh

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@

documentation:
	doxygen ./Doxyfile

ACLOCAL_AMFLAGS = -I m4
