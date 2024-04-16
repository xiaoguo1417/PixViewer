#pragma once

#include <QtWidgets/QMainWindow>
#include <QPainter>
#include <QPaintEvent>
#include <QLabel>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_PixViewer.h"
#include "ImageShot.h"
#include "About.h"

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
        ADJ_MOVE,                           // 整体平移，无缩放操作
        ADJ_LEFTTOP,                        // 左上角缩放
        ADJ_LEFTBOTTOM,                     // 左下角缩放
        ADJ_RIGTHTOP,                       // 右上角缩放
        ADJ_RIGHTBOTTOM,                    // 右下角缩放
        ADJ_MIDTOP,                         // 上中角缩放
        ADJ_MIDBOTTOM,                      // 下中缩放
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
    void InitStatusBar();

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
    void AdjSelectionRect(QPointF mousePos);
    void AdjControlRect();

    ImageShot* m_shot = new ImageShot();
    QPointF ltPoint;        //ROI左上角点，图像坐标系
    QPointF rbPoint;        //ROI右下角点
    QPointF w_ltPoint;      //ROI左上角点，widget坐标系
    QPointF w_rbPoint;
    QRect m_selection;      //ROI矩形区域，图像坐标系
    QRectF m_wSelection;    //widget坐标系
    ADJ_TYPE leftButtonType;//左键按下调整的类型

    //对比相关
    void PaintEvent(QWidget* widget, QPointF basePt, qreal scale, QImage img);
    void RefPaintEvent();
    void CmpPaintEvent();
    void RefMouseMoveEvent(QMouseEvent* event);
    void CmpMouseMoveEvent(QMouseEvent* event);
    void RefWheelMoveEvent(QWheelEvent* event);
    void CmpWheelMoveEvent(QWheelEvent* event);
    void ImgAdaptView(QImage img, QWidget* widget);//调整图片所在位置
    QTransform getTransform(QPointF basePt, qreal scale);
    QPointF worldToScreen(QPointF pt, QPointF basePt, qreal scale);
    QPointF screenToWorld(QPointF pt, QPointF basePt, qreal scale);
    QVector<QLineF> getPixelBoundary(QRect rect, QPointF basePt, qreal scale);

    bool isCmping;
    bool isSyncing;//同步模式
    qreal ref_scale;//参考图像及对比图像的放大倍数及坐标点
    qreal cmp_scale;
    QPointF ref_basePt;
    QPointF cmp_basePt;
    QImage ref_img;
    QImage cmp_img;
    int cmp_ch;//图片通道数量
    int ref_ch;

    //矩形区域
    QRectF lt_ControlRect;
    QRectF mt_ControlRect;
    QRectF rt_ControlRect;
    QRectF lb_ControlRect;
    QRectF mb_ControlRect;
    QRectF rb_ControlRect;
    QRectF lm_ControlRect;
    QRectF rm_ControlRect;

private slots:
    void OpenPic();

    void shot();
    void endShotting();
    void savePicture();
    void resetShot();
    void modifyShot(QRect rect);

    void cmp();
    void switchSyncMode();
    void OnBtBackClicked();
    void OnBtOpen1Clicked();
    void OnBtOpen2Clicked();
};
