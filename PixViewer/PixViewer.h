#pragma once

#include <QtWidgets/QMainWindow>
#include <QPainter>
#include <QPaintEvent>
#include <QLabel>
#include <QDebug>
#include <QFileDialog>
#include "ui_PixViewer.h"
#include "ImageShot.h"

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
    enum ADJ_TYPE//左键按住时调整的分类
    {
        ADJ_SLID,                           // 滑动获取初始区域，默认
        ADJ_NOSCALE,                        // 整体平移，无缩放操作
        ADJ_LEFTTOP,                        // 左上角缩放
        ADJ_LEFTBOTTOM,                     // 左下角缩放
        ADJ_RIGTHTOP,                       // 右上角缩放
        ADJ_RIGHTBOTTOM,                    // 右下角缩放
        ADJ_MIDUP,                          // 上中角缩放
        ADJ_MIDBELLOW,                      // 下中缩放
        ADJ_MIDLEFT,                        // 左中角缩放
        ADJ_MIDRIGHT                        // 右中角缩放
    };

    void myPaintEvent();
    QTransform getTransform();
    QVector<QLineF> getPixelBoundary(QRect rect);
    QPointF worldToScreen(QPointF pt);
    QPointF screenToWorld(QPointF pt);
    void InitViewer();
    void ImgAdaptView();

    Ui::PixViewerClass ui;
    QImage m_img;//全局图片
    int m_ch;//图片通道数量
    QString filename;//图片位置,name
    qreal m_scale;
    qreal e_scale;
    QPointF m_scaleBasePt;
    QPointF m_lastPressedMousePt;
    QRect m_refined;
    bool isShotting;
    QPointF curWPt;//当前点在图像中的坐标
    QPointF tlPt;//矩形区域的左上角点

    //状态栏
    QLabel* sizeLabel;
    QLabel* versionLabel;
    QLabel* posLabel;
    QLabel* scaleLabel;

    QColor bgColor;//背景色

    bool m_buttonLeftPressed;
    bool m_buttonMiddlePressed;

    //截图相关
    ImageShot* m_shot = new ImageShot();
    QPointF ltPoint;        //ROI左上角点，图像坐标系
    QPointF rbPoint;        //ROI右下角点
    QPointF w_ltPoint;      //ROI左上角点，widget坐标系
    QPointF w_rbPoint;
    QRect m_selection;      //ROI矩形区域，图像坐标系
    QRectF m_wSelection;    //widget坐标系
    ADJ_TYPE leftButtonType;//左键按下调整的类型

private slots:
    void shot();
    void endShotting();
    void savePicture();
    void resetShot();
    void modifyShot(QRect rect);
};
