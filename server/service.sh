#!/bin/bash
# Taken from https://gist.github.com/naholyr/4275302
### BEGIN INIT INFO
# Provides:          eve-lock
# Required-Start:    $local_fs $network $named $time $syslog
# Required-Stop:     $local_fs $network $named $time $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Description:       A simple utility to control the eve lock
### END INIT INFO

#DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

DIR="/home/eve-lock/lock"

SCRIPT="$DIR/auth -f $DIR/prints auth"
RUNAS=eve-lock #must be a member of dialout and plugdev

PIDFILE=$DIR/eve-lock.pid
LOGFILE=$DIR/eve-lock.log
ERRFILE=$DIR/eve-lock.err

status() {
  if [ -f $PIDFILE ] && kill -0 $(cat $PIDFILE); then
    echo 'Service running' >&2
  else
    echo 'Service stopped' >&2
  fi
  return 1
}


start() {
  if [ -f $PIDFILE ] && kill -0 $(cat $PIDFILE); then
    echo 'Service already running' >&2
    return 1
  fi
  echo 'Starting service…' >&2
  local CMD="$SCRIPT >> \"$LOGFILE\" 2>> \"$ERRFILE\" & echo \$!"
  su -c "$CMD" $RUNAS > "$PIDFILE"
  echo 'Service started' >&2
}

stop() {
  if [ ! -f "$PIDFILE" ] || ! kill -0 $(cat "$PIDFILE"); then
    echo 'Service not running' >&2
    return 1
  fi
  echo 'Stopping service…' >&2
  kill -15 $(cat "$PIDFILE") && rm -f "$PIDFILE"
  echo 'Service stopped' >&2
}

uninstall() {
  echo -n "Are you really sure you want to uninstall this service? That cannot be undone. [yes|No] "
  local SURE
  read SURE
  if [ "$SURE" = "yes" ]; then
    stop
    rm -f "$PIDFILE"
    echo "Notice: log files is not be removed: '$LOGFILE' '$ERRFILE'" >&2
    update-rc.d -f <NAME> remove
    rm -fv "$0"
  fi
}

case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  status)
    status
    ;;
  uninstall)
    uninstall
    ;;
  restart)
    stop
    start
    ;;
  *)
    echo "Usage: $0 {status|start|stop|restart|uninstall}"
esac
