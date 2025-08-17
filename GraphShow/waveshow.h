#ifndef WAVESHOW_H
#define WAVESHOW_H

#include <QWidget>
#include "qcustomplot.h"
#include <QStringList>

class WaveShow : public QWidget
{
    Q_OBJECT
public:
    explicit WaveShow(QCustomPlot *customPlot, QWidget *parent = nullptr);

    void addData(int lineIndex, double key, double value);
    void cleanData();
    void autoResizeY();
    void setLineVisible(int lineIndex,bool showFlag);
    void updateTracer();
public slots:

signals:
    void vLineValue(QStringList);

private:
    QCustomPlot *lineChart;
    QTimer dataTimer;
    QCPItemTracer *itemDemoPhaseTracer;
    int m_viewWidth = 20;

    QVector<QCPItemStraightLine*> vLines;
    QVector<QCPItemTracer*>       tracers;
    QVector<QCPItemText*>         labels;

//    QCPItemStraightLine *vLine;   // 竖直线
//    QCPItemTracer       *tracer;  // 自动贴曲线
//    QCPItemText         *label;   // 显示值
};

#endif // WAVESHOW_H
