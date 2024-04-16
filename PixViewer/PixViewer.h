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
    enum ADJ_TYPE//�����סʱ�����ķ���
    {
        ADJ_SLID,                           // ������ȡ��ʼ����Ĭ��
        ADJ_MOVE,                           // ����ƽ�ƣ������Ų���
        ADJ_LEFTTOP,                        // ���Ͻ�����
        ADJ_LEFTBOTTOM,                     // ���½�����
        ADJ_RIGTHTOP,                       // ���Ͻ�����
        ADJ_RIGHTBOTTOM,                    // ���½�����
        ADJ_MIDTOP,                         // ���н�����
        ADJ_MIDBOTTOM,                      // ��������
        ADJ_MIDLEFT,                        // ���н�����
        ADJ_MIDRIGHT                        // ���н�����
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
    QImage m_img;//ȫ��ͼƬ
    int m_ch;//ͼƬͨ������
    QString filename;//ͼƬλ��,name
    qreal m_scale;
    qreal e_scale;
    QPointF m_scaleBasePt;
    QPointF m_lastPressedMousePt;
    QRect m_refined;
    bool isShotting;
    QPointF curWPt;//��ǰ����ͼ���е�����
    QPointF tlPt;//������������Ͻǵ�

    //״̬��
    QLabel* sizeLabel;
    QLabel* versionLabel;
    QLabel* posLabel;
    QLabel* scaleLabel;

    QColor bgColor;//����ɫ

    bool m_buttonLeftPressed;
    bool m_buttonMiddlePressed;

    //��ͼ���
    void AdjSelectionRect(QPointF mousePos);
    void AdjControlRect();

    ImageShot* m_shot = new ImageShot();
    QPointF ltPoint;        //ROI���Ͻǵ㣬ͼ������ϵ
    QPointF rbPoint;        //ROI���½ǵ�
    QPointF w_ltPoint;      //ROI���Ͻǵ㣬widget����ϵ
    QPointF w_rbPoint;
    QRect m_selection;      //ROI��������ͼ������ϵ
    QRectF m_wSelection;    //widget����ϵ
    ADJ_TYPE leftButtonType;//������µ���������

    //�Ա����
    void PaintEvent(QWidget* widget, QPointF basePt, qreal scale, QImage img);
    void RefPaintEvent();
    void CmpPaintEvent();
    void RefMouseMoveEvent(QMouseEvent* event);
    void CmpMouseMoveEvent(QMouseEvent* event);
    void RefWheelMoveEvent(QWheelEvent* event);
    void CmpWheelMoveEvent(QWheelEvent* event);
    void ImgAdaptView(QImage img, QWidget* widget);//����ͼƬ����λ��
    QTransform getTransform(QPointF basePt, qreal scale);
    QPointF worldToScreen(QPointF pt, QPointF basePt, qreal scale);
    QPointF screenToWorld(QPointF pt, QPointF basePt, qreal scale);
    QVector<QLineF> getPixelBoundary(QRect rect, QPointF basePt, qreal scale);

    bool isCmping;
    bool isSyncing;//ͬ��ģʽ
    qreal ref_scale;//�ο�ͼ�񼰶Ա�ͼ��ķŴ����������
    qreal cmp_scale;
    QPointF ref_basePt;
    QPointF cmp_basePt;
    QImage ref_img;
    QImage cmp_img;
    int cmp_ch;//ͼƬͨ������
    int ref_ch;

    //��������
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
