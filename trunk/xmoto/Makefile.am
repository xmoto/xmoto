SUBDIRS = src bin extra po

man_MANS = xmoto.6.gz

EXTRA_DIST = m4/ChangeLog  $(man_MANS) bootstrap configure_mingw_from_linux.sh xmoto_icone.ico

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@

ACLOCAL_AMFLAGS = -I m4
