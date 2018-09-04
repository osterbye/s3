#!/bin/sh

echo "Starting CAN test"
while :
do
    cansend can0 123#1122334455667788
    sleep 0.35
done

exit 0
