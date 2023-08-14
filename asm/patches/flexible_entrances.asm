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


; Normally the game checks if you were crawling when you entered a load zone to decide if you should be crawling after the load
; This can lead to some softlocks if you load into a crawlspace from a standing entrance
; Add a check for an unused param (X rotation & 0x0001), and force crawling if it is set
.org 0x02411df4 ; in daPy_lk_c::playerInit
    b custom_crawl_check
.org @NextFreeSpace
.global custom_crawl_check
custom_crawl_check:
    lhz r7, 0x2F8(r24) ; get original X rotation (currently just a copy of spawn X rotation)
    andi. r7, r7, 0x0001 ; mask out our param
    beq crawl_not_forced ; custom param is not set
    
    lbz r7, 0x1177(r9) ; load current mode
    cmpwi r7, 2
    beq crawl_not_forced ; already crawling forward
    cmpwi r7, 3
    beq crawl_not_forced ; already crawling backward

    li r7, 0x2 ; mode 2 (crawling forward)
    stb r7, 0x1177(r9) ; store this mode (is ignored for some spawn types)

crawl_not_forced:
    cmpwi r6, 0xE ; replace the line we overwrote to jump here
    b 0x02411df8

; This is some untested code that should allow voids to respawn crawling
; Don't believe it is ever needed though
; ; When reloading a room with startCode -1 (voids, ect)
; .org 0x025c1860 ; normally stores 0 to X rotation
;     b load_custom_restart_flag
; .org @NextFreeSpace
; .global load_custom_restart_flag
; load_custom_restart_flag:
;     lbz r5, 0x114B(r8) ; custom flag at +0x3 in dSv_restart_c
;     sth r5, 0x10(r27) ; store to X rotation
;     b 0x025c1864 ; continue
; 
; ; When setting a new restart location
; .org 0x024133d8
;     b set_custom_restart_flag
; .org @NextFreeSpace
; .global set_custom_restart_flag
; set_custom_restart_flag:
;     bl dSv_restart_c_setRoom ; replace the line we overwrote to jump here
;     lhz r4, 0x2F8(r31) ; load original X rotation (copy of the spawn point's X rotation)
;     stb r4, 0x3(r3) ; store flag to restart
