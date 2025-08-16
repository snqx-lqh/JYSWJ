#ifndef WAVESHOW_H
#define WAVESHOW_H

#include <QWidget>
#include "qcustomplot.h"

class WaveShow : public QWidget
{
    Q_OBJECT
public:
    explicit WaveShow(QCustomPlot *customPlot, QWidget *parent = nullptr);

    void addData(int lineIndex, double key, double value);
    void cleanData();
    void autoResizeY();
    void setLineVisible(int lineIndex,bool showFlag);
public slots:

signals:

private:
    QCustomPlot *lineChart;
    QTimer dataTimer;
    QCPItemTracer *itemDemoPhaseTracer;
    int m_viewWidth = 20;
};

#endif // WAVESHOW_H
