SUBDIRS = src bin

all: xmoto xmoto-edit

xmoto: src/xmoto
	ln -s src/xmoto xmoto

xmoto-edit: src/xmoto-edit
	ln -s src/xmoto-edit xmoto-edit

EXTRA_DIST = bootstrap

uninstall-hook:
	rm -r $(datadir)/@PACKAGE@
