echo Transmit Carrier : Stop

set comport=99

set target=43362A2
rem set target=4390A1
rem set target=43341B0

..\wl%target% --serial %comport% fqacurcy 0
..\wl%target% --serial %comport% down
..\wl%target% --serial %comport% up