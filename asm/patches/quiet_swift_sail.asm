; Normally the swift sail makes a "pwing" sound every time you pull it out
; This can get kind of annoying especially if you do a lot of sail pumping (spamming swift sail to travel faster)

; Remove the pwing sound
.org 0x0247EA2C
    nop

; Remove the wind sound when pulling swift sail
.org 0x0247EA20
    nop

; Remove the cloth sound when putting away swift sail
.org 0x0247E9AC
    nop
