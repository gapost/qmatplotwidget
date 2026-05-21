%global debug_package %{nil}

Name:           qmatplotwidget
Version:        0.2.0
Release:        1%{?dist}
Summary:        A Qt plot widget with MATLAB-style interface

License:        MIT
URL:            https://gitlab.com/qdaq/qmatplotwidget
Source0:        %{name}-v%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qwt-qt5-devel

Requires:       qt5-qtbase
Requires:       qwt-qt5

%description
QMatPlotWidget is a Qt-based plotting widget that provides a MATLAB-style
interface. This package installs the shared library.

%package devel
Summary:        Development files for QMatPlotWidget
Requires:       %{name} = %{version}-%{release}
Requires:       qt5-qtbase-devel
%description devel
Headers and CMake config files for building software that uses QMatPlotWidget.

%prep
%setup -q -n %{name}-v%{version}

%build
%{cmake}
%{cmake_build}

%install
rm -rf %{buildroot}
%{cmake_install}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%doc README.md
%license LICENSE
%{_libdir}/libQMatPlotWidget.so.0*

%files devel
%{_includedir}/qmatplotwidget.h
%{_includedir}/qmatplotwidget_export.h
%{_includedir}/QMatPlotWidget
%{_libdir}/libQMatPlotWidget.so
%{_libdir}/cmake/QMatPlotWidget/

%changelog
* Thu May 21 2026 GA
- Switch to shared library
* Thu Jan 01 2026 GA
- Initial RPM spec for QMatPlotWidget
