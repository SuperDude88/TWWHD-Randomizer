; These modify places in the game's code to work better with randomized hints.


; Stop Old man Ho-Ho from disappearing under some conditions.
; When HoHo is created
.org 0x021e5560 ; Don't check if Cabana Octo is defeated
  li r3, 0
.org 0x021e5524 ; Don't check if the stone head above Savage Labyrinth is destroyed
  li r3, 0
.org 0x021e558c ; Don't check if the Two-Eye Reef Octo is defeated
  li r3, 0

; When HoHo is executing
.org 0x021e6924 ; Don't check if Cabana Octo is defeated
  li r3, 0
.org 0x021e6900 ; Don't check if the stone head above Savage Labyrinth is destroyed
  li r3, 0
.org 0x021e6950 ; Don't check if the Two-Eye Reef Octo is defeated
  li r3, 0



; Make the Big Octo Great Fairy always give an item hint.
; In vanilla she hinted about Fire & Ice Arrows, so she didn't give the hint if your current bow was anything but Hero's Bow.
.org 0x02095be8
	nop ; Remove the conditional branch for if your current bow is not the Hero's Bow and just always show the hint.



; Patch the King of Red Lions to string some hint textboxes together
; HD messages seem to have a limit of 10 boxes before they get confused and loop around, probably an array size in a related class
.org 0x02477e48 ; When KoRL opens a textbox
  b korl_hint_message_checks

.org @NextFreeSpace
.global num_korl_messages
num_korl_messages:
.byte 1 ; Single hint message by default
.align 2

.global korl_hint_message_checks
korl_hint_message_checks:
  beq continue_triforce_text
  ; Check if we went through messages already
  lbz r10, 0x3A0(r31)
  cmpwi r10, 0

  ; Load total number of messages
  lis r10, num_korl_messages@ha
  addi r10, r10, num_korl_messages@l
  lbz r10, 0 (r10)

  ; Reset remaining messages if we've gone through them all
  bgt skip_init_message_num
  stb r10, 0x3A0(r31)

  skip_init_message_num:
    ; Find out how many messages we've progressed, add that to base ID
    lbz r12, 0x3A0(r31) ; Remaining messages
    subf r10, r12, r10 ; Subtract from total
    addi r10, r10, 0xD73 ; D73 (3443) is the first text ID we use
    stw r10, 0x648(r31)
    ; Update remaining messages count
    subi r12, r12, 0x1
    stb r12, 0x3A0(r31)

    b 0x02477e58 ; continue as normal text with our ID

continue_triforce_text:
  b 0x02477e4c ; continue with message ID 0x1682 (5762)


.org 0x024731DC
  lbz r4, 0x3A0(r31) ; Load remaining messages
  cmpwi r4, 0
  ble 0x02473438
  li r3, 1
  b 0x0247343c

.org 0x0247A754 ; Skip a check, not sure what this does but it interferes
  b 0x0247A7CC

; Remove some other message set stuff that seems to interfere
.org 0x024740c8
  nop
.org 0x024740fc
  nop
.org 0x02474198
  nop
.org 0x024741e4
  nop
