#include "PixViewer.h"

PixViewer::PixViewer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	this->show();
	this->hide();
	InitViewer();

	setMouseTracking(true);//ʵ��mouseMoveEvent
	ui.centralWidget->setMouseTracking(true);
	ui.widget->setMouseTracking(true);

	ui.widget->installEventFilter(this);//�¼�������

	connect(ui.action_shot, &QAction::triggered, this, &PixViewer::shot);
	connect(m_shot, SIGNAL(endShot()), this, SLOT(endShotting()));
	connect(m_shot, SIGNAL(savePic()), this, SLOT(savePicture()));
	connect(m_shot, SIGNAL(reset()), this, SLOT(resetShot()));
	connect(m_shot, SIGNAL(sig_modify(QRect)), this, SLOT(modifyShot(QRect)));
}

PixViewer::~PixViewer()
{}

void PixViewer::InitViewer()
{
	e_scale = 0.707;
	m_scale = 1.0;
	m_scaleBasePt = QPointF(0, 0);
	isShotting = false;

	filename = "wallhaven-x6l1ed.jpg";
	m_img.load(filename);
	if (m_img.format() == QImage::Format_Mono || m_img.format() == QImage::Format_Indexed8) m_ch = 1;
	else m_ch = 3;

	ImgAdaptView();

	//״̬����ʼ��
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
	versionLabel->setText("v2.0.0 (Apr 7 2024)");//�汾��Ϣ

	ui.statusBar->addPermanentWidget(versionLabel);
	ui.statusBar->addWidget(posLabel);
	ui.statusBar->addWidget(scaleLabel);
	ui.statusBar->addWidget(sizeLabel);

	bgColor = QColor(240, 240, 240);

	m_buttonLeftPressed = false;
	m_buttonMiddlePressed = false;
	leftButtonType = ADJ_SLID;
}

//ͼƬ����Ӧ����
void PixViewer::ImgAdaptView()
{
	//������������ķŴ������ѡ��С�Ľ��зŴ�
	int iw = m_img.width();
	int ih = m_img.height();
	int w = ui.widget->width();
	int h = ui.widget->height();
	double sw = 0.8 * w / (1.0 * iw);
	double sh = 0.8 * h / (1.0 * ih);
	double s = std::min(sw, sh);

	//�������Ͻǵ���widget������
	int x = w / 2 - (iw * s) / 2;
	int y = h / 2 - (ih * s) / 2;

	m_scaleBasePt = QPoint(x, y);
	m_scale = s;
	repaint();
}

bool PixViewer::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui.widget && event->type() == QEvent::Paint)//�����¼�
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
	p.restore();//���ص�saveʱ��״̬����Ĭ������ϵ״̬

	if (isShotting) {
		//���Ʊ���
		QPointF pic_ltPoint = worldToScreen(QPointF(0, 0));
		QPointF pic_rbPoint = worldToScreen(QPointF(m_img.width(), m_img.height()));
		QRect w_pic((int)pic_ltPoint.x(), (int)pic_ltPoint.y(), (int)pic_rbPoint.x() - (int)pic_ltPoint.x(), (int)pic_rbPoint.y() - (int)pic_ltPoint.y());
		//QColor shadowColor = QColor(0, 0, 0, 100);
		QPainter p_bg(ui.widget);
		p_bg.setBrush(QBrush(QColor(0, 0, 0, 100))); // ����һ����ɫ�ɲ�
		p_bg.drawRect(w_pic);

		//���¾������������, ����
		w_ltPoint = worldToScreen(ltPoint);
		w_rbPoint = worldToScreen(rbPoint);
		m_wSelection = QRectF(w_ltPoint, w_rbPoint);
		QPainter p_select(ui.widget);
		p_select.setRenderHint(QPainter::Antialiasing, true);
		QPen pen;
		pen.setBrush(QBrush(QColor(65, 149, 235, 255)));
		pen.setStyle(Qt::DashLine);
		pen.setWidth(3);
		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		p_select.setPen(pen);
		p_select.drawRect(m_wSelection);

		//��ѡ������ͼƬ�����޸�
		QPainter p_tar(ui.widget);
		p_tar.setRenderHint(QPainter::Antialiasing, true);
		// ����ͼ�����������Ϊ��ͼ�񡣷��ص�ͼ���ǴӸ�ͼ���е�λ��(rectangle.x()��         
		// rectangle.y())���Ƶģ�����ʼ�վ��и������εĴ�С��
		QPoint ltP(ltPoint.x(), ltPoint.y());
		QPoint rbP(rbPoint.x(), rbPoint.y());
		m_selection = QRect(ltP, rbP);
		QImage targetImage = m_img.copy(m_selection);
		p_tar.drawImage(m_wSelection, targetImage);
	}

	if (m_scale > 30) {
		//1.����ӳ��ڵ�����
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
		//1.����ӳ��ڵ�����
		QPointF tl = screenToWorld(QPointF(ui.widget->x(), ui.widget->y()));
		QPointF br = screenToWorld(QPointF(ui.widget->size().width(), ui.widget->size().height()));

		//2.��ȡÿ���������ֵ
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

		//��������ֵ
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
	//��m_scaleBasePtΪ���ģ��Ŵ���С
	if (!m_img.isNull()) {
		transform.translate(m_scaleBasePt.x(), m_scaleBasePt.y());//����ԭ������¶��壬����Ϊ���¶����x���꣬y����
		transform.scale(m_scale, m_scale);
	}
	return transform;
}

//��ͼ�����굽�������ϵ��
QPointF PixViewer::worldToScreen(QPointF pt)
{
	QTransform t = getTransform();
	return t.map(pt);//���ر仯��ĵ�
}

//��ȡת�����󲢽���ת�������������굽ͼ������ϵ������
QPointF PixViewer::screenToWorld(QPointF pt)
{
	QTransform t = getTransform();
	QTransform t1 = t.inverted();//�����
	return t1.map(pt);//��ʾ�ѵ�ʹ�ñ任����matrix����ת��
}

//�߽���
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
		//��Ϊ����״̬��1 ��ק��ȡ����2 ��������
		if (leftButtonType != ADJ_SLID) {

		}
		else {
			QPoint sPoint1 = event->globalPos();
			w_rbPoint = ui.widget->mapFromGlobal(sPoint1);
			rbPoint = screenToWorld(w_rbPoint);
		}
	}
	if (m_buttonMiddlePressed) {
		QPointF mv = event->pos() - m_lastPressedMousePt;
		//mv *= m_scale;
		m_scaleBasePt += mv;
		m_lastPressedMousePt = event->pos();
	}

	//������ɣ�������������קû���⣬�Ҷ�ֵ���񲻶�
	if (!m_img.isNull()) {
		QPoint sPoint1 = event->globalPos();
		QPoint widgetPoint = ui.widget->mapFromGlobal(sPoint1);
		QPointF pt2 = screenToWorld(widgetPoint);
		
		curWPt = pt2;//���µ�ǰ�����꣬����ûɶ��
		
		//��ȡ���ص����꼰����ֵ
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


	//���ܾ���������
	double horiz01 = m_wSelection.width() * 0.1;
	double vert01 = m_wSelection.height() * 0.1;
	QRectF lt_ControlRect = QRectF(QPointF(m_wSelection.topLeft() - QPointF(horiz01, vert01)), QSize(horiz01 * 2, vert01 * 2));
	QRectF mt_ControlRect = QRectF(QPointF(m_wSelection.topLeft() + QPointF(horiz01, -vert01)), QSize(horiz01 * 6, vert01 * 2));
	QRectF rt_ControlRect = QRectF(QPointF(m_wSelection.topRight() - QPointF(horiz01, vert01)), QSize(horiz01 * 2, vert01 * 2));
	QRectF lb_ControlRect = QRectF(QPointF(m_wSelection.bottomLeft() - QPointF(horiz01, vert01)), QSize(horiz01 * 2, vert01 * 2));
	QRectF mb_ControlRect = QRectF(QPointF(m_wSelection.bottomLeft() + QPointF(horiz01, -vert01)), QSize(horiz01 * 6, vert01 * 2));
	QRectF rb_ControlRect = QRectF(QPointF(m_wSelection.bottomRight() - QPointF(horiz01, vert01)), QSize(horiz01 * 2, vert01 * 2));
	QRectF lm_ControlRect = QRectF(QPointF(m_wSelection.topLeft() + QPointF(-horiz01, vert01)), QSize(horiz01 * 2, vert01 * 6));
	QRectF rm_ControlRect = QRectF(QPointF(m_wSelection.topRight() + QPointF(-horiz01, vert01)), QSize(horiz01 * 2, vert01 * 6));

	// ���������״
	setCursor(Qt::ArrowCursor);
	QPoint sPoint1 = event->globalPos();
	QPoint mousePos = ui.widget->mapFromGlobal(sPoint1);
	if (m_wSelection.contains(mousePos)) setCursor(Qt::SizeAllCursor);//��ѡ��ľ����ڣ�ʮ�ּ�ͷ
	if (lt_ControlRect.contains(mousePos) || rb_ControlRect.contains(mousePos)) setCursor(Qt::SizeFDiagCursor);
	else if (mt_ControlRect.contains(mousePos) || mb_ControlRect.contains(mousePos)) setCursor(Qt::SizeVerCursor);
	else if (lm_ControlRect.contains(mousePos) || rm_ControlRect.contains(mousePos)) setCursor(Qt::SizeHorCursor);
	else if (rt_ControlRect.contains(mousePos) || lb_ControlRect.contains(mousePos)) setCursor(Qt::SizeBDiagCursor);

	repaint();
}

void PixViewer::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && isShotting) {
		m_buttonLeftPressed = true;
		QPoint sPoint1 = event->globalPos();
		w_ltPoint = ui.widget->mapFromGlobal(sPoint1);
		ltPoint = screenToWorld(w_ltPoint);
	}
	else if (event->button() == Qt::LeftButton && !isShotting) {
		m_buttonMiddlePressed = true;
	}
	else if (event->button() == Qt::MidButton) {
		
	}
	else if (event->button() == Qt::RightButton && isShotting) {
		m_buttonMiddlePressed = true;
	}
	m_lastPressedMousePt = event->pos();
}

void PixViewer::mouseReleaseEvent(QMouseEvent* event)
{
	m_buttonLeftPressed = false;
	m_buttonMiddlePressed = false;
	leftButtonType = ADJ_SLID;

	if (event->button() == Qt::LeftButton && isShotting) {
		m_selection = QRect(QPoint(qMin(ltPoint.x(), rbPoint.x()), qMin(ltPoint.y(), rbPoint.y())), 
			QPoint(qMax(ltPoint.x(), rbPoint.x()), qMax(ltPoint.y(), rbPoint.y())));
		//���ε��ӽ���
		m_shot->setRect(m_selection);
		repaint();
	}
}

void PixViewer::wheelEvent(QWheelEvent* event)//��ȡÿ�α任��ԭ���λ�ü��Ŵ���
{
	if (!m_img.isNull()) {
		QPoint angle = event->angleDelta();//���ع��ֶ���������,��ˮƽ�ʹ�ֱ����

		QPoint sPoint1 = event->globalPos();
		QPoint pt1 = ui.widget->mapFromGlobal(sPoint1);

		QPointF p = worldToScreen(QPointF(0, 0));//ͼ���(0,0)���Ӧ����Ļ���꣬�ҵ���ǰͼ�����Ͻǵ��ڳ�ʼ����ϵ�µ�����ֵ

		QPointF pt2;
		if (angle.y() > 0) {
			pt2 = pt1 + (p - pt1) / e_scale;//��һ�·Ŵ�0.618
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

	m_shot->setParent(this);//ָ��������
	m_shot->setWindowFlags(m_shot->windowFlags() | Qt::Dialog);
	QPoint dis = QPoint(this->geometry().x() + 10, this->geometry().y() + this->height() - m_shot->height() - 38);
	m_shot->move(dis);//move��ԭ���Ǹ����ڵ����Ͻǣ�  ���û�и����ڣ������漴Ϊ������

	m_shot->show();
}

void PixViewer::endShotting()
{
	isShotting = false;
}

void PixViewer::savePicture()
{
	QString filename1 = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("Images (*.png *.bmp *.jpg *.tif)")); //ѡ��·��
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