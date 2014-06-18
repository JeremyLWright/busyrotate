
mkdir -p t
./busyrotate t 125829120 10485760 0 &

while [ true ]
do
    touch t/log.0
    dd if=/dev/urandom bs=1M count=2 | base64 >> t/log.0
done
