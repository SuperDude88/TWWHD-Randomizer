.org 0x026feb1c
    bl check_b_button_held
    cmpwi r3, 0

.org 0x026fe628
    bl check_b_button_held
    cmpwi r3, 0
    
.org 0x026fcf18
    bl check_b_button_held
    cmpwi r3, 0
    
.org 0x026fd290
    bl check_b_button_held
    cmpwi r3, 0
    
.org 0x026fd3dc
    bl check_b_button_held
    cmpwi r3, 0
    
.org 0x026fd9b8
    bl check_b_button_held
    cmpwi r3, 0
    
.org 0x026fdb40
    bl check_b_button_held
    cmpwi r3, 0
    
.org 0x026fe81c
    bl check_b_button_held
    cmpwi r3, 0

; This function already exists at 0x020076e0, but a single bl can not jump that far
; We reimplement it so it's close enough
.org @NextFreeSpace
.global check_b_button_held
check_b_button_held:
    lis        r3,something_button_related@ha
    lwz        r3,something_button_related@l(r3)
    lwz        r3,0x124(r3)
    rlwinm     r3,r3,0x0,0x1e,0x1e
    subic      r0,r3,0x1
    subfe      r3,r0,r3
    blr
