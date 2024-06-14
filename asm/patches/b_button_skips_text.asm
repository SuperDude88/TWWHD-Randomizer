.org 0x026feb1c
    lwz r0, 0x50(r30)

.org 0x026fe628
    lwz r10, 0x50(r8)
    
.org 0x026fcf18
    lwz r8, 0x50(r30)
    
.org 0x026fd290
    lwz r0, 0x50(r30)
    
.org 0x026fd3dc
    lwz r12, 0x50(r11)
    
.org 0x026fd9b8
    lwz r9, 0x50(r30)
    
.org 0x026fdb40
    lwz r9, 0x50(r30)
    
.org 0x026fe818
    lwz r8, 0x50(r30)

.org 0x026ba644
    lwz r0, 0x50(r3)

; Normally the game checks if the continue icon is done appearing before letting you close the textbox
; The animation is a little bit strange if you end it early but it's much faster
.org 0x026F7EA0
    nop ; always move straight to the waiting state

; It also checks if the icon is done disappearing before starting the next textbox
; Remove that check too
.org 0x026F802C
    nop

; 0x026014F4 checks the textbox draw type attribute (putting this here for reference)
