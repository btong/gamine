%define name gamine
%define version 1.1rc1
%define release %mkrel 1

Summary:  An interactive game for young children
Name:     %{name}
Version:  %{version}
Release:  %{release}
URL:       http://www.gnunux.info/projets/gamine/
Source:    http://www.gnunux.info/projets/gamine/%{name}-%{version}.tar.gz
License:   WTFPL
Group:     Education
BuildRoot: %_tmppath/%{name}-%{version}-%{release}-buildroot
BuildRequires: gtk2-devel pkgconfig libxml2-devel gettext
Requires: gstreamer0.10-plugins-base
Requires:  libstdc++6 >= 4.4.1
Requires:  glibc >= 2.10.1


%description
Gamine is a game designed for 2 years old children who are not able to use
a keyboard. The child uses the mouse to draw coloured dots and lines on the screen.

%prep
%setup -q

%build
make PREFIX=/usr sysconfdir=/etc

%install
rm -rf $RPM_BUILD_ROOT
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
make install PREFIX=$RPM_BUILD_ROOT/usr sysconfdir=$RPM_BUILD_ROOT/etc
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL


%clean
rm -rf $RPM_BUILD_ROOT

%pre
if [ "$1" -gt 1 ]; then
	export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
	gconftool-2 --makefile-uninstall-rule \
      		%{_sysconfdir}/gconf/schemas/gamine.schemas
fi

%post
%update_menus
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule \
	%{_sysconfdir}/gconf/schemas/gamine.schemas

%preun
if [ "$1" -gt 1 ]; then
	export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
	gconftool-2 --makefile-uninstall-rule \
      		%{_sysconfdir}/gconf/schemas/gamine.schemas
fi

%postun
%clean_menus


%files
%defattr(-,root,root)
%doc README COPYING ChangeLog README.pencil README.sounds
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/gamine.svg
%{_datadir}/locale/fr/LC_MESSAGES/gamine.mo
%{_sysconfdir}/gconf/schemas/%{name}.schemas


%changelog
* Sun Dec 19 2010 GnunuX <gnunux@gnunux.info> 1.1
* Sun Dec 13 2009 Texstar <texstar at gmail.com> 1.0rc1-2pclos2010
- fix rpm group Education

* Wed Oct 28 2009 slick50 <lxgator@gmail.com> 1.0rc1-1pclos2010
- initial build

