#!/bin/bash
# wrapper to run entrance with env vars

printenv

unset XAUTHORI{ZATION,TY}

/usr/bin/entrance
