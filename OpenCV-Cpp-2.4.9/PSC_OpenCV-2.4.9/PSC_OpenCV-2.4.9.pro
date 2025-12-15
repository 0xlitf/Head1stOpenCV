TEMPLATE = subdirs

#CONFIG += ordered

SUBDIRS += computer_vision Counter_AutoTraining

computer_vision.subdir = $$PWD/computer vision
Counter_AutoTraining.subdir = $$PWD/Counter_AutoTraining

Counter_AutoTraining.depends += computer_vision
