Name:		mangler
Version:	1.2.0beta1
Release:	1%{?dist}
Summary:	Mangler is a Ventrilo compatible client for Linux

Group:		Productivity/Networking/Talk/Clients
License:	GPL
URL:		http://www.mangler.org/
Source0:	%{name}-%{version}.tar.bz2
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	gtkmm24-devel speex-devel gsm-devel pulseaudio-libs-devel alsa-lib-devel espeak-devel libgdbus-devel xosd-devel
Requires:	gtkmm24 speex gsm pulseaudio-libs espeak libgdbus xosd

%description
Mangler is a VOIP client that is capable of connecting to Ventrilo 3.x servers


%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/usr/bin/mangler
/usr/lib/libventrilo3.a
/usr/lib/libventrilo3.la
/usr/lib/libventrilo3.so
/usr/lib/libventrilo3.so.0
/usr/lib/libventrilo3.so.0.0.0
/usr/share/applications/mangler.desktop
/usr/share/pixmaps/mangler_logo.svg
%doc



%changelog

