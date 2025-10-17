#include "widget.h"
#include <cmath>


#define M 32
#define PERIOD 10
// #define N (3*1000/PERIOD)
#define N (300)

Widget::Widget(QWidget *parent) : QMatPlotWidget(parent),
    x(N+1), y(N+1), z(N+1), toff(0), t_(M)
{
//    for(int i=0; i<=N; ++i)
//    {
//        x[i] = 1.*i/N*6;
//        y[i] = sin(x[i]*PI*2);
//        z[i] = sin(x[i]*PI*2 + PI/2);
//    }

    kt = 1.*PERIOD/1000;
    ky = 2.*M_PI*3./N;

    plot(x,y,QString("o-"));
    plot(x,z,QString("^--"));
    setTitle("QMatPlotWidget, FPS=");
    setXlabel("t (s)");
    setYlabel("sin(ωt)");
    // setXlim(QPointF(0.,6.));
    //setYlim(QPointF(-1.1,1.1));
    setGrid(1);

    t_.fill(0.f);
    clock_.start();
    startTimer(PERIOD);
}

Widget::~Widget()
{

}

void Widget::timerEvent( QTimerEvent * )
{
    t_[toff & (M-1)] = 1.e-9f * clock_.nsecsElapsed();

    //y.incOffset();

    //if ((toff % 3)==0) z.decOffset();

    x.push(kt*toff);
    y.push(sin(0.02*ky*toff)*sin(ky*toff));
    z.push(0.3*(rng.generateDouble()-0.5));

    replot();

    if ((toff % 500)==0) {
        int i2 = toff & (M-1);
        int i1 = (i2+1) & (M-1);
        float FPS = 1.f*(M-1)/(t_[i2] - t_[i1]);
        setTitle(QString("QMatPlotWidget, FPS=%1").arg(FPS,0,'f',1));
    }

    toff++;
}
