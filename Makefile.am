SUBDIRS = intl src bin extra po

man_MANS = xmoto.6.gz

EXTRA_DIST = m4/ChangeLog  $(man_MANS) bootstrap configure_mingw_from_linux.sh make_windows_package.sh \
	debian/NEWS \
	debian/changelog\
	debian/compat\
	debian/control\
	debian/copyright\
	debian/docs\
	debian/rules\
	debian/svn-deblayout\
	debian/watch\
	debian/xmoto-data.install\
	debian/xmoto.dirs\
	debian/xmoto.install\
	debian/xmoto.menu

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@

ACLOCAL_AMFLAGS = -I m4
