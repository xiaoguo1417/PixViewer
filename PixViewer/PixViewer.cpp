#include "PixViewer.h"

PixViewer::PixViewer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	this->show();
	this->hide();

	InitViewer();

	setMouseTracking(true);//实现mouseMoveEvent
	ui.centralWidget->setMouseTracking(true);
	ui.stackedWidget->setMouseTracking(true);
	ui.page->setMouseTracking(true);
	ui.widget->setMouseTracking(true);

	ui.widget->installEventFilter(this);//事件过滤器

	connect(ui.action_shot, &QAction::triggered, this, &PixViewer::shot);
	connect(m_shot, SIGNAL(endShot()), this, SLOT(endShotting()));
	connect(m_shot, SIGNAL(savePic()), this, SLOT(savePicture()));
	connect(m_shot, SIGNAL(reset()), this, SLOT(resetShot()));
	connect(m_shot, SIGNAL(sig_modify(QRect)), this, SLOT(modifyShot(QRect)));

	ui.widget_ref->installEventFilter(this);
	ui.widget_cmp->installEventFilter(this);
	ui.page_2->setMouseTracking(true);
	ui.groupBox->setMouseTracking(true);
	ui.widget_ref->setMouseTracking(true);
	ui.groupBox_2->setMouseTracking(true);
	ui.widget_cmp->setMouseTracking(true);
	connect(ui.action_cmp, &QAction::triggered, this, &PixViewer::cmp);
	connect(ui.bt_sync, &QPushButton::clicked, this, &PixViewer::switchSyncMode);
}

PixViewer::~PixViewer()
{}

void PixViewer::InitViewer()
{
	ui.stackedWidget->setCurrentIndex(0);

	e_scale = 0.707;
	m_scale = 1.0;
	m_scaleBasePt = QPointF(0, 0);
	isShotting = false;
	isCmping = false;
	isSyncing = true;

	filename = "lena.bmp";
	m_img.load(filename);
	if (m_img.format() == QImage::Format_Mono || m_img.format() == QImage::Format_Indexed8) m_ch = 1;
	else m_ch = 3;

	ImgAdaptView();

	//状态栏初始化
	sizeLabel = new QLabel("size", this);
	versionLabel = new QLabel("size", this);
	posLabel = new QLabel("pos", this);
	scaleLabel = new QLabel(QString::number(m_scale * 100) + "%", this);
	sizeLabel->setMinimumSize(150, 20);
	posLabel->setMinimumSize(300, 20);
	scaleLabel->setMinimumSize(150, 20);
	sizeLabel->setAlignment(Qt::AlignCenter);
	posLabel->setAlignment(Qt::AlignCenter);
	scaleLabel->setAlignment(Qt::AlignCenter);	
	versionLabel->setText("v2.0.0 (Apr 7 2024)");//版本信息

	ui.statusBar->addPermanentWidget(versionLabel);
	ui.statusBar->addWidget(posLabel);
	ui.statusBar->addWidget(scaleLabel);
	ui.statusBar->addWidget(sizeLabel);

	bgColor = QColor(240, 240, 240);

	m_buttonLeftPressed = false;
	m_buttonMiddlePressed = false;
	leftButtonType = ADJ_SLID;
}

//图片自适应放置
void PixViewer::ImgAdaptView()
{
	//计算两个方向的放大比例，选择小的进行放大
	int iw = m_img.width();
	int ih = m_img.height();
	int w = ui.widget->width();
	int h = ui.widget->height();
	double sw = 0.8 * w / (1.0 * iw);
	double sh = 0.8 * h / (1.0 * ih);
	double s = std::min(sw, sh);

	//计算左上角点在widget中坐标
	int x = w / 2 - (iw * s) / 2;
	int y = h / 2 - (ih * s) / 2;

	m_scaleBasePt = QPoint(x, y);
	m_scale = s;
	repaint();
}

bool PixViewer::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui.widget && event->type() == QEvent::Paint) {//拦截事件{
		myPaintEvent();
		return true;
	}
	if (watched == ui.widget_ref && event->type() == QEvent::Paint) {
		RefPaintEvent();
	}
	if (watched == ui.widget_cmp && event->type() == QEvent::Paint) {
		CmpPaintEvent();
	}
	if (watched == ui.widget_ref && event->type() == QEvent::MouseMove) {
		QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
		RefMouseMoveEvent(me);
	}
	if (watched == ui.widget_cmp && event->type() == QEvent::MouseMove) {
		QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
		CmpMouseMoveEvent(me);
	}
	if (watched == ui.widget_ref && event->type() == QEvent::Wheel) {
		QWheelEvent* we = dynamic_cast<QWheelEvent*>(event);
		RefWheelMoveEvent(we);
	}
	if (watched == ui.widget_cmp && event->type() == QEvent::Wheel) {
		QWheelEvent* we = dynamic_cast<QWheelEvent*>(event);
		CmpWheelMoveEvent(we);
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
		//绘制背景
		QPointF pic_ltPoint = worldToScreen(QPointF(0, 0));
		QPointF pic_rbPoint = worldToScreen(QPointF(m_img.width(), m_img.height()));
		QRect w_pic((int)pic_ltPoint.x(), (int)pic_ltPoint.y(), (int)pic_rbPoint.x() - (int)pic_ltPoint.x(), (int)pic_rbPoint.y() - (int)pic_ltPoint.y());
		//QColor shadowColor = QColor(0, 0, 0, 100);
		QPainter p_bg(ui.widget);
		p_bg.setBrush(QBrush(QColor(0, 0, 0, 100))); // 设置一个黑色蒙层
		p_bg.drawRect(w_pic);

		//更新矩形区域点坐标, 画框
		w_ltPoint = worldToScreen(ltPoint);
		w_rbPoint = worldToScreen(rbPoint);
		m_wSelection = QRectF(w_ltPoint, w_rbPoint);
		QPainter p_select(ui.widget);
		p_select.setRenderHint(QPainter::Antialiasing, true);
		QPen pen;
		pen.setBrush(QBrush(QColor(162, 216, 234, 255)));
		pen.setStyle(Qt::DashLine);
		pen.setWidth(5);
		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		p_select.setPen(pen);
		p_select.drawRect(m_wSelection);

		//将选中区域图片更新修改
		QPainter p_tar(ui.widget);
		p_tar.setRenderHint(QPainter::Antialiasing, true);
		// 返回图像的子区域作为新图像。返回的图像是从该图像中的位置(rectangle.x()，         
		// rectangle.y())复制的，并且始终具有给定矩形的大小。
		QPoint ltP(ltPoint.x(), ltPoint.y());
		QPoint rbP(rbPoint.x(), rbPoint.y());
		m_selection = QRect(ltP, rbP);
		QImage targetImage = m_img.copy(m_selection);
		p_tar.drawImage(m_wSelection, targetImage);
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

	if (m_scale > 64) {
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
				if (m_ch == 1) valueString.push_back(QString::number(qGray(pixValue)));
				else if (m_ch == 3) {
					valueString.push_back(QString::number(qRed(pixValue)));
					valueString.push_back(QString::number(qGreen(pixValue)));
					valueString.push_back(QString::number(qBlue(pixValue)));
				}
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
			if (m_ch == 1) {
				int strWidth = p.fontMetrics().width(valueString[i]);
				int strHeight = p.fontMetrics().height();
				QPointF fontPoint = QPointF(pointsOnScreen[i] - QPointF(strWidth / 2.0, -strHeight / 4.0));
				QRect fontRect(fontPoint.x() - 1, fontPoint.y() - strHeight + strHeight / 8, strWidth + 2, strHeight +strHeight / 8);
				p.setPen(Qt::gray);
				p.setBrush(QBrush(Qt::gray));
				p.drawRect(fontRect);
				p.setPen(Qt::black);
				p.drawText(fontPoint, valueString[i]);
			}
			else if (m_ch == 3) {
				int strWidth = p.fontMetrics().width(valueString[3 * i]);
				int strHeight = p.fontMetrics().height() * 3;
				QPointF fontPoint = QPointF(pointsOnScreen[i] - QPointF(strWidth / 2.0, strHeight/2.0));
				QRect fontRect(fontPoint.x() - 1, fontPoint.y() - 1, strWidth + 2, strHeight +strHeight / 8);
				p.setPen(Qt::gray);
				p.setBrush(QBrush(Qt::gray));
				p.drawRect(fontRect);
				p.setPen(Qt::black);
				QPointF shiftPoint(0, strHeight / 3);
				p.drawText(fontPoint + shiftPoint, valueString[3 * i]);
				p.drawText(fontPoint + shiftPoint * 2, valueString[3 * i + 1]);
				p.drawText(fontPoint + shiftPoint * 3, valueString[3 * i + 2]);
			}
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
	if (m_buttonLeftPressed && isShotting) {
		//分为两lei状态：1 拖拽获取区域；2 调整区域
		QPoint sPoint1 = event->globalPos();
		QPointF mousePoint = ui.widget->mapFromGlobal(sPoint1);
		
		AdjSelectionRect(mousePoint);
		rbPoint = screenToWorld(w_rbPoint);
		ltPoint = screenToWorld(w_ltPoint);

		m_wSelection = QRect(QPoint(qMin(w_ltPoint.x(), w_rbPoint.x()), qMin(w_ltPoint.y(), w_rbPoint.y())),
			QPoint(qMax(w_ltPoint.x(), w_rbPoint.x()), qMax(w_ltPoint.y(), w_rbPoint.y())));
		AdjControlRect();
	}
	else if (!m_buttonLeftPressed && isShotting) {
		// 设置鼠标形状
		setCursor(Qt::ArrowCursor);
		QPoint sPoint1 = event->globalPos();
		QPoint mousePos = ui.widget->mapFromGlobal(sPoint1);
		if (m_wSelection.contains(mousePos)) setCursor(Qt::SizeAllCursor);//在选择的矩形内，十字箭头
		if (lt_ControlRect.contains(mousePos) || rb_ControlRect.contains(mousePos)) setCursor(Qt::SizeFDiagCursor);
		else if (mt_ControlRect.contains(mousePos) || mb_ControlRect.contains(mousePos)) setCursor(Qt::SizeVerCursor);
		else if (lm_ControlRect.contains(mousePos) || rm_ControlRect.contains(mousePos)) setCursor(Qt::SizeHorCursor);
		else if (rt_ControlRect.contains(mousePos) || lb_ControlRect.contains(mousePos)) setCursor(Qt::SizeBDiagCursor);
	}

	if (m_buttonMiddlePressed) {
		QPoint sPoint1 = event->globalPos();
		QPoint widgetPoint = ui.widget->mapFromGlobal(sPoint1);
		QPointF mv = widgetPoint - m_lastPressedMousePt;
		//mv *= m_scale;
		m_scaleBasePt += mv;
		m_lastPressedMousePt = widgetPoint;
	}

	//调试完成
	if (!m_img.isNull()) {
		QPoint sPoint1 = event->globalPos();
		QPoint widgetPoint = ui.widget->mapFromGlobal(sPoint1);
		QPointF pt2 = screenToWorld(widgetPoint);
		
		curWPt = pt2;//更新当前点坐标，好像没啥用
		
		//获取像素点坐标及像素值
		int x = pt2.x();
		int y = pt2.y();
		QRgb pixValue = m_img.pixel(x, y);
		QString pixstr;
		if (m_ch == 1)
			pixstr = "[" + QString::number(x) + "," + QString::number(y) + "]---" + QString::number(qGray(pixValue));
		else if (m_ch == 3) {
			pixstr = "[" + QString::number(x) + "," + QString::number(y) + "]---[" + QString::number(qRed(pixValue))
				+ "," + QString::number(qGreen(pixValue)) + "," + QString::number(qBlue(pixValue)) + "]";
		}
		posLabel->setText(pixstr);
	}
	repaint();
}

void PixViewer::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && isShotting) {
		m_buttonLeftPressed = true;
		QPoint sPoint1 = event->globalPos();
		QPointF mousePos = ui.widget->mapFromGlobal(sPoint1);
		leftButtonType = ADJ_SLID;

		//设置箭头类型及ADJ类型
		if (lt_ControlRect.contains(mousePos)) {
			leftButtonType = ADJ_LEFTTOP;
			setCursor(Qt::SizeFDiagCursor);
		}
		else if (rb_ControlRect.contains(mousePos)) {
			leftButtonType = ADJ_RIGHTBOTTOM;
			setCursor(Qt::SizeFDiagCursor);
		}
		else if (mt_ControlRect.contains(mousePos)) {
			leftButtonType = ADJ_MIDTOP;
			setCursor(Qt::SizeVerCursor);
		}
		else if (mb_ControlRect.contains(mousePos)) {
			leftButtonType = ADJ_MIDBOTTOM;
			setCursor(Qt::SizeVerCursor);
		}
		else if (lm_ControlRect.contains(mousePos)) {
			leftButtonType = ADJ_MIDLEFT;
			setCursor(Qt::SizeHorCursor);
		}
		else if (rm_ControlRect.contains(mousePos)) {
			leftButtonType = ADJ_MIDRIGHT;
			setCursor(Qt::SizeHorCursor);
		}
		else if (rt_ControlRect.contains(mousePos)) {
			leftButtonType = ADJ_RIGTHTOP;
			setCursor(Qt::SizeBDiagCursor);
		}
		else if (lb_ControlRect.contains(mousePos)) {
			leftButtonType = ADJ_LEFTBOTTOM;
			setCursor(Qt::SizeBDiagCursor);
		}
		else if (m_wSelection.contains(mousePos)) {
			QRectF s_select(m_wSelection.topLeft() + QPointF(0.2 * m_wSelection.width(), 0.2 * m_wSelection.height()), m_wSelection.size() * 0.6);
			if (s_select.contains(mousePos)) {
				leftButtonType = ADJ_MOVE;
				setCursor(Qt::SizeAllCursor);
			}
		}

		if (leftButtonType == ADJ_SLID) {
			w_ltPoint = mousePos;
			ltPoint = screenToWorld(w_ltPoint);
		}
	}
	else if (event->button() == Qt::LeftButton && !isShotting) {
		m_buttonMiddlePressed = true;
	}
	else if (event->button() == Qt::MidButton) {
		
	}
	else if (event->button() == Qt::RightButton && isShotting) {
		m_buttonMiddlePressed = true;
	}
	QPoint sPoint1 = event->globalPos();
	m_lastPressedMousePt = ui.widget->mapFromGlobal(sPoint1);
}

void PixViewer::mouseReleaseEvent(QMouseEvent* event)
{
	m_buttonLeftPressed = false;
	m_buttonMiddlePressed = false;
	leftButtonType = ADJ_SLID;

	if (event->button() == Qt::LeftButton && isShotting) {
		m_selection = QRect(QPoint(qMin(ltPoint.x(), rbPoint.x()), qMin(ltPoint.y(), rbPoint.y())), 
			QPoint(qMax(ltPoint.x(), rbPoint.x()), qMax(ltPoint.y(), rbPoint.y())));
		//传参到子界面
		m_shot->setRect(m_selection);
		repaint();
	}
}

void PixViewer::wheelEvent(QWheelEvent* event)//获取每次变换后原点的位置及放大倍数
{
	if (!m_img.isNull()) {
		QPoint angle = event->angleDelta();//返回滚轮度数的增量,有水平和垂直滚轮

		QPoint sPoint1 = event->globalPos();
		QPoint pt1 = ui.widget->mapFromGlobal(sPoint1);

		QPointF p = worldToScreen(QPointF(0, 0));//图像的(0,0)点对应的屏幕坐标，找到当前图像左上角点在初始坐标系下的坐标值

		QPointF pt2;
		if (angle.y() > 0) {
			pt2 = pt1 + (p - pt1) / e_scale;//滚一下放大0.618
			m_scale /= e_scale;
		}
		else {
			pt2 = pt1 + (p - pt1) * e_scale;
			m_scale *= e_scale;
		}
		m_scaleBasePt = pt2;
		repaint();
	}

	scaleLabel->setText(QString::number(m_scale * 100) + "%");
}

void PixViewer::shot()
{
	isShotting = true;

	m_shot->setParent(this);//指定父窗口
	m_shot->setWindowFlags(m_shot->windowFlags() | Qt::Dialog);
	QPoint dis = QPoint(this->geometry().x() + 10, this->geometry().y() + this->height() - m_shot->height() - 38);
	m_shot->move(dis);//move的原点是父窗口的左上角，  如果没有父窗口，则桌面即为父窗口

	m_shot->show();
}

void PixViewer::endShotting()
{
	isShotting = false;

	lt_ControlRect = QRectF(0, 0, 0, 0);
	mt_ControlRect = QRectF(0, 0, 0, 0);
	rt_ControlRect = QRectF(0, 0, 0, 0);
	lb_ControlRect = QRectF(0, 0, 0, 0);
	mb_ControlRect = QRectF(0, 0, 0, 0);
	rb_ControlRect = QRectF(0, 0, 0, 0);
	lm_ControlRect = QRectF(0, 0, 0, 0);
	rm_ControlRect = QRectF(0, 0, 0, 0);
}

void PixViewer::savePicture()
{
	QString filename1 = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("Images (*.png *.bmp *.jpg *.tif)")); //选择路径
	QImage shotted = m_img.copy(m_selection);
	shotted.save(filename1);
}

void PixViewer::resetShot()
{
	w_ltPoint = QPoint(0, 0);
	w_rbPoint = QPoint(0, 0);
	m_selection = QRect(0, 0, 0, 0);
	ltPoint = QPointF(0, 0);
	rbPoint = QPointF(0, 0);
	m_shot->setRect(m_selection);
	repaint();
}

void PixViewer::modifyShot(QRect rect)
{
	ltPoint = rect.topLeft();
	rbPoint = rect.bottomRight();
	m_selection = QRect(QPoint(qMin(ltPoint.x(), rbPoint.x()), qMin(ltPoint.y(), rbPoint.y())), QPoint(qMax(ltPoint.x(), rbPoint.x()), qMax(ltPoint.y(), rbPoint.y())));

	QPointF temp = worldToScreen(ltPoint);
	w_ltPoint = temp;
	QPointF temp2 = worldToScreen(rbPoint);
	w_rbPoint = temp2;

	repaint();
}

//根据调整类型改变矩形区域，传入当前鼠标位置
void PixViewer::AdjSelectionRect(QPointF mousePos)
{
	if (leftButtonType == ADJ_SLID) {
		w_rbPoint = mousePos;
	}
	else if (leftButtonType == ADJ_MOVE) {
		QPointF mv = mousePos - m_lastPressedMousePt;
		w_ltPoint += mv;
		w_rbPoint += mv;
		m_lastPressedMousePt = mousePos;
	}
	else if (leftButtonType == ADJ_LEFTTOP) {
		QPointF mv = mousePos - w_ltPoint;
		w_ltPoint += mv;
	}
	else if (leftButtonType == ADJ_LEFTBOTTOM) {//
		double lbPoint_x = w_ltPoint.x();
		double lbPoint_y = w_rbPoint.y();
		double movex = mousePos.x() - lbPoint_x;
		double movey = mousePos.y() - lbPoint_y;
		w_ltPoint += QPointF(movex, 0);
		w_rbPoint += QPointF(0, movey);
	}
	else if (leftButtonType == ADJ_RIGTHTOP) {
		double rtPoint_x = w_rbPoint.x();
		double rtPoint_y = w_ltPoint.y();
		double movex = mousePos.x() - rtPoint_x;
		double movey = mousePos.y() - rtPoint_y;
		w_ltPoint += QPointF(0, movey);
		w_rbPoint += QPointF(movex, 0);
	}
	else if (leftButtonType == ADJ_RIGHTBOTTOM) {//
		QPointF mv = mousePos - w_rbPoint;
		w_rbPoint += mv;
	}
	else if (leftButtonType == ADJ_MIDTOP) {
		double movey = mousePos.y() - w_ltPoint.y();
		w_ltPoint += QPointF(0, movey);
	}
	else if (leftButtonType == ADJ_MIDBOTTOM) {
		double movey = mousePos.y() - w_rbPoint.y();
		w_rbPoint += QPointF(0, movey);
	}
	else if (leftButtonType == ADJ_MIDLEFT) {
		double movex = mousePos.x() - w_ltPoint.x();
		w_ltPoint += QPointF(movex, 0);
	}
	else if (leftButtonType == ADJ_MIDRIGHT) {
		double movex = mousePos.x() - w_rbPoint.x();
		w_rbPoint += QPointF(movex, 0);
	}
}

//根据选择区域建立四周矩形选择区域
void PixViewer::AdjControlRect()
{
	//四周矩形区域建立
	double horiz01 = m_wSelection.width() * 0.1;
	double vert01 = m_wSelection.height() * 0.1;
	lt_ControlRect = QRectF(QPointF(m_wSelection.topLeft() - QPointF(horiz01, vert01)), QSize(horiz01 * 2, vert01 * 2));
	mt_ControlRect = QRectF(QPointF(m_wSelection.topLeft() + QPointF(horiz01, -vert01)), QSize(horiz01 * 6, vert01 * 2));
	rt_ControlRect = QRectF(QPointF(m_wSelection.topRight() - QPointF(horiz01, vert01)), QSize(horiz01 * 2, vert01 * 2));
	lb_ControlRect = QRectF(QPointF(m_wSelection.bottomLeft() - QPointF(horiz01, vert01)), QSize(horiz01 * 2, vert01 * 2));
	mb_ControlRect = QRectF(QPointF(m_wSelection.bottomLeft() + QPointF(horiz01, -vert01)), QSize(horiz01 * 6, vert01 * 2));
	rb_ControlRect = QRectF(QPointF(m_wSelection.bottomRight() - QPointF(horiz01, vert01)), QSize(horiz01 * 2, vert01 * 2));
	lm_ControlRect = QRectF(QPointF(m_wSelection.topLeft() + QPointF(-horiz01, vert01)), QSize(horiz01 * 2, vert01 * 6));
	rm_ControlRect = QRectF(QPointF(m_wSelection.topRight() + QPointF(-horiz01, vert01)), QSize(horiz01 * 2, vert01 * 6));
}

/*--------------------------------------
* 分割线，对比图；为了不改变上面的初始代码，下面很多功能基本都重新又复制过来一遍
--------------------------------------*/
void PixViewer::cmp()
{
	isCmping = true;
	ui.stackedWidget->setCurrentIndex(1);

	//放置图片
	if (!m_img.isNull()) {
		ref_img = m_img;
		cmp_img = m_img;
		ref_ch = m_ch;
		cmp_ch = m_ch;
		ImgAdaptView(ref_img, ui.widget_ref);
	}
}

void PixViewer::PaintEvent(QWidget* widget, QPointF basePt, qreal scale, QImage img)
{
	QPainter p(widget);
	p.fillRect(widget->rect(), bgColor);
	p.save();

	QTransform transform = getTransform(basePt, scale);
	p.setWorldTransform(transform);
	p.drawImage(QPoint(0, 0), img);
	p.restore();//返回到save时的状态，即默认坐标系状态

	if (scale > 30) {
		//1.获得视场内点坐标
		QPointF tl = screenToWorld(QPointF(ui.widget->x(), ui.widget->y()), basePt, scale);
		QPointF br = screenToWorld(QPointF(ui.widget->size().width(), ui.widget->size().height()), basePt, scale);
		QRect rectInView(tl.toPoint(), br.toPoint());

		QPen pen(Qt::gray);
		p.setPen(pen);
		QVector<QLineF> lines = getPixelBoundary(rectInView);
		p.drawLines(lines);
		//p.restore();
	}

	if (scale > 64) {
		//1.获得视场内点坐标
		QPointF tl = screenToWorld(QPointF(ui.widget->x(), ui.widget->y()), basePt, scale);
		QPointF br = screenToWorld(QPointF(ui.widget->size().width(), ui.widget->size().height()), basePt, scale);

		//2.获取每个点的像素值
		QRect rectInView(tl.toPoint(), br.toPoint());
		QVector<QPointF> pointsOnScreen;
		QVector<QString> valueString;

		for (int y = rectInView.y(); y < rectInView.y() + rectInView.height(); ++y) {
			for (int x = rectInView.x(); x < rectInView.x() + rectInView.width(); ++x) {
				QRgb pixValue = m_img.pixel(x, y);
				QPointF pointOnScreen = worldToScreen(QPointF(x + 0.5, y + 0.5), basePt, scale);
				pointsOnScreen.push_back(pointOnScreen);
				if (m_ch == 1) valueString.push_back(QString::number(qGray(pixValue)));
				else if (m_ch == 3) {
					valueString.push_back(QString::number(qRed(pixValue)));
					valueString.push_back(QString::number(qGreen(pixValue)));
					valueString.push_back(QString::number(qBlue(pixValue)));
				}
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
			if (m_ch == 1) {
				int strWidth = p.fontMetrics().width(valueString[i]);
				int strHeight = p.fontMetrics().height();
				QPointF fontPoint = QPointF(pointsOnScreen[i] - QPointF(strWidth / 2.0, -strHeight / 4.0));
				QRect fontRect(fontPoint.x() - 1, fontPoint.y() - strHeight + strHeight / 8, strWidth + 2, strHeight + strHeight / 8);
				p.setPen(Qt::gray);
				p.setBrush(QBrush(Qt::gray));
				p.drawRect(fontRect);
				p.setPen(Qt::black);
				p.drawText(fontPoint, valueString[i]);
			}
			else if (m_ch == 3) {
				int strWidth = p.fontMetrics().width(valueString[3 * i]);
				int strHeight = p.fontMetrics().height() * 3;
				QPointF fontPoint = QPointF(pointsOnScreen[i] - QPointF(strWidth / 2.0, strHeight / 2.0));
				QRect fontRect(fontPoint.x() - 1, fontPoint.y() - 1, strWidth + 2, strHeight + strHeight / 8);
				p.setPen(Qt::gray);
				p.setBrush(QBrush(Qt::gray));
				p.drawRect(fontRect);
				p.setPen(Qt::black);
				QPointF shiftPoint(0, strHeight / 3);
				p.drawText(fontPoint + shiftPoint, valueString[3 * i]);
				p.drawText(fontPoint + shiftPoint * 2, valueString[3 * i + 1]);
				p.drawText(fontPoint + shiftPoint * 3, valueString[3 * i + 2]);
			}
		}
	}
}

void PixViewer::RefPaintEvent()
{
	if (m_img.isNull()) return;
	PaintEvent(ui.widget_ref, ref_basePt, ref_scale, ref_img);
}

void PixViewer::CmpPaintEvent()
{
	if (m_img.isNull()) return;
	PaintEvent(ui.widget_cmp, cmp_basePt, cmp_scale, cmp_img);
}

void PixViewer::RefMouseMoveEvent(QMouseEvent* event)
{
	if (!isCmping || ref_img.isNull() || event == nullptr) return;

	if (m_buttonMiddlePressed) {
		QPoint sPoint1 = event->globalPos();
		QPoint widgetPoint = ui.widget->mapFromGlobal(sPoint1);
		QPointF mv = widgetPoint - m_lastPressedMousePt;
		ref_basePt += mv;//同步模式
		if (isSyncing) cmp_basePt += mv;
		m_lastPressedMousePt = widgetPoint;
		repaint();
	}
}

void PixViewer::CmpMouseMoveEvent(QMouseEvent* event)
{
	if (!isCmping || cmp_img.isNull() || event == nullptr) return;

	if (m_buttonMiddlePressed) {
		QPoint sPoint1 = event->globalPos();
		QPoint widgetPoint = ui.widget->mapFromGlobal(sPoint1);
		QPointF mv = widgetPoint - m_lastPressedMousePt;
		cmp_basePt += mv;//同步模式
		if (isSyncing) ref_basePt += mv;
		m_lastPressedMousePt = widgetPoint;
		repaint();
	}
}

void PixViewer::RefWheelMoveEvent(QWheelEvent* event)
{
	if (!isCmping || ref_img.isNull() || event == nullptr) return;

	QPoint angle = event->angleDelta();//返回滚轮度数的增量,有水平和垂直滚轮
	QPoint sPoint1 = event->globalPos();
	QPoint pt1 = ui.widget_ref->mapFromGlobal(sPoint1);
	QPointF p = worldToScreen(QPointF(0, 0), ref_basePt, ref_scale);//图像的(0,0)点对应的屏幕坐标，找到当前图像左上角点在初始坐标系下的坐标值

	QPointF pt2;
	if (angle.y() > 0) {
		pt2 = pt1 + (p - pt1) / e_scale;//滚一下放大0.618
		ref_scale /= e_scale;
		if (isSyncing) cmp_scale /= e_scale;
	}
	else {
		pt2 = pt1 + (p - pt1) * e_scale;
		ref_scale *= e_scale;
		if (isSyncing) cmp_scale *= e_scale;
	}
	ref_basePt = pt2;
	if (isSyncing) cmp_basePt = pt2;
	repaint();
}

void PixViewer::CmpWheelMoveEvent(QWheelEvent* event)
{
	if (!isCmping || cmp_img.isNull() || event == nullptr) return;

	QPoint angle = event->angleDelta();//返回滚轮度数的增量,有水平和垂直滚轮
	QPoint sPoint1 = event->globalPos();
	QPoint pt1 = ui.widget_cmp->mapFromGlobal(sPoint1);
	QPointF p = worldToScreen(QPointF(0, 0), cmp_basePt, cmp_scale);//图像的(0,0)点对应的屏幕坐标，找到当前图像左上角点在初始坐标系下的坐标值

	QPointF pt2;
	if (angle.y() > 0) {
		pt2 = pt1 + (p - pt1) / e_scale;//滚一下放大0.618
		cmp_scale /= e_scale;
		if (isSyncing) ref_scale /= e_scale;
	}
	else {
		pt2 = pt1 + (p - pt1) * e_scale;
		cmp_scale *= e_scale;
		if (isSyncing) ref_scale *= e_scale;
	}
	cmp_basePt = pt2;
	if (isSyncing) ref_basePt = pt2;
	repaint();
}

void PixViewer::ImgAdaptView(QImage img, QWidget* widget)
{
	int iw = img.width();
	int ih = img.height();
	int w = widget->width();
	int h = widget->height();
	double sw = 0.8 * w / (1.0 * iw);
	double sh = 0.8 * h / (1.0 * ih);
	double s = std::min(sw, sh);

	//计算左上角点在widget中坐标
	int x = w / 2 - (iw * s) / 2;
	int y = h / 2 - (ih * s) / 2;

	ref_basePt = QPoint(x, y);
	ref_scale = s;
	cmp_basePt = QPoint(x, y);
	cmp_scale = s;
	repaint();
}

//重构函数用于对比图像
QTransform PixViewer::getTransform(QPointF basePt, qreal scale)
{
	QTransform transform;
	//以m_scaleBasePt为中心，放大缩小
	transform.translate(basePt.x(), basePt.y());//坐标原点的重新定义，参数为重新定义的x坐标，y坐标
	transform.scale(scale, scale);
	return transform;
}

QPointF PixViewer::worldToScreen(QPointF pt, QPointF basePt, qreal scale)
{
	QTransform t = getTransform(basePt, scale);
	return t.map(pt);//返回变化后的点
}


QPointF PixViewer::screenToWorld(QPointF pt, QPointF basePt, qreal scale)
{
	QTransform t = getTransform(basePt, scale);
	QTransform t1 = t.inverted();//逆矩阵
	return t1.map(pt);//表示把点使用变换矩阵matrix进行转换
}

QVector<QLineF> PixViewer::getPixelBoundary(QRect rect, QPointF basePt, qreal scale)
{
	QVector<QLineF> lines;
	QPointF left, right, top, buttom;
	for (int y = rect.y(); y < rect.y() + rect.height(); ++y) {
		left = worldToScreen(QPointF(rect.x() - 1, y), basePt, scale);
		right = worldToScreen(QPointF(rect.x() + rect.width() + 1, y), basePt, scale);
		lines.append(QLineF(left, right));
	}
	for (int x = rect.x(); x < rect.x() + rect.width(); ++x)
	{
		top = worldToScreen(QPointF(x, rect.y() - 1), basePt, scale);
		buttom = worldToScreen(QPointF(x, rect.y() + rect.height() + 1), basePt, scale);
		lines.append(QLineF(top, buttom));
	}
	return lines;
}

void PixViewer::switchSyncMode()
{
	QString t = ui.bt_sync->text();
	if (ui.bt_sync->text() == QString::fromLocal8Bit(" 同步模式")) {
		isSyncing = false;
		ui.bt_sync->setStyleSheet("QPushButton#bt_sync{"
			"background-color: rgb(1,39,119);"
			"border:1px solid red;"
			"border-color: rgb(1,39,119);"
			"color:rgb(255, 255, 255);"
			"}");
		ui.bt_sync->setText(QString::fromLocal8Bit(" 异步模式"));
	}
	else if (ui.bt_sync->text() == QString::fromLocal8Bit(" 异步模式")) {
		isSyncing = true;
		ui.bt_sync->setText(QString::fromLocal8Bit(" 同步模式"));
		ui.bt_sync->setStyleSheet("QPushButton#bt_sync{"
			"background-color: rgb(221,117,161);"
			"border:1px solid red;"
			"border-color: rgb(221,117,161);"
			"color:rgb(255, 255, 255);"
			"}");
		cmp_basePt = ref_basePt;
		cmp_scale = ref_scale;
		repaint();
	}
}