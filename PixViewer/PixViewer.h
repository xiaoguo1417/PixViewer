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
    enum ADJ_TYPE//�����סʱ�����ķ���
    {
        ADJ_SLID,                           // ������ȡ��ʼ����Ĭ��
        ADJ_NOSCALE,                        // ����ƽ�ƣ������Ų���
        ADJ_LEFTTOP,                        // ���Ͻ�����
        ADJ_LEFTBOTTOM,                     // ���½�����
        ADJ_RIGTHTOP,                       // ���Ͻ�����
        ADJ_RIGHTBOTTOM,                    // ���½�����
        ADJ_MIDUP,                          // ���н�����
        ADJ_MIDBELLOW,                      // ��������
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
    ImageShot* m_shot = new ImageShot();
    QPointF ltPoint;        //ROI���Ͻǵ㣬ͼ������ϵ
    QPointF rbPoint;        //ROI���½ǵ�
    QPointF w_ltPoint;      //ROI���Ͻǵ㣬widget����ϵ
    QPointF w_rbPoint;
    QRect m_selection;      //ROI��������ͼ������ϵ
    QRectF m_wSelection;    //widget����ϵ
    ADJ_TYPE leftButtonType;//������µ���������

private slots:
    void shot();
    void endShotting();
    void savePicture();
    void resetShot();
    void modifyShot(QRect rect);
};
