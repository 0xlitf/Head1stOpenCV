TEMPLATE = subdirs

#CONFIG += ordered

SUBDIRS += app resources cva psc

app.subdir = $$PWD/src/app
cva.subdir = $$PWD/src/cva
psc.subdir = $$PWD/src/psc
resources.subdir = $$PWD/src/resources

app.depends += cva psc resources