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



    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
//    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
//    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
    lineChart->replot();
    lineChart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // 竖直线 new QVector<bool>();

    // 自动贴曲线的 tracer
    QCPItemStraightLine *line = new QCPItemStraightLine(lineChart);
    line->setPen(QPen(Qt::red, 1, Qt::DashLine));
    line->point1->setType(QCPItemPosition::ptPlotCoords);
    line->point2->setType(QCPItemPosition::ptPlotCoords);
    double x0 = lineChart->xAxis->range().center();
    line->point1->setCoords(x0, QCPRange::minRange);
    line->point2->setCoords(x0, QCPRange::maxRange);
    vLines.append(line);

    // 初始化时一次性 new 4 个
    for (int i = 0; i < 4; ++i)
    {
        QCPItemTracer *t = new QCPItemTracer(lineChart);
        t->setGraph(lineChart->graph(i));
        t->setStyle(QCPItemTracer::tsCircle);
        t->setPen(QPen(Qt::black));
        switch(i){
            case 0: t->setBrush(QColor(0, 170, 255)); break;
            case 1: t->setBrush(QColor(255, 85, 0)); break;
            case 2: t->setBrush(QColor(85, 255, 0)); break;
            case 3: t->setBrush(QColor(0, 255, 255)); break;
        }
        t->setSize(6);
        tracers.append(t);               // 存进容器
    }

    connect(lineChart->xAxis,QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
            this,[this](const QCPRange &newRange){
                m_viewWidth = newRange.upper - newRange.lower;
                if (!lineChart->viewport().contains(lineChart->mapFromGlobal(QCursor::pos())))
                {
                    // 鼠标在图外：跟随最新数据
                    double centerX = newRange.upper;  // 逻辑坐标的中心
                    vLines[0]->point1->setCoords(centerX, QCPRange::minRange);
                    vLines[0]->point2->setCoords(centerX, QCPRange::maxRange);
                    updateTracer();
                }
                else
                {
                    ;
                }
            });

    connect(lineChart, &QCustomPlot::mouseMove, this, [this](QMouseEvent *ev){
        if (lineChart->viewport().contains(ev->pos()))
            {
                // 鼠标在图内：跟随鼠标
                double x = lineChart->xAxis->pixelToCoord(ev->pos().x());
                vLines[0]->point1->setCoords(x, QCPRange::minRange);
                vLines[0]->point2->setCoords(x, QCPRange::maxRange);
                updateTracer();
            }
            else
            {
                ;
            }

      });
}

void WaveShow::addData(int lineIndex, double key, double value)
{
    lineChart->graph(lineIndex)->addData(key,value);
    lineChart->replot();

    lineChart->xAxis->setRange(key, m_viewWidth , Qt::AlignRight);
}


void WaveShow::updateTracer()
{
    double x = vLines[0]->point1->key();   // 竖线当前 x 坐标
    tracers[0]->setGraphKey(x);
    tracers[0]->updatePosition();          // 让 tracer 贴到曲线
    tracers[1]->setGraphKey(x);
    tracers[1]->updatePosition();          // 让 tracer 贴到曲线
    tracers[2]->setGraphKey(x);
    tracers[2]->updatePosition();          // 让 tracer 贴到曲线
    tracers[3]->setGraphKey(x);
    tracers[3]->updatePosition();          // 让 tracer 贴到曲线

    QStringList vLineValueStr;
    vLineValueStr<<QString::number(tracers[0]->position->value())
                 <<QString::number(tracers[1]->position->value())
                 <<QString::number(tracers[2]->position->value())
                 <<QString::number(tracers[3]->position->value());
    emit vLineValue(vLineValueStr);

    lineChart->replot(QCustomPlot::rpQueuedReplot);
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
    tracers[lineIndex]->setVisible(showFlag);
    lineChart->replot(QCustomPlot::rpQueuedReplot);
}


