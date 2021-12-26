; skip some code so the long intro movie is skipped.
.org 0x025aed5c
	b 0x025aee18
.org 0x025aef08
	nop
.org 0x025aeec0
	mr. r0, r30


; This makes the warps out of boss rooms always skip the cutscene usually shown the first time you beat the boss and warp out.
.org 0x024d602c
	li r3, 1
	blr


; Remove the cutscene where the Tower of the Gods rises out of the sea.
; To do this we modify the goddess statue's code to skip starting the raising cutscene.
; Instead we branch to code that ends the current pearl-placing event after the tower raised event bit is set.
.org 0x02336d68
	b 0x02336d94


; In order to get rid of the cutscene where the player warps down to Hyrule 3, we set the HYRULE_3_WARP_CUTSCENE event bit in the custom function for initializing a new game.
; But then that results in the warp appearing even before the player should unlock it.
; So we replace a couple places that check that event bit to instead call a custom function that returns whether the warp should be unlocked or not.
.org 0x024d4ec0
	bl check_hyrule_warp_unlocked
.org 0x024d53d0
	bl check_hyrule_warp_unlocked
; Custom function that checks if the warp down to Hyrule should be unlocked.
; Requirements: Must have all 8 pieces of the Triforce.
.org @NextFreeSpace
.global check_hyrule_warp_unlocked
check_hyrule_warp_unlocked:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lis r3, 0x1020
  lwz r3,-0x7b24(r3)
  
  addi r3, r3, 0xD4
  bl getTriforceNum
  cmpwi r3, 8
  bge hyrule_warp_unlocked
  
  hyrule_warp_not_unlocked:
  li r3, 0
  b hyrule_warp_end
  
  hyrule_warp_unlocked:
  li r3, 1
  
  hyrule_warp_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


; Change the conditions that cause certain letters to be sent to Link so they don't depend on seeing cutscenes.
.org 0x023a421c
	li r3, 4
	bl isStageBossEnemy
.org 0x025542e0
	li r3, 2
	bl isStageBossEnemy
.org 0x02554308
	li r3, 2
	bl isStageBossEnemy


; Modify the code for warping with the Ballad of Gales to get rid of the cutscene that accompanies it.
.org 0x02482764
  ; Get rid of the line that checks if KoRL has reached a high enough Y pos to start the warp yet.
	nop
.org 0x0247dcac
  ; Get rid of the line that plays the warping music, since it would continue playing after the warp has happened.
	nop


; skipping song replays breaks a lot


; Change Tott to only dance once to teach you the Song of Passing, instead of twice.
.org 0x022efb2c
	li r0, 1


; Change the NPC version of Makar that spawns when you kill Kalle Demos to not initiate the event where he talks to you and thanks you for saving him.
; In addition to being unnecessary, that cutscene has an extremely small chance of softlocking the game even in vanilla.
.org 0x0221f530
	li r3, 1


; Modify the item get funcs for the 3 pearls to call custom functions that automatically place the pearls as soon as you get them.
.org 0x0254f52c
	b give_pearl_and_raise_totg_if_necessary
.org 0x0254f540
	b give_pearl_and_raise_totg_if_necessary
.org 0x0254f554
	b give_pearl_and_raise_totg_if_necessary
.org @NextFreeSpace
; Custom function that gives a goddess pearl and also places it in the statue's hands automatically, and raises TotG if the player has all 3 pearls.
.global give_pearl_and_raise_totg_if_necessary
give_pearl_and_raise_totg_if_necessary:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r4 ; Preserve argument r4, which has the pearl index to give
  
  bl onSymbol ; Replace the call we overwrote to jump here, which gives the player a specific pearl
  
  lis r3, 0x1020
  lwz r3, -0x7b24(r3)
  addi r3,r3,0x644
  
  ; Check the pearl index to know which event flag to set for the pearl being placed
  cmpwi r31, 0
  beq place_nayrus_pearl
  cmpwi r31, 1
  beq place_dins_pearl
  cmpwi r31, 2
  beq place_farores_pearl
  b check_should_raise_totg
  
  place_nayrus_pearl:
  li r4, 0x1410 ; Placed Nayru's Pearl
  bl onEventBit
  b check_should_raise_totg
  
  place_dins_pearl:
  li r4, 0x1480 ; Placed Din's Pearl
  bl onEventBit
  b check_should_raise_totg
  
  place_farores_pearl:
  li r4, 0x1440 ; Placed Farore's Pearl
  bl onEventBit
  
  check_should_raise_totg:
  lis r5, 0x1020
  lwz r5, -0x7b24(r3)
  addi r5,r5,0xDF
  lbz r4, 0 (r5)
  cmpwi r4, 7
  bne after_raising_totg ; Only raise TotG if the player owns all 3 pearls
  
  li r4, 0x1E40 ; TOWER_OF_THE_GODS_RAISED
  bl onEventBit
  li r4, 0x2E80 ; PEARL_TOWER_CUTSCENE
  bl onEventBit
  after_raising_totg:
  
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


; After you kill Puppet Ganon, he would normally respawn you in his room but override the layer to be layer 9 for the cutscene there.
; We set the switch for having already seen that cutscene in the new game initialization code, but then the rope you need to climb doesn't appear because the layer is wrong.
; We remove the layer override from Puppet Ganon's call to setNextStage.
.org 0x0207fbfc
	li r6, -1


; Change all treasure chests to open quickly.
; Removes the build up music, uses the short opening event instead of the long dark room event, and use the short chest opening animation.
; This change also fixes the bug where the player can duplicate items by using storage on the non-wooden chests.
.org 0x024b7408
	b 0x024b745c
.org 0x024b74d4
	nop
; some data edits that would be compiled strangely


; Prevent Ivan from automatically triggering the cutscene where the Killer Bees tell you about Mrs. Marie's birthday and the Joy Pendant in the tree.
; (It can still be manually triggered by talking to any of the Killer Bees, in case you actually want to activate the Joy Pendant in the tree.)
.org 0x02299808
	b 0x02299860
; But because the above cutscene is also what normally allows you to give Joy Pendants to Mrs. Marie, we instead change the event bit she checks to enable that (1E04) to instead check the event bit for her having given you the reward for catching the Killer Bees (1F80).
; This is so the player doesn't need to manually trigger the above cutscene to do the Joy Pendant trading quests.
.org 0x022400ac
	li r4, 0x1F80
.org 0x02240688
	li r4, 0x1F80

.close
