#include "waveshow.h"

WaveShow::WaveShow(QCustomPlot *customPlot,QWidget *parent) : QWidget(parent)
{
    lineChart = customPlot;

    // generate some data:
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i];  // let's plot a quadratic function
    }
    // create graph and assign data to it:
    lineChart->addGraph();
    lineChart->graph(0)->setPen(QPen(QColor(0, 170, 255)));
    lineChart->addGraph();
    lineChart->graph(1)->setPen(QPen(QColor(255, 85, 0)));
    lineChart->addGraph();
    lineChart->graph(2)->setPen(QPen(QColor(85, 255, 0)));
    lineChart->addGraph();
    lineChart->graph(3)->setPen(QPen(QColor(0, 255, 255)));

//    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
//    timeTicker->setTimeFormat("%h:%m:%s");
//    lineChart->xAxis->setTicker(timeTicker);
    lineChart->axisRect()->setupFullAxesBox();
    lineChart->yAxis->setRange(-100.0, 100.0);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(lineChart->xAxis, SIGNAL(rangeChanged(QCPRange)), lineChart->xAxis2, SLOT(setRange(QCPRange)));
    connect(lineChart->yAxis, SIGNAL(rangeChanged(QCPRange)), lineChart->yAxis2, SLOT(setRange(QCPRange)));

    connect(lineChart->xAxis,QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
            this,[this](const QCPRange &newRange){
                m_viewWidth = newRange.upper - newRange.lower;
            });

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
//    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
//    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
    lineChart->replot();
    lineChart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    connect(lineChart->xAxis,
            QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
            this, [this](const QCPRange &newRange){
                // 只记录宽度，不修正位置
                m_viewWidth = newRange.upper - newRange.lower;
            });
}

void WaveShow::addData(int lineIndex, double key, double value)
{
    lineChart->graph(lineIndex)->addData(key,value);
    lineChart->replot();

    lineChart->xAxis->setRange(key, m_viewWidth , Qt::AlignRight);
}

void WaveShow::cleanData()
{
    // 1. 把四条曲线的数据全部清空
    for (int i = 0; i < 4; ++i)
        if (lineChart->graph(i))
            lineChart->graph(i)->data()->clear();   // 注意 data() 返回的是 QSharedPointer，要 ->clear()

    // 2. 恢复到默认坐标轴范围（按需自己定）
    lineChart->xAxis->setRange(-100.0, 100);               // x 轴 0~8
    lineChart->yAxis->setRange(-100.0, 100.0);

    lineChart->replot();                            // 重绘
}

void WaveShow::autoResizeY()
{

}

void WaveShow::setLineVisible(int lineIndex, bool showFlag)
{
    lineChart->graph(lineIndex)->setVisible(showFlag);
    lineChart->replot(QCustomPlot::rpQueuedReplot);
}


