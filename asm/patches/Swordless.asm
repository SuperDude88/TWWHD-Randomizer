;TODO: test these

.org 0x0213ae90
	rlwinm. r8,r10,0,15,15

.org 0x0213aea8
	b 0x0213aebc

.org 0x025b2030
	b give_temporary_sword_during_ganondorf_fight_in_swordless

.org @NextFreeSpace
.global give_temporary_sword_during_ganondorf_fight_in_swordless
give_temporary_sword_during_ganondorf_fight_in_swordless:
  
  bl FUN_025200d4
  addi r3, r3, 0x5133
  lis r4, GTower_str@ha
  addi r4, r4, GTower_str@l
strcmp_2_start:
  lbzu r5, 0x1(r3)
  lbzu r6, 0x1(r4)
  cmplw r5, r6
  bne after_strcmp_2
  cmplwi r5, 0
  bne strcmp_2_start

after_strcmp_2:
  subf. r3, r5, r6
  bne give_temporary_sword_during_ganondorf_fight_in_swordless_end

  lis r30, gameInfo_ptr@ha
  lwz r30, gameInfo_ptr@l(r30)
  lbz r0, 0x2E (r30) ; Read the player's currently equipped sword ID
  cmpwi r0, 0xFF
  ; If the player has any sword equipped, don't replace it with the Hero's Sword
  bne give_temporary_sword_during_ganondorf_fight_in_swordless_end
  
  li r0, 0x38
  stb r0, 0x2E (r30) ; Set the player's currently equipped sword ID to the regular Hero's Sword
  
give_temporary_sword_during_ganondorf_fight_in_swordless_end:
  bl FUN_025200d4
  b 0x025b2034 ; Return

.org 0x025b26f4
	b give_temporary_sword_in_orcas_house_in_swordless

.org @NextFreeSpace
.global give_temporary_sword_in_orcas_house_in_swordless
give_temporary_sword_in_orcas_house_in_swordless:
  bl FUN_025200d4
  addi r3, r3, 0x5133
  lis r4, Ojhous_str@ha ; Pointer to the string "Ojhous", the stage for Orca's house
  addi r4, r4, Ojhous_str@l
 strcmp_3_start:
  lbzu r5, 0x1(r3)
  lbzu r6, 0x1(r4)
  cmplw r5, r6
  bne after_strcmp_3
  cmplwi r5, 0
  bne strcmp_3_start

 after_strcmp_3:
  subf r3, r5, r6
  cmpwi r3, 0
  ; If the player did not just enter Orca's house, skip giving a temporary sword
  bne give_temporary_sword_in_orcas_house_in_swordless_end
  
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  lbz r0, 0x2E (r3) ; Read the player's currently equipped sword ID
  cmpwi r0, 0xFF
  ; If the player has any sword equipped, don't replace it with the Hero's Sword
  bne give_temporary_sword_in_orcas_house_in_swordless_end

  li r0, 0x38
  stb r0, 0x2E (r3) ; Set the player's currently equipped sword ID to the regular Hero's Sword
  
  mr r5, r3
  b 0x025b26fc
  
give_temporary_sword_in_orcas_house_in_swordless_end:
  lwz r4, gameInfo_ptr@l(r27) ; Replace the line we overwrote to branch here
  b 0x025b26f8 ; Return

.org 0x025b26f8
	b remove_temporary_sword_when_loading_stage_in_swordless

.org @NextFreeSpace
.global remove_temporary_sword_when_loading_stage_in_swordless
remove_temporary_sword_when_loading_stage_in_swordless:
  lbz r0, 0xd4 (r4) ; Read the player's owned swords bitfield
  cmpwi r0, 0
  ; If the player owns any sword, don't remove their equipped sword since it's not temporary
  bne remove_temporary_sword_when_loading_stage_in_swordless_end
  
  li r0, 0xFF
  stb r0, 0x2E (r4) ; Set the player's currently equipped sword ID to no sword
  
remove_temporary_sword_when_loading_stage_in_swordless_end:
  lbz r5,0x68(r4) ; Replace the line we overwrote to jump here
  b 0x025b26fc ; Return

.close
