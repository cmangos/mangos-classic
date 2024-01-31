#!/bin/bash
# continued Massive Network Game Object Server
# autorestart Script

crashcount=0

case $1 in
    start )
        screen -dmS run-realmd $PWD/$0 detached
        echo "MaNGOS realm started"
    ;;
    detached )
        while :
        do
                echo `date` >> crash.log
                cmd="./realmd"
                $cmd
                status=$?
                echo "Status after downtime is: $status"
                mv Server.log Server$(date +%F-%H:%M).log && touch Realmd.log
                if [ "$status" == "2" ]; then
                   echo `date` ", MaNGOS realm restarted."
                elif [ "$status" == "0" ]; then
                   echo "date" ", MaNGOS realm shut down."
                   exit 0
                else
                   mv crash.log $(date +%F-%H:%M).log && touch crash.log
                   echo "date" ", realm daemon crashed."
                   ((crashcount=crashcount+1))
                   if [ "$crashcount" -gt 50 ]; then
                      exit 0
                   fi
                fi
        done
        ;;
esac
