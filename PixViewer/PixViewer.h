#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PixViewer.h"

class PixViewer : public QMainWindow
{
    Q_OBJECT

public:
    PixViewer(QWidget *parent = nullptr);
    ~PixViewer();

private:
    Ui::PixViewerClass ui;
};
