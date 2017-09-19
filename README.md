# entrance - An EFL based display manager
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/Obsidian-StudiosInc/entrance/blob/master/LICENSE)
[![Build Status](https://img.shields.io/travis/Obsidian-StudiosInc/entrance/master.svg?colorA=9977bb&style=plastic)](https://travis-ci.org/Obsidian-StudiosInc/entrance)
[![Build Status](https://img.shields.io/shippable/59415c1aa155af0700adbcb3/master.svg?colorA=9977bb&style=plastic)](https://app.shippable.com/projects/59415c1aa155af0700adbcb3/)
[![Code Quality](https://img.shields.io/coverity/scan/12936.svg?colorA=9977bb&style=plastic)](https://scan.coverity.com/projects/obsidian-studiosinc-entrance)


This is a fork and current development version.
It is ALIVE! IT WORKS! (for me ©)

![A screenshot of Entrance](https://user-images.githubusercontent.com/12835340/29548111-4d3460fa-86cc-11e7-8e19-3b7456be3190.jpg)

## Known Issues
- Custom user set background is broken
[Issue #4](https://github.com/Obsidian-StudiosInc/entrance/issues/4)
- Problems stopping/restarting entrance after log in
[Issue #5](https://github.com/Obsidian-StudiosInc/entrance/issues/5)
- Settings UI is broken, hidden for now
[Issue #6](https://github.com/Obsidian-StudiosInc/entrance/issues/6)

## About
Entrance is EFL Unix Display Manager. Entrance allows a user to choose a 
X WM/Desktop session to launch. Entrance is alive and working again for 
logging into X sessions! The project has been resurrected from the dead 
to live on once again...

## History
Entrance is a long story. There has been 2 different code bases and 
projects both using the name entrance.

### 3rd Generation
This project is the 3rd generation, fork of the 2nd Generation code 
base. With a lot of fixes, and initial removal of incomplete and/or 
broken code. Rather than fix as is, looking to replace functionality 
with new code and different ways of accomplishing similar functionality.

This generation is currently in development, and should be usable. 
Please open issues for any problems encountered. Which should be 
expected given the state of the project.

### 2nd Generation 
Sometime later another came along,
[Michael Bouchaud](https://github.com/eyoz)/@eyoz who renamed his 
project elsa to Entrance. Which is where the current code base came 
from. It is not known if this ever was completed or worked, but 
does not function correctly. If it is usable to log in at all.

The broken, incomplete, unmaintained 2nd Genration Entrance 
resides in Enlightenment's
[entrance git 
repository](https://git.enlightenment.org/misc/entrance.git/).

### 1st Generation
There was a project long ago that worked, and went 
[MIA](http://xcomputerman.com/pages/entrance.html). Copies of the 
sources for some releases. Ideally would great to get a copy of the old 
entrance repo to add to this one for historical purposes. If you have a 
copy of the old Entrance repository, please open an issue and provide 
a link. That would be much appreciated!

## Build
Entrance presently uses meson build system, autotools has been dropped. 

### Build using meson
```
prefix=/usr/share
meson \
	--prefix "${prefix}" \
	--bindir "${prefix}/bin" \
	--sbindir "${prefix}/sbin" \
	--datadir "${prefix}/share" \
	--sysconfdir "/etc" \
	. build
ninja -C build
```

On most systems you likely need a pam file. Meson will install this file.
```
cp data/entrance.pam.d /etc/pam.d/entrance
```

The systemd service file is presently not installed. It may or may not 
be usable and/or correct. Please see the section on logind/elogind for 
further information.

## Configuration
Most things can be configured in entrance.conf, /etc/entrance/entrance.conf
Various aspects do no work. Please file issues for anything that is not 
configurable or does not work. It is known that things like background 
do not work.

## Usage
In order to start entrance you need a system init script or systemd (untested). 
This may differ based on your operating system. entrance does not 
provide an init script at this time, and likely will not run if started
directly. Entrance should be invoked via init script or systemd service. 
There is a provided systemd service file for entrance. It is not know if 
this works or not.

## logind/elogind aka systemd
Presently not supported beyond build systems, no code written, just a 
service file. There are plans to support logind/elogind for Wayland and 
X. There is no ETA at this time.
