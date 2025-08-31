#include "waveshow.h"

WaveShow::WaveShow(QCustomPlot *customPlot,QWidget *parent) : QWidget(parent)
{
    lineChart = customPlot;

    // 增加 8 组曲线
    for(int i = 0;i< 8;i++){
       lineChart->addGraph();
    }
    lineChart->graph(0)->setPen(QPen(QColor(0, 121, 255)));   // 科技蓝
    lineChart->graph(1)->setPen(QPen(QColor(120, 94, 240)));  // 深紫蓝
    lineChart->graph(2)->setPen(QPen(QColor(255, 110, 176))); // 粉紫色
    lineChart->graph(3)->setPen(QPen(QColor(0, 212, 190)));   // 薄荷绿
    lineChart->graph(4)->setPen(QPen(QColor(255, 153, 0)));   // 琥珀橙
    lineChart->graph(5)->setPen(QPen(QColor(102, 187, 235))); // 浅天蓝
    lineChart->graph(6)->setPen(QPen(QColor(255, 87, 87)));   // 珊瑚红
    lineChart->graph(7)->setPen(QPen(QColor(160, 214, 134))); // 嫩绿色

    lineChart->axisRect()->setupFullAxesBox();
    lineChart->yAxis->setRange(-100.0, 100.0);
    connect(lineChart->xAxis, SIGNAL(rangeChanged(QCPRange)), lineChart->xAxis2, SLOT(setRange(QCPRange)));
    connect(lineChart->yAxis, SIGNAL(rangeChanged(QCPRange)), lineChart->yAxis2, SLOT(setRange(QCPRange)));
    lineChart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // 添加竖直参考线
    QCPItemStraightLine *line = new QCPItemStraightLine(lineChart);
    line->setPen(QPen(Qt::red, 1, Qt::DashLine));
    line->point1->setType(QCPItemPosition::ptPlotCoords);
    line->point2->setType(QCPItemPosition::ptPlotCoords);
    double x0 = lineChart->xAxis->range().center();
    line->point1->setCoords(x0, QCPRange::minRange);
    line->point2->setCoords(x0, QCPRange::maxRange);
    vLines.append(line);

    // 为每条曲线添加 tracer
    for (int i = 0; i < 8; ++i)
    {
        QCPItemTracer *t = new QCPItemTracer(lineChart);
        t->setGraph(lineChart->graph(i));
        t->setStyle(QCPItemTracer::tsCircle);
        t->setPen(QPen(Qt::black));
        switch(i){
            case 0: t->setBrush(QColor(0, 121, 255)); break;
            case 1: t->setBrush(QColor(120, 94, 240)); break;
            case 2: t->setBrush(QColor(255, 110, 176)); break;
            case 3: t->setBrush(QColor(0, 212, 190)); break;
            case 4: t->setBrush(QColor(255, 153, 0)); break;
            case 5: t->setBrush(QColor(102, 187, 235)); break;
            case 6: t->setBrush(QColor(255, 87, 87)); break;
            case 7: t->setBrush(QColor(160, 214, 134)); break;
        }
        t->setSize(6);
        tracers.append(t);               // 存进容器
    }

    // x轴范围改变后
    connect(lineChart->xAxis,QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged),
            this,[this](const QCPRange &newRange){
        // 新的x轴范围
        m_viewWidth = newRange.upper - newRange.lower;
        if (!lineChart->viewport().contains(lineChart->mapFromGlobal(QCursor::pos())))
        {
            // 鼠标在图外：跟随最新数据
            double centerX = newRange.upper;  // 逻辑坐标的中心
            vLines[0]->point1->setCoords(centerX, QCPRange::minRange);
            vLines[0]->point2->setCoords(centerX, QCPRange::maxRange);
            updateTracer();
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
    for (int i = 0; i < tracers.size(); ++i) {
        if (lineChart->graph(i)->dataCount() > 0)  // 确认曲线有数据
        {
            tracers[i]->setGraphKey(x);
            tracers[i]->updatePosition();
        }
    }

    QStringList vLineValueStr;
    for (int i = 0; i < tracers.size(); ++i)
    {
        if (lineChart->graph(i)->dataCount() > 0)
            vLineValueStr << QString::number(tracers[i]->position->value());
        else
            vLineValueStr << "0"; // 或者 "0" 之类的占位
    }
    emit vLineValue(vLineValueStr);

    lineChart->replot(QCustomPlot::rpQueuedReplot);
}

void WaveShow::cleanData()
{
    // 1. 把四条曲线的数据全部清空
    for (int i = 0; i < 8; ++i)
        if (lineChart->graph(i))
            lineChart->graph(i)->data()->clear();   // 注意 data() 返回的是 QSharedPointer，要 ->clear()

    // 2. 恢复到默认坐标轴范围（按需自己定）
    lineChart->xAxis->setRange(-100.0, 100);               // x 轴 0~8
    lineChart->yAxis->setRange(-100.0, 100.0);

    lineChart->replot();                            // 重绘
}


void WaveShow::setLineVisible(int lineIndex, bool showFlag)
{
    lineChart->graph(lineIndex)->setVisible(showFlag);
    tracers[lineIndex]->setVisible(showFlag);
    lineChart->replot(QCustomPlot::rpQueuedReplot);
}


