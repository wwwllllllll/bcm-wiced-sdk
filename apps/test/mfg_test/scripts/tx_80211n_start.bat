echo Transmit 802.11n : Start

set comport=99

set target=43362A2
rem set target=4390A1
rem set target=43341B0

..\wl%target% --serial %comport% down
..\wl%target% --serial %comport% country ALL
..\wl%target% --serial %comport% band b
..\wl%target% --serial %comport% chanspec -c 11 -b 2 -w 20 -s 0
..\wl%target% --serial %comport% mpc 0
..\wl%target% --serial %comport% ampdu 1
..\wl%target% --serial %comport% bi 65000
..\wl%target% --serial %comport% frameburst 1
..\wl%target% --serial %comport% rateset 11b
..\wl%target% --serial %comport% up
..\wl%target% --serial %comport% txant 0
..\wl%target% --serial %comport% antdiv 0
..\wl%target% --serial %comport% nrate -m 7 -s 0
..\wl%target% --serial %comport% phy_watchdog 0
..\wl%target% --serial %comport% disassoc
..\wl%target% --serial %comport% phy_forcecal 1
..\wl%target% --serial %comport% phy_activecal
..\wl%target% --serial %comport% txpwr1 -1
@echo sleep 3 seconds
@ping -n 3 127.0.0.1 > nul
..\wl%target% --serial %comport% pkteng_start 00:90:4c:aa:bb:cc tx 40 1000 0