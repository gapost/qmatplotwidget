#include "QMatPlotWidget.h"

#include "qwtbackend.h"

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
#include <QtMath>
#include <QScreen>

#include <math.h>

QMatPlotWidget::QMatPlotWidget(QWidget *parent)
    : QWidget(parent)
    , backend_(new QwtBackend(this))
    , axisScaleX_(Linear)
    , axisScaleY_(Linear)
    , grid_on_(false)
    , colorOrder_(defaultColorOrder())
    , colorIndex_(0)
    , colorMap_(colorMap(Viridis, 64))
{
    QVBoxLayout* const vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    vbox->addWidget((QwtBackend *) backend_);
    setLayout(vbox);

    backend_->setGrid(grid_on_);

    // colorOrder_ << Qt::blue
    //             << Qt::red
    //             << Qt::darkGreen
    //             << Qt::magenta
    //             << Qt::darkBlue
    //             << Qt::darkMagenta
    //             << Qt::darkCyan
    //             << Qt::darkRed;
}

QMatPlotWidget::~QMatPlotWidget(void)
{
}

//getters
QString QMatPlotWidget::title() const
{
    return backend_->title();
}
QString QMatPlotWidget::xlabel() const
{
    return backend_->xlabel();
}
QString QMatPlotWidget::ylabel() const
{
    return backend_->ylabel();
}
bool QMatPlotWidget::autoScaleX() const
{
    return backend_->autoScaleX();
}
bool QMatPlotWidget::autoScaleY() const
{
    return backend_->autoScaleY();
}
QPointF QMatPlotWidget::xlim() const
{
    return backend_->xlim();
}
QPointF QMatPlotWidget::ylim() const
{
    return backend_->ylim();
}

//setters
void QMatPlotWidget::setTitle(const QString& s)
{
    backend_->setTitle(s);
}
void QMatPlotWidget::setXlabel(const QString& s)
{
    backend_->setXlabel(s);
}
void QMatPlotWidget::setYlabel(const QString& s)
{
    backend_->setYlabel(s);
}
void QMatPlotWidget::setAutoScaleX(bool on)
{
    backend_->setAutoScaleX(on);
}
void QMatPlotWidget::setAutoScaleY(bool on)
{
    backend_->setAutoScaleY(on);
}
void QMatPlotWidget::setAxisScaleX(AxisScale sc)
{
    if (sc==axisScaleX_) return;
    backend_->setAxisScaleX(sc);
    axisScaleX_ = sc;
}
void QMatPlotWidget::setAxisScaleY(AxisScale sc)
{
    if (sc==axisScaleY_) return;
    backend_->setAxisScaleY(sc);
    axisScaleY_ = sc;
}
void QMatPlotWidget::setGrid(bool on)
{
    if (grid_on_==on) return;
    backend_->setGrid(on);
    grid_on_ = on;
}

void QMatPlotWidget::setAxisEqual()
{
    backend_->setAxisEqual();
}

void QMatPlotWidget::onAxisClicked(int axisid, const QPoint &pos)
{
    QMenu *menu = createAxisContextMenu(axisid);
    menu->exec(pos);
}
void QMatPlotWidget::setXlim(const QPointF& v)
{
    backend_->setXlim(v);
}
void QMatPlotWidget::setYlim(const QPointF &v)
{
    backend_->setYlim(v);
}
void QMatPlotWidget::setColorOrder(const QVector<QRgb> &c)
{
    colorOrder_ = c;
    colorIndex_ = 0;
}

void QMatPlotWidget::setColorMap(const QVector<QRgb> &c)
{
    colorMap_ = c;
}

void QMatPlotWidget::__plot__(AbstractDataSeriesAdaptor *data,
                              const QString &attr,
                              const QColor &clr)
{
    LineSpec opt = LineSpec::fromMatlabLineSpec(attr);

    QColor plotClr;
    if (opt.clr.isValid())
        plotClr = opt.clr;
    else if (clr.isValid())
        plotClr = clr;
    else
        plotClr = QColor(colorOrder_[colorIndex_++ % colorOrder_.size()]);

    opt.clr = plotClr;

    backend_->plot(data, opt);
}

void QMatPlotWidget::__errorbar__(AbstractErrorBarAdaptor *d, const QString &attr, const QColor &clr)
{
    LineSpec opt = LineSpec::fromMatlabLineSpec(attr);

    QColor plotClr;
    if (opt.clr.isValid())
        plotClr = opt.clr;
    else if (clr.isValid())
        plotClr = clr;
    else
        plotClr = QColor(colorOrder_[colorIndex_++ % colorOrder_.size()]);

    opt.clr = plotClr;

    backend_->errorbar(d, opt);
}

void QMatPlotWidget::clear()
{
    backend_->clear();
    colorIndex_ = 0;
    replot();
}
void QMatPlotWidget::replot()
{
    backend_->replot();
}

QSize QMatPlotWidget::minimumSizeHint() const
{
    return QSize(400, 300);
}

bool QMatPlotWidget::exportToFile(const QString& fname, const QSize& sz)
{
    return backend_->exportToFile(fname, sz);
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

    a = menu->addSeparator();

    a = menu->addAction(QString("Linear Scale"),
                        this,
                        isX ? SLOT(setLinearScaleX()) : SLOT(setLinearScaleY()));
    a->setCheckable(true);
    a->setChecked(isX ? linearScaleX() : linearScaleY());

    a = menu->addAction(QString("Log Scale"),
                        this,
                        isX ? SLOT(setLogScaleX()) : SLOT(setLogScaleY()));
    a->setCheckable(true);
    a->setChecked(isX ? logScaleX() : logScaleY());

    a = menu->addAction(QString("Time Scale"),
                        this,
                        isX ? SLOT(setTimeScaleX()) : SLOT(setTimeScaleY()));
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

QMatPlotWidget::LineSpec QMatPlotWidget::LineSpec::fromMatlabLineSpec(const QString &attr)
{
    // MATLAB-Octave-style linestyles for reference
    // mind the order of appearence (e.g. first look for "--", then for "-")
    static const char *linestyles[] = {
        "-.",
        "--",
        "-",
        ":",
    };

    //Respective Qwt names for QwtPlotCurve::setPen Style: (a Qt::PenStyle object)
    static const Qt::PenStyle penstyles[] = {Qt::DashDotLine,
                                             Qt::DashLine,
                                             Qt::SolidLine,
                                             Qt::DotLine,
                                             Qt::NoPen};

    // MATLAB/OCTAVE color spec codes
    static const char *colorCodes = "krgbymcw";

    // Respective Qt colors
    static const Qt::GlobalColor qtColors[]
        = {Qt::black, Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::magenta, Qt::cyan, Qt::white};

    LineSpec opt;

    if (attr.isEmpty())
        return opt;

    // analyze attribute string
    QString str(attr);
    // get linestyle
    if (!str.isEmpty()) {
        int i = 0;
        for (; i < 4; i++) {
            int idx;
            if ((idx = str.indexOf(linestyles[i])) != -1) {
                str.remove(idx, strlen(linestyles[i]));
                break;
            }
        }
        opt.penStyle = penstyles[i];
    }
    // get marker style
    if (!str.isEmpty()) {
        int i = 0;
        for (; i < nMarkers; i++) {
            int idx;
            if ((idx = str.indexOf(markers[i])) != -1) {
                str.remove(idx, 1);
                break;
            }
        }
        opt.markerStyle = i;
    }
    // get color
    if (!str.isEmpty()) {
        int i = 0;
        for (; i < 8; i++) {
            int idx;
            if ((idx = str.indexOf(QChar(colorCodes[i]), Qt::CaseInsensitive)) != -1) {
                str.remove(idx, 1);
                break;
            }
        }
        if (i < 8)
            opt.clr = QColor(qtColors[i]);
    }
    // check that there is something to show
    if (opt.penStyle == Qt::NoPen && opt.markerStyle == nMarkers)
        opt.penStyle = Qt::SolidLine;

    return opt;
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
