#include "PixViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //设置全局字体
    QFont globalFont;
    globalFont.setFamily("Microsoft YaHei"); //设置字体为微软雅黑
    a.setFont(globalFont);

    PixViewer w;
    w.show();
    return a.exec();
}
