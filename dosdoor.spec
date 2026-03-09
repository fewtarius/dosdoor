Name:           dosdoor
Version:        %{version}
Release:        1%{?dist}
Summary:        DOS door game emulator for BBS systems

License:        GPLv2
URL:            https://github.com/fewtarius/dosdoor
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  flex
BuildRequires:  bison
BuildRequires:  slang-devel

%description
dosdoor is a stripped-down DOS emulator purpose-built for running BBS door
games. Derived from dosemu 1.4.0 with sound, graphics, networking, and other
subsystems removed. Includes software x86 CPU emulation (simx86) for running
on ARM and x86_64 hosts.

Supported door games include Legend of the Red Dragon (LORD), Barren Realms
Elite (BRE), Operation: Overkill II, The Simpsons Trivia, and Darkness.

%prep
%setup -q

%build
./build.sh

%install
rm -rf %{buildroot}
install -d %{buildroot}%{_bindir}
install -d %{buildroot}%{_datadir}/%{name}/freedos/dosemu
install -d %{buildroot}%{_datadir}/%{name}/freedos/bin
install -d %{buildroot}%{_datadir}/%{name}/freedos/tmp
install -d %{buildroot}%{_sysconfdir}/%{name}

# Binary
install -m 755 build/bin/dosdoor %{buildroot}%{_bindir}/dosdoor

# FreeDOS files
install -m 644 freedos/kernel.sys %{buildroot}%{_datadir}/%{name}/freedos/
install -m 644 freedos/command.com %{buildroot}%{_datadir}/%{name}/freedos/
install -m 644 freedos/config.sys %{buildroot}%{_datadir}/%{name}/freedos/
install -m 644 freedos/autoexec.bat %{buildroot}%{_datadir}/%{name}/freedos/
for f in freedos/dosemu/*.{sys,com}; do
    [ -f "$f" ] && install -m 644 "$f" %{buildroot}%{_datadir}/%{name}/freedos/dosemu/
done

# Configuration
install -m 644 etc/dosemu.conf %{buildroot}%{_sysconfdir}/%{name}/dosdoor.conf
install -m 644 etc/global.conf %{buildroot}%{_sysconfdir}/%{name}/global.conf

%files
%license COPYING COPYING.DOSEMU
%doc README.md INTEGRATION.md TESTING.md
%{_bindir}/dosdoor
%{_datadir}/%{name}/
%config(noreplace) %{_sysconfdir}/%{name}/

%changelog
