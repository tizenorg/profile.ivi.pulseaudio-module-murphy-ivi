Summary: Pulseaudio module for enforcing policy decisions in the audio domain
Name: pulseaudio-module-murphy-ivi
Version:  0.2.8
Release: 0
License: LGPLv2.1
Group: System Environment/Daemons
URL: https://github.com/otcshare/pulseaudio-module-murphy-ivi
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: pkgconfig(pulseaudio-module-devel)
BuildRequires: pkgconfig(libpulse)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: libtool-ltdl-devel
Requires: pulseaudio
Conflicts: pulseaudio-module-combine-sink
Conflicts: pulseaudio-module-augment-properties


%description
This package contains a pulseaudio module that enforces (mostly audio) routing,
corking and muting policy decisions.

%prep
%setup -q

%build
PAVER="`/usr/bin/pkg-config --silence-errors --modversion libpulse | \
tr -d \\n | sed -e 's/\([0123456789.]\+\).*/\1/'`"
./bootstrap.sh

unset LD_AS_NEEDED
%configure --disable-static \
--with-module-dir=%{_libdir}/pulse-$PAVER/modules \
--with-dbus \
--with-documentation=no
make

%install
rm -rf $RPM_BUILD_ROOT
%make_install
rm -f %{_libdir}/pulse-*/modules/module-*.la


%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/pulse-*/modules/module-*.so
%{_sysconfdir}/dbus-1/system.d/pulseaudio-murphy-ivi.conf
