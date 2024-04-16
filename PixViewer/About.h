#pragma once

#include <QWidget>
#include "ui_About.h"

class About : public QWidget
{
	Q_OBJECT

public:
	About(QWidget *parent = nullptr);
	~About();

private:
	Ui::AboutClass ui;
};
