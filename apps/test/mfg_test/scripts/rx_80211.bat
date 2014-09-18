echo Receive 802.11

set comport=99

set target=43362A2
rem set target=4390A1
rem set target=43341B0

..\wl%target% --serial %comport% down
..\wl%target% --serial %comport% mpc 0
..\wl%target% --serial %comport% country ALL
..\wl%target% --serial %comport% scansuppress 1
..\wl%target% --serial %comport% channel 1
..\wl%target% --serial %comport% bi 65535
..\wl%target% --serial %comport% up
@echo sleep 3 seconds
@ping -n 3 127.0.0.1 > nul
..\wl%target% --serial %comport% counters
