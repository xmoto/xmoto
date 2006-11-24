SUBDIRS = po src bin extra
# tools

man_MANS = xmoto.6.gz #xmoto-edit.6.gz

all: xmoto #xmoto-edit

xmoto: 
	ln -sf src/xmoto xmoto

#xmoto-edit: 
#	ln -sf src/xmoto-edit xmoto-edit

EXTRA_DIST = m4/ChangeLog  $(man_MANS) bootstrap

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@

ACLOCAL_AMFLAGS = -I m4
