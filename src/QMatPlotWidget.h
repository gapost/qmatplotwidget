#ifndef _QMATPLOTWIDGET_H_
#define _QMATPLOTWIDGET_H_

#include <QVector>
#include <QWidget>

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
    Q_PROPERTY(bool grid READ grid WRITE setGrid)
    Q_PROPERTY(QPointF xlim READ xlim WRITE setXlim)
    Q_PROPERTY(QPointF ylim READ ylim WRITE setYlim)
    Q_PROPERTY(QVector<QColor> colorOrder READ colorOrder WRITE setColorOrder)

public:
    explicit QMatPlotWidget(QWidget* parent = 0);
    virtual ~QMatPlotWidget();

    //getters
    QString title() const;
    QString xlabel() const;
    QString ylabel() const;
    bool autoScaleX() const;
    bool autoScaleY() const;
    bool timeScaleX() const { return timeScaleX_; }
    bool timeScaleY() const { return timeScaleY_; }
    bool logScaleX() const { return logScaleX_; }
    bool logScaleY() const { return logScaleY_; }
    bool grid() const { return grid_on_; }
    QPointF xlim() const;
    QPointF ylim() const;
    QVector<QColor> colorOrder() const { return colorOrder_; }

    //setters
    void setTitle(const QString& s);
    void setXlabel(const QString& s);
    void setYlabel(const QString& s);
    void setAutoScaleX(bool on);
    void setAutoScaleY(bool on);
    void setTimeScaleX(bool on);
    void setTimeScaleY(bool on);
    void setLogScaleX(bool on);
    void setLogScaleY(bool on);
    void setGrid(bool on);
    void setXlim(const QPointF& v);
    void setYlim(const QPointF& v);
    void setColorOrder(const QVector<QColor>& c);


    // QWidget overrides
    QSize sizeHint () const override;
    QSize minimumSizeHint () const override;

public slots:
    void clear();
    void replot();

public:
    template<class VectorType>
    void plot(const VectorType& x, const VectorType& y, const QString &attr = QString(), const QColor& clr = QColor());
    template<class VectorType>
    void plot(const VectorType& y, const QString &attr = QString(), const QColor& clr = QColor());

private:

    class Implementation;
    Implementation* const impl_;
    friend class Implementation;

    bool timeScaleX_, timeScaleY_,
        logScaleX_, logScaleY_, grid_on_;

    QVector<QColor> colorOrder_;
    int colorIndex_;

    //template<class VectorType> class DataSeries;

    class AbstractDataSeries
    {
    public:
        virtual ~AbstractDataSeries() {}
        virtual size_t size() const = 0;
        virtual QPointF sample(size_t i) const = 0;
        virtual QRectF boundingRect() const = 0;
    };

    friend class DataHelper;

    void plot_(AbstractDataSeries* d, const QString &attr, const QColor& clr);
};

template<class V_>
class SingleDataSeries_ : public QMatPlotWidget::AbstractDataSeries
{
    V_ vy;
public:
    explicit SingleDataSeries_(const V_& y) : vy(y)
    {
    }
    size_t size() const override { return vy.size(); }
    QPointF sample( size_t i ) const override { return QPointF(1.*i,vy[i]); }
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
    plot_(data,attr,clr);
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
    size_t size() const override { return qMin(vx.size(),vy.size()); }
    QPointF sample( size_t i ) const override
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
    plot_(data,attr,clr);
}





#endif //_QMATPLOTWIDGET_H_
