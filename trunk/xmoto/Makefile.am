SUBDIRS = src bin tools extra

man_MANS = xmoto.6.gz xmoto-edit.6.gz

all: xmoto xmoto-edit

xmoto: src/xmoto
	ln -sf src/xmoto xmoto

xmoto-edit: src/xmoto-edit
	ln -sf src/xmoto-edit xmoto-edit

EXTRA_DIST = $(man_MANS) bootstrap

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@
