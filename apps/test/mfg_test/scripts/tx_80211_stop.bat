echo Transmit 802.11 : Stop

set comport=99

set target=43362A2
rem set target=4390A1
rem set target=43341B0

..\wl%target% --serial %comport% pkteng_stop tx
