#!/bin/bash
# wrapper to run entrance with env vars

printenv

unset XAUTHORI{ZATION,TY}
unset XDG_{SESSION_ID,RUNTIME_DIR}

/usr/sbin/entrance
