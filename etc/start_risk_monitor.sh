#!/bin/sh

version_home="/opt/version/lawson/Monitor/version"
stop_script="/opt/version/lawson/Monitor/version/script/stop_risk_monitor.sh"
pid_path="/run/ig/risk_monitor.pid"

execute_file="/opt/version/lawson/Monitor/version/Monitor_V1.0.5/risk_monitor"
run_path="/opt/version/lawson/Monitor/version/config_20220818"

${stop_script}

cd ${version_home}
if [ ! -f $execute_file ]
then
    echo "$execute_file not exist!"
    exit 0
fi

if [ ! -d $run_path ]
then
    echo "$run_path not a directory!"
    exit 0
fi

cur_date=$(date -d "now" +%Y%m%d)
file_name=${execute_file##*/}
echo "start running: $execute_file $run_path"
nohup $execute_file $run_path > $run_path/$cur_date_$file_name.out 2>&1 &
echo $! > ${pid_path}
cat ${pid_path}

sleep 1
ps -ef | grep "$execute_file"

exit 0
