#!/bin/bash

sleep_time=300
args=""
processes=""
waitlist=""
filtered_args=""

function current_processes() {
	ps -elf| tr -s " " | cut -d " " -f4 | tail -n+2
}

function compose_waitlist() {
	for i in $processes; do
		for j in $@; do
			if [ "$j" = "$i" ]; then
				waitlist+=" $j"
			fi
		done
	done
}

function filter_args() {
	for arg in $args; do
		if ! [ "${arg:0:1}" == "-" ]; then
			if ! echo "$arg" | grep -q "^[0-9]*$"; then
			echo " $arg"
			fi
		fi
	done
}

#1st arg is mode to run in (s for single or 
function check_pids() {
	stop=0
	processes=$(current_processes)
	if [ "$1" = "s" ]; then
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
	elif [ "$1" = "a" ]; then
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

function check_jobs(){
	mode=$1
	stop=0
	shift
	if [ "$mode" = "s" ]; then
		stop=0
	elif [ "$mode" = "a" ]; then
		stop=0
	fi
	for jb in $@; do
		if [ "$jb" = "Exit" -o "$jb" = "Done" ]; then
			stop=0
		fi
	done

}

#1st arg is mode s for single
function main_loop(){
	stop=0
	
	while [ $stop = 0 ] ; do
		sleep $sleep_time
		jbs=jobs | tr -s " " | cut -f2
		check_jobs "$1" "$jbs"
		stop=$(check_pids "$1")
	done
}

function display_help(){
	echo "-h for help," ; echo "-t set time interval to check on, ex: -t400 (checks every 400s)," ; echo "-s to wait for any of pids/functions to end"; echo "last argument is the function to run"
}
args="$@"
filtered_args=$(filter_args)
for job in "$@"; do
	$job >/dev/null &
done

processes=$(current_processes)

waitlist=$(compose_waitlist)


for par in $*; do
	case "$par" in
		-h) display_help;;
		-t*) sleep_time=${par: 2};;
		-s) main_loop "s" ; exit 0 ;;
	esac
done

if [ "$#" = 0 ]; then
	display_help
elif [ "$#" -ge 2 ]; then
	main_loop "a"
fi



