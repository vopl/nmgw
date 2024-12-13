#!/bin/bash

if [[ "$#" -ne 2 && "$#" -ne 1 ]]; then
  echo "usage: $0 start|restart|stop|status [exe]" >&2
  exit 1
fi

case $2 in
#    "")
#        exename="mcgt"
#    ;;
    "rendezvous")
        exename=$2
    ;;
    *)
        echo "unrecognized executable: $2"
        exit 1
    ;;
esac

dir=`readlink -f $0`
dir=`dirname $dir`
cd $dir
exe=$dir/${exename}


function my_pidof
{
    ps aux|grep "$1" | grep -v grep | awk '{print $2}'
}

function start
{
    if [ -f "$dir/nostart" ]; then
        echo "does not start because of $dir/nostart existence"
    else
        pid=`my_pidof $exe`
        if [ -z "$pid" ]; then
            mkdir -p $dir/../log/${exename}
            dt=`date +"%Y-%m-%d"`
            ($exe >>"$dir/../log/${exename}/${dt}.log" 2>&1)&
            pid=$!
            disown
            if [ -z "$pid" ]; then
                echo "unable to start $exe"
            else
                echo "started, pid: $pid"
                sleep 0.1
                pid=`my_pidof $exe`
                if [ -z "$pid" ];
                then
                    echo "stopped"
                fi
            fi
        else
            echo "already started, pid:$pid" > /dev/null
        fi
    fi
}

function status
{
    pid=`my_pidof $exe`
    if [ -z "$pid" ]; then
        echo "stopped"
    else
        echo "started, pid:$pid"
    fi
}

function stop
{
    pid=`my_pidof $exe`
    if [ -z "$pid" ]; then
        echo "already stopped" > /dev/null
    else
        echo -n "kill $pid"
        cnt=0
        while [ $cnt -lt 10 ] && [ -n "$pid" ] && kill $pid 2>/dev/null; do
            sleep 0.02
            pid=`my_pidof $exe`
            let cnt=cnt+1
            echo -n "."
        done

        while [ $cnt -lt 20 ] && [ -n "$pid" ] && kill $pid 2>/dev/null; do
            sleep 0.2
            pid=`my_pidof $exe`
            let cnt=cnt+1
            echo -n "."
        done

        pid=`my_pidof $exe`
        if [ -n "$pid" ]; then
            echo -n " force"
            kill -s KILL $pid 2>/dev/null
        fi

        pid=`my_pidof $exe`
        if [ -n "$pid" ]; then
            echo -n " force2"
            kill -s ABRT $pid 2>/dev/null
            kill -s SEGV $pid 2>/dev/null
            kill -s ILL $pid 2>/dev/null
        fi

        pid=`my_pidof $exe`
        echo " done$pid"
    fi
}


case $1 in
    "start")
        start
    ;;
    "restart")
        stop
        start
    ;;
    "status")
        status
    ;;
    "stop")
        stop
    ;;
    *)
        echo "start|stop|status action required"
        exit 2
    ;;
esac
