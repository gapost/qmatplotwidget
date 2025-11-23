#ifndef _QMATPLOTWIDGET_H_
#define _QMATPLOTWIDGET_H_

#include "DataAdaptors.h"

#include <QVector>
#include <QWidget>
#include <QDialog>

class QMenu;

class  QMatPlotWidget : public QWidget
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
    enum AxisScale {
        Linear = 0,
        Log,
        Time
    };
    Q_ENUM(AxisScale)

    enum ColorMapType { Viridis, Turbo, Jet, Gray };
    Q_ENUM(ColorMapType)

    struct LineSpec
    {
        //MATLAB-Octave-style markerstyles for reference (not all)
        constexpr static const char *markers = "+o*.xsd^v>< ";
        static const int nMarkers = 11;
        int markerStyle{nMarkers}; // emty = no marker
        Qt::PenStyle penStyle{Qt::SolidLine};
        QColor clr;

        static LineSpec fromMatlabLineSpec(const QString &attr);
    };

public:
    explicit QMatPlotWidget(QWidget* parent = 0);
    virtual ~QMatPlotWidget();

    //getters
    QString title() const;
    QString xlabel() const;
    QString ylabel() const;
    bool autoScaleX() const;
    bool autoScaleY() const;
    AxisScale axisScaleX() const { return axisScaleX_; }
    AxisScale axisScaleY() const { return axisScaleY_; }
    bool timeScaleX() const { return axisScaleX_==Time; }
    bool timeScaleY() const { return axisScaleY_==Time; }
    bool logScaleX() const { return axisScaleX_==Log; }
    bool logScaleY() const { return axisScaleY_==Log; }
    bool linearScaleX() const { return axisScaleX_==Linear; }
    bool linearScaleY() const { return axisScaleY_==Linear; }
    bool grid() const { return grid_on_; }
    QPointF xlim() const;
    QPointF ylim() const;
    QVector<QRgb> colorOrder() const { return colorOrder_; }
    QVector<QRgb> colorMap() const { return colorMap_; }

    static QVector<QRgb> colorMap(ColorMapType t, int n = 64);
    static QVector<QRgb> defaultColorOrder();

    //setters
    void setTitle(const QString& s);
    void setXlabel(const QString& s);
    void setYlabel(const QString& s);
    void setXlim(const QPointF& v);
    void setYlim(const QPointF& v);
    void setColorOrder(const QVector<QRgb> &c);
    void setColorMap(const QVector<QRgb> &c);
    void setColorMap(ColorMapType t, int n = 64) { setColorMap(colorMap(t, n)); }

    // QWidget overrides
    QSize sizeHint () const override;
    QSize minimumSizeHint () const override;

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
    void setTimeScaleX()   { setAxisScaleX(Time); }
    void setTimeScaleY()   { setAxisScaleY(Time); }
    void setLogScaleX()    { setAxisScaleX(Log); }
    void setLogScaleY()    { setAxisScaleY(Log); }
    void setLinearScaleX() { setAxisScaleX(Linear); }
    void setLinearScaleY() { setAxisScaleY(Linear); }
    void setAxisEqual();

    void onAxisClicked(int axisid, const QPoint &pos);

public:
    template<class VectorType>
    void plot(const VectorType& x, const VectorType& y,
              const QString &attr = QString(), const QColor& clr = QColor());
    template<class VectorType>
    void plot(const VectorType& y,
              const QString &attr = QString(), const QColor& clr = QColor());
    template<class VectorType>
    void stairs(const VectorType &x,
                const VectorType &y,
                const QString &attr = QString(),
                const QColor &clr = QColor());
    template<class VectorType>
    void stairs(const VectorType &y, const QString &attr = QString(), const QColor &clr = QColor());

    template<class VectorType>
    void errorbar(const VectorType &y,
                  const VectorType &dy,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());
    template<class VectorType>
    void errorbar(const VectorType &x,
                  const VectorType &y,
                  const VectorType &dy,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());
    template<class VectorType>
    void errorbar(const VectorType &y,
                  double dy,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());
    template<class VectorType>
    void errorbar(const VectorType &x,
                  const VectorType &y,
                  double dy,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());
    template<class VectorType>
    void errorbar(const VectorType &x,
                  const VectorType &y,
                  const VectorType &dym,
                  const VectorType &dyp,
                  const QString &attr = QString(),
                  const QColor &clr = QColor());

    template<class VectorType>
    void image(const VectorType &x, const VectorType &y, const VectorType &z, int columns);
    template<class VectorType>
    void image(const VectorType &z, int columns);

    template<class VectorType>
    void imagesc(const VectorType &x, const VectorType &y, const VectorType &z, int columns);
    template<class VectorType>
    void imagesc(const VectorType &z, int columns);

    struct Backend;

protected:
    virtual QMenu* createAxisContextMenu(int axisid);
    virtual void axisPropertyDialog(int axisid);

    void __plot__(AbstractDataSeriesAdaptor *d, const QString &attr, const QColor &clr);
    void __errorbar__(AbstractErrorBarAdaptor *d, const QString &attr, const QColor &clr);
    // void __image__(AbstractImageData *d, bool scale);

protected slots:
    void xAxisPropDlg() { axisPropertyDialog(0); }
    void yAxisPropDlg() { axisPropertyDialog(1); }

private:
    //class Implementation;
    //Implementation* const impl_;
    //friend class Implementation;

    Backend *const backend_;

    AxisScale axisScaleX_, axisScaleY_;
    bool grid_on_;

    QVector<QRgb> colorOrder_;
    int colorIndex_;
    QVector<QRgb> colorMap_;
};

template<class VectorType>
inline void QMatPlotWidget::errorbar(const VectorType &x,
                                     const VectorType &y,
                                     const VectorType &dym,
                                     const VectorType &dyp,
                                     const QString &attr,
                                     const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(x, y, dym, dyp), attr, clr);
}

template<class VectorType>
inline void QMatPlotWidget::errorbar(
    const VectorType &x, const VectorType &y, double dy, const QString &attr, const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(x, y, dy), attr, clr);
}

template<class VectorType>
inline void QMatPlotWidget::errorbar(const VectorType &y,
                                     double dy,
                                     const QString &attr,
                                     const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(y, dy), attr, clr);
}

template<class VectorType>
inline void QMatPlotWidget::errorbar(const VectorType &x,
                                     const VectorType &y,
                                     const VectorType &dy,
                                     const QString &attr,
                                     const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(x, y, dy), attr, clr);
}

template<class VectorType>
inline void QMatPlotWidget::errorbar(const VectorType &y,
                                     const VectorType &dy,
                                     const QString &attr,
                                     const QColor &clr)
{
    __errorbar__(new ErrorBarAdaptor<VectorType>(y, dy), attr, clr);
}

struct QMatPlotWidget::Backend
{
    virtual bool exportToFile(const QString &fname, const QSize &sz) = 0;
    virtual void clear() = 0;
    virtual void replot() = 0;
    virtual void plot(AbstractDataSeriesAdaptor *d, const QMatPlotWidget::LineSpec &l) = 0;
    virtual void errorbar(AbstractErrorBarAdaptor *d, const QMatPlotWidget::LineSpec &l) = 0;
    virtual void image(AbstractImageAdaptor *d, bool scale, const QVector<QRgb> &cmap) = 0;
    virtual QPointF xlim() const = 0;
    virtual QPointF ylim() const = 0;
    virtual QString title() const = 0;
    virtual QString xlabel() const = 0;
    virtual QString ylabel() const = 0;
    virtual bool autoScaleX() const = 0;
    virtual bool autoScaleY() const = 0;
    //setters
    virtual void setTitle(const QString &s) = 0;
    virtual void setXlabel(const QString &s) = 0;
    virtual void setYlabel(const QString &s) = 0;
    virtual void setAutoScaleX(bool on) = 0;
    virtual void setAutoScaleY(bool on) = 0;
    virtual void setAxisScaleX(QMatPlotWidget::AxisScale sc) = 0;
    virtual void setAxisScaleY(QMatPlotWidget::AxisScale sc) = 0;
    virtual void setGrid(bool on) = 0;
    virtual void setXlim(const QPointF &v) = 0;
    virtual void setYlim(const QPointF &v) = 0;
    virtual void setAxisEqual() = 0;
};

template<class VectorType>
inline void QMatPlotWidget::imagesc(const VectorType &z, int columns)
{
    backend_->image(new ImageAdaptor<VectorType>(z, columns), true, colorMap_);
}

template<class VectorType>
inline void QMatPlotWidget::imagesc(const VectorType &x,
                                    const VectorType &y,
                                    const VectorType &z,
                                    int columns)
{
    backend_->image(new ImageAdaptor<VectorType>(x, y, z, columns), true, colorMap_);
}

template<class VectorType>
inline void QMatPlotWidget::image(const VectorType &z, int columns)
{
    backend_->image(new ImageAdaptor<VectorType>(z, columns), false, colorMap_);
}

template<class VectorType>
inline void QMatPlotWidget::image(const VectorType &x,
                                  const VectorType &y,
                                  const VectorType &z,
                                  int columns)
{
    backend_->image(new ImageAdaptor<VectorType>(x, y, z, columns), false, colorMap_);
}

template<class VectorType>
inline void QMatPlotWidget::plot(const VectorType &y, const QString &attr, const QColor &clr)
{
    __plot__(new DataSeriesAdaptor<VectorType>(y), attr, clr);
}

template<class VectorType>
inline void QMatPlotWidget::plot(const VectorType &x,
                                 const VectorType &y,
                                 const QString &attr,
                                 const QColor &clr)
{
    __plot__(new DataSeriesAdaptor<VectorType>(x, y), attr, clr);
}

template<class VectorType>
inline void QMatPlotWidget::stairs(const VectorType &y, const QString &attr, const QColor &clr)
{
    __plot__(new StairsAdaptor<VectorType>(y), attr, clr);
}

template<class VectorType>
inline void QMatPlotWidget::stairs(const VectorType &x,
                                   const VectorType &y,
                                   const QString &attr,
                                   const QColor &clr)
{
    __plot__(new StairsAdaptor<VectorType>(x, y), attr, clr);
}

class QLineEdit;
class QCheckBox;
class QComboBox;

class QMatPlotAxisDlg : public QDialog
{
    Q_OBJECT

    QLineEdit* edtTitle;
    QCheckBox* chkAutoScale;
    QLineEdit* edtMinVal;
    QLineEdit* edtMaxVal;
    QComboBox* cbAxisScale;
    friend class QMatPlotWidget;

public:
    QMatPlotAxisDlg(QWidget* parent);

private slots:
    void onAutoScale(bool on);

};





#endif //_QMATPLOTWIDGET_H_
