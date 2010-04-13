#include <QtGui/QApplication>
#include "MainWindow.h"
#include <QPlastiqueStyle>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(new QPlastiqueStyle());
    MainWindow w;
    w.showMinimized();
    return a.exec();
}
