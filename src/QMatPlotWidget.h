#ifndef _QMATPLOTWIDGET_H_
#define _QMATPLOTWIDGET_H_

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
    Q_PROPERTY(bool timeScaleX READ timeScaleX WRITE setTimeScaleX)
    Q_PROPERTY(bool timeScaleY READ timeScaleY WRITE setTimeScaleY)
    Q_PROPERTY(bool logScaleX READ logScaleX WRITE setLogScaleX)
    Q_PROPERTY(bool logScaleY READ logScaleY WRITE setLogScaleY)
    Q_PROPERTY(bool linearScaleX READ linearScaleX WRITE setLinearScaleX)
    Q_PROPERTY(bool linearScaleY READ linearScaleY WRITE setLinearScaleY)
    Q_PROPERTY(AxisScale axisScaleX READ axisScaleX WRITE setAxisScaleX)
    Q_PROPERTY(AxisScale axisScaleY READ axisScaleY WRITE setAxisScaleY)
    Q_PROPERTY(bool grid READ grid WRITE setGrid)
    Q_PROPERTY(QPointF xlim READ xlim WRITE setXlim)
    Q_PROPERTY(QPointF ylim READ ylim WRITE setYlim)
    Q_PROPERTY(QVector<QColor> colorOrder READ colorOrder WRITE setColorOrder)

public:
    enum AxisScale {
        Linear = 0,
        Log,
        Time
    };
    Q_ENUM(AxisScale)

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
    QVector<QColor> colorOrder() const { return colorOrder_; }

    //setters
    void setTitle(const QString& s);
    void setXlabel(const QString& s);
    void setYlabel(const QString& s);
    void setXlim(const QPointF& v);
    void setYlim(const QPointF& v);
    void setColorOrder(const QVector<QColor>& c);

    // QWidget overrides
    QSize sizeHint () const override;
    QSize minimumSizeHint () const override;

public slots:
    void clear();
    void replot();

    // slot setters
    void setAutoScaleX(bool on);
    void setAutoScaleY(bool on);
    void setAxisScaleX(AxisScale sc);
    void setAxisScaleY(AxisScale sc);
    void setTimeScaleX(bool on);
    void setTimeScaleY(bool on);
    void setLogScaleX(bool on);
    void setLogScaleY(bool on);
    void setLinearScaleX(bool on);
    void setLinearScaleY(bool on);
    void setGrid(bool on);

public:
    template<class VectorType>
    void plot(const VectorType& x, const VectorType& y,
              const QString &attr = QString(), const QColor& clr = QColor());
    template<class VectorType>
    void plot(const VectorType& y,
              const QString &attr = QString(), const QColor& clr = QColor());

    class AbstractDataSeries
    {
    public:
        virtual ~AbstractDataSeries() {}
        virtual int size() const = 0;
        virtual QPointF sample(int i) const = 0;
        virtual QRectF boundingRect() const = 0;
    };

    friend class DataHelper;

    void plotDataSeries(AbstractDataSeries* d, const QString &attr, const QColor& clr);

protected:
    virtual QMenu* createAxisContextMenu(int axisid);
    virtual void axisPropertyDialog(int axisid);

protected slots:
    void xAxisPropDlg() { axisPropertyDialog(0); }
    void yAxisPropDlg() { axisPropertyDialog(1); }

private:

    class Implementation;
    Implementation* const impl_;
    friend class Implementation;

    AxisScale axisScaleX_, axisScaleY_;
    bool grid_on_;

    QVector<QColor> colorOrder_;
    int colorIndex_;


};

template<class V_>
class SingleDataSeries_ : public QMatPlotWidget::AbstractDataSeries
{
    V_ vy;
public:
    explicit SingleDataSeries_(const V_& y) : vy(y)
    {
    }
    int size() const override { return vy.size(); }
    QPointF sample( int i ) const override { return QPointF(1.*i,vy[i]); }
    QRectF boundingRect() const override
    {
        if (!size()) return QRectF();
        qreal y1(vy[0]), y2(y1);
        for(int i=1; i<size(); ++i)
        {
            if (vy[i]<y1) y1 = vy[i];
            else if (vy[i]>y2) y2 = vy[i];
        }
        return QRectF(0,y1,size()-1,y2-y1);
    }
};

template<class VectorType>
void QMatPlotWidget::plot(const VectorType& y, const QString &attr, const QColor& clr)
{
    typedef SingleDataSeries_<VectorType> mydatat;
    mydatat* data = new mydatat(y);
    plotDataSeries(data,attr,clr);
}


template<class V_>
class DataSeries_ : public QMatPlotWidget::AbstractDataSeries
{
    V_ vx;
    V_ vy;
public:
    DataSeries_(const V_& x, const V_& y) : vx(x), vy(y)
    {
    }
    DataSeries_(const DataSeries_& other) : vx(other.vx), vy(other.vy)
    {
    }
    int size() const override { return qMin(vx.size(),vy.size()); }
    QPointF sample( int i ) const override
    {
        return QPointF(vx[i],vy[i]);
    }
    QRectF boundingRect() const override
    {
        if (!size()) return QRectF();
        qreal x1(vx[0]), x2(x1);
        qreal y1(vy[0]), y2(y1);
        for(int i=1; i<size(); ++i)
        {
            if (vx[i]<x1) x1 = vx[i];
            else if (vx[i]>x2) x2 = vx[i];
            if (vy[i]<y1) y1 = vy[i];
            else if (vy[i]>y2) y2 = vy[i];
        }
        return QRectF(x1,y1,x2-x1,y2-y1);
    }
};



template<class VectorType>
void QMatPlotWidget::plot(const VectorType& x, const VectorType& y, const QString &attr, const QColor& clr)
{
    DataSeries_<VectorType>* data = new DataSeries_<VectorType>(x,y);
    plotDataSeries(data,attr,clr);
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
