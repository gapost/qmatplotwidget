#include "widget.h"
#include <cmath>

#define N 121

#define PI 3.141592653589793

Widget::Widget(QWidget *parent) : QMatPlotWidget(parent),
    x(N+1), y(N+1), z(N+1)
{
    for(int i=0; i<=N; ++i)
    {
        x[i] = 1.*i/N*6;
        y[i] = sin(x[i]*PI*2);
        z[i] = sin(x[i]*PI*2 + PI/2);
    }

    plot(x,y,QString("o-"));
    plot(x,z,QString("^--"));
    setTitle("QMatPlotWidget");
    setXlabel("ωt/2π");
    setYlabel("sin(ωt)");

    startTimer(20);
}

Widget::~Widget()
{

}

void Widget::timerEvent( QTimerEvent * )
{
    y.offset++;
    clear();
    plot(x,y,QString("o-"));
    plot(x,z,QString("^--"));
}
