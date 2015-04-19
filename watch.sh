#!/bin/bash
watch -n 1 "ps -ef | grep router.out | grep -v grep | grep -v watch"
