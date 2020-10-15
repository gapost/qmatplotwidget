#ifndef WIDGET_H
#define WIDGET_H


#include <QVector>

#include "QMatPlotWidget.h"

class Vector : public QVector<double>
{
public:
    explicit Vector(int n) : QVector<double>(n), offset(0)
    {}
    explicit Vector(const Vector& v) : QVector<double>(v), offset(v.offset)
    {}
    int offset;

    double operator[](int i) const
    {
        return at((i + offset) % size());
    }
    double& operator[](int i)
    {
        return data()[i];
    }
};


class Widget : public QMatPlotWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

protected:
    void timerEvent( QTimerEvent *e ) override;

    Vector x,y,z;
};

#endif // WIDGET_H
