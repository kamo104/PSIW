#!/bin/bash

sleep_time=300
processes=""
waitlist=""
master_command=""
temp_pid=""

single=0

function current_processes() {
        ps -elf| tr -s " " | cut -d " " -f4 | tail -n+2
}

#1st arg is mode to run in (1 for single or 0 for all)
function check_pids() {
    stop=0
    processes=$(current_processes)
    if [ "$1" = "1" ]; then
            for j in $waitlist; do
                    is=0;
                    for i in $processes; do
                            if [ "$j" == "$i" ]; then
                                    is=1
                                    break
                            fi
                    done
                    if [ $is = 0 ]; then
                            stop=1
                            break
                    fi
            done
    elif [ "$1" = "0" ]; then
            stop=1
            for j in $waitlist; do
                    is=0;
                    for i in $processes; do
                            if [ "$j" == "$i" ]; then
                                    is=1
                                    break
                            fi
                    done
                    if [ $is = 1 ]; then
                            stop=0
                            break
                    fi
            done
    fi

    echo $stop
}

#1st arg is mode to run in (1 for single or 0 for all)
function main_loop(){
    stop=0

    while [ $stop = 0 ] ; do
            sleep $sleep_time
            stop=$(check_pids "$1")
    done
}

function display_help(){
    echo "-h for help," ; echo "-t set time interval to check on, ex: -t400 (checks every 400s)," ; echo "-c command to execute upon finish, ex: -c\"echo All done: \""; echo "-s to wait for any of pids/functions to end"; echo "Make sure that every switch is placed at the front"
}
function execute_command(){
        eval "$*" >/dev/null 2>&1 &
	echo "$!"
}




if [ "$#" = 0 ]; then
    display_help
else
        for par in "$@"; do
                case "$par" in
                        -h) display_help; exit 0;;
                        -t[0-9]*) sleep_time=${par: 2}; shift ;;
                        -s) single=1; shift;;
                        -c[[:alnum:]]*) master_command=${par: 2}; shift ;;
                        [0-9]*) waitlist+=" $par";;
                        *) temp_pid=$(execute_command "$par") ; waitlist+=" $temp_pid";;
                esac
        done

        main_loop $single
        $master_command "$@"
fi

