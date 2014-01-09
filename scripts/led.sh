#!/bin/sh

LED_GPIO=18
LED_VALUE="/sys/class/gpio/gpio$LED_GPIO/value"

while true
do
	#off
	echo "1" > "$LED_VALUE"
	sleep 1
	#on
	echo "0" > "$LED_VALUE"
	sleep 1
done
