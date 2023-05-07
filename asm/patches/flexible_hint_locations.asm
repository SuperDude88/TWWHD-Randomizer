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
