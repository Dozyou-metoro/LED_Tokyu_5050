all:main

main:led_rand.c led_panel.c led_dir.c
	g++ -o led_rand led_rand.c led_panel.c led_dir.c -Wall -I /home/metoro/rpi-rgb-led-matrix/include -L /home/metoro/rpi-rgb-led-matrix/lib -lrgbmatrix -I /home/metoro/stb -lwiringPi -g -O0

