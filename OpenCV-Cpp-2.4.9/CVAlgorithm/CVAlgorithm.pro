TEMPLATE = subdirs

#CONFIG += ordered

SUBDIRS += app resources cva

app.subdir = $$PWD/src/app
cva.subdir = $$PWD/src/cva
resources.subdir = $$PWD/src/resources

app.depends += cva resources