#include "PixViewer.h"

PixViewer::PixViewer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	InitViewer();

	setMouseTracking(true);//实现mouseMoveEvent
	ui.centralWidget->setMouseTracking(true);
	ui.widget->setMouseTracking(true);

	ui.widget->installEventFilter(this);//事件过滤器
}

PixViewer::~PixViewer()
{}

void PixViewer::InitViewer()
{
	e_scale = 0.707;
	m_scale = 1.0;
	m_scaleBasePt = QPointF(0, 0);
	isShotting = false;

	filename = "lena.bmp";
	m_img.load(filename);

	//状态栏初始化
	sizeLabel = new QLabel("size", this);
	versionLabel = new QLabel("size", this);
	posLabel = new QLabel("pos", this);
	scaleLabel = new QLabel("scale", this);
	sizeLabel->setMinimumSize(150, 20);
	posLabel->setMinimumSize(150, 20);
	scaleLabel->setMinimumSize(150, 20);
	sizeLabel->setAlignment(Qt::AlignCenter);
	posLabel->setAlignment(Qt::AlignCenter);
	scaleLabel->setAlignment(Qt::AlignCenter);
	ui.statusBar->addWidget(sizeLabel);
	versionLabel->setText("v2.0.0 (Apr 7 2024)");//版本信息
	ui.statusBar->addPermanentWidget(versionLabel);
	ui.statusBar->addWidget(posLabel);
	ui.statusBar->addWidget(scaleLabel);

	bgColor = QColor(240, 240, 240);

	m_buttonLeftPressed = false;
	m_buttonMiddlePressed = false;
}

bool PixViewer::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui.widget && event->type() == QEvent::Paint)//拦截事件
	{
		myPaintEvent();
		return true;
	}
	return QWidget::eventFilter(watched, event);
}

void PixViewer::myPaintEvent()
{
	if (m_img.isNull()) return;

	QPainter p(ui.widget);
	p.fillRect(ui.widget->rect(), bgColor);
	p.save();

	QTransform transform = getTransform();
	p.setWorldTransform(transform);
	p.drawImage(QPoint(0, 0), m_img);
	p.restore();//返回到save时的状态，即默认坐标系状态

	if (isShotting) {
		QRectF w_rect = QRectF(w_ltPoint, w_rbPoint);
		//画框
		QPen pen;
		pen.setBrush(Qt::red);
		pen.setStyle(Qt::DashLine);
		pen.setWidth(3);
		p.setPen(pen);
		p.drawRect(w_rect);
	}

	if (m_scale > 30) {
		//1.获得视场内点坐标
		QPointF tl = screenToWorld(QPointF(ui.widget->x(), ui.widget->y()));
		QPointF br = screenToWorld(QPointF(ui.widget->size().width(), ui.widget->size().height()));
		QRect rectInView(tl.toPoint(), br.toPoint());

		QPen pen(Qt::gray);
		p.setPen(pen);
		QVector<QLineF> lines = getPixelBoundary(rectInView);
		p.drawLines(lines);
		//p.restore();
	}

	if (m_scale > 48) {
		//1.获得视场内点坐标
		QPointF tl = screenToWorld(QPointF(ui.widget->x(), ui.widget->y()));
		QPointF br = screenToWorld(QPointF(ui.widget->size().width(), ui.widget->size().height()));

		//2.获取每个点的像素值
		QRect rectInView(tl.toPoint(), br.toPoint());
		QVector<QPointF> pointsOnScreen;
		QVector<QString> valueString;

		for (int y = rectInView.y(); y < rectInView.y() + rectInView.height(); ++y) {
			for (int x = rectInView.x(); x < rectInView.x() + rectInView.width(); ++x) {
				QRgb pixValue = m_img.pixel(x, y);
				QPointF pointOnScreen = worldToScreen(QPointF(x + 0.5, y + 0.5));
				pointsOnScreen.push_back(pointOnScreen);
				valueString.push_back(QString::number(qGray(pixValue)));
			}
		}

		//画出像素值
		QPen pen(bgColor);
		p.setPen(pen);
		QFont font;
		font.setFamily("Times");
		font.setPointSize(8);
		p.setFont(font);
		for (int i = 0; i < pointsOnScreen.size(); ++i) {
			int strWidth = p.fontMetrics().width(valueString[i]);
			int strHeight = p.fontMetrics().height();
			QPointF fontPoint = QPointF(pointsOnScreen[i] - QPointF(strWidth / 2.0, -strHeight / 4.0));
			QRect fontRect(fontPoint.x() - 1, fontPoint.y() - strHeight + strHeight / 8, strWidth + 2, strHeight + +strHeight / 8);
			p.setPen(Qt::gray);
			p.setBrush(QBrush(Qt::gray));
			p.drawRect(fontRect);
			p.setPen(Qt::black);
			p.drawText(fontPoint, valueString[i]);
		}
	}

	QString size = QString::number(m_img.width()) + "*" + QString::number(m_img.height());
	sizeLabel->setText(size);
	ui.statusBar->addWidget(sizeLabel);
}

QTransform PixViewer::getTransform()
{
	QTransform transform;
	//以m_scaleBasePt为中心，放大缩小
	if (!m_img.isNull()) {
		transform.translate(m_scaleBasePt.x(), m_scaleBasePt.y());//坐标原点的重新定义，参数为重新定义的x坐标，y坐标
		transform.scale(m_scale, m_scale);
	}
	return transform;
}

//从图像坐标到鼠标坐标系下
QPointF PixViewer::worldToScreen(QPointF pt)
{
	QTransform t = getTransform();
	return t.map(pt);//返回变化后的点
}

//获取转换矩阵并将点转换，从鼠标点坐标到图像坐标系下坐标
QPointF PixViewer::screenToWorld(QPointF pt)
{
	QTransform t = getTransform();
	QTransform t1 = t.inverted();//逆矩阵
	return t1.map(pt);//表示把点使用变换矩阵matrix进行转换
}

//边界线
QVector<QLineF> PixViewer::getPixelBoundary(QRect rect)
{
	QVector<QLineF> lines;
	QPointF left, right, top, buttom;
	for (int y = rect.y(); y < rect.y() + rect.height(); ++y)
	{
		left = worldToScreen(QPointF(rect.x() - 1, y));
		right = worldToScreen(QPointF(rect.x() + rect.width() + 1, y));
		lines.append(QLineF(left, right));
	}
	for (int x = rect.x(); x < rect.x() + rect.width(); ++x)
	{
		top = worldToScreen(QPointF(x, rect.y() - 1));
		buttom = worldToScreen(QPointF(x, rect.y() + rect.height() + 1));
		lines.append(QLineF(top, buttom));
	}
	return lines;
}

void PixViewer::mouseMoveEvent(QMouseEvent* event)
{
	if (m_buttonLeftPressed & isShotting) {
		QPointF curPt = event->pos();
		w_rbPoint = QPoint(curPt.x() - 13, curPt.y() - 89);
		QPointF pt2 = screenToWorld(w_rbPoint);
		rbPoint = pt2;
	}
	else if (m_buttonMiddlePressed) {
		QPointF mv = event->pos() - m_lastPressedMousePt;
		//mv *= m_scale;
		m_scaleBasePt += mv;
		m_lastPressedMousePt = event->pos();
		repaint();
	}

	//调试完成，不进行缩放拖拽没问题，灰度值好像不对
	if (!m_img.isNull()) {
		QPoint sPoint1 = event->globalPos();
		QPoint widgetPoint = ui.widget->mapFromGlobal(sPoint1);
		QPointF pt2 = screenToWorld(widgetPoint);
		
		curWPt = pt2;//更新当前点坐标，好像没啥用
		
		//获取像素点坐标及像素值
		int x = pt2.x();
		int y = pt2.y();
		QRgb pixValue = m_img.pixel(x, y);
		QString str = "[" + QString::number(x) + "," + QString::number(y) + "]---" + QString::number(qGray(pixValue));
		posLabel->setText(str);
	}
}

void PixViewer::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && isShotting) {
		m_buttonLeftPressed = true;
		QPointF curPt = event->pos();
		w_ltPoint = QPoint(curPt.x() - 13, curPt.y() - 89);//转换到widget左上角为原点
		QPointF pt1 = screenToWorld(w_ltPoint);
		ltPoint = pt1;
	}
	else if (event->button() == Qt::LeftButton && !isShotting) {
		m_buttonMiddlePressed = true;
	}
	else if (event->button() == Qt::MidButton) {
		
	}
	else if (event->button() == Qt::RightButton) {
		
	}
	m_lastPressedMousePt = event->pos();
}

void PixViewer::mouseReleaseEvent(QMouseEvent* event)
{
	m_buttonLeftPressed = false;
	m_buttonMiddlePressed = false;

	if (event->button() == Qt::LeftButton && isShotting) {
		m_selection = QRect(QPoint(qMin(ltPoint.x(), rbPoint.x()), qMin(ltPoint.y(), rbPoint.y())), 
			QPoint(qMax(ltPoint.x(), rbPoint.x()), qMax(ltPoint.y(), rbPoint.y())));
		QRectF w_rect = QRectF(w_ltPoint, w_rbPoint);
		repaint();
	}

	if (event->button() == Qt::RightButton) {
		m_selection = QRect();
		m_refined = QRect();
		repaint();
	}
}

void PixViewer::wheelEvent(QWheelEvent* event)//获取每次变换后原点的位置及放大倍数
{
	if (!m_img.isNull()) {
		QPoint angle = event->angleDelta();//返回滚轮度数的增量,有水平和垂直滚轮

		QPointF pt1 = event->posF();
		//pt1 = QPoint(pt1.x(), pt1.y());
		pt1 = QPoint(pt1.x() - 13, pt1.y() - 89);//修正坐标原点不在widget左上角的问题
		QPointF p = worldToScreen(QPointF(0, 0));//图像的(0,0)点对应的屏幕坐标，找到当前图像左上角点在初始坐标系下的坐标值

		QPointF pt2;
		if (angle.y() > 0)
		{
			pt2 = pt1 + (p - pt1) / e_scale;//滚一下放大0.618
			m_scale /= e_scale;
		}
		else
		{
			pt2 = pt1 + (p - pt1) * e_scale;
			m_scale *= e_scale;
		}

		QTransform rotationTransform;
		QPointF c(m_img.width() / qreal(2), m_img.height() / qreal(2));
		rotationTransform.translate(c.x(), c.y());
		/*rotationTransform.rotate(m_rotation);*/
		rotationTransform.translate(-c.x(), -c.y());

		QTransform scalingTransform;
		scalingTransform.scale(m_scale, m_scale);

		QPointF a3 = (scalingTransform.map(rotationTransform.map(QPointF(0, 0)))); //ok

		QPointF pt3 = pt2 - a3;
		m_scaleBasePt = pt3;
		repaint();
	}

	scaleLabel->setText(QString::number(m_scale * 100) + "%");
}

