#!/usr/bin/env python

import sys
from rffe_nuttx_lib import *

try:
    ip_addr = sys.argv[1]
    fw_file = sys.argv[2]
    version = sys.argv[3]
except:
    print("Usage: " + sys.argv[0] + " ip firmware_file version")
    exit(1)

print("Connecting to " + ip_addr + " ...")
try:
    rffe = RFFEControllerBoard(ip_addr)
except:
    print("Couldn't connect to device!")
    exit(1)

print("Connection established, writing new firmware...")
rffe.reprogram(fw_file, version)
print("Finished!")
