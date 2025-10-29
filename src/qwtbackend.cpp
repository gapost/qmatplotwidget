#include "qwtbackend.h"

#include <QDebug>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QRegularExpression>
#include <QScreen>
#include <QSet>
#include <QVBoxLayout>
#include <QValidator>
#include <QtMath>

#include <qwt_color_map.h>
#include <qwt_interval_symbol.h>
#include <qwt_math.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_dict.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_series_data.h>
#include <qwt_symbol.h>

#include <cmath>

class FormattedPicker : public QwtPlotPicker
{
public:
    FormattedPicker(int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QWidget *pc)
        : QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, pc)
    {}

protected:
    QwtText trackerTextF(const QPointF &pos) const override
    {
        //Since the "paintAttributes", [text+background colour] act on QwtTexts
        //break up the creation of trackerTextF: one function to create the text
        //(as a QString), and another to set attributes and return the object.
        QString S = createLabelText(pos);
        QwtText trackerText;
        trackerText.setBackgroundBrush(Qt::lightGray);
        trackerText.setText(S);
        trackerText.setColor(Qt::black);
        return trackerText;
    }
    QString createLabelText(const QPointF &pos) const
    {
        QwtText lx = plot()->axisScaleDraw(QwtPlot::xBottom)->label(pos.x());
        QwtText ly = plot()->axisScaleDraw(QwtPlot::yLeft)->label(pos.y());
        QString S(lx.text());
        S += QChar(',');
        S += ly.text();
        return S;
    }
};

class Zoomer : public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QWidget *canvas)
        : QwtPlotZoomer(xAxis, yAxis, canvas)
    {}

protected:
    void begin() override
    {
        if (zoomRectIndex() == 0) {
            setZoomBase(false);

            const QRectF &rect = this->scaleRect();
            QwtPlot *plt = plot();
            if (plt->axisAutoScale(QwtPlot::xBottom)) {
                plt->setAxisScale(QwtPlot::xBottom, rect.left(), rect.right());
            }
            if (plt->axisAutoScale(QwtPlot::yLeft)) {
                plt->setAxisScale(QwtPlot::yLeft, rect.top(), rect.bottom());
            }
        }

        QwtPlotZoomer::begin();
    }
    bool end(bool ok = true) override { return QwtPlotZoomer::end(ok); }
    void widgetMouseDoubleClickEvent(QMouseEvent *) override
    {
        QwtPlot *plt = plot();
        if (!plt)
            return;
        plt->setAxisAutoScale(QwtPlot::xBottom);
        plt->setAxisAutoScale(QwtPlot::yLeft);

        setZoomStack(zoomStack(), 0); //QStack<QwtDoubleRect>());
    }
};

class ScalePicker : public QObject
{
    QwtBackend *mPlot_;

public:
    explicit ScalePicker(QwtBackend *plot)
        : QObject(plot)
        , mPlot_(plot)
    {
        for (uint i = 0; i < QwtPlot::axisCnt; i++) {
            QwtScaleWidget *scaleWidget = plot->axisWidget(i);
            if (scaleWidget)
                scaleWidget->installEventFilter(this);
        }
    }

    bool eventFilter(QObject *object, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::RightButton) {
                QwtScaleWidget *scaleWidget = qobject_cast<QwtScaleWidget *>(object);
                if (scaleWidget) {
                    mouseClicked(scaleWidget, mouseEvent);
                    return true;
                }
            }
        }
        return QObject::eventFilter(object, event);
    }

    void mouseClicked(const QwtScaleWidget *scale, QMouseEvent *event)
    {
        const QPoint &pos = event->pos();
        QRect rect = scaleRect(scale);

        int margin = 10; // 10 pixels tolerance
        rect.setRect(rect.x() - margin,
                     rect.y() - margin,
                     rect.width() + 2 * margin,
                     rect.height() + 2 * margin);

        if (rect.contains(pos)) // No click on the title
        {
            int axis;
            switch (scale->alignment()) {
            case QwtScaleDraw::LeftScale:
                axis = QwtPlot::yLeft;
                break;
            case QwtScaleDraw::RightScale:
                axis = QwtPlot::yRight;
                break;
            case QwtScaleDraw::BottomScale:
                axis = QwtPlot::xBottom;
                break;
            case QwtScaleDraw::TopScale:
                axis = QwtPlot::xTop;
                break;
            default:
                axis = QwtPlot::yLeft;
                break;
            }
            mPlot_->doAxisClicked(axis, scale->mapToGlobal(pos));
        }
    }
    QRect scaleRect(const QwtScaleWidget *scale) const
    {
        const int bld = scale->margin();
        const int mjt = qCeil(scale->scaleDraw()->maxTickLength());
        const int sbd = scale->startBorderDist();
        const int ebd = scale->endBorderDist();

        QRect rect;
        switch (scale->alignment()) {
        case QwtScaleDraw::LeftScale: {
            rect.setRect(scale->width() - bld - mjt, sbd, mjt, scale->height() - sbd - ebd);
            break;
        }
        case QwtScaleDraw::RightScale: {
            rect.setRect(bld, sbd, mjt, scale->height() - sbd - ebd);
            break;
        }
        case QwtScaleDraw::BottomScale: {
            rect.setRect(sbd, bld, scale->width() - sbd - ebd, mjt);
            break;
        }
        case QwtScaleDraw::TopScale: {
            rect.setRect(sbd, scale->height() - bld - mjt, scale->width() - sbd - ebd, mjt);
            break;
        }
        }
        return rect;
    }
};

class TimeScaleDraw : public QwtScaleDraw
{
public:
    QwtText label(double secsSinceEpoch) const override
    {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(1000 * secsSinceEpoch);
        return dt.toString("hh:mm:ss");
    }
};

class SciScaleDraw : public QwtScaleDraw
{
public:
    QwtText label(double v) const override { return QString::number(v, 'g'); }
};

class TimeScaleEngine : public QwtLinearScaleEngine
{
protected:
    static double conversion_factor(double T)
    {
        double C = 1.; // conversion const
        if (T <= 60)
            C = 1.; // secs up to 1 min
        else if (T <= 3600)
            C = 60; // mins up to 1hr
        else if (T <= 24 * 3600)
            C = 60 * 60; // hr up to 1 day
        else
            C = 24 * 60 * 60; // else in days
        return C;
    }

public:
    void autoScale(int maxSteps, double &x1, double &x2, double &stepSize) const override
    {
        double C = conversion_factor(fabs(x2 - x1));
        x1 /= C;
        x2 /= C; // convert to  s,m,hr,d ...
        QwtLinearScaleEngine::autoScale(maxSteps, x1, x2, stepSize);
        x1 *= C;
        x2 *= C;
        stepSize *= C; // convert to s
    }
};

class DataHelper : public QwtSeriesData<QPointF>
{
public:
    AbstractDataSeriesAdaptor *d;

    DataHelper(AbstractDataSeriesAdaptor *a)
        : d(a)
    {}
    DataHelper(const DataHelper &other)
        : d(other.d)
    {}
    virtual ~DataHelper() { delete d; }

    size_t size() const override { return d->size(); }
    QPointF sample(size_t i) const override { return d->sample(i); }
    QRectF boundingRect() const override { return d->boundingRect(); }
};

class ErrorBarSampleHelper : public QwtSeriesData<QPointF>
{
public:
    AbstractErrorBarAdaptor *d;

    ErrorBarSampleHelper(AbstractErrorBarAdaptor *a)
        : d(a)
    {}
    ErrorBarSampleHelper(const ErrorBarSampleHelper &other)
        : d(other.d)
    {}
    virtual ~ErrorBarSampleHelper() { delete d; }

    size_t size() const override { return d->size(); }
    QPointF sample(size_t i) const override { return d->sample(i); }
    QRectF boundingRect() const override { return d->boundingRect(); } // ??? why not boundingRect
};

class ErrorBarIntervalHelper : public QwtSeriesData<QwtIntervalSample>
{
public:
    AbstractErrorBarAdaptor *d;

    ErrorBarIntervalHelper(AbstractErrorBarAdaptor *a)
        : d(a)
    {}
    ErrorBarIntervalHelper(const ErrorBarIntervalHelper &other)
        : d(other.d)
    {}
    virtual ~ErrorBarIntervalHelper() {}

    size_t size() const override { return d->size(); }
    QwtIntervalSample sample(size_t i) const override
    {
        QPointF s = d->sample(i);
        QPointF v = d->interval(i);
        return QwtIntervalSample(s.x(), v.x(), v.y());
    }
    QRectF boundingRect() const override { return d->errorBoundingRect(); }
};

class StairsDataHelper : public QwtSeriesData<QPointF>
{
public:
    AbstractDataSeriesAdaptor *d;

    StairsDataHelper(AbstractDataSeriesAdaptor *a)
        : d(a)
    {}
    StairsDataHelper(const StairsDataHelper &other)
        : d(other.d)
    {}
    virtual ~StairsDataHelper() { delete d; }

    size_t size() const override { return d->size(); }
    QPointF sample(size_t i) const override { return d->sample(i); }
    QRectF boundingRect() const override { return d->boundingRect(); }
};

struct ImageHelper : public QwtMatrixRasterData
{
    AbstractImageAdaptor *d;
    bool scale;

    ImageHelper(AbstractImageAdaptor *a, bool scale)
        : d(a)
    {
        init();
    }
    ImageHelper(const ImageHelper &other)
        : d(other.d)
        , scale(other.scale)
    {
        init();
    }
    virtual ~ImageHelper() { delete d; }
    void init()
    {
        QPointF v = d->xlim();
        setInterval(Qt::XAxis, QwtInterval(v.x(), v.y()));
        v = d->ylim();
        setInterval(Qt::YAxis, QwtInterval(v.x(), v.y()));
        v = d->zlim();
        setInterval(Qt::ZAxis, QwtInterval(v.x(), v.y()));

        int ncols = d->columns();
        int nrows = d->rows();
        QVector<double> m(ncols * nrows);

        for (int k = 0; k < m.size(); ++k)
            m[k] = d->value(k);

        setValueMatrix(m, ncols);
    }
};

class ColorMapHelper : public QwtColorMap
{
    QVector<QRgb> map;
    bool scale_;

public:
    explicit ColorMapHelper(const QVector<QRgb> &m, bool scale = true)
        : QwtColorMap(QwtColorMap::RGB)
        , map(m)
        , scale_(scale)
    {}

    QRgb rgb(const QwtInterval &interval, double value) const override
    {
        if (qIsNaN(value))
            return 0u;

        double v = value;
        if (scale_) {
            const double width = interval.width();
            if (width <= 0.0)
                return 0u;

            v = (value - interval.minValue()) / width * map.size();
        }

        int i = std::floor(v);
        i = std::min(i, map.size() - 1);
        i = std::max(i, 0);
        return map[i];
    }

#if  QWT_VERSION >= 0x060300
	uint colorIndex( int numColors, const QwtInterval& interval, double value ) const override
#else
    unsigned char colorIndex(const QwtInterval &interval, double value) const override
#endif
    {
        if (qIsNaN(value))
            return 0u;

        const double width = interval.width();
        if (width <= 0.0)
            return 0u;

        const double ratio = (value - interval.minValue()) / width;

        int i = std::floor(ratio * map.size());
        return std::min(i, map.size() - 1);
    }
};

QwtBackend::QwtBackend(QMatPlotWidget *parent)
    : QwtPlot(parent)
    , mMatPlot_(parent)
{
    QwtPlotCanvas *cnv = new QwtPlotCanvas();

    alignScales();

    //cnv->setFrameStyle( QFrame::Box | QFrame::Plain );
    //cnv->setLineWidth( 1 );

    cnv->setStyleSheet("border: 1px solid black; background: white");

    setCanvas(cnv);

    //setCanvasBackground(QColor(Qt::white));

    QFont font;
    font = axisFont(QwtPlot::xBottom);
    font.setPointSize(8);
    setAxisFont(QwtPlot::xBottom, font);
    setAxisFont(QwtPlot::yLeft, font);

    QwtText txt = QwtPlot::title();
    font = txt.font();
    font.setPointSize(10);
    txt.setFont(font);
    QwtPlot::setTitle(txt);

    txt = axisTitle(QwtPlot::xBottom);
    font = txt.font();
    font.setPointSize(9);
    txt.setFont(font);
    setAxisTitle(QwtPlot::xBottom, txt);
    setAxisTitle(QwtPlot::yLeft, txt);

    grid_ = new QwtPlotGrid;
    grid_->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    //grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid_->attach(this);

    setAxisScaleDraw(QwtPlot::xBottom, new SciScaleDraw());
    setAxisScaleDraw(QwtPlot::yLeft, new SciScaleDraw());

    zoomer = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, canvas());
    zoomer->setRubberBand(QwtPicker::RectRubberBand);
    zoomer->setRubberBandPen(QPen(Qt::darkGray, 0, Qt::DashLine));

    //zoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    zoomer->setTrackerMode(QwtPicker::AlwaysOff);

    // RightButton: zoom out by 1
    // Ctrl+RightButton: zoom out to full size

    zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::LeftButton, Qt::ShiftModifier);

    picker = new FormattedPicker(QwtPlot::xBottom,
                                 QwtPlot::yLeft,
                                 //        QwtPicker::PointSelection | QwtPicker::DragSelection,
                                 QwtPlotPicker::CrossRubberBand,
                                 QwtPicker::AlwaysOn,
                                 canvas());
    picker->setRubberBand(QwtPicker::CrossRubberBand);

    scalepicker = new ScalePicker(this);

    setAutoReplot(true);

    grid_->enableX(mMatPlot_->grid());
    grid_->enableY(mMatPlot_->grid());

    connect(this, &QwtBackend::axisClicked, mMatPlot_, &QMatPlotWidget::onAxisClicked);
}

//
//  Taken from qwt/examples/refreshtest
//
void QwtBackend::alignScales()
{
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    for (int i = 0; i < QwtPlot::axisCnt; i++) {
        QwtScaleWidget *scaleWidget = axisWidget(i);
        if (scaleWidget)
            scaleWidget->setMargin(0);

        QwtScaleDraw *scaleDraw = axisScaleDraw(i);
        if (scaleDraw)
            scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    }

    plotLayout()->setAlignCanvasToScales(true);
}

//Respective Qwt names for QwtSymbol::Style
static const QwtSymbol::Style qwt_dotstyles[] = {QwtSymbol::Cross,
                                                 QwtSymbol::Ellipse,
                                                 QwtSymbol::Star1,
                                                 QwtSymbol::Rect,
                                                 QwtSymbol::XCross,
                                                 QwtSymbol::Rect,
                                                 QwtSymbol::Diamond,
                                                 QwtSymbol::UTriangle,
                                                 QwtSymbol::DTriangle,
                                                 QwtSymbol::RTriangle,
                                                 QwtSymbol::LTriangle,
                                                 QwtSymbol::Hexagon,
                                                 QwtSymbol::NoSymbol};

void QwtBackend::plot(AbstractDataSeriesAdaptor *d, const QMatPlotWidget::LineSpec &opt)
{
    QwtPlotCurve *curve = new QwtPlotCurve;

    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setPen(opt.clr, 0.0, opt.penStyle);
    if (opt.markerStyle < QMatPlotWidget::LineSpec::nMarkers) {
        //set a discernible but not very intrusive size for markers
        //make the marker same color as plot
        QwtSymbol *symbol = new QwtSymbol(qwt_dotstyles[opt.markerStyle],
                                          QBrush(Qt::white),
                                          QPen(opt.clr),
                                          QSize(4, 4));
        curve->setSymbol(symbol);
    }

    curve->setData(new DataHelper(d));

    curve->attach(this);

    replot();
}

class MyIntervalCurve : public QwtPlotIntervalCurve
{
public:
    QRectF boundingRect() const override
    {
        QRectF rect = QwtPlotSeriesItem::boundingRect();
        //if (orientation() == Qt::Vertical)
        //    rect.setRect(rect.y(), rect.x(), rect.height(), rect.width());

        return rect;
    }
};

void QwtBackend::errorbar(AbstractErrorBarAdaptor *d, const QMatPlotWidget::LineSpec &opt)
{
    QwtPlotCurve *curve = new QwtPlotCurve;

    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setStyle(QwtPlotCurve::Lines);
    curve->setPen(opt.clr, 0.0, opt.penStyle);
    if (opt.markerStyle < QMatPlotWidget::LineSpec::nMarkers) {
        //set a discernible but not very intrusive size for markers
        //make the marker same color as plot
        QwtSymbol *symbol = new QwtSymbol(qwt_dotstyles[opt.markerStyle],
                                          QBrush(Qt::white),
                                          QPen(opt.clr),
                                          QSize(4, 4));
        curve->setSymbol(symbol);
    }

    curve->setData(new ErrorBarSampleHelper(d));

    curve->attach(this);

    MyIntervalCurve *intervalCurve = new MyIntervalCurve;
    intervalCurve->setStyle(QwtPlotIntervalCurve::NoCurve);
    intervalCurve->setPen(Qt::white);

    QColor c(opt.clr); // skip alpha

    QwtIntervalSymbol *errorBar = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
    errorBar->setWidth(8); // should be something even
    errorBar->setPen(c);

    intervalCurve->setSymbol(errorBar);
    intervalCurve->setRenderHint(QwtPlotItem::RenderAntialiased, false);

    intervalCurve->setSamples(new ErrorBarIntervalHelper(d));
    intervalCurve->attach(this);

    replot();
}

void QwtBackend::image(AbstractImageAdaptor *d, bool scale, const QVector<QRgb> &cmap)
{
    QwtPlotSpectrogram *d_spectrogram = new QwtPlotSpectrogram();
    d_spectrogram->setRenderThreadCount(0); // use system specific thread count

    d_spectrogram->setColorMap(new ColorMapHelper(cmap, scale));

    d_spectrogram->setData(new ImageHelper(d, scale));
    d_spectrogram->attach(this);

    replot();
}

void QwtBackend::setAxisScaling(int axisid, QMatPlotWidget::AxisScale sc)
{
    switch (sc) {
    case QMatPlotWidget::Linear:
        setAxisScaleEngine(axisid, new QwtLinearScaleEngine());
        setAxisScaleDraw(axisid, new SciScaleDraw());
        break;
    case QMatPlotWidget::Log:
        setAxisScaleEngine(axisid, new QwtLogScaleEngine());
        setAxisScaleDraw(axisid, new SciScaleDraw());
        break;
    case QMatPlotWidget::Time:
        setAxisScaleEngine(axisid, new TimeScaleEngine());
        setAxisScaleDraw(axisid, new TimeScaleDraw());
    }
}

void QwtBackend::clear()
{
    // Rtti_PlotItem = 0 , Rtti_PlotGrid , Rtti_PlotScale , Rtti_PlotLegend ,
    //     Rtti_PlotMarker , Rtti_PlotCurve , Rtti_PlotSpectroCurve , Rtti_PlotIntervalCurve ,
    //     Rtti_PlotHistogram , Rtti_PlotSpectrogram , Rtti_PlotGraphic , Rtti_PlotTradingCurve ,
    //     Rtti_PlotBarChart , Rtti_PlotMultiBarChart , Rtti_PlotShape , Rtti_PlotTextLabel ,
    //     Rtti_PlotZone , Rtti_PlotVectorField , Rtti_PlotUserItem = 1000

    detachItems(QwtPlotItem::Rtti_PlotMarker, true);
    detachItems(QwtPlotItem::Rtti_PlotIntervalCurve, true);
    detachItems(QwtPlotItem::Rtti_PlotCurve, true);
    detachItems(QwtPlotItem::Rtti_PlotSpectrogram, true);

    replot();
}

bool QwtBackend::exportToFile(const QString &fname, const QSize &sz)
{
    QwtPlotRenderer plotRenderer;
    QSize szmm(sz);
    if (szmm.isEmpty()) { // convert actual widget size to mm
        QScreen *myScreen = screen();
        szmm = size();
        szmm.rwidth() = int(szmm.width() / myScreen->logicalDotsPerInchX() * 25.4);
        szmm.rheight() = int(szmm.height() / myScreen->logicalDotsPerInchY() * 25.4);
    }
    return plotRenderer.exportTo(this, fname, szmm);
}
