#ifndef DATAADAPTORS_H
#define DATAADAPTORS_H

#include <QPointF>
#include <QRectF>

struct AbstractDataSeriesAdaptor
{
    virtual ~AbstractDataSeriesAdaptor() {}
    virtual int size() const = 0;
    virtual QPointF sample(int i) const = 0;
    virtual QRectF boundingRect() const = 0;
};

struct AbstractErrorBarAdaptor
{
    virtual ~AbstractErrorBarAdaptor() {}
    virtual int size() const = 0;
    virtual QPointF sample(int i) const = 0;
    virtual QPointF interval(int i) const = 0;
    virtual QRectF boundingRect() const = 0;
    virtual QRectF errorBoundingRect() const = 0;
};

struct AbstractImageAdaptor
{
    virtual ~AbstractImageAdaptor() {}
    virtual int rows() const = 0;
    virtual int columns() const = 0;
    virtual double value(int k) const = 0;
    virtual QPointF xlim() const = 0;
    virtual QPointF ylim() const = 0;
    virtual QPointF zlim() const = 0;
};

template<class VectorType>
class ImageAdaptor : public AbstractImageAdaptor
{
    VectorType x_, y_, z_;
    int cols_;
    bool zonly_;

public:
    explicit ImageAdaptor(const VectorType &z, int columns)
        : z_(z)
        , cols_(columns)
        , zonly_(true)
    {}
    ImageAdaptor(const VectorType &x, const VectorType &y, const VectorType &z, int columns)
        : x_(x)
        , y_(y)
        , z_(z)
        , cols_(columns)
        , zonly_(false)
    {}
    ImageAdaptor(const ImageAdaptor &other) = default;
    int rows() const override { return z_.size() / cols_; }
    int columns() const override { return cols_; }
    double value(int k) const override { return z_[k]; }
    QPointF xlim() const override
    {
        if (zonly_) {
            return QPointF(0, cols_);
        } else {
            return QPointF(x_[0], x_[x_.size() - 1]);
        }
    }
    QPointF ylim() const override
    {
        if (zonly_) {
            return QPointF(0, rows());
        } else {
            return QPointF(y_[0], y_[y_.size() - 1]);
        }
    }
    QPointF zlim() const override
    {
        if (!z_.size())
            return QPointF(0., 1.);
        double vmin = z_[0], vmax = z_[0];
        for (int i = 1; i < z_.size(); ++i) {
            vmin = std::min(vmin, z_[i]);
            vmax = std::max(vmax, z_[i]);
        }
        return QPointF(vmin, vmax);
    }
};

template<class V_>
class DataSeriesAdaptor : public AbstractDataSeriesAdaptor
{
    V_ vx;
    V_ vy;
    bool yonly_;

public:
    DataSeriesAdaptor(const V_ &y)
        : vy(y)
        , yonly_(true)
    {}
    DataSeriesAdaptor(const V_ &x, const V_ &y)
        : vx(x)
        , vy(y)
        , yonly_(false)
    {}
    DataSeriesAdaptor(const DataSeriesAdaptor &other) = default;
    int size() const override { return yonly_ ? vy.size() : qMin(vx.size(), vy.size()); }
    QPointF sample(int i) const override
    {
        return yonly_ ? QPointF(i, vy[i]) : QPointF(vx[i], vy[i]);
    }
    QRectF boundingRect() const override
    {
        if (!size())
            return QRectF();

        if (yonly_) {
            qreal y1(vy[0]), y2(y1);
            for (int i = 1; i < size(); ++i) {
                if (vy[i] < y1)
                    y1 = vy[i];
                else if (vy[i] > y2)
                    y2 = vy[i];
            }
            return QRectF(0, y1, size() - 1, y2 - y1);
        }

        qreal x1(vx[0]), x2(x1);
        qreal y1(vy[0]), y2(y1);
        for (int i = 1; i < size(); ++i) {
            if (vx[i] < x1)
                x1 = vx[i];
            else if (vx[i] > x2)
                x2 = vx[i];
            if (vy[i] < y1)
                y1 = vy[i];
            else if (vy[i] > y2)
                y2 = vy[i];
        }
        return QRectF(x1, y1, x2 - x1, y2 - y1);
    }
};

template<class VectorType>
class StairsAdaptor : public AbstractDataSeriesAdaptor
{
    VectorType x_, y_;
    bool yonly_;

public:
    explicit StairsAdaptor(const VectorType &y)
        : y_(y)
        , yonly_(true)
    {}
    StairsAdaptor(const VectorType &x, const VectorType &y)
        : x_(x)
        , y_(y)
        , yonly_(false)
    {}
    StairsAdaptor(const StairsAdaptor &other) = default;
    int size() const override
    {
        return yonly_ ? 2 * y_.size() - 1 : 2 * std::min(x_.size(), y_.size()) - 1;
    }
    QPointF sample(int i) const override
    {
        int ix = (i + 1) >> 1;
        int iy = i >> 1;
        return yonly_ ? QPointF(ix, y_[iy]) : QPointF(x_[ix], y_[iy]);
    }
    QRectF boundingRect() const override
    {
        if (!size())
            return QRectF();

        if (yonly_) {
            qreal y1(y_[0]), y2(y1);
            int N = size() >> 1;
            for (int i = 1; i < N; ++i) {
                if (y_[i] < y1)
                    y1 = y_[i];
                else if (y_[i] > y2)
                    y2 = y_[i];
            }
            return QRectF(0, y1, N - 1, y2 - y1);
        } else {
            qreal x1(x_[0]), x2(x1);
            qreal y1(y_[0]), y2(y1);
            int N = size() >> 1;
            for (int i = 1; i < N; ++i) {
                if (x_[i] < x1)
                    x1 = x_[i];
                else if (x_[i] > x2)
                    x2 = x_[i];
                if (y_[i] < y1)
                    y1 = y_[i];
                else if (y_[i] > y2)
                    y2 = y_[i];
            }
            return QRectF(x1, y1, x2 - x1, y2 - y1);
        }
    }
};

template<class VectorType>
class ErrorBarAdaptor : public AbstractErrorBarAdaptor
{
    VectorType x_, y_, ym_, yp_;
    bool yonly_;

public:
    ErrorBarAdaptor(const VectorType &y, double err)
        : y_(y)
        , ym_(y.size())
        , yp_(y.size())
        , yonly_(true)
    {
        for (int i = 0; i < y.size(); ++i) {
            ym_[i] = y[i] - err;
            yp_[i] = y[i] + err;
        }
    }
    ErrorBarAdaptor(const VectorType &x, const VectorType &y, double err)
        : x_(x)
        , y_(y)
        , ym_(y.size())
        , yp_(y.size())
        , yonly_(false)
    {
        for (int i = 0; i < y.size(); ++i) {
            ym_[i] = y[i] - err;
            yp_[i] = y[i] + err;
        }
    }
    ErrorBarAdaptor(const VectorType &y, const VectorType &err)
        : y_(y)
        , ym_(y.size())
        , yp_(y.size())
        , yonly_(true)
    {
        for (int i = 0; i < y.size(); ++i) {
            ym_[i] = y[i] - err[i];
            yp_[i] = y[i] + err[i];
        }
    }
    ErrorBarAdaptor(const VectorType &x, const VectorType &y, const VectorType &err)
        : x_(x)
        , y_(y)
        , ym_(y.size())
        , yp_(y.size())
        , yonly_(false)
    {
        for (int i = 0; i < y.size(); ++i) {
            ym_[i] = y[i] - err[i];
            yp_[i] = y[i] + err[i];
        }
    }
    ErrorBarAdaptor(const VectorType &x,
                    const VectorType &y,
                    const VectorType &errm,
                    const VectorType &errp)
        : x_(x)
        , y_(y)
        , ym_(y.size())
        , yp_(y.size())
        , yonly_(false)
    {
        for (int i = 0; i < y.size(); ++i) {
            ym_[i] = y[i] - errm[i];
            yp_[i] = y[i] + errp[i];
        }
    }
    ErrorBarAdaptor(const ErrorBarAdaptor &other) = default;
    int size() const override { return yonly_ ? y_.size() : std::min(x_.size(), y_.size()); }
    QPointF sample(int i) const override
    {
        return yonly_ ? QPointF(i, y_[i]) : QPointF(x_[i], y_[i]);
    }
    QPointF interval(int i) const override { return QPointF(ym_[i], yp_[i]); }
    QRectF boundingRect() const override
    {
        if (!size())
            return QRectF();

        if (yonly_) {
            qreal y1(y_[0]), y2(y1);
            for (int i = 1; i < size(); ++i) {
                if (y_[i] < y1)
                    y1 = y_[i];
                else if (y_[i] > y2)
                    y2 = y_[i];
            }
            return QRectF(0, y1, size() - 1, y2 - y1);
        }

        qreal x1(x_[0]), x2(x1);
        qreal y1(y_[0]), y2(y1);
        for (int i = 1; i < size(); ++i) {
            if (x_[i] < x1)
                x1 = x_[i];
            else if (x_[i] > x2)
                x2 = x_[i];
            if (y_[i] < y1)
                y1 = y_[i];
            else if (y_[i] > y2)
                y2 = y_[i];
        }
        return QRectF(x1, y1, x2 - x1, y2 - y1);
    }
    QRectF errorBoundingRect() const override
    {
        if (!size())
            return QRectF();

        if (yonly_) {
            qreal y1(ym_[0]), y2(yp_[0]);
            for (int i = 1; i < size(); ++i) {
                y1 = std::min(y1, ym_[i]);
                y2 = std::max(y2, yp_[i]);
            }
            return QRectF(0, y1, size() - 1, y2 - y1);
        }

        qreal x1(x_[0]), x2(x1);
        qreal y1(ym_[0]), y2(yp_[0]);
        for (int i = 1; i < size(); ++i) {
            x1 = std::min(x1, x_[i]);
            x2 = std::max(x2, x_[i]);
            y1 = std::min(y1, ym_[i]);
            y2 = std::max(y2, yp_[i]);
        }
        return QRectF(x1, y1, x2 - x1, y2 - y1);
    }
};

#endif // DATAADAPTORS_H
