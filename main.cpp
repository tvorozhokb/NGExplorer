#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    MainWindow w;
    w.show();
    return app.exec();
}
