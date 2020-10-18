#include "QMatPlotWidget.h"

#include <QDebug>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QSet>
#include <QRegularExpression>
#include <QDateTime>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMenu>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QValidator>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_dict.h>
#include <qwt_series_data.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_math.h>

#include <math.h>

struct LineSpec
{
    LineSpec() : penStyle(Qt::NoPen),
        markerStyle(QwtSymbol::NoSymbol)
    {}
    Qt::PenStyle penStyle;
    QwtSymbol::Style markerStyle;
    QColor clr;
};

void parseMatlabLineSpec(const QString& attr, LineSpec& opt);

class FormattedPicker : public QwtPlotPicker
{
public:
    FormattedPicker(int xAxis, int yAxis,
        RubberBand rubberBand, DisplayMode trackerMode,
        QWidget *pc) : QwtPlotPicker(xAxis,yAxis,rubberBand,trackerMode,pc)
    {
    }
protected:
    QwtText trackerTextF	(	const QPointF & 	pos	 ) const override
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
    QString createLabelText (const QPointF & pos) const
    {
        QwtText lx = plot()->axisScaleDraw(QwtPlot::xBottom)->label(pos.x());
        QwtText ly = plot()->axisScaleDraw(QwtPlot::yLeft)->label(pos.y());
        QString S( lx.text() );
        S += QChar(',');
        S += ly.text();
        return S;
    }
};

class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QWidget *canvas):
        QwtPlotZoomer(xAxis, yAxis, canvas)
    {}
protected:
    void begin () override
    {
        if (zoomRectIndex()==0)
        {
            setZoomBase(false);

            const QRectF &rect = this->scaleRect();
            QwtPlot *plt = plot();
            if (plt->axisAutoScale(QwtPlot::xBottom))
            {
                plt->setAxisScale(QwtPlot::xBottom,rect.left(),rect.right());
            }
            if (plt->axisAutoScale(QwtPlot::yLeft))
            {
                plt->setAxisScale(QwtPlot::yLeft,rect.top(),rect.bottom());
            }
        }

        QwtPlotZoomer::begin();
    }
    bool end (bool ok=true) override
    {
        return QwtPlotZoomer::end(ok);
    }
    void widgetMouseDoubleClickEvent (QMouseEvent *) override
    {
        QwtPlot *plt = plot();
        if ( !plt ) return;
        plt->setAxisAutoScale(QwtPlot::xBottom);
        plt->setAxisAutoScale(QwtPlot::yLeft);

        setZoomStack(zoomStack(),0); //QStack<QwtDoubleRect>());
    }
};


class TimeScaleDraw: public QwtScaleDraw
{
public:
    QwtText label(double secsSinceEpoch) const override
    {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(1000*secsSinceEpoch);
        return dt.toString("hh:mm:ss");
    }
};

class SciScaleDraw: public QwtScaleDraw
{
public:
    QwtText label(double v) const override
    {
        return QString::number(v,'g');
    }
};

class TimeScaleEngine: public QwtLinearScaleEngine
{
protected:
    static double conversion_factor(double T)
    {
        double C = 1.; // conversion const
        if (T<=60) C = 1.; // secs up to 1 min
        else if (T<=3600) C = 60; // mins up to 1hr
        else if (T<=24*3600) C = 60*60; // hr up to 1 day
        else C = 24*60*60; // else in days
        return C;
    }
public:
    void autoScale(int maxSteps,
        double &x1, double &x2, double &stepSize) const override
    {
        double C = conversion_factor(fabs(x2-x1));
        x1 /= C; x2 /= C; // convert to  s,m,hr,d ...
        QwtLinearScaleEngine::autoScale(maxSteps,x1,x2,stepSize);
        x1 *= C; x2 *= C; stepSize *= C; // convert to s
    }
};



class QMatPlotWidget::Implementation : public QwtPlot
{
public:
    class ScalePicker: public QObject
    {
        QMatPlotWidget::Implementation* mPlot_;
    public:
        ScalePicker( QMatPlotWidget::Implementation *plot ) : QObject(plot),
            mPlot_(plot)
        {
            for ( uint i = 0; i < QwtPlot::axisCnt; i++ )
            {
                QwtScaleWidget *scaleWidget = plot->axisWidget( i );
                if ( scaleWidget )
                    scaleWidget->installEventFilter( this );
            }
        }

        bool eventFilter( QObject * object, QEvent * event) override
        {
            if ( event->type() == QEvent::MouseButtonPress  )
            {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
                if (mouseEvent->button()==Qt::RightButton)
                {
                    QwtScaleWidget *scaleWidget = qobject_cast<QwtScaleWidget *>( object );
                    if ( scaleWidget )
                    {
                        mouseClicked( scaleWidget, mouseEvent );
                        return true;
                    }
                }
            }
            return QObject::eventFilter( object, event );
        }

        void mouseClicked( const QwtScaleWidget * scale, QMouseEvent * event)
        {
            const QPoint & pos = event->pos();
            QRect rect = scaleRect( scale );

            int margin = 10; // 10 pixels tolerance
            rect.setRect( rect.x() - margin, rect.y() - margin,
                          rect.width() + 2 * margin, rect.height() +  2 * margin );

            if ( rect.contains( pos ) ) // No click on the title
            {
                int axis;
                switch( scale->alignment() )
                {
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
                }
                //qDebug() << "Mouse " << event->type() << " on axis " << axis << ", v=" << value;
                QMenu* menu = mPlot_->mMatPlot_->createAxisContextMenu(axis);
                menu->exec(scale->mapToGlobal(pos));
            }
        }
        QRect scaleRect( const QwtScaleWidget * scale) const
        {
            const int bld = scale->margin();
            const int mjt = qCeil( scale->scaleDraw()->maxTickLength() );
            const int sbd = scale->startBorderDist();
            const int ebd = scale->endBorderDist();

            QRect rect;
            switch( scale->alignment() )
            {
                case QwtScaleDraw::LeftScale:
                {
                    rect.setRect( scale->width() - bld - mjt, sbd,
                        mjt, scale->height() - sbd - ebd );
                    break;
                }
                case QwtScaleDraw::RightScale:
                {
                    rect.setRect( bld, sbd,
                        mjt, scale->height() - sbd - ebd );
                    break;
                }
                case QwtScaleDraw::BottomScale:
                {
                    rect.setRect( sbd, bld,
                        scale->width() - sbd - ebd, mjt );
                    break;
                }
                case QwtScaleDraw::TopScale:
                {
                    rect.setRect( sbd, scale->height() - bld - mjt,
                        scale->width() - sbd - ebd, mjt );
                    break;
                }
            }
            return rect;
        }
    };
    QwtPlotGrid* grid_;
    QwtPlotZoomer* zoomer;
    QwtPlotPanner* panner;
    QwtPlotPicker* picker;
    ScalePicker* scalepicker;
    QMatPlotWidget* mMatPlot_;

    Implementation(QMatPlotWidget* parent) : QwtPlot(parent),
        mMatPlot_(parent)
    {
        QwtPlotCanvas *cnv = new QwtPlotCanvas();

        alignScales();

        cnv->setFrameStyle( QFrame::Box | QFrame::Plain );
        cnv->setLineWidth( 1 );

        setCanvas( cnv );

        setCanvasBackground(QColor(Qt::white));

        QFont font;
        font = axisFont(QwtPlot::xBottom);
        font.setPointSize(8);
        setAxisFont(QwtPlot::xBottom,font);
        setAxisFont(QwtPlot::yLeft,font);

        QwtText txt = QwtPlot::title();
        font = txt.font();
        font.setPointSize(10);
        txt.setFont(font);
        QwtPlot::setTitle(txt);

        txt = axisTitle(QwtPlot::xBottom);
        font = txt.font();
        font.setPointSize(9);
        txt.setFont(font);
        setAxisTitle(QwtPlot::xBottom,txt);
        setAxisTitle(QwtPlot::yLeft,txt);


        grid_ = new QwtPlotGrid;
        grid_->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
        //grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
        grid_->attach(this);

        setAxisScaleDraw(QwtPlot::xBottom, new SciScaleDraw());
        setAxisScaleDraw(QwtPlot::yLeft, new SciScaleDraw());

        zoomer = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, canvas());
        zoomer->setRubberBand(QwtPicker::RectRubberBand);
        zoomer->setRubberBandPen(QPen(Qt::darkGray,0,Qt::DashLine));

        //zoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
        zoomer->setTrackerMode(QwtPicker::AlwaysOff);

            // RightButton: zoom out by 1
            // Ctrl+RightButton: zoom out to full size


        zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                Qt::RightButton, Qt::ControlModifier);
        zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                Qt::RightButton);

        panner = new QwtPlotPanner(canvas());
        panner->setMouseButton(Qt::LeftButton,Qt::ShiftModifier);

        picker = new FormattedPicker(QwtPlot::xBottom, QwtPlot::yLeft,
    //        QwtPicker::PointSelection | QwtPicker::DragSelection,
            QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
            canvas());
        picker->setRubberBand(QwtPicker::CrossRubberBand);

        scalepicker = new ScalePicker(this);

        setAutoReplot(true);
    }

    //
    //  Taken from qwt/examples/refreshtest
    //
    void alignScales()
    {
        // The code below shows how to align the scales to
        // the canvas frame, but is also a good example demonstrating
        // why the spreaded API needs polishing.

        for ( int i = 0; i < QwtPlot::axisCnt; i++ )
        {
            QwtScaleWidget *scaleWidget = axisWidget( i );
            if ( scaleWidget )
                scaleWidget->setMargin( 0 );

            QwtScaleDraw *scaleDraw = axisScaleDraw( i );
            if ( scaleDraw )
                scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
        }

        plotLayout()->setAlignCanvasToScales( true );
    }

    void setAxisScaling(int axisid, QMatPlotWidget::AxisScale sc)
    {
        switch (sc)
        {
        case Linear:
            setAxisScaleEngine(axisid, new QwtLinearScaleEngine());
            setAxisScaleDraw(axisid, new SciScaleDraw());
            break;
        case Log:
            setAxisScaleEngine(axisid, new QwtLogScaleEngine());
            setAxisScaleDraw(axisid, new SciScaleDraw());
            break;
        case Time:
            setAxisScaleEngine(axisid, new TimeScaleEngine());
            setAxisScaleDraw(axisid, new TimeScaleDraw());
        }
    }


};


QMatPlotWidget::QMatPlotWidget(QWidget* parent) :
    QWidget(parent),
    impl_(new Implementation(this)),
    axisScaleX_(Linear), axisScaleY_(Linear),
    grid_on_(false),
    colorIndex_(0)
{
    QVBoxLayout* const vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    vbox->addWidget(impl_);
    setLayout(vbox);

    impl_->grid_->enableX(grid_on_);
    impl_->grid_->enableY(grid_on_);

    colorOrder_ << Qt::blue
                << Qt::red
                << Qt::darkGreen
                << Qt::magenta
                << Qt::darkBlue
                << Qt::darkMagenta
                << Qt::darkCyan
                << Qt::darkRed;
}

QMatPlotWidget::~QMatPlotWidget(void)
{
}

//getters
QString QMatPlotWidget::title() const
{
    return impl_->QwtPlot::title().text();
}
QString QMatPlotWidget::xlabel() const
{
    return impl_->axisTitle(QwtPlot::xBottom).text();
}
QString QMatPlotWidget::ylabel() const
{
    return impl_->axisTitle(QwtPlot::yLeft).text();
}
bool QMatPlotWidget::autoScaleX() const
{
    return impl_->axisAutoScale(QwtPlot::xBottom);
}
bool QMatPlotWidget::autoScaleY() const
{
    return impl_->axisAutoScale(QwtPlot::yLeft);
}
QPointF QMatPlotWidget::xlim() const
{
    return QPointF(impl_->axisScaleDiv(QwtPlot::xBottom).lowerBound(),
                   impl_->axisScaleDiv(QwtPlot::xBottom).upperBound());
}
QPointF QMatPlotWidget::ylim() const
{
    return QPointF(impl_->axisScaleDiv(QwtPlot::yLeft).lowerBound(),
                   impl_->axisScaleDiv(QwtPlot::yLeft).upperBound());
}

//setters
void QMatPlotWidget::setTitle(const QString& s)
{
   impl_->QwtPlot::setTitle(s);
}
void QMatPlotWidget::setXlabel(const QString& s)
{
    impl_->setAxisTitle(QwtPlot::xBottom,s);
}
void QMatPlotWidget::setYlabel(const QString& s)
{
    impl_->setAxisTitle(QwtPlot::yLeft,s);
}
void QMatPlotWidget::setAutoScaleX(bool on)
{
    impl_->setAxisAutoScale(QwtPlot::xBottom,on);
}
void QMatPlotWidget::setAutoScaleY(bool on)
{
    impl_->setAxisAutoScale(QwtPlot::yLeft,on);
}
void QMatPlotWidget::setAxisScaleX(AxisScale sc)
{
    if (sc==axisScaleX_) return;
    impl_->setAxisScaling(QwtPlot::xBottom, sc);
    axisScaleX_ = sc;
}
void QMatPlotWidget::setAxisScaleY(AxisScale sc)
{
    if (sc==axisScaleY_) return;
    impl_->setAxisScaling(QwtPlot::yLeft, sc);
    axisScaleY_ = sc;
}
void QMatPlotWidget::setTimeScaleX(bool on)
{
    if (on != (axisScaleX_==Time)) setAxisScaleX(on ? Time : Linear);
}
void QMatPlotWidget::setTimeScaleY(bool on)
{
    if (on != (axisScaleY_==Time)) setAxisScaleY(on ? Time : Linear);
}
void QMatPlotWidget::setLogScaleX(bool on)
{
    if (on != (axisScaleX_==Log)) setAxisScaleX(on ? Log : Linear);
}
void QMatPlotWidget::setLogScaleY(bool on)
{
    if (on != (axisScaleY_==Log)) setAxisScaleY(on ? Log : Linear);
}
void QMatPlotWidget::setLinearScaleX(bool on)
{
    if (on != (axisScaleX_==Linear)) setAxisScaleX(on ? Linear : Log);
}
void QMatPlotWidget::setLinearScaleY(bool on)
{
    if (on != (axisScaleY_==Linear)) setAxisScaleY(on ? Linear : Log);
}
void QMatPlotWidget::setGrid(bool on)
{
    if (grid_on_==on) return;
    impl_->grid_->enableX(on);
    impl_->grid_->enableY(on);
    grid_on_ = on;
}
void QMatPlotWidget::setXlim(const QPointF& v)
{
    impl_->setAxisScale(QwtPlot::xBottom,v.x(),v.y()) ;
}
void QMatPlotWidget::setYlim(const QPointF &v)
{
    impl_->setAxisScale(QwtPlot::yLeft,v.x(),v.y()) ;
}
void QMatPlotWidget::setColorOrder(const QVector<QColor>& c)
{
    colorOrder_ = c;
    colorIndex_ = 0;
}

class DataHelper : public QwtSeriesData< QPointF >
{
    public:
    QMatPlotWidget::AbstractDataSeries* d;

    DataHelper(QMatPlotWidget::AbstractDataSeries* a) : d(a)
    {
    }
    DataHelper(const DataHelper& other) : d(other.d)
    {
    }
    virtual ~DataHelper()
    {
        delete d;
    }

    size_t size() const override { return d->size(); }
    QPointF sample( size_t i ) const override { return d->sample(i); }
    QRectF boundingRect() const override { return d->boundingRect(); }
};

void QMatPlotWidget::plotDataSeries(AbstractDataSeries* data, const QString &attr, const QColor &clr)
{
    LineSpec opt;
    parseMatlabLineSpec(attr, opt);

    QColor plotClr;
    if (opt.clr.isValid())
        plotClr = opt.clr;
    else if (clr.isValid())
        plotClr = clr;
    else
        plotClr = colorOrder_[colorIndex_++ % colorOrder_.size()];

    QwtPlotCurve* curve = new QwtPlotCurve;

    curve->setStyle(QwtPlotCurve::Lines);
    curve->setPen(plotClr, 0.0, opt.penStyle);
    if (opt.markerStyle!=QwtSymbol::NoSymbol)
    {
        //set a discernible but not very intrusive size for markers
        //make the marker same color as plot
        QwtSymbol * symbol = new QwtSymbol(
                    opt.markerStyle,
                    QBrush(Qt::white),
                    QPen(plotClr),
                    QSize(4,4)
                    );
        curve->setSymbol(symbol);
    }

    curve->setData(new DataHelper(data));

    curve->attach(impl_);

    impl_->replot();
}

void QMatPlotWidget::clear()
{
    impl_->detachItems(QwtPlotItem::Rtti_PlotCurve,true);

    colorIndex_ = 0;

    impl_->replot();
}
void QMatPlotWidget::replot()
{
    impl_->replot();
}

QSize QMatPlotWidget::minimumSizeHint() const
{
    return QSize(400, 300);
}
QSize QMatPlotWidget::sizeHint() const
{
    return QSize(600, 450);
}
QMenu* QMatPlotWidget::createAxisContextMenu(int axisid)
{
    QMenu* menu = new QMenu(this);

    bool isX = (axisid == QwtPlot::xBottom) || (axisid == QwtPlot::xTop);
    QChar axChar = isX ? 'X' : 'Y';

    QAction* a;
    a = menu->addAction(QString("Auto Scale %1").arg(axChar),
                        this,
                        isX ? SLOT(setAutoScaleX(bool)) : SLOT(setAutoScaleY(bool)));
    a->setCheckable(true);
    a->setChecked(isX ? autoScaleX() : autoScaleY());

    a = menu->addAction(QString("Linear Scale"),
                        this,
                        isX ? SLOT(setLinearScaleX(bool)) : SLOT(setLinearScaleY(bool)));
    a->setCheckable(true);
    a->setChecked(isX ? linearScaleX() : linearScaleY());

    a = menu->addAction(QString("Log Scale"),
                        this,
                        isX ? SLOT(setLogScaleX(bool)) : SLOT(setLogScaleY(bool)));
    a->setCheckable(true);
    a->setChecked(isX ? logScaleX() : logScaleY());

    a = menu->addAction(QString("Time Scale"),
                        this,
                        isX ? SLOT(setTimeScaleX(bool)) : SLOT(setTimeScaleY(bool)));
    a->setCheckable(true);
    a->setChecked(isX ? timeScaleX() : timeScaleY());

    a = menu->addSeparator();

    a = menu->addAction(QString("%1 Axis Properties").arg(axChar),
                        this,
                        isX ? SLOT(xAxisPropDlg()) : SLOT(yAxisPropDlg()));

    return menu;
}
void QMatPlotWidget::axisPropertyDialog(int axisid)
{
    QMatPlotAxisDlg dlg(this);
    bool isX = axisid==0;

    dlg.edtTitle->setText(isX ? xlabel() : ylabel());
    dlg.chkAutoScale->setChecked(isX ? autoScaleX() : autoScaleY());
    QPointF lims = isX ? xlim() : ylim();
    dlg.edtMinVal->setText(QString::number(lims.x()));
    dlg.edtMaxVal->setText(QString::number(lims.y()));
    dlg.cbAxisScale->setCurrentIndex(isX ? axisScaleX() : axisScaleY());

    if (dlg.exec()==QDialog::Accepted)
    {
        if (isX)
        {
            setXlabel(dlg.edtTitle->text());
            setAutoScaleX(dlg.chkAutoScale->isChecked());
            if (!autoScaleX())
            {
                QPointF rng(dlg.edtMinVal->text().toDouble(),
                            dlg.edtMaxVal->text().toDouble());
                setXlim(rng);
            }
            setAxisScaleX((AxisScale(dlg.cbAxisScale->currentIndex())));
        }
        else
        {
            setYlabel(dlg.edtTitle->text());
            setAutoScaleY(dlg.chkAutoScale->isChecked());
            if (!autoScaleY())
            {
                QPointF rng(dlg.edtMinVal->text().toDouble(),
                            dlg.edtMaxVal->text().toDouble());
                setYlim(rng);
            }
            setAxisScaleY((AxisScale(dlg.cbAxisScale->currentIndex())));
        }
    }
}

void parseMatlabLineSpec(const QString& attr, LineSpec& opt)
{
    // MATLAB-Octave-style linestyles for reference
    // mind the order of appearence (e.g. first look for "--", then for "-")
    static const char* linestyles[] =
    {"-.","--","-",":",};

    //Respective Qwt names for QwtPlotCurve::setPen Style: (a Qt::PenStyle object)
    static const Qt::PenStyle penstyles[] =
    {Qt::DashDotLine,Qt::DashLine,Qt::SolidLine,Qt::DotLine,Qt::NoPen};

    //MATLAB-Octave-style markerstyles for reference (not all)
    static const char* markerstyles[] =
    {"+","o","*",".","x","s","d","^","v",">","<","h"};

    //Respective Qwt names for QwtSymbol::Style
    static const QwtSymbol::Style dotstyles[] =
    {QwtSymbol::Cross,  QwtSymbol::Ellipse, QwtSymbol::Star1,   QwtSymbol::Rect,
     QwtSymbol::XCross, QwtSymbol::Rect,    QwtSymbol::Diamond, QwtSymbol::UTriangle,
     QwtSymbol::DTriangle, QwtSymbol::RTriangle, QwtSymbol::LTriangle, QwtSymbol::Hexagon,
     QwtSymbol::NoSymbol};

    // MATLAB/OCTAVE color spec codes
    static const char* colorCodes = "krgbymcw";

    // Respective Qt colors
    static const Qt::GlobalColor qtColors[] =
    { Qt::black,  Qt::red,     Qt::green, Qt::blue,
      Qt::yellow, Qt::magenta, Qt::cyan,  Qt::white};

    if (attr.isEmpty()) return;

    // analyze attribute string
    QString str(attr);
    // get linestyle
    if (!str.isEmpty())
    {
        int i = 0;
        for(; i<4; i++)
        {
            int idx;
            if ((idx = str.indexOf(linestyles[i]))!=-1) {
                str.remove(idx,strlen(linestyles[i]));
                break;
            }
        }
        opt.penStyle = penstyles[i];
    }
    // get marker style
    if (!str.isEmpty())
    {
        int i = 0;
        for(; i<12; i++)
        {
            int idx;
            if ((idx = str.indexOf(markerstyles[i]))!=-1) {
                str.remove(idx,strlen(markerstyles[i]));
                break;
            }
        }
        opt.markerStyle = dotstyles[i];
    }
    // get color
    if (!str.isEmpty())
    {
        int i = 0;
        for(; i<8; i++)
        {
            int idx;
            if ((idx = str.indexOf(QChar(colorCodes[i]),Qt::CaseInsensitive))!=-1) {
                str.remove(idx,1);
                break;
            }
        }
        if (i<8) opt.clr = QColor(qtColors[i]);
    }
    // check that there is something to show
    if (opt.penStyle==Qt::NoPen && opt.markerStyle==QwtSymbol::NoSymbol)
        opt.penStyle = Qt::SolidLine;


}

/////////////////// QMatPlotAxisDlg ///////////////////////////

QMatPlotAxisDlg::QMatPlotAxisDlg(QWidget *parent) : QDialog(parent)
{
     resize(260, 290);

     QVBoxLayout* verticalLayout = new QVBoxLayout(this);
     QFormLayout* formLayout = new QFormLayout();
     QLabel* label;

     label = new QLabel(this);
     label->setText("Title");
     formLayout->setWidget(0, QFormLayout::LabelRole, label);
     edtTitle = new QLineEdit(this);
     edtTitle->setObjectName(QStringLiteral("edtTitle"));
     formLayout->setWidget(0, QFormLayout::FieldRole, edtTitle);

     label = new QLabel(this);
     label->setText("Autoscale");
     formLayout->setWidget(1, QFormLayout::LabelRole, label);
     chkAutoScale = new QCheckBox(this);
     chkAutoScale->setObjectName(QStringLiteral("chkAutoScale"));
     formLayout->setWidget(1, QFormLayout::FieldRole, chkAutoScale);

     label = new QLabel(this);
     label->setText("Min Value");
     formLayout->setWidget(2, QFormLayout::LabelRole, label);
     edtMinVal = new QLineEdit(this);
     edtMinVal->setObjectName(QStringLiteral("edtMinVal"));
     edtMinVal->setValidator(new QDoubleValidator(this));
     formLayout->setWidget(2, QFormLayout::FieldRole, edtMinVal);

     label = new QLabel(this);
     label->setText("Max Value");
     formLayout->setWidget(3, QFormLayout::LabelRole, label);
     edtMaxVal = new QLineEdit(this);
     edtMaxVal->setObjectName(QStringLiteral("edtMaxVal"));
     edtMaxVal->setValidator(new QDoubleValidator(this));
     formLayout->setWidget(3, QFormLayout::FieldRole, edtMaxVal);

     label = new QLabel(this);
     label->setText("Scale");
     formLayout->setWidget(4, QFormLayout::LabelRole, label);
     cbAxisScale = new QComboBox(this);
     cbAxisScale->setObjectName(QStringLiteral("cbAxisScale"));
     cbAxisScale->insertItems(0, QStringList()
              << "Linear"
              << "Log"
              << "Time");
     formLayout->setWidget(4, QFormLayout::FieldRole, cbAxisScale);

     verticalLayout->addLayout(formLayout);

     QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
     buttonBox->setObjectName(QStringLiteral("buttonBox"));
     buttonBox->setOrientation(Qt::Horizontal);
     buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
     buttonBox->setCenterButtons(false);

     verticalLayout->addWidget(buttonBox);

     setLayout(verticalLayout);

     QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
     QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
     QObject::connect(chkAutoScale,SIGNAL(toggled(bool)), this, SLOT(onAutoScale(bool)));

}

void QMatPlotAxisDlg::onAutoScale(bool on)
{
    edtMinVal->setEnabled(!on);
    edtMaxVal->setEnabled(!on);
}



