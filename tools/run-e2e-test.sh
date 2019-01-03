#!/bin/sh -e

if [ $# -ne 2 ] ; then
	echo "Usage: $0 MODE TESTNAME" >&2
	exit 1
fi

BASEDIR=`dirname $0`
E2E_DIR=$BASEDIR/../e2e

MODE="$1"
TEST="$2"

if [ "$MODE" != "safe" -a "$MODE" != "unsafe" ] ; then
	echo "ERROR: unsupported mode '$MODE' (must be 'safe' or 'unsafe')" >&2
	exit 1
fi

SCRIPT="$E2E_DIR/$TEST.sh"
REFXML="$E2E_DIR/$TEST.$MODE.xml"

PID="/tmp/$TEST.pid"; export PID
XML="/tmp/$TEST.xml"; export XML
CLIENT_OUT="/tmp/$TEST.client.out"
SERVER_OUT="/tmp/$TEST.server.out"

# kill old qsslcaudit instance, if any
if [ -e "$PID" ] ; then
	kill `cat $PID` || true
	sleep 1
	rm "$PID"
fi

# launch the new one
(. "$SCRIPT" && server )> "$SERVER_OUT" &

# wait 10s
for _ in `seq 20` ; do sleep .5 && [ -e "$PID" ] && break; done
if [ ! -e "$PID" ] ; then
	echo "ERROR: PID file '$PID' has not appeared in 10s (qsslcaudit failed to start?)" >&2
	exit 2
fi

# hammer util pidfile disappears
for _ in `seq 100` ; do
	[ ! -e "$PID" ] && break
	echo ; echo ; echo "===== `date` =====" ; echo
	(. "$SCRIPT" && client) 2>&1
	sleep .5
done > "$CLIENT_OUT"
if [ -e "$PID" ] ; then
	echo "ERROR: too many client interations" >&2
	exit 3
fi

if ! diff -b -u "$REFXML" "$XML" ; then
	echo "===== $SERVER_OUT ====="
	cat "$SERVER_OUT"
	echo
	echo "===== $CLIENT_OUT ====="
	cat "$CLIENT_OUT"
	false
else
	rm "$XML" "$SERVER_OUT" "$CLIENT_OUT"
fi