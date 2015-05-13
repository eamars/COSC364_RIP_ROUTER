#!/bin/bash
# Remove the record from last run
echo "" > log/router1.log
echo "" > log/router2.log
echo "" > log/router3.log
echo "" > log/router4.log
echo "" > log/router5.log
echo "" > log/router6.log
echo "" > log/router7.log

./router.out start routers/normal/router1.cfg >> log/router1.log
./router.out start routers/normal/router2.cfg >> log/router2.log
./router.out start routers/normal/router3.cfg >> log/router3.log
./router.out start routers/normal/router4.cfg >> log/router4.log
./router.out start routers/nornal/router5.cfg >> log/router5.log
./router.out start routers/normal/router6.cfg >> log/router6.log
./router.out start routers/normal/router7.cfg >> log/router7.log

