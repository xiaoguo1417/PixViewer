#pragma once

#include <QWidget>
#include "ui_ImageShot.h"
#include<QPainter>
#include <QMouseEvent>

class ImageShot : public QWidget
{
	Q_OBJECT

public:
	ImageShot(QWidget *parent = Q_NULLPTR);
	~ImageShot();
	void setRect(QRect rect);

protected:
	virtual void paintEvent(QPaintEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);

private:
	Ui::ImageShot ui;
	QPoint m_startPoint;
	QRect selection;

private slots:
	void onBtCancleClicked();
	void onBtSaveClicked();
	void onBtResetClicked();
	void onBtModifyClicked();

signals:
	void endShot();
	void savePic();
	void reset();
	void sig_modify(QRect);
};
