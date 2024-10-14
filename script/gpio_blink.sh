#!/bin/bash
while true;
do
state17=$(gpioget 0 17)
gpioset 0 18=$state17
done