# QMatPlotWidget

A Qt plot widget with a MATLAB/OCTAVE-like interface. 

> part of **QDaq** (https://gitlab.com/qdaq/qdaq) - Qt-based Data Acquisition
>

![screenshot](./dist/Screenshot_20251023.png)

```c++
    #include <QMatPlotWidget>
    #include <vector>

    const int N = 40;
    std::vector<double> x(N+1), y(N+1);
    for(int i=0; i<=N; ++i)
    {
        x[i] = -1. + 2.*i/N;
        y[i] = x[i]*x[i];
    }

    QMatPlotWidget w;
    w.plot(x,y,"o--");
    w.setXlabel("x");
    w.setYlabel("y");
    w.setTitle("y1(x)=x^2, y2(x)=x^3");
    w.show();
```

QMatPlotWidget provides only the MATLAB-compatible interface and does not do the actual plotting. This is implemented by a "backend".

Currently, the backend is `Qwt` (https://qwt.sourceforge.io/).



