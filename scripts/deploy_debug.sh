#!/bin/bash

cd ../

servername="root@10.42.0.28"
passwd="root"

sshpass -p $passwd ssh $servername "bash -c \"killall -9 nachat\""
sshpass -p $passwd ssh $servername "bash -c \"killall -9 nachat\""

sshpass -p $passwd ssh $servername "bash -c \"rm /kobo/tmp/exec\""
sshpass -p $passwd scp build/src/nachat $servername:/kobo/tmp/exec
sshpass -p $passwd ssh $servername "bash -c \"chroot /kobo /launch_app.sh\""
