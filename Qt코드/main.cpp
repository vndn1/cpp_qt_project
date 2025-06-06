#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("감정 분석기 (심리 거리 측정)");
    w.show();
    return a.exec();
}
