#include <QApplication>
#include "main_window.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("Matrix Animation");
    app.setOrganizationName("DotGlitch");

    MainWindow w;
    w.show();
    return app.exec();
}