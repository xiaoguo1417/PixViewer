#include "PixViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //����ȫ������
    QFont globalFont;
    globalFont.setFamily("Microsoft YaHei"); //��������Ϊ΢���ź�
    a.setFont(globalFont);

    PixViewer w;
    w.show();
    return a.exec();
}
