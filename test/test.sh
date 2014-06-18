
mkdir -p t
./busyrotate t 10240 1024 0 &

while [ true ]
do
    touch t/log.0
    dd if=/dev/urandom bs=1024 count=1 | base64 >> t/log.0
    sleep 5
done
