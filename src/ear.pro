TEMPLATE = subdirs
SUBDIRS = gui libear

gui.subdir = gui
gui.depends = libear

libear.subdir = libear
libear.depends =
