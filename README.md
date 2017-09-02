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

Entrance is a long story. There was a project long ago that worked, and 
went [MIA](http://xcomputerman.com/pages/entrance.html). Another came 
along [Michael Bouchaud](https://github.com/eyoz)/@eyoz and renamed his 
project elsa to Entrance. Which is where the current code base came 
from. It is not known if this ever was completed or worked, but 
did not function correctly if it is usable to log in at all. Thus the 
existence of this fork of that code base.

## Build
Entrance presently uses meson build system, autotools has been dropped. 

### Build using meson
```
prefix=/usr/share
meson \
	--prefix "${preffix}" \
	--bindir "${preffix}/bin" \
	--sbindir "${preffix}/sbin" \
	--datadir "${preffix}/share" \
	--sysconfdir "/etc" \
	. build
ninja -C build
```

On most systems you likely need a pam file. 
```
cp data/entrance.pam.d /etc/pam.d/entrance
```

There are some others, entrance.arch and entrance.other. May need other 
variations for different operating systems/linux distributions.

## Configuration
Most things can be configured in entrance.conf, /etc/entrance/entrance.conf

You could put a customized icon for your user in
/var/cache/entrance/users/(username).edj with groupname "entrance/user/icon"

## Usage
In order to start entrance you need a system init script or systemd (untested). 
This may differ based on your operating system. entrance does not 
provide an init script at this time, and likely will not run if started
directly. Entrance should be invoked via init script or systemd service. 
There is a provided systemd service file for entrance in the  

## logind/elogind aka systemd
Presently not supported beyond build systems, no code written, just a 
service file. There are plans to support logind/elogind for Wayland and 
X. There is no ETA at this time.
