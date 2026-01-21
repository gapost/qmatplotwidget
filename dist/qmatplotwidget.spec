%global debug_package %{nil}

Name:           qmatplotwidget
Version:        0.2.0
Release:        1%{?dist}
Summary:        A Qt plot widget with MATLAB-style interface

License:        MIT
URL:            https://gitlab.com/qdaq/qmatplotwidget
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  pkgconfig
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qwt-qt5-devel

# Runtime requires (adjust package names for your distribution if necessary)
Requires:       qt5-qtbase
Requires:       qwt-qt5

# If the project installs static libraries only, keep BuildArch default (no change).
# BuildArch:    x86_64

%description
QMatPlotWidget is a Qt-based plotting widget that provides a MATLAB-style
interface. This package installs the library, headers and CMake config files
so other CMake projects can find_package(QMatPlotWidget CONFIG REQUIRED).

%prep
%setup -q

%build
%{cmake}
%{cmake_build}

%install
rm -rf %{buildroot}
%{cmake_install}

# Provide CMake package config and FindQwt.cmake if present
# (these should have been installed under %{_libdir}/cmake/qmatplotwidget)

%files 
%doc README.md
%license LICENSE
# headers
%{_includedir}/qmatplotwidget.h
%{_includedir}/QMatPlotWidget
# library (static or shared)
%{_libdir}/libQMatPlotWidget.*
# CMake package config and exported targets
%{_libdir}/cmake/QMatPlotWidget/*

%changelog
* Thu Jan 01 2026 GA
- Initial RPM spec for QMatPlotWidget
