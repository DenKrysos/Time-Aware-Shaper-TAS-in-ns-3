# Time-Aware Shaper (TAS) implemented in ns-3

An implementation of the scheduled traffic forwarding mechanism »Time-Aware Shaper (TAS)« (IEEE 802.1Qbv), as known from »Time-Sensitive Networking (TSN)«, as a queue discipline extension to the network simulator »ns-3«.

## Compatibility
Developed with <ns-3.31>. Adapted and tested with <ns-3.36>.

## Installation
1) Copy the "contrib" folder from "ns-3_Implementation" into the ns-3's root-directory. (The contrib-dir belongs aside folders like "scratch", "utils", "examples".)
2) From the ns-3 root-dir execute
```bash
./ns3 configure --enable-examples --enable-tests
```
3) ns-3 compiles with
```bash
./ns3 build
```


## Example Simulations
Some examples can be found in the directory
```
"ns-3_Implementation/contrib/tsn/examples"
```
These examples come in two variants:

- The examples "tsn-example1" to "tsn-example3" are using the point-to-point-module
- The examples "tsn-example4" & "tsn-example5" utilize the CSMA Channel



## Testing & Evaluation
The Directory
```
"ns-3_Implementation/contrib/tsn/test"  ->  File: "tas-test.cc"
```
keeps a Simulation to take some measurements (tas-test.cc).

Execute this via
```bash
./waf --run="tas-test"
```

#### tas-test:
The Simulation from tas-test.cc works in conjunction with the Python-Script from the directory
```
"TSNtester"
```

tas-test creates a simulation with 2 CSMA Netdevices. The channel has correction and a Packet-Filter that assigns every ARP packet the Priority 7 available.
The Python-Tester always opens this priority because otherwise, no packets would be sent. This is the case since due to the random schedules, to frequently timeouts would occur, which causes the ARP requests to stop sending.


### Executing the Python-Script

1) If desired, create a new Project with a Python IDE of personal choice and then copy the content from the "TSNtester" folder over to this project's directory
2) Following Modules have to be installed:
```python
import matplotlib.pyplot as plt
import numpy as np
import subprocess
import dpkt
import socket
import math
import re
import copy
import random
```
3) Fill in the favoured parameters and start simulation
