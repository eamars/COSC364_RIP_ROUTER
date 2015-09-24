# COSC364 Assignment [![Build Status](https://travis-ci.org/eamars/COSC364_RIP_ROUTER.svg)](https://travis-ci.org/eamars/COSC364_RIP_ROUTER)

The Routing Information Protocol (RIP) is one of the oldest distance-vector routing protocols, which employs the hop count as a routing metric. RIP prevents routing loops by implementing a limit on the number of hops allowed in a path from the source to a destination. The maximum number of hops allowed for RIP is 15.

The RIP router implemented in COSC364 is just a prototype for discovering and exchange routing informations for the given network

##How to start the router

	$ ./router.out start router_config_file.cfg

##How to terminate the router

	$ ./router.out stop router_config_file.cfg

##Format of router config file

There are three essential entry for an rip router, the router-id, input-ports and outputs.

router-id: positive integer

input-ports: port or ports separated by comma ranging from 1000 to 65535

outputs: comma separated list of "port-cost-destination"
