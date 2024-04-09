#include "ImageShot.h"

ImageShot::ImageShot(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlag(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	connect(ui.bt_cancel, SIGNAL(clicked()), this, SLOT(onBtCancleClicked()));
	connect(ui.bt_save, SIGNAL(clicked()), this, SLOT(onBtSaveClicked()));
	connect(ui.bt_reset, SIGNAL(clicked()), this, SLOT(onBtResetClicked()));
	connect(ui.bt_modify, SIGNAL(clicked()), this, SLOT(onBtModifyClicked()));
}

ImageShot::~ImageShot()
{
}

void ImageShot::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.fillRect(this->rect(), QColor(0, 0, 0, 0x20)); /* 设置透明颜色 */
	//painter.fillRect(this->rect(), Qt::gray);
}

void  ImageShot::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		/* 捕获按下时坐标 */
		m_startPoint = frameGeometry().topLeft() - event->globalPos();
	}
}

void  ImageShot::mouseMoveEvent(QMouseEvent *event)
{
	/* 移动窗口 */
	this->move(event->globalPos() + m_startPoint);
}

void  ImageShot::setRect(QRect rect)
{
	selection = rect;
	ui.spin_x->setValue(selection.x());
	ui.spin_y->setValue(selection.y());
	ui.spin_width->setValue(selection.width());
	ui.spin_height->setValue(selection.height());
}

void  ImageShot::onBtCancleClicked()
{
	this->close();
	emit endShot();
}

void ImageShot::onBtModifyClicked()
{
	int x = ui.spin_x->value();
	int y = ui.spin_y->value();
	int w = ui.spin_width->value();
	int h = ui.spin_height->value();

	QRect modifyRect = QRect(x, y, w, h);
	emit sig_modify(modifyRect);
}

void ImageShot::onBtResetClicked()
{	
	emit reset();
}

void ImageShot::onBtSaveClicked()
{
	emit savePic();
}

