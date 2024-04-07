#include "PixViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PixViewer w;
    w.show();
    return a.exec();
}
