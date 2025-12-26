
#pragma execution_character_set("utf-8")

#include "application.h"

int main(int argc, char *argv[]) {
    qputenv("QT_DEVICE_PIXEL_RATIO", "auto");

    App::Application app(argc, argv);

    return app.startApplication();
}
