#ifndef _QMATPLOTWIDGET_H_
#define _QMATPLOTWIDGET_H_

#include <QPointF>
#include <QRectF>
#include <QVector>
#include <QWidget>
#include <QDialog>

class QMenu;
struct AbstractDataSeriesAdaptor;
struct AbstractErrorBarAdaptor;
struct AbstractImageAdaptor;

class QMatPlotWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString xlabel READ xlabel WRITE setXlabel)
    Q_PROPERTY(QString ylabel READ ylabel WRITE setYlabel)
    Q_PROPERTY(bool autoScaleX READ autoScaleX WRITE setAutoScaleX)
    Q_PROPERTY(bool autoScaleY READ autoScaleY WRITE setAutoScaleY)
    Q_PROPERTY(AxisScale axisScaleX READ axisScaleX WRITE setAxisScaleX)
    Q_PROPERTY(AxisScale axisScaleY READ axisScaleY WRITE setAxisScaleY)
    Q_PROPERTY(bool grid READ grid WRITE setGrid)
    Q_PROPERTY(QPointF xlim READ xlim WRITE setXlim)
    Q_PROPERTY(QPointF ylim READ ylim WRITE setYlim)
    Q_PROPERTY(QVector<QRgb> colorOrder READ colorOrder WRITE setColorOrder)
    Q_PROPERTY(QVector<QRgb> colorMap READ colorMap WRITE setColorMap)

public:
    enum AxisScale
    {
        Linear = 0,
        Log,
        Time
    };
    Q_ENUM(AxisScale)

    enum ColorMapType
    {
        Viridis,
        Turbo,
        Jet,
        Gray
    };
    Q_ENUM(ColorMapType)

    struct LineSpec
    {
        // MATLAB-Octave-style markerstyles for reference (not all)
        constexpr static const char *markers = "+o*.xsd^v>< ";
        static const int nMarkers = 11;
        int markerStyle{nMarkers}; // emty = no marker
        Qt::PenStyle penStyle{Qt::SolidLine};
        QColor clr;

        static LineSpec fromMatlabLineSpec(const QString &attr);
    };

public:
    explicit QMatPlotWidget(QWidget *parent = 0);
    virtual ~QMatPlotWidget();

    // getters
    QString title() const;
    QString xlabel() const;
    QString ylabel() const;
    bool autoScaleX() const;
    bool autoScaleY() const;
    AxisScale axisScaleX() const { return axisScaleX_; }
    AxisScale axisScaleY() const { return axisScaleY_; }
    bool timeScaleX() const { return axisScaleX_ == Time; }
    bool timeScaleY() const { return axisScaleY_ == Time; }
    bool logScaleX() const { return axisScaleX_ == Log; }
    bool logScaleY() const { return axisScaleY_ == Log; }
    bool linearScaleX() const { return axisScaleX_ == Linear; }
    bool linearScaleY() const { return axisScaleY_ == Linear; }
    bool grid() const { return grid_on_; }
    QPointF xlim() const;
    QPointF ylim() const;
    QVector<QRgb> colorOrder() const { return colorOrder_; }
    QVector<QRgb> colorMap() const { return colorMap_; }

    static QVector<QRgb> colorMap(ColorMapType t, int n = 64);
    static QVector<QRgb> defaultColorOrder();

    // setters
    void setTitle(const QString &s);
    void setXlabel(const QString &s);
    void setYlabel(const QString &s);
    void setXlim(const QPointF &v);
    void setYlim(const QPointF &v);
    void setColorOrder(const QVector<QRgb> &c);
    void setColorMap(const QVector<QRgb> &c);
    void setColorMap(ColorMapType t, int n = 64) { setColorMap(colorMap(t, n)); }

    // QWidget overrides
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    bool exportToFile(const QString &fname, const QSize &sz = QSize());

public slots:
    void clear();
    void replot();

    // slot setters
    void setAutoScaleX(bool on);
    void setAutoScaleY(bool on);
    void setAxisScaleX(AxisScale sc);
    void setAxisScaleY(AxisScale sc);
    void setGrid(bool on);

    // helpers
    void setTimeScaleX() { setAxisScaleX(Time); }
    void setTimeScaleY() { setAxisScaleY(Time); }
    void setLogScaleX() { setAxisScaleX(Log); }
    void setLogScaleY() { setAxisScaleY(Log); }
    void setLinearScaleX() { setAxisScaleX(Linear); }
    void setLinearScaleY() { setAxisScaleY(Linear); }
    void setAxisEqual();

    void onAxisClicked(int axisid, const QPoint &pos);

public:
    template <class VectorType>
    void plot(const VectorType &x, const VectorType &y,
              const QString &attr = QString(), const QColor &clr = QColor());
    template <class VectorType>
    void plot(const VectorType &y,
              const QString &attr = QString(), const QColor &clr = QColor());
    template <class VectorType>
    void stairs(const VectorType &x,
                const VectorType &y,
                const QString &attr = QString(),
                const QColor &clr = QColor());
    template <class VectorType>
    void stairs(const VectorType &y, const QString &attr = QString(), const QColor &clr = QColor());

    template <class VectorType>
    void errorbar(const VectorType &y,
                  const VectorType &dy,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());
    template <class VectorType>
    void errorbar(const VectorType &x,
                  const VectorType &y,
                  const VectorType &dy,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());
    template <class VectorType>
    void errorbar(const VectorType &y,
                  double dy,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());
    template <class VectorType>
    void errorbar(const VectorType &x,
                  const VectorType &y,
                  double dy,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());
    template <class VectorType>
    void errorbar(const VectorType &x,
                  const VectorType &y,
                  const VectorType &dym,
                  const VectorType &dyp,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());

    template <class VectorType>
    void image(const VectorType &x, const VectorType &y, const VectorType &z, int columns);
    template <class VectorType>
    void image(const VectorType &z, int columns);

    template <class VectorType>
    void imagesc(const VectorType &x, const VectorType &y, const VectorType &z, int columns);
    template <class VectorType>
    void imagesc(const VectorType &z, int columns);

    struct Backend;

protected:
    virtual QMenu *createAxisContextMenu(int axisid);
    virtual void axisPropertyDialog(int axisid);

    void __plot__(AbstractDataSeriesAdaptor *d, const QString &attr, const QColor &clr);
    void __errorbar__(AbstractErrorBarAdaptor *d, const QString &attr, const QColor &clr);
    void __image__(AbstractImageAdaptor *d, bool scale);

protected slots:
    void xAxisPropDlg() { axisPropertyDialog(0); }
    void yAxisPropDlg() { axisPropertyDialog(1); }

private:
    Backend *const backend_;

    AxisScale axisScaleX_, axisScaleY_;
    bool grid_on_;

    QVector<QRgb> colorOrder_;
    int colorIndex_;
    QVector<QRgb> colorMap_;
};

/*---- Templated plot functions -------*/

struct AbstractDataSeriesAdaptor
{
    virtual ~AbstractDataSeriesAdaptor() {}
    virtual int size() const = 0;
    virtual QPointF sample(int i) const = 0;
    virtual QRectF boundingRect() const = 0;
};

template <class V_>
class DataSeriesAdaptor : public AbstractDataSeriesAdaptor
{
    V_ vx;
    V_ vy;
    bool yonly_;

public:
    DataSeriesAdaptor(const V_ &y)
        : vy(y), yonly_(true)
    {
    }
    DataSeriesAdaptor(const V_ &x, const V_ &y)
        : vx(x), vy(y), yonly_(false)
    {
    }
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

        if (yonly_)
        {
            qreal y1(vy[0]), y2(y1);
            for (int i = 1; i < size(); ++i)
            {
                if (vy[i] < y1)
                    y1 = vy[i];
                else if (vy[i] > y2)
                    y2 = vy[i];
            }
            return QRectF(0, y1, size() - 1, y2 - y1);
        }

        qreal x1(vx[0]), x2(x1);
        qreal y1(vy[0]), y2(y1);
        for (int i = 1; i < size(); ++i)
        {
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

template <class VectorType>
class StairsAdaptor : public AbstractDataSeriesAdaptor
{
    VectorType x_, y_;
    bool yonly_;

public:
    explicit StairsAdaptor(const VectorType &y)
        : y_(y), yonly_(true)
    {
    }
    StairsAdaptor(const VectorType &x, const VectorType &y)
        : x_(x), y_(y), yonly_(false)
    {
    }
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

        if (yonly_)
        {
            qreal y1(y_[0]), y2(y1);
            int N = size() >> 1;
            for (int i = 1; i < N; ++i)
            {
                if (y_[i] < y1)
                    y1 = y_[i];
                else if (y_[i] > y2)
                    y2 = y_[i];
            }
            return QRectF(0, y1, N - 1, y2 - y1);
        }
        else
        {
            qreal x1(x_[0]), x2(x1);
            qreal y1(y_[0]), y2(y1);
            int N = size() >> 1;
            for (int i = 1; i < N; ++i)
            {
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

template <class VectorType>
inline void QMatPlotWidget::plot(const VectorType &y, const QString &attr, const QColor &clr)
{
    __plot__(new DataSeriesAdaptor<VectorType>(y), attr, clr);
}

template <class VectorType>
inline void QMatPlotWidget::plot(const VectorType &x,
                                 const VectorType &y,
                                 const QString &attr,
                                 const QColor &clr)
{
    __plot__(new DataSeriesAdaptor<VectorType>(x, y), attr, clr);
}

template <class VectorType>
inline void QMatPlotWidget::stairs(const VectorType &y, const QString &attr, const QColor &clr)
{
    __plot__(new StairsAdaptor<VectorType>(y), attr, clr);
}

template <class VectorType>
inline void QMatPlotWidget::stairs(const VectorType &x,
                                   const VectorType &y,
                                   const QString &attr,
                                   const QColor &clr)
{
    __plot__(new StairsAdaptor<VectorType>(x, y), attr, clr);
}

/*---- Templated errorbar functions -------*/

struct AbstractErrorBarAdaptor
{
    virtual ~AbstractErrorBarAdaptor() {}
    virtual int size() const = 0;
    virtual QPointF sample(int i) const = 0;
    virtual QPointF interval(int i) const = 0;
    virtual QRectF boundingRect() const = 0;
    virtual QRectF errorBoundingRect() const = 0;
};

template <class VectorType>
class ErrorBarAdaptor : public AbstractErrorBarAdaptor
{
    VectorType x_, y_, ym_, yp_;
    bool yonly_;

public:
    ErrorBarAdaptor(const VectorType &y, double err)
        : y_(y), ym_(y.size()), yp_(y.size()), yonly_(true)
    {
        for (int i = 0; i < y.size(); ++i)
        {
            ym_[i] = y[i] - err;
            yp_[i] = y[i] + err;
        }
    }
    ErrorBarAdaptor(const VectorType &x, const VectorType &y, double err)
        : x_(x), y_(y), ym_(y.size()), yp_(y.size()), yonly_(false)
    {
        for (int i = 0; i < y.size(); ++i)
        {
            ym_[i] = y[i] - err;
            yp_[i] = y[i] + err;
        }
    }
    ErrorBarAdaptor(const VectorType &y, const VectorType &err)
        : y_(y), ym_(y.size()), yp_(y.size()), yonly_(true)
    {
        for (int i = 0; i < y.size(); ++i)
        {
            ym_[i] = y[i] - err[i];
            yp_[i] = y[i] + err[i];
        }
    }
    ErrorBarAdaptor(const VectorType &x, const VectorType &y, const VectorType &err)
        : x_(x), y_(y), ym_(y.size()), yp_(y.size()), yonly_(false)
    {
        for (int i = 0; i < y.size(); ++i)
        {
            ym_[i] = y[i] - err[i];
            yp_[i] = y[i] + err[i];
        }
    }
    ErrorBarAdaptor(const VectorType &x,
                    const VectorType &y,
                    const VectorType &errm,
                    const VectorType &errp)
        : x_(x), y_(y), ym_(y.size()), yp_(y.size()), yonly_(false)
    {
        for (int i = 0; i < y.size(); ++i)
        {
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

        if (yonly_)
        {
            qreal y1(y_[0]), y2(y1);
            for (int i = 1; i < size(); ++i)
            {
                if (y_[i] < y1)
                    y1 = y_[i];
                else if (y_[i] > y2)
                    y2 = y_[i];
            }
            return QRectF(0, y1, size() - 1, y2 - y1);
        }

        qreal x1(x_[0]), x2(x1);
        qreal y1(y_[0]), y2(y1);
        for (int i = 1; i < size(); ++i)
        {
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

        if (yonly_)
        {
            qreal y1(ym_[0]), y2(yp_[0]);
            for (int i = 1; i < size(); ++i)
            {
                y1 = std::min(y1, ym_[i]);
                y2 = std::max(y2, yp_[i]);
            }
            return QRectF(0, y1, size() - 1, y2 - y1);
        }

        qreal x1(x_[0]), x2(x1);
        qreal y1(ym_[0]), y2(yp_[0]);
        for (int i = 1; i < size(); ++i)
        {
            x1 = std::min(x1, x_[i]);
            x2 = std::max(x2, x_[i]);
            y1 = std::min(y1, ym_[i]);
            y2 = std::max(y2, yp_[i]);
        }
        return QRectF(x1, y1, x2 - x1, y2 - y1);
    }
};

template <class VectorType>
inline void QMatPlotWidget::errorbar(const VectorType &x,
                                     const VectorType &y,
                                     const VectorType &dym,
                                     const VectorType &dyp,
                                     const QString &attr,
                                     const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(x, y, dym, dyp), attr, clr);
}

template <class VectorType>
inline void QMatPlotWidget::errorbar(
    const VectorType &x, const VectorType &y, double dy, const QString &attr, const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(x, y, dy), attr, clr);
}

template <class VectorType>
inline void QMatPlotWidget::errorbar(const VectorType &y,
                                     double dy,
                                     const QString &attr,
                                     const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(y, dy), attr, clr);
}

template <class VectorType>
inline void QMatPlotWidget::errorbar(const VectorType &x,
                                     const VectorType &y,
                                     const VectorType &dy,
                                     const QString &attr,
                                     const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(x, y, dy), attr, clr);
}

template <class VectorType>
inline void QMatPlotWidget::errorbar(const VectorType &y,
                                     const VectorType &dy,
                                     const QString &attr,
                                     const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(y, dy), attr, clr);
}

/*---- Templated image functions -------*/

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

template <class VectorType>
class ImageAdaptor : public AbstractImageAdaptor
{
    VectorType x_, y_, z_;
    int cols_;
    bool zonly_;

public:
    explicit ImageAdaptor(const VectorType &z, int columns)
        : z_(z), cols_(columns), zonly_(true)
    {
    }
    ImageAdaptor(const VectorType &x, const VectorType &y, const VectorType &z, int columns)
        : x_(x), y_(y), z_(z), cols_(columns), zonly_(false)
    {
    }
    ImageAdaptor(const ImageAdaptor &other) = default;
    int rows() const override { return z_.size() / cols_; }
    int columns() const override { return cols_; }
    double value(int k) const override { return z_[k]; }
    QPointF xlim() const override
    {
        if (zonly_)
        {
            return QPointF(0, cols_);
        }
        else
        {
            return QPointF(x_[0], x_[x_.size() - 1]);
        }
    }
    QPointF ylim() const override
    {
        if (zonly_)
        {
            return QPointF(0, rows());
        }
        else
        {
            return QPointF(y_[0], y_[y_.size() - 1]);
        }
    }
    QPointF zlim() const override
    {
        if (!z_.size())
            return QPointF(0., 1.);
        double vmin = z_[0], vmax = z_[0];
        for (int i = 1; i < z_.size(); ++i)
        {
            vmin = std::min(vmin, z_[i]);
            vmax = std::max(vmax, z_[i]);
        }
        return QPointF(vmin, vmax);
    }
};

template <class VectorType>
inline void QMatPlotWidget::imagesc(const VectorType &z, int columns)
{
    __image__(new ImageAdaptor<VectorType>(z, columns), true);
}

template <class VectorType>
inline void QMatPlotWidget::imagesc(const VectorType &x,
                                    const VectorType &y,
                                    const VectorType &z,
                                    int columns)
{
    __image__(new ImageAdaptor<VectorType>(x, y, z, columns), true);
}

template <class VectorType>
inline void QMatPlotWidget::image(const VectorType &z, int columns)
{
    __image__(new ImageAdaptor<VectorType>(z, columns), false);
}

template <class VectorType>
inline void QMatPlotWidget::image(const VectorType &x,
                                  const VectorType &y,
                                  const VectorType &z,
                                  int columns)
{
    __image__(new ImageAdaptor<VectorType>(x, y, z, columns), false);
}

#endif //_QMATPLOTWIDGET_H_
