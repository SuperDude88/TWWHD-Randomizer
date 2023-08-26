; Helmaroc King's fight can very wildly depending on which attacks he decides to do
; Change his code so he doesn't fly around and waste time
.org 0x0206DB9C ; Checks if a random float is < 0.3 to decide if he should do a flying attack or try to land
  b 0x0206DBBC ; Always make Helmaroc try to land
.org 0x0206E9B4 ; Checks if a random float is >= 0.25 to decide if he should try a gust attack or peck
  b 0x0206E9C4 ; Always make Helmaroc peck
.org 0x0206D868 ; Checks if a random float is >= .08 to decide if he should fly around quickly or go out really far
  nop ; Always do a short fly-around
