#!/bin/bash

while :;
	do
	clear;
	echo "$(date)"
	ps -ef | grep router.out | grep -v grep | grep -v watch
	sleep 1;
done
