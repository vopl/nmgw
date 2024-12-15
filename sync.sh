#!/bin/bash
set -e
host=185.185.40.120

if [[ "$#" -ne 1 ]]; then
  echo "usage: $0 outAndroid|outLinux" >&2
  exit 1
fi

do_outAndroid=no
do_outLinux=no

case $1 in
    "outAndroid")
        do_outAndroid=yes
    ;;
    "outLinux")
        do_outLinux=yes
    ;;
    *)
        echo "unrecognized arg: $1"
        exit 1
    ;;
esac


opts="-avr -ztu -h --info=progress2 -e 'ssh -p 36271'"

if [[ "$do_outAndroid" == "yes" ]]; then
    cmd="ssh -p 36271 nmgw@${host} mkdir -p /home/nmgw/binaries/$(date +%4Y-%m-%d)"
    echo sync local to remote: $cmd
    sh -c "$cmd"

    cmd="rsync $opts ./bin/{entry,gate}.apk nmgw@${host}:/home/nmgw/binaries/$(date +%4Y-%m-%d)/bin/"
    echo sync local to remote: $cmd
    sh -c "$cmd"
fi

if [[ "$do_outLinux" == "yes" ]]; then
    cmd="ssh -p 36271 nmgw@${host} mkdir -p /home/nmgw/binaries/$(date +%4Y-%m-%d)"
    echo sync local to remote: $cmd
    sh -c "$cmd"

    cmd="rsync $opts ./bin/ nmgw@${host}:/home/nmgw/binaries/$(date +%4Y-%m-%d)/bin/"
    echo sync local to remote: $cmd
    sh -c "$cmd"

    cmd="rsync $opts ./lib/ nmgw@${host}:/home/nmgw/binaries/$(date +%4Y-%m-%d)/lib/"
    echo sync local to remote: $cmd
    sh -c "$cmd"

    cmd="rsync $opts ./etc/ nmgw@${host}:/home/nmgw/binaries/$(date +%4Y-%m-%d)/etc/"
    echo sync local to remote: $cmd
    sh -c "$cmd"
fi
