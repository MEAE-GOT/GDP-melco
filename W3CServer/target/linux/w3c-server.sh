#!/bin/bash
# Linux service controller script

service=W3CServer
params="-secure"

cd "/usr/bin"

case "$1" in
    start)
        ./$service params
        ;;
    stop)
        killall $service
        ;;
    restart)
        killall $service
        ./$service params
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac
