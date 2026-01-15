#ifndef _QMATPLOTWIDGET_P_H_
#define _QMATPLOTWIDGET_P_H_

#include "qmatplotwidget.h"

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
    // setters
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

class QLineEdit;
class QCheckBox;
class QComboBox;

class QMatPlotAxisDlg : public QDialog
{
    Q_OBJECT

    QLineEdit *edtTitle;
    QCheckBox *chkAutoScale;
    QLineEdit *edtMinVal;
    QLineEdit *edtMaxVal;
    QComboBox *cbAxisScale;
    friend class QMatPlotWidget;

public:
    QMatPlotAxisDlg(QWidget *parent);

private slots:
    void onAutoScale(bool on);
};

#endif
// #ifndef _QMATPLOTWIDGET_P_H_
