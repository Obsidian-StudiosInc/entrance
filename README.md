# entrance - An EFL based display manager
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/Obsidian-StudiosInc/entrance/blob/master/LICENSE)
[![Build Status](https://img.shields.io/travis/Obsidian-StudiosInc/entrance/wltjr.svg?colorA=9977bb&style=plastic)](https://travis-ci.org/Obsidian-StudiosInc/entrance)
[![Build Status](https://img.shields.io/shippable/59415c1aa155af0700adbcb3/wltjr.svg?colorA=9977bb&style=plastic)](https://app.shippable.com/projects/59415c1aa155af0700adbcb3/)
[![Code Quality](https://img.shields.io/coverity/scan/12936.svg?colorA=9977bb&style=plastic)](https://scan.coverity.com/projects/obsidian-studiosinc-entrance)


This is a fork and current development version Entrance, a EFL based 
display manager for Enlightenment and maybe Tizen. Or any device that 
has EFL.

It is ALIVE! IT WORKS! (for me ©)

## About
Entrance is alive and working again for logging into X sessions! The 
project has been resurrected from the dead to live on once again...

Entrance is a long story. There was a project long ago that worked, and 
went [MIA](http://xcomputerman.com/pages/entrance.html). Another came 
along [Michael Bouchaud](https://github.com/eyoz)/@eyoz and renamed his 
project elsa to Entrance. Which is where the current code base came 
from. It is not known if this ever was completed or worked, but 
presently does not function correctly if it is usable to log in at all. 

entrance should allow a user to choose a WM/Desktop session to launch.
That is the eventual goal, and long term to support Wayland in 
addition to X.

## Build
To build and install run the following

```
./autogen.sh --sysconfdir=/etc --prefix=/usr
make
make install
```

On most systems you likely need the pam file. 
```
cp data/entrance.other /etc/pam.d/entrance
```

## Configuration
Most things can be configured in entrance.conf, /etc/entrance/entrance.conf

You could put a customized icon for your user in
/var/cache/entrance/users/(username).edj with groupname "entrance/user/icon"

## GRUB2
Support for GRUB2 will likely be dropped. Seems like a strange 
dependency and feature. Reboot should not require integration with boot 
manager.

To enable grub2 reboot feature use --enable-grub2 on configure. But you 
need to add this line 'GRUB_DEFAULT=saved' to /etc/default/grub

## Systemd
Any support there will likley be dropped as well. Elogind/logind 
function maybe retained.
