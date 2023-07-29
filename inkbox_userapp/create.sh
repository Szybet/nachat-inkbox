#!/bin/bash -x

cd ../

cp build/src/nachat inkbox_userapp/nachat/app-bin/nachat.bin
cp build/src/*.so inkbox_userapp/nachat/app-lib/

# Very important
rm -f inkbox_userapp/nachat.isa.dgst
rm -f inkbox_userapp/nachat.isa

mksquashfs inkbox_userapp/nachat/* inkbox_userapp/nachat.isa

# Yes, here are my private keys. Is providing this info a security threat? no.
openssl dgst -sha256 -sign /home/szybet/inkbox-keys/userapps.pem -out inkbox_userapp/nachat.isa.dgst inkbox_userapp/nachat.isa

# Create the zip
cd inkbox_userapp/
rm -rf nachat.zip
mkdir -p tmp_nachat_dir/nachat/
cp app.json tmp_nachat_dir/nachat/
cp nachat.isa tmp_nachat_dir/nachat/
cp nachat.isa.dgst tmp_nachat_dir/nachat/
cd tmp_nachat_dir
zip -r nachat.zip nachat/
mv nachat.zip ../
cd ..
rm -rf tmp_nachat_dir

servername="root@10.42.0.28"
passwd="root"

sshpass -p $passwd ssh $servername "bash -c \"ifsctl mnt rootfs rw\""
# sshpass -p $passwd ssh $servername "bash -c \"rm -r /data/onboard/.apps/nachat\""
sshpass -p $passwd ssh $servername "bash -c \"mkdir /data/onboard/.apps/nachat\""
sshpass -p $passwd ssh $servername "bash -c \"rm  /data/onboard/.apps/nachat/nachat.isa\""
sshpass -p $passwd ssh $servername "bash -c \"rm  /data/onboard/.apps/nachat/nachat.isa.dgst\""
sshpass -p $passwd ssh $servername "bash -c \"rm  /data/onboard/.apps/nachat/app.json\""

cd ..
sshpass -p $passwd scp inkbox_userapp/app.json $servername:/data/onboard/.apps/nachat/
sshpass -p $passwd scp inkbox_userapp/nachat.isa.dgst $servername:/data/onboard/.apps/nachat/
sshpass -p $passwd scp inkbox_userapp/nachat.isa $servername:/data/onboard/.apps/nachat/

sshpass -p $passwd ssh $servername "bash -c \"touch /kobo/tmp/rescan_userapps\""

sshpass -p $passwd ssh $servername "bash -c \"sync\""

sshpass -p $passwd ssh $servername "bash -c \"killall -9 nachat-debug.sh\"" || EXIT_CODE=0
sshpass -p $passwd ssh $servername "bash -c \"killall -9 nachat.sh\"" || EXIT_CODE=0

sshpass -p $passwd ssh $servername "bash -c \"rc-service inkbox_gui restart\"" # to get logs
# sshpass -p $passwd ssh $servername "bash -c \"rc-service gui_apps restart\""

# To update main json
# sshpass -p $passwd ssh $servername "bash -c \"touch /kobo/tmp/rescan_userapps\"" # This gets deleted by service restart
#sshpass -p $passwd ssh $servername "bash -c \"killall inkbox-bin\""
#sleep 10
