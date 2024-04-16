#include "About.h"

About::About(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.label_url1->setOpenExternalLinks(true);
	ui.label_url2->setOpenExternalLinks(true);
}

About::~About()
{}
