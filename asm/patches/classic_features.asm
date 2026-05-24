; Restore some behaviors that were removed in the remake
; Planned to support other, non-bug features too but storage is the most requested currently (and it's pretty easy to unpatch)

; HD patched Wind Waker dives by canceling the Wind Waker if Link started to fall while using it
; Remove the falling check (there's technically a timer too but this check is the real problem)
.org 0x0243A28C
  nop

; In SD, canceling the Wind Waker as you hit the ground gives you storage
; HD added some code that properly transitioned Link back into his waiting state
; Remove that code so dry storage works again
.org 0x023F22B0
  b 0x023F22E4

; Pausing with the Wind Waker out is useful for tricks with camera lock
; HD doesn't let you pause until you have at least 1 song
; Remove this check so you can always pause
.org 0x0270F058
  nop


; HD doesn't let you pause while Link is dead
; Remove this restriction to match SD behavior
.org 0x0270D9BC
  nop
  
; HD doesn't let you enter loading zones while Link is dead
; Remove this restriction to match SD behavior
.org 0x023FEBC4
  nop


; HD capped the values of cCcD_Stts::m_cc_move to prevent the big pig throw
; Uncap those values to allow the big pig to fly really far
.org 0x021932A4 ; In daKb_execute
  b 0x021932E8 ; Skip capping the X value
.org 0x021932F0 ; In daKb_execute
  b 0x02193328 ; Skip capping the Z value

; Nintendo also moved Abe further from the house to prevent you from getting the glitchy pick-up
; For some insane reason they did this through code, so remove that also
.org 0x022F7E24 ; In daNpc_Ym1_c::createInit
  b 0x022F7E70 ; Skip overwriting his position
