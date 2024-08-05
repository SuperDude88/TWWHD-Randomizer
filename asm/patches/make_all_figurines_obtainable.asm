; Change some requirements for figurines to work better with the randomizer

; A few legendary pictographs are tied to cutscenes which are removed in the randomizer
; Change some of the checks in daNpcPhoto_c::isPhotoDxOk() to use more relevant requirements
.org 0x022D01A4 ; Condition for the King of Hyrule's pictograph (normally event bit 0x2D02, watched the Tetra -> Zelda cutscene)
    addi r3, r8, 0xD4
    bl getTriforceNum
    cmpwi r3, 8 ; Check if you have the full triforce (opened the yellow warp to Hyrule)
    beq 0x022D0304
    nop
.org 0x022d01D4 ; Condition for Ganondorf's pictograph (normally event bit 0x3910, watched his cutscene at the end of FF2)
    li r3, 2 ; FF stage ID
    bl isStageBossEnemy ; Check if Helmaroc King has been defeated
    nop
.org 0x022D029C ; Condition for Jabun's pictograph (normally event bit 0x3920, finishing the Nayru's pearl cutscene)
    ; gameinfo pointer was just loaded into r7
    li r4, 0x1C ; Switch for breaking the stone wall blocking Jabun's cave
    addi r3, r7, 0x3A0 ; Offset to the Great Sea's dSv_memBit_c instance
    bl dSv_membit_c_isSwitch ; Check if you have broken the stone wall
.org 0x022D02B8 ; Condition for the Fairy Queen's pictograph (normally event bit 0x1001, watching her cutscene at Mother and Child Isles)
    li r4, 1 ; Ballad of Gales song index
    addi r3, r9, 0xD4
    bl isTact ; Check if you have Ballad of Gales (-> can get inside M&C)
.org 0x022D02D4 ; Condition for Fado's pictograph (normally event bit 0x2D20, meeting him inside Gale Isle)
    li r3, 7 ; WT stage ID
    bl isStageBossEnemy ; Check if Molgera has been defeated
    nop
.org 0x022D02F0 ; Condition for Laruto's pictograph (normally event bit 0x2D40, meeting her inside Headstone Isle)
    li r3, 6 ; ET stage ID
    bl isStageBossEnemy ; Check if Jalhalla has been defeated
    nop

; Some figurines are missable even in the original game
; This is more problematic in the randomizer where the deluxe pictbox might not be accessible early enough
; Change some figurines so they are not missable
.org 0x022A20F4 ; When checking if this is the Tetra figurine
    li r3, 0x88 ; Check dSnap index for the King of Hyrule instead
.org 0x022A2180 ; After adding pirate figurines
    b 0x022A21B0 ; Also add the Zelda figurine
.org 0x022A21B0 ; When checking if this is the King figurine
    li r3, 0x72
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r4, r3, 24
    mr r3, r29
    bl setFigure ; Give the Tetra figurine instead
.org 0x022A2194
    bne 0x022A21D8 ; Skip the normal check for the King figurine, we reordered things

.org 0x022A20C8 ; In daNpcMt_c::giveNewFigurines (made up the name, this function is new in HD)
    b check_give_additional_figurines
.org @NextFreeSpace
.global check_give_additional_figurines
check_give_additional_figurines:
    li r3, 0xBA ; dSnap index for Octoroks
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r3, r3, 24
    cmplw r31, r3
    bne check_give_zephos_cyclos_figurine
    li r3, 0xC6 ; dSnap index for Big Octos
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r4, r3, 24
    mr r3, r29
    bl setFigure
    b 0x022A21D8 ; Continue loop with the next new figurine

check_give_zephos_cyclos_figurine:
    li r3, 0x99 ; dSnap index for Valoo
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r3, r3, 24
    cmplw r31, r3
    bne check_give_helmaroc_figurine
    li r3, 0x9A ; dSnap index for Cyclos
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r4, r3, 24
    mr r3, r29
    bl setFigure
    b 0x022A21D8 ; Continue loop with the next new figurine

check_give_helmaroc_figurine:
    li r3, 0xCE ; dSnap index for Ganondorf
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r3, r3, 24
    cmplw r31, r3
    bne check_give_red_wizzrobe_figurine
    li r3, 0xCA ; dSnap index for Helmaroc
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r4, r3, 24
    mr r3, r29
    bl setFigure
    b 0x022A21D8 ; Continue loop with the next new figurine

check_give_red_wizzrobe_figurine:
    li r3, 0xC4 ; dSnap index for normal Wizzrobes
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r3, r3, 24
    cmplw r31, r3
    bne check_give_additional_figurines_end
    li r3, 0xC5 ; dSnap index for the Red Wizzrobe
    bl dSnap_PhotoIndex2TableIndex
    clrlwi r4, r3, 24
    mr r3, r29
    bl setFigure
    b 0x022A21D8 ; Continue loop with the next new figurine

check_give_additional_figurines_end:
    li r3, 0x4A ; Replace the line we overwrote to jump here
    b 0x022A20CC ; Continue with the original checks

; Phantom Ganon, Puppet Ganon, and Mighty Darknuts are still missable
; All of these are in Ganon's Tower and do not lock any item checks, so you
; will always be able to wait until you have deluxe picto before defeating them
