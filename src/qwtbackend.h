#ifndef QWTBACKEND_H
#define QWTBACKEND_H

#include "QMatPlotWidget.h"
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_draw.h>
#include <qwt_text.h>

class QwtPlotGrid;
class QwtPlotGrid;
class QwtPlotZoomer;
class QwtPlotPanner;
class QwtPlotPicker;
class ScalePicker;

class QwtBackend : public QwtPlot, public QMatPlotWidget::Backend
{
    Q_OBJECT
public:
    QwtBackend(QMatPlotWidget *parent);

    void alignScales();
    virtual bool exportToFile(const QString &fname, const QSize &sz) override;
    virtual void clear() override;
    virtual void replot() override { QwtPlot::replot(); }
    virtual void plot(AbstractDataSeriesAdaptor *d, const QMatPlotWidget::LineSpec &l) override;
    virtual void errorbar(AbstractErrorBarAdaptor *d, const QMatPlotWidget::LineSpec &opt) override;
    virtual void image(AbstractImageAdaptor *d, bool scale, const QVector<QRgb> &cmap) override;
    void setAxisScaling(int axisid, QMatPlotWidget::AxisScale sc);
    virtual QString title() const override { return QwtPlot::title().text(); }
    virtual QString xlabel() const override { return axisTitle(QwtPlot::xBottom).text(); }
    virtual QString ylabel() const override { return axisTitle(QwtPlot::yLeft).text(); }
    virtual bool autoScaleX() const override { return axisAutoScale(QwtPlot::xBottom); }
    virtual bool autoScaleY() const override { return axisAutoScale(QwtPlot::yLeft); }
    virtual QPointF xlim() const override
    {
        double lb = axisScaleDiv(QwtPlot::xBottom).lowerBound();
        double ub = axisScaleDiv(QwtPlot::xBottom).upperBound();
        return QPointF(lb, ub);
    }
    virtual QPointF ylim() const override
    {
        double lb = axisScaleDiv(QwtPlot::yLeft).lowerBound();
        double ub = axisScaleDiv(QwtPlot::yLeft).upperBound();
        return QPointF(lb, ub);
    }

    //setters
    virtual void setTitle(const QString &s) override { QwtPlot::setTitle(s); }
    virtual void setXlabel(const QString &s) override { setAxisTitle(QwtPlot::xBottom, s); }
    virtual void setYlabel(const QString &s) override { setAxisTitle(QwtPlot::yLeft, s); }
    virtual void setAutoScaleX(bool on) override { setAxisAutoScale(QwtPlot::xBottom, on); }
    virtual void setAutoScaleY(bool on) override { setAxisAutoScale(QwtPlot::yLeft, on); }
    virtual void setAxisScaleX(QMatPlotWidget::AxisScale sc) override
    {
        setAxisScaling(QwtPlot::xBottom, sc);
    }
    virtual void setAxisScaleY(QMatPlotWidget::AxisScale sc) override
    {
        setAxisScaling(QwtPlot::yLeft, sc);
    }
    virtual void setGrid(bool on) override
    {
        grid_->enableX(on);
        grid_->enableY(on);
    }
    virtual void setXlim(const QPointF &v) override
    {
        setAxisScale(QwtPlot::xBottom, v.x(), v.y());
    }
    virtual void setYlim(const QPointF &v) override { setAxisScale(QwtPlot::yLeft, v.x(), v.y()); }
    void setAxisEqual() override;

    QMatPlotWidget *mMatPlot_;
    QwtPlotGrid *grid_;
    QwtPlotZoomer *zoomer;
    QwtPlotPanner *panner;
    QwtPlotPicker *picker;
    ScalePicker *scalepicker;

    void doAxisClicked(int axisid, const QPoint &pos) { emit axisClicked(axisid, pos); }

signals:
    void axisClicked(int axisid, const QPoint &pos);
};

#endif // QWTBACKEND_H
