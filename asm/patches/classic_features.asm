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
