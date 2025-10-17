#ifndef WIDGET_H
#define WIDGET_H


#include <QVector>
#include <QElapsedTimer>
#include <QRandomGenerator>

#include "QMatPlotWidget.h"

// explicitly shared vector class with "offset indexing"
class SharedVector
{
    typedef QVector<double> vector_t;
    struct myshareddata : QSharedData {
        vector_t v;
        int offset_;
        explicit myshareddata(int n) : v(n), offset_(0)
        {}
        explicit myshareddata(const myshareddata& o) : QSharedData(o), v(o.v), offset_(o.offset_)
        {}
    };
    QExplicitlySharedDataPointer<myshareddata> d_ptr;

public:

    explicit SharedVector(int n) : d_ptr(new myshareddata(n))
    {}
    explicit SharedVector(const SharedVector& v) : d_ptr(v.d_ptr)
    {}

    int size() const { return d_ptr->v.size(); }
    double & operator[](int i) {
        int j = ((uint)i + offset()) % size();
        return d_ptr->v[j];
    }
    const double & operator[](int i) const {
        int j = ((uint)i + offset()) % size();
        return d_ptr->v[j];
    }
    int offset() const { return d_ptr->offset_; }
    void incOffset() { d_ptr->offset_++; }
    void decOffset() { d_ptr->offset_--; }
};

// explicitly shared circular buffer class
class CircularBuffer
{
    typedef QVector<double> vector_t;
    struct myshareddata : QSharedData {
        vector_t v;
        int n;
        int idx;
        explicit myshareddata(int sz) : v(sz, 0.), n(0), idx(0)
        {}
        explicit myshareddata(const myshareddata& o) : QSharedData(o),
            v(o.v), n(o.n), idx(o.idx)
        {}
        void push(const double& d) {
            v[idx++] = d;
            idx %= v.size();
            if (n<v.size()) n++;
        }
        int didx(int i) const {
            if (n<v.size()) return i;
            else return (idx + i) % n;
        }
    };
    QExplicitlySharedDataPointer<myshareddata> d_ptr;

public:

    explicit CircularBuffer(int n) : d_ptr(new myshareddata(n))
    {}
    explicit CircularBuffer(const CircularBuffer& v) : d_ptr(v.d_ptr)
    {}

    int size() const { return d_ptr->n; }
//    double & operator[](int i) {
//        int j = d_ptr->didx(i);
//        return d_ptr->v[j];
//    }
    const double & operator[](int i) const {
        int j = d_ptr->didx(i);
        return d_ptr->v[j];
    }
    void push(const double& d) {
        d_ptr->push(d);
    }

};


class Widget : public QMatPlotWidget
{
    Q_OBJECT

    //SharedVector x,y,z;
    CircularBuffer x,y,z;
    int toff;
    QElapsedTimer clock_;
    QVector<float> t_;

    double kt,ky;

    QRandomGenerator rng;

public:
    Widget(QWidget *parent = 0);
    ~Widget();

protected:
    void timerEvent( QTimerEvent *e ) override;


};

#endif // WIDGET_H
