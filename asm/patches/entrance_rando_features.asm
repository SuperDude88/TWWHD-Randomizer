; These patches change some entrances' behavior to be more intuitive/fun in entrance randomizer.
; This file is for patches which cause noticable behavior/traversal requirement changes,
; since they should only apply when actually shuffling entrances.

; Normally, going through a small key-locked door backwards still requires the key, but does not consume it
; This is a little unintuitive if you don't have these doors memorized and enter a dungeon from a (mini)boss entrance
; Change this behavior so you can open the doors backwards without a key
; (Doors check for keys by looking at the stage info (*not* the UI key counter) in setEventPrm for door10/door12/kddoor)
.org 0x021271A0 ; in daDoor10_c::setEventPrm
    b check_entering_door10_forwards
.org @NextFreeSpace
.global check_entering_door10_forwards
check_entering_door10_forwards:
    lbz r0, 0x3BC(r31) ; what side Link is opening the door from
    cmpwi r0, 0 ; 0 -> opening forwards
    beq door10_entering_lock_forwards
    b 0x021271AC ; continue with opening the door
door10_entering_lock_forwards:
    lbz r0, 0x7B8(r9) ; replace the line we overwrote to jump here
    b 0x021271A4 ; continue to check the key counter
    
.org 0x02129324 ; in daDoor12_c::setEventPrm
    b check_entering_door12_forwards
.org @NextFreeSpace
.global check_entering_door12_forwards
check_entering_door12_forwards:
    lbz r0, 0x3BC(r31) ; what side Link is opening the door from
    cmpwi r0, 0 ; 0 -> opening forwards
    beq door12_entering_lock_forwards
    b 0x02129330 ; continue with opening the door
door12_entering_lock_forwards:
    lbz r12, 0x7B8(r12) ; replace the line we overwrote to jump here
    b 0x02129328 ; continue to check the key counter


; The door on Private Oasis always requires the cabana deed, even from inside the cabana
; This can also be annoying with entrance randomizer, so change it to only require deed when entering
.org 0x021A4548 ; In daKnob00_c::actionVilla
    b check_leaving_cabana
.org @NextFreeSpace
.global check_leaving_cabana
check_leaving_cabana:
    ; Check if Link is open the door from the front
    lbz r0, 0x3BC(r31) ; What side Link is opening the door from
    cmpwi r0, 0 ; 0 -> Opening forwards
    beq cabana_set_normal_event_data

    ; setEventPrm sets event data based on the state the door is in
    ; If we are opening from the back, pretend we are in the waiting state so Deed isn't required
    li r3, 1 ; Wait state
    stb r3, 0x5A4(r31) ; Store to door's state
    mr r3, r31 ; Replace the line we overwrote to jump here
    bl daKnob_00_setEventPrm ; Set event data as if we're in the waiting state

    ; If we don't reset back to the Cabana state, the door will stay permanently in the waiting state on its next execute cycle
    ; This means that walking behind the door on Private Oasis and going back in front unlocks it even without Cabana Deed
    ; Which is definitely undesirable
    li r3, 8 ; Villa state
    stb r3, 0x5A4(r31) ; Restore to door's state so it doesn't break completely
    b 0x021A4550 ; Return
cabana_set_normal_event_data:
    mr r3, r31 ; Replace the line we overwrote to jump here
    b 0x021A454C ; Continue setting up the door event as normal


; Alter savewarping so that players respawn at their last visited ocean sector
; instead of whatever sector the cave/interior/area normally tries to savewarp
; them to. This is so players don't end up savewarping to an island they weren't on.
.org 0x025220C4
    bl set_return_place_as_last_visited_ocean_sector
.org @NextFreeSpace
.global set_return_place_as_last_visited_ocean_sector
set_return_place_as_last_visited_ocean_sector:
    lis r5, some_gfx_ptr@ha
    lwz r5, some_gfx_ptr@l(r5)
    lwz r5, 0x218 (r5)
    lbz r5, 0x3E (r5)
    addi r5, r5, 1
    b 0x025B50DC ; dSv_player_return_place_c::set
