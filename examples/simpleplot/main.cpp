#include "QMatPlotWidget.h"
#include <QApplication>

typedef QVector<qreal> Vector;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMatPlotWidget w;
    w.setWindowTitle("QMatPlotWidget Example: simpleplot");
    w.show();

    const int N = 40;
    Vector x(N+1), y1(N+1), y2(N+1);
    for(int i=0; i<=N; ++i)
    {
        x[i] = -1. + 2.*i/N;
        y1[i] = x[i]*x[i];
        y2[i] = y1[i]*x[i];
    }

    w.plot(x,y1,"o-");
    w.plot(x,y2,"^:");
    w.plot(Vector({0, 0}),Vector({-1, 1}),"k-");
    w.plot(Vector({-1, 1}),Vector({0, 0}),"k-");
    w.setXlabel("x");
    w.setYlabel("y");
    w.setTitle("y1(x)=x^2, y2(x)=x^3");


    return app.exec();
}
