SUBDIRS = src bin

all: xmoto xmoto-edit

xmoto: src/xmoto
	ln -sf src/xmoto xmoto

xmoto-edit: src/xmoto-edit
	ln -sf src/xmoto-edit xmoto-edit

EXTRA_DIST = bootstrap

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@
