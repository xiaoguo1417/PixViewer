#pragma once

#include <QtWidgets/QMainWindow>
#include <QPainter>
#include <QPaintEvent>
#include <QLabel>
#include <QDebug>
#include "ui_PixViewer.h"

class PixViewer : public QMainWindow
{
    Q_OBJECT

public:
    PixViewer(QWidget *parent = nullptr);
    ~PixViewer();
    bool eventFilter(QObject* watched, QEvent* event);

protected:
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

private:
    void myPaintEvent();
    QTransform getTransform();
    QVector<QLineF> getPixelBoundary(QRect rect);
    QPointF worldToScreen(QPointF pt);
    QPointF screenToWorld(QPointF pt);
    void InitViewer();

    Ui::PixViewerClass ui;
    QImage m_img;//全局图片
    QString filename;//图片位置,name
    qreal m_scale;
    qreal e_scale;
    QPointF m_scaleBasePt;
    QPointF m_lastPressedMousePt;
    QRect m_selection;
    QRect m_refined;
    bool isShotting;
    QPointF curWPt;//当前点在图像中的坐标
    QPointF tlPt;//矩形区域的左上角点
    QPointF ltPoint;
    QPointF rbPoint;
    QPointF w_ltPoint;
    QPointF w_rbPoint;

    //状态栏
    QLabel* sizeLabel;
    QLabel* versionLabel;
    QLabel* posLabel;
    QLabel* scaleLabel;

    QColor bgColor;//背景色

    bool m_buttonLeftPressed;
    bool m_buttonMiddlePressed;
};
