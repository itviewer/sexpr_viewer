#include "sexprviewer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SexprViewer w;
    w.show();
    return a.exec();
}
