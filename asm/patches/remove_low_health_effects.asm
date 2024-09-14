; Skip playing the low health beep if Link is at full health
; This could be done without the free space code but HD has relocations in it
; And things would break when I inevitably forget to remove them
.org 0x02024408 ; in JAIZelBasic::processHeartGaugeSound
  b check_low_health_full ; if health is <= 1 heart, before playing sound
.org @NextFreeSpace
.global check_low_health_full
check_low_health_full:
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  lhz r3, 0x20(r3) ; load max health

  ; If max health is below 2 hearts, health can appear full and the beep will still play
  cmplwi r3, 8 ; 8 health -> 2 hearts
  blt skip_low_health_beep ; Skip sound if we have less than 2 max hearts
  
  mr r3, r31 ; replace the instruction we overwrote to jump here
  b 0x0202440C ; continue and play sound

skip_low_health_beep:
  b 0x02024458 ; continue without playing sound


; Remove the low health animation at full health
.org 0x023DD1B8 ; in daPy_lk_c::checkRestHPAnime
  b remove_low_health_anim_at_full_health
.org @NextFreeSpace
.global remove_low_health_anim_at_full_health
remove_low_health_anim_at_full_health:
  lhz r3, 0x20(r12) ; Load max health (r12 has a pointer to SaveData)
  lhz r4, 0x22(r12) ; Load current health
  rlwinm r3, r3, 0, 0, 29 ; r6 &= ~0x3 (round max HP down to a full heart to exclude unfinished heart pieces)
  subfc r3, r4, r3
  cmpwi r3, 2 ; Check if 2 quarter hearts of health have been removed
  bge play_low_health_anim

  li r3, 0
  b 0x023DD1BC ; return false

play_low_health_anim:
  li r3, 1
  b 0x023DD1BC ; return true
