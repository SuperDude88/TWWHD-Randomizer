; These modify places in the game's code to work better with randomized entrances.

; Quitting auction normally calls dComIfGp_setNextStage(0.0, "sea", 3, 0xb, 0xff, 0, 1, 0) to put you back on Windfall
; This can be very inconvenient depending on what entrance lead to the auction house
; Instead call dStage_changeScene(1, 0.0, 0, 0) and use the same exit as the door
; Then overwrite the exit's wipe type (0xB) with the original auction wipe type (0x0) so the fade out looks correct
.org 0x0205cb8c
    ; f1 already has the correct float arg
    li r3, 1 ; SCLS index in first param
    li r4, 0 ; Mode in second param
    li r5, 0 ; Room number with the SCLS entry in third param (-1 for stage)
    ; This next instruction has a relocation that would overwrite half of whatever we change it to
    addi r6, r6, 0 ; Change it to mess with an unused argument so the relocation is irrelevant
    bl dStage_changeScene
    bl FUN_025200d4 ; get gComIfG_inf_c instance in r3
    li r4, 0 ; Wipe type
    stb r4, 0x514D(r3) ; overwrite the wipe type that the SCLS entry used
