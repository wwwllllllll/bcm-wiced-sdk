echo Transmit Carrier : Start

set comport=99

set target=43362A2
rem set target=4390A1
rem set target=43341B0

..\wl%target% --serial %comport% down
..\wl%target% --serial %comport% country ALL
..\wl%target% --serial %comport% band b
..\wl%target% --serial %comport% mpc 0
..\wl%target% --serial %comport% up
..\wl%target% --serial %comport% out
..\wl%target% --serial %comport% fqacurcy 6
