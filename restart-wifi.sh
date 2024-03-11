#!/bin/bash
INTERFACE="wlp0s20f3"

nmcli device disconnect $INTERFACE && nmcli device connect $INTERFACE
