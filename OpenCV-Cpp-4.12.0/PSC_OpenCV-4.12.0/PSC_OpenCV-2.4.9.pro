TEMPLATE = subdirs

#CONFIG += ordered

SUBDIRS += computer_vision \
    using_computer_vision

computer_vision.subdir = $$PWD/computer_vision
using_computer_vision.subdir = $$PWD/using_computer_vision

using_computer_vision.depends += computer_vision
