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
.org 0x02481050 ; in daShip_c::setNextMessage
  b check_has_next_hint_message

.org @NextFreeSpace
.global last_korl_hint_message_number
last_korl_hint_message_number:
.int 0xD73 ; Single hint message by default

.global check_has_next_hint_message
check_has_next_hint_message:
  lwz r12, 0x648(r31) ; load the current message

  cmpwi r12, 0xD73 ; first message we use for hints
  blt continue_check_other_messages
  lis r11, last_korl_hint_message_number@ha
  lwz r11, last_korl_hint_message_number@l(r11)
  cmpw r12, r11 ; compare to our last hint message
  bgt continue_check_other_messages
  beq continue_no_next_hint_message

  addi r12, r12, 0x1 ; increment current message
  stw r12, 0x648(r31) ; store updated message
  b 0x024814C0 ; continue with the new message

continue_no_next_hint_message:
  b 0x024814DC ; continue without setting another message

continue_check_other_messages:
  b 0x02481054

; KoRL also has some late-game text that triggers based on a few milestones:
; - Opening the dark portal ("If you wish to return to the world above" / "we must return to Ganon's Tower")
; - Breaking the barrier ("Long ago, Ganon's Tower was an impenetrable fortress")
; - Defeating the Mighty Darknuts in the Master Sword Chamber ("So, despite our efforts, the princess")
; - Being in Hyrule before any of the other 3 have triggered ("I am concerned about Princess Zelda")
; These don't make as much sense in the randomizer, and cause issues if KoRL hints are enabled
; Skip checking for them and just show the randomizer's text instead
.org 0x024734F4 ; In daShip_c::setInitMessage
  beq 0x024741E0 ; Show message 3443 (0xD73)
