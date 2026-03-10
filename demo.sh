#! /bin/bash

# variable definition
MODULE_NAME="motor_driver"
MODULE_FILE="motor_driver.ko"
SYSFS_PATH="/sys/kernel/my_servo/servo_angle"

echo "=== Start executing the testing flow ==="

# 1. check root
if [ "$EUID" -ne 0 ]; then
	echo "Error: To execute this script, please using sudo (sudo ./demo.sh)"
	exit 1
fi

# 2. remove old module
if lsmod | grep -q "$MODULE_NAME"; then
	echo "Trying to remove old module: $MODULE_NAME..."
	rmmod $MODULE_NAME
	if [ $? -eq 0 ]; then
		echo "Success to remove old module"
	else
		echo "Error: Cannot remove old module"
		exit 1
	fi
else
	echo "The module $MODULE_NAME isn't in the system. SKIP..."
fi

# 3. insert module
echo "Inserting the new module, $MODULE_FILE..."
insmod $MODULE_FILE
if [ $? -eq 0 ]; then
	echo "Success to insert module"
else
	echo "Failed to insert module...Please check the dmesg info"
	exit 1
fi

# 4. check the Sysfs node exit or not
echo "Checking the Node, $SYSFS_PATH"
sleep 0.5
if [ -f "$SYSFS_PATH" ]; then
	echo "Successed: Node exit"
else
	echo "Failed: Cannot find the Sysfs node"
	dmesg | tail -n 10
	exit 1
fi

# 5. Execute demo
echo "Executing Demo Test: 90 degree..."
echo 90 > $SYSFS_PATH
sleep 1
echo "Executing Demo Test: 180 degree..."
echo 180 > $SYSFS_PATH
sleep 1
echo "Executing Demo Test: 0 degree..."
echo 0 >$SYSFS_PATH
echo "=== Testing Over ==="
