#!/bin/sh

#GPIO6_5 -> GPIO53 (6*8+5 = 53)
#GPIO6_6 -> GPIO54 (6*8+6 = 54)

#(normal mode)
ir_cut_enable()
{
	# pin_mux
	echo "53" > /sys/class/gpio/unexport; 	# GPIO6_5
	echo "54" > /sys/class/gpio/unexport;  	# GPIO6_6
	echo "53" > /sys/class/gpio/export; 	# GPIO6_5
	echo "54" > /sys/class/gpio/export;  	# GPIO6_6
	
	# dir
	echo "out" > /sys/class/gpio/gpio53/direction;
	echo "out" > /sys/class/gpio/gpio54/direction;
	
	# data, GPIO6_6: 0, GPIO6_5: 1  (normal mode)
	echo "1" > /sys/class/gpio/gpio53/value;
	echo "0" > /sys/class/gpio/gpio54/value;
	
	#sleep 1s
	sleep 1;
	
	# back to original 
	echo "0" > /sys/class/gpio/gpio53/value;
	echo "0" > /sys/class/gpio/gpio54/value;
}

# (ir mode)
ir_cut_disable()
{
	# pin_mux
	echo "53" > /sys/class/gpio/unexport; 	# GPIO6_5
	echo "54" > /sys/class/gpio/unexport;  	# GPIO6_6
	echo "53" > /sys/class/gpio/export;		# GPIO6_5
	echo "54" > /sys/class/gpio/export;  	# GPIO6_6
	
	# dir
	echo "out" > /sys/class/gpio/gpio53/direction;
	echo "out" > /sys/class/gpio/gpio54/direction;
	
	# data, GPIO6_6: 1, GPIO6_5: 0  (ir mode)
	echo "0" > /sys/class/gpio/gpio53/value;
	echo "1" > /sys/class/gpio/gpio54/value;
	
	#sleep 1s
	sleep 1;
	
	# back to original 
	echo "0" > /sys/class/gpio/gpio53/value;
	echo "0" > /sys/class/gpio/gpio54/value;
}

if [ $# -eq 0 ]; then
    echo "ir mode : ./demo_config.sh 1";
else
    if [ $1 -eq 0 ]; then
        echo "normal mode, ir_cut on"
        ir_cut_enable > /dev/null;
    fi

    if [ $1 -eq 1 ]; then
        echo "ir mode, ir_cut off"
        ir_cut_disable > /dev/null;
    fi
fi
