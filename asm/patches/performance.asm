; Modify some things that affect the game's performance
; Aimed mostly at racers/console players who want to be as quick as emulator

; Reduce the size of a few particle lists, they seem to be one of the main lag producers in HD
.org 0x025a79d4 ; cut ptclArray by half
    li r5, 1500
; .org 0x025a79dc ; cut emtrArray by half
;     li r6, 75
; .org 0x025a79e4 ; cut fieldArray by half
;     li r7, 100
