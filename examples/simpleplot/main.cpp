#include <QApplication>
#include <QGridLayout>
#include "QMatPlotWidget.h"

#include <QPushButton>

#include <cmath>

typedef QVector<qreal> Vector;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget w;
    w.setWindowTitle("QMatPlotWidget: 2D Matlab/Octave Plot Types");
    QGridLayout *grid = new QGridLayout;
    w.setLayout(grid);

    QMatPlotWidget *plot[2][2];
    {
        QMatPlotWidget *w = new QMatPlotWidget;
        //w->layout()->setMargin(20);

        const int N = 40;
        Vector x(N + 1), y1(N + 1), y2(N + 1);
        for (int i = 0; i <= N; ++i) {
            x[i] = -1. + 2. * i / N;
            y1[i] = x[i] * x[i];
            y2[i] = y1[i] * x[i];
            // test for NaN values
            //if (std::fabs(x[i]) < 0.25)
            // y1[i] = NAN;
        }

        w->plot(x, y1, "o-");
        w->plot(x, y2, "^-");
        w->plot(Vector({0, 0}), Vector({-1, 1}), "k-");
        w->plot(Vector({-1, 1}), Vector({0, 0}), "k-");
        w->setXlabel("x");
        w->setYlabel("y");
        w->setTitle("simple, y1(x)=x^2, y2(x)=x^3");
        grid->addWidget(w, 0, 0);
        plot[0][0] = w;
    }

    {
        QMatPlotWidget *w = new QMatPlotWidget;
        //w->layout()->setMargin(20);

        const int N = 10;
        Vector x(N + 1), y(N + 1);
        for (int i = 0; i <= N; ++i) {
            x[i] = i;
            y[i] = i + 1;
        }
        y[N] = y[N - 1];

        w->stairs(x, y, "o-");

        w->setXlabel("x");
        w->setYlabel("y");
        w->setTitle("stairs");

        grid->addWidget(w, 0, 1);
        plot[0][1] = w;
    }

    {
        QMatPlotWidget *w = new QMatPlotWidget;
        w->setColorMap(QMatPlotWidget::Turbo);
        //w->layout()->setMargin(20);

        //Vector matrix{1, 2, 4, 1, 6, 3, 5, 2, 4, 2, 1, 5, 5, 4, 2, 3};
        const int N = 17;
        const int O = 8;
        const int S = 6;
        const int numRows = N;
        const int numColumns = N;
        Vector matrix(numRows * numColumns);
        int k = 0;
        for (int i = 0; i < numRows; ++i)
            for (int j = 0; j < numColumns; ++j) {
                //matrix[k++] = std::floor(1.0 * (i + numRows * j) / matrix.size() * 256);
                double x = 1. * (i - O) / S;
                double y = 1. * (j - O) / S;
                matrix[k++] = exp(-(x * x + y * y));
            }
        Vector x{-(N / 2), N / 2};
        Vector y{-(N / 2), N / 2};
        w->imagesc(x, y, matrix, numColumns);

        w->setXlabel("x");
        w->setYlabel("y");
        w->setTitle("image");

        grid->addWidget(w, 1, 0);
        plot[1][0] = w;
    }

    {
        QMatPlotWidget *w = new QMatPlotWidget;
        //w->layout()->setMargin(20);

        const int N = 20;
        Vector x(N + 1), y1(N + 1), y2(N + 1), dy2(N + 1);
        for (int i = 0; i <= N; ++i) {
            x[i] = 1.0 * i / N;
            y1[i] = 1.0 * std::sqrt(x[i]);
            y2[i] = 2.0 / (1.0 + 4 * x[i] * x[i]);
            dy2[i] = 0.05 / y2[i];
        }
        qreal dy1 = 0.1;

        w->errorbar(x, y1, dy1, "o-");

        w->errorbar(x, y2, dy2, "^-");

        QPointF lx = w->xlim();
        w->plot(Vector({lx.x(), lx.y()}), Vector({0, 0}), "k-");
        w->setXlabel("x");
        w->setYlabel("y");
        w->setTitle("errorbar");

        grid->addWidget(w, 1, 1);
        plot[1][1] = w;
    }

    // {
    //     QPushButton *b = new QPushButton("Clear errorbar");
    //     grid->addWidget(b, 2, 0);
    //     QObject::connect(b, &QPushButton::pressed, plot[1][1], &QMatPlotWidget::clear);
    // }

    w.resize(800, 600);
    w.show();

    return app.exec();
}
