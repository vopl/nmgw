#!/bin/bash
set -e
host=185.185.40.120

if [[ "$#" -ne 1 ]]; then
  echo "usage: $0 outApk|outElf" >&2
  exit 1
fi

do_outApk=no

case $1 in
    "outApk")
        do_outApk=yes
    ;;
    "outElf")
        do_outElf=yes
    ;;
    *)
        echo "unrecognized arg: $1"
        exit 1
    ;;
esac


opts="-avr -ztu -h --info=progress2 -e 'ssh -p 36271'"

if [[ "$do_outApk" == "yes" ]]; then
    cmd="rsync $opts ./bin/{entry,gate}.apk nmgw@${host}:/home/nmgw/binaries/$(date +%4Y-%m-%d)/"
    echo sync local to remote: $cmd
    sh -c "$cmd"
fi

if [[ "$do_outElf" == "yes" ]]; then
    cmd="rsync $opts ./bin/{entry,gate,rendezvous} nmgw@${host}:/home/nmgw/binaries/$(date +%4Y-%m-%d)/"
    echo sync local to remote: $cmd
    sh -c "$cmd"
fi
