; Modify King of Red Lions's code so he doesn't stop you when you veer off the path he wants you to go on.
.org 0x02474b58
	b 0x02474f10
.org 0x02474b7c
	b 0x02474f10
.org 0x02474b9c
	b 0x02474f10
.org 0x0247a574
	b 0x0247a5cc


; Fishmen usually won't appear until Gohma is dead. This removes that check from their code so they appear from the start.
.org 0x022def78
	nop


; Normally Medli would disappear once you own the Master Sword (Half Power).
; This could make the Earth Temple uncompletable if you get the Master Sword (Half Power) before doing it.
; So we slightly modify Medli's code to not care about your sword.
.org 0x02286314
	b 0x022863e8
.org 0x0221f8d8
	b 0x0221f8ec

; Originally the withered trees and the Koroks next to them only appear after you get Farore's Pearl.
; This gets rid of all those checks so they appear from the start of the game.
.org 0x023465dc
	nop
.org 0x021f3f1c
	li r3, 1
.org 0x021f3fb8
	li r3, 1
.org 0x021f4064
	li r3, 1
.org 0x021f40fc
	li r3, 1
.org 0x021f8be0
	nop


; The warp object down to Hyrule sets the event bit to change FF2 into FF3 once the event bit for seeing Tetra transform into Zelda is set.
; We want FF2 to stay permanently, so we skip over the line that sets this bit.
.org 0x024d4f1c
	nop


; Changes the way spoils and bait work from the vanilla game.
; Normally if you encountered spoils or bait as a field item without owning the Bait Bag/Spoils bag, it would turn itself into a single green rupee instead so you can't get the items without a bag to put them in.
; In the randomizer we allow these to drop even without having the bags so you can get these items early.
.org 0x02551220
	li r3, 1
.org 0x02551244
	li r3, 1


; Normally the Earth/Wind Temple song tablets rely on whether you have the Earth God's Lyric or Wind God's Aria to tell which version they are.
; For example, the second tablet halfway through Earth Temple will act like the first one at the entrance if you don't own the Earth God's Lyric yet. As a result, it will give you the Earth God's Lyric, and then teleport you back to the entrance for the Zora sage cutscene.
; So we remove the checks for if you have the songs yet, and instead always act as if the player has them.
.org 0x0237305c
	b 0x02372f78
.org 0x02372f74
	nop
.org 0x02374648
	nop
.org 0x023746cc
	nop


; Fixes a bug with the recollection boss fights that can happen if you skip at least one of the original boss fights.
; If you fight a recollection boss without fighting the original form of that boss first, and then you fight a different recollection boss who you did fight the original form of, then when you kill that second boss your entire inventory will be replaced by null items (item ID 00, would be a heart pickup but in your inventory it looks like an empty bottle).
; To fix this we simply remove the feature of resetting the player's inventory to what it was during the original form of the boss fight entirely, so the player's inventory is always left alone.
; Replace functions related to this with instant returns.
.org 0x02522c60
	blr
.org 0x025221f0
	blr
.org 0x025224a4
	blr


; The death zone in between Forest Haven and Forbidden Woods disappears once you have Farore's Pearl.
; This makes it frustrating to make the trip to Forbidden Woods since you have to go all the way through Forest Haven every time you fail.
; So we change this void to always be there, even after you own Farore's Pearl.
.org 0x024b2798
	b 0x024b27b4


; In vanilla, the mailbox will not give you any mail until you own Din's Pearl.
; Remove that condition so that only the letter-specific requirements matter.
.org 0x023a4a98
	nop


; When the player enters Wind Temple, reset Makar's position to the starting room, or to near the warp pot the player exited.
; This is to prevent possible softlocks where Makar can teleport to later rooms in the dungeon for seemingly no reason.
.org 0x0221fae8
	bl reset_makar_position_to_start_of_dungeon
.org 0x0221fac4
	nop
.org @NextFreeSpace
.global reset_makar_position_to_start_of_dungeon
reset_makar_position_to_start_of_dungeon:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl FUN_025200d4
  lha r8,0x513c(r3)
  
  ; Search through the list of places Makar can spawn from to see which one corresponds to the player's last spawn ID, if any.
  lis r9, makar_possible_wt_spawn_positions@ha
  addi r9, r9, makar_possible_wt_spawn_positions@l
  li r0, 6 ; 5 possible spawn points
  mtctr r0
  makar_spawn_point_search_loop_start:
  lbz r0, 0 (r9) ; Spawn ID
  cmpw r0, r8
  beq reset_makar_found_matching_spawn_point ; Found the array element corresponding to this spawn ID
  addi r9, r9, 0x10 ; Increase pointer to point to next element
  bdnz makar_spawn_point_search_loop_start ; Loop
  
  ; The player came from a spawn that doesn't correspond to any of the elements in the array, don't change Makar's position.
  b after_resetting_makar_position
  
  reset_makar_found_matching_spawn_point:
  lwz r8, 4 (r9) ; X pos
  stw r8, 0 (r5)
  lwz r8, 8 (r9) ; Y pos
  stw r8, 4 (r5)
  lwz r8, 0xC (r9) ; Z pos
  stw r8, 8 (r5)
  
  lha r8, 2 (r9) ; Rotation
  sth r8, 0xC (r5)
  mr r6, r8 ; Argument r6 to setRestartOption needs to be the rotation
  mr r25, r8 ; Also modify the local variable rotation in Makar's code (for when he calls set__19dSv_player_priest)
  
  lbz r8, 1 (r9) ; Room index
  stb r8, 0xE (r5)
  mr r7, r8 ; Argument r7 to setRestartOption needs to be the room index
  mr r26, r8 ; Also modify the local variable room index in Makar's code (for when he calls set__19dSv_player_priest)
  
  after_resetting_makar_position:
  
  addi r3, r10, 0x1148 ; We overwrite r3 earlier, need to restore it for setRestartOption call
  bl setRestartOption ; Replace the function call we overwrote to call this custom function
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global makar_possible_wt_spawn_positions
makar_possible_wt_spawn_positions:
  ; Spawn ID, room index, rotation, X pos, Y pos, Z pos
  ; WT entrance
  .byte 15, 0xF
  .short 0x94A0
  .float -3651.02, 1557.67, 13235.2
  ; First WT warp pot
  .byte 22, 0
  .short 0x4000
  .float -4196.33, 754.518, 7448.5
  ; Second WT warp pot
  .byte 23, 2
  .short 0xB000
  .float 668.107, 1550, 2298.75
  ; Third WT warp pot
  .byte 24, 12
  .short 0xC000
  .float 14203.1, -5062.49, 8948.05
  ; Inter-dungeon warp pot in WT
  .byte 69, 3
  .short 0x4000
  .float -4146.65, 1100, 47.88
  ; Boss Door
  .byte 70, 12
  .short 0
  .float 14100, -5062.49, 9300


; When the player enters Earth Temple, reset Medli's position to the starting room, or to near the warp pot the player exited.
; This is to prevent an issue where the player can't get past the first room without Medli unless they have Deku Leaf.
.org 0x02286abc
	bl reset_medli_position_to_start_of_dungeon
.org 0x02286a98
	nop
.org @NextFreeSpace
.global reset_medli_position_to_start_of_dungeon
reset_medli_position_to_start_of_dungeon:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  bl FUN_025200d4
  lha r8,0x513c(r3)
  
  ; Search through the list of places Medli can spawn from to see which one corresponds to the player's last spawn ID, if any.
  lis r9, medli_possible_et_spawn_positions@ha
  addi r9, r9, medli_possible_et_spawn_positions@l
  li r0, 6 ; 6 possible spawn points
  mtctr r0
  medli_spawn_point_search_loop_start:
  lbz r0, 0 (r9) ; Spawn ID
  cmpw r0, r8
  beq reset_medli_found_matching_spawn_point ; Found the array element corresponding to this spawn ID
  addi r9, r9, 0x10 ; Increase pointer to point to next element
  bdnz medli_spawn_point_search_loop_start ; Loop
  
  ; The player came from a spawn that doesn't correspond to any of the elements in the array, don't change Medli's position.
  b after_resetting_medli_position
  
  reset_medli_found_matching_spawn_point:
  lwz r8, 4 (r9) ; X pos
  stw r8, 0 (r5)
  lwz r8, 8 (r9) ; Y pos
  stw r8, 4 (r5)
  lwz r8, 0xC (r9) ; Z pos
  stw r8, 8 (r5)
  
  lha r8, 2 (r9) ; Rotation
  sth r8, 0xC (r5)
  mr r6, r8 ; Argument r6 to setRestartOption needs to be the rotation
  mr r26, r8 ; Also modify the local variable rotation in Medli's code (for when she calls set__19dSv_player_priest)      might need to be r25 instead
  
  lbz r8, 1 (r9) ; Room index
  stb r8, 0xE (r5)
  mr r7, r8 ; Argument r7 to setRestartOption needs to be the room index
  mr r28, r8 ; Also modify the local variable room index in Medli's code (for when she calls set__19dSv_player_priest)      might need to be r26 instead
  
  after_resetting_medli_position:
  
  addi r3, r10, 0x1148 ; We overwrite r3 earlier, need to restore it for setRestartOption call
  bl setRestartOption ; Replace the function call we overwrote to call this custom function
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global medli_possible_et_spawn_positions
medli_possible_et_spawn_positions:
  ; Spawn ID, room index, rotation, X pos, Y pos, Z pos
  ; ET entrance
  .byte 0, 0
  .short 0x8000
  .float -7215.21, -200, 5258.79
  ; First ET warp pot
  .byte 22, 2
  .short 0xE000
  .float -2013.11, 200, -1262.97
  ; Second ET warp pot
  .byte 23, 6
  .short 0x8000
  .float 4750, 350, -2251.06
  ; Third ET warp pot
  .byte 24, 15
  .short 0xE000
  .float -2371.38, -2000, 8471.54
  ; Inter-dungeon warp pot in ET
  .byte 69, 1
  .short 0
  .float -8010, 1000, -1508.94
  ; Boss Door Exit
  .byte 17, 15
  .short 0x8000
  .float -5780, -2300, 9700


; If a Moblin sees you when you have no sword equipped, it will catch you and bring you to the jail cell in FF1.
; Skip all the sword checks and pretend the player does have a sword so that this doesn't happen.
.org 0x021cd230
	b 0x021cd298
.org 0x021ca1a8
	b 0x021ca210


; Make invisible walls that appear only when you have no sword never appear so swordless works better.
.org 0x023164c8
	; This code is run for invisible walls that have their switch index set to FF - an invalid switch used to indicate that the player's sword should be checked instead.
	; We make chk_appear always return false.
	li r3, 0


; Allow pigs to be enraged when the player has no sword equipped.
.org 0x0219264c
	nop
.org 0x02190ef4
	b 0x02190f0c


; Remove some code that hides the quest markers when files are created
.org 0x02677d3c
  blr
; Skip the function that normally updates the markers
.org 0x02678b74
	blr

; Fixes new game+ so that picto box related things aren't flagged as done already.
; (Note: There are some more things the new game+ initialization function besides these ones that seem like they could potentially cause other issues, but their purpose is unknown, so theyre not removed for now.)
.org 0x025b9b98
	nop
.org 0x025b9bb0
	nop
.org 0x025b9c28
	nop
.org 0x025b9c48
	nop
.org 0x025b9c5c
	nop


; Make the stone door and whirlpool blocking Jabun's cave appear even when the Endless Night event bit is not set.
.org 0x023149ec ; Branch taken if Endless Night event bit is not set
	nop ; Remove it
.org 0x0231987c ; In is_exist__Q29daObjAuzu5Act_cCFv
	b 0x023198b4 ; Skip over the Endless Night event bit check and just always return true


; Make Komali disappear from his room from the start, instead of waiting until after you own Din's Pearl.
; This is so there aren't two Komali's in the game at the same time.
.org 0x02227da4
	li r31, 0
	b 0x02227de0


; Prevent the lava outside the entrance to DRC from solidifying once you own Din's Pearl.
; This is so the puzzle with throwing the bomb flowers doesn't become pointless.
.org 0x02339554
	li r3, 0


; Always spawn the Moblins and Darknuts inside Hyrule Castle.
.org 0x025255f4 ; In getLayerNo
	 b 0x025252dc ; Skip checking various event bits and the number of triforce shards owned, just always use the same layer


; Originally the Windfall bomb shop owner only lowers the price of the bombs he sells to be reasonable after you have obtained Nayru's Pearl.
; Remove these checks so he always sells them at the lower prices from the start of the game.
; Note: Technically this shop owner is not properly coded to refuse to sell bombs to the player if they don't own the bombs upgrade yet.
; However, this doesn't matter in practice because he will only sell you bombs if you have less than your maximum, and there is no way to use them up and get less than your maximum until you own the bombs upgrade.
; The only side effect of this is that his purchase error message will be the same as if you were simply full on bombs ("you just can't carry that much"), but that accurate enough that it's fine.
.org 0x0220b5cc
	li r3, 1
.org 0x02209af4
	li r3, 1
.org 0x022099a0
	li r3, 1
.org 0x02209c48
	li r3, 1
.org 0x02209c70
	li r3, 1
.org 0x02209f88
	li r3, 1
.org 0x02209fcc
	li r3, 1
.org 0x02208d34
	li r3, 1


; Remove a check that stops time passage until you get the Wind Waker
.org 0x0255B46C
  li r3, 1

; Make KoRL assume you always have the sail
; This avoids several textboxes he normally gives the player
.org 0x0247348C
  li r3, 1
.org 0x024731E8
  li r3, 1

; Several NPCs show different text until you own the sail
; Remove this check so they give their normal text
.org 0x022C7A4C ; Kamo
  b 0x022C7A70
.org 0x022C7D4C ; Kane
  b 0x022C7D70
.org 0x022c7e40 ; Candy
  b 0x022c7e64
.org 0x022d9520 ; Zunari
  b 0x022d95d8
.org 0x022e0a5c ; Fishmen
  b 0x022e0a70



; Make it easier to have the Great Deku Tree mark the Koroks on your sea chart.
; In HD you need to exit Forest Haven while you own Farore's Pearl, talk to an Island Korok with a withering tree, then re-enter and talk to the Deku Tree, specifically choosing the option to ask about Forest Water.
; We skip a couple of checks and go straight to the dialogue tree so that just talking to the Deku Tree and asking about the Forest Water is enough.
.org 0x0222C9BC ; In daNpc_De1_c::getMsg
  b 0x0222CA1C ; Skip straight to the question-asking text

; Change the branching so you can ask about forest water (to mark the map), then use the other options
.org 0x0222C6D8 ; In daNpc_De1_c::next_msgStatus
  li r4, 0x3940 ; Event bit for putting koroks on the sea chart
.org 0x0222C6E8
  cmpwi r3, 1 ; Invert the branch so you can still ask about KoRL afterwards

; Remove a check that requires an empty bottle for the koroks to be marked
.org 0x026790B0
  li r3, 1



; Make Kogoli (a Rito on Dragon Roost Island) not disappear after Medli is awakened as a sage.
.org 0x021FCDE4 ; In daNpc_Bm1_c::init_BMD_1(void)
  ; Normally Kogoli checks event bit 1620 (Medli is in dungeon mode and can be lifted/called) and deletes himself if it's set.
  ; We change him to ignore that bit or otherwise he would not appear at all, since Medli is awakened from the start in the randomizer.
  nop



 ; Modify the Tower Servants to work in any order
.org 0x022AC06C ; In daNpc_Os_c::eventOrderCheck
  ; Prevent the east Servant of the Tower from starting an event when that servant's switch is set.
  ; This stops the Command Melody stone tablet from disappearing prematurely, and also removes the servant's unnecessary line of dialogue.
  ; This branch is normally taken for the west and north servants (Os1 and Os2). We modify it to be taken for all servants.
  b 0x022AC0B4
.org 0x022AE9C8
  ; Normally the servants would not keep the light beam they shoot on until even bit 0x1B01 is set, which happens once the north servant is returned.
  ; Remove this check so the beam is always on for servants that are on their pedestals.
  nop
  
; Normally, the east Servant of the Tower would set event bit 0x2510 to tell the Command Melody stone tablet that it can disappear permanently because the player has finished using it.
; But we allow that tablet to be used before returning that servant, so we have to change it so the tablet itself sets that event bit.
.org 0x0235BF38
  b set_item_obtained_from_totg_tablet_event_bit
.org @NextFreeSpace
.global set_item_obtained_from_totg_tablet_event_bit
set_item_obtained_from_totg_tablet_event_bit:
  lis r3,gameInfo_ptr@ha
  lwz r3,gameInfo_ptr@l(r3)
  addi r3,r3, 0x644
  li r4, 0x2510 ; Learned Command Melody from the TotG stone tablet
  bl onEventBit
  lwz r0, 0x14(sp) ; replace the line we overwrote to jump here
  b 0x0235BF3C
; The west and north doors in the TotG hub room usually do not glow until they have been unlocked.
; But rather than just checking if the door is actually unlocked directly, they are hardcoded to
; check the vanilla conditions for unlocking them, resulting in them not being properly lit up from the start in the randomizer.
; We remove these conditions and make them always glow until the corresponding servant behind the door has been returned.
.org 0x0252C670 ; In dDoor_hkyo_c::proc(dDoor_info_c *)
  ; For the west door.
  ; Originally checked if the Command Melody is in your inventory.
  nop
.org 0x0252C6A4 ; In dDoor_hkyo_c::proc(dDoor_info_c *)
  ; For the north door.
  ; Originally checked if the west servant has been returned.
  nop
