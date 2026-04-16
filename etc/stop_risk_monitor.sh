#!/bin/sh

#./versions/IGTrade_V1.1.1
#cd ./versions
execute_file="/opt/version/lawson/Monitor/version/Monitor_V1.0.5/risk_monitor"
run_path="/opt/version/lawson/Monitor/version/config_20220818"

command=`ps -ef | grep $execute_file | grep -v grep | awk '{print $2,$9}'`
if [ -z "$command" ]
then
    echo "not process for execute_file: $execute_file"
    exit 0
fi

OLD_IFS="$IFS"
IFS=" "

array=($command)
IFS="$OLD_IFS"

run_pid=''
run_para=''
a=0
for var in ${array[@]}
do
  if [ $(($a%2)) == 0 ]
  then
    run_pid=$var
  else
    run_para=$var
    if [ $run_para = $run_path ]
    then
        echo "kill pid:$run_pid process:$execute_file $run_para"
        kill -9 $run_pid
    fi
  fi
  a=`expr $a + 1`
done

sleep 1
ps -ef | grep "$execute_file"

exit 0
