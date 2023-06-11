; Normally the Great Fairies will give you the next tier max-up upgrade, depending on what you currently have.
; So if you already got the 1000 Rupee Wallet from one Great Fairy, the second one will give you the 5000 Rupee Wallet, regardless of the order you reach them in.
; This patch changes it so they always give a constant item even if you already have it, so that the item can be randomized safely.
.org 0x02095B20
	cmpwi r3, 0x0
.org 0x02095B48
	cmplwi r3, 0x2
.org 0x02095B70
	cmplwi r3, 0x4


; The event where the player gets the Wind's Requiem actually gives that song to the player twice.
; The first one is hardcoded into Zephos's AI and only gives the song.
; The second is part of the event, and also handles the text, model, animation, etc, of getting the song.
; Getting the same item twice is a problem for some items, such as rupees. So we remove the first one.
.org 0x022451D8
	b 0x022451E0


; The 6 Heart Containers that appear after you kill a boss are all created by the function createItemForBoss.
; createItemForBoss hardcodes the item ID 8, and doesn't care which boss was just killed. This makes it hard to randomize boss drops without changing all 6 in sync.
; So we make some changes to createItemForBoss and the places that call it so that each boss can give a different item.
; Nop out the instruction that loads 8 into r4. This way it simply passes whatever it got as argument r4 into the next function call to createItem.
.org 0x025D8A70
	nop
; Second we modify the code for the "disappear" cloud of smoke when the boss dies.
; This cloud of smoke is what spawns the item when Gohma, Kalle Demos, Helmaroc King, and Jalhalla die.
; So we need a way to pass the item ID from the boss's code to the disappear cloud's parameters and store them there.
; Read the item ID parameter when the cloud is about to call createItemForBoss.
.org 0x0212456C
	lbz r4, 0x00B0(r12)
; Third we change how the boss item ACTR calls createItemForBoss.
; (This is the ACTR that appears if the player skips getting the boss item after killing the boss, and instead comes back and does the whole dungeon again.)
; Normally it sets argument r4 to 1, but createItemForBoss doesn't even use argument r4.
; So we change it to load one of its params (mask: 0000FF00) and use that as argument r4.
; This param was unused and just 00 in the original game, but the randomizer will set it to the item ID it randomizes to that location.
; Then we will be calling createItemForBoss with the item ID to spawn in argument r4. Which due to the above change, will be used correctly now.
.org 0x020D0AE4
	lbz r4, 0x00B2(r31)

; a few other registers copy the value from r4 after it has a constant loaded
; we change r4, so we need to replace the original copies with loads
.org 0x020D0AF0
	li r8, 0x1

.org 0x02124578
	li r7, 0x0

.org 0x02124580
	li r8, 0x0

.org 0x021031f0
	li r6, 0x0


; The heart container item get function (item_func_utuwa_heart) usually handles setting the flag for having taken the current dungeon's boss item.
; But if the player got a heart container somewhere in a dungeon other than from the boss, this could cause the boss's actual item to disappear.
; We modify the code to remove the calls to set the flag.
.org 0x0254DCA8
	nop
.org 0x0254DCD8
	nop


; Normally when the player takes a boss item drop, it would not set the flag for having taken the current dungeon's boss item, since in vanilla that was handled by the heart container's item get function.
; That could allow the player to get the item over and over again since the item never disappears.
; So we modify createItemForBoss to pass an item flag to createItem, so that the item properly keeps track of whether it has been taken.
; We use item flag 15 for all boss items, since that flag is not used by any items in any of the dungeons.
; (Note that since we're just setting an item flag, the special flag for the dungeon's boss item being taken is never set. But I don't believe that should cause any issues.)
.org 0x025D8A84
	li r5, 0x15


; The Great Fairy inside the Big Octo is hardcoded to double your max magic meter (and fill up your current magic meter too).
; Since we randomize what item she gives you, we need to remove this code so that she doesn't always give you the increased magic meter.
.org 0x02097064
	nop	
.org 0x02097074
	nop
; Also, the magic meter upgrade item itself only increases your max MP.
; In the vanilla game, the Great Fairy would also refill your MP for you.
; Therefore we modify the code of the magic meter upgrade to also refill your MP.
.org 0x02550138
	li r12, 32
	sth r12, 0x5B60(r3)


; When salvage points decide if they should show their ray of light, they originally only checked if you
; have the appropriate Triforce Chart deciphered if the item there is actually a Triforce Shard.
; We don't want the ray of light to show until the chart is deciphered, so we change the salvage point code
; to check the chart index instead of the item ID when determining if it's a Triforce or not.
.org 0x024675F4
	; In HD, removing some triforce charts meant their indexes were no longer in a continuous block
	; We overwrite the calls to getItemNo and isTriforce to manually check each one
	cmpwi r31, 0x1
	beq 0x0246760C
	cmpwi r31, 0x3
	beq 0x0246760C
	cmpwi r31, 0x5
	bne 0x024676b4
	addi r31, r31, 0x60 ; Add 0x60 to r31 to simulate the return value of getItemNo


; The first instance of Medli, who gives the letter for Komali, can disappear under certain circumstances.
; For example, owning the half-power Master Sword makes her disappear. Deliving the letter to Komali also makes her disappear.
; So in order to avoid the item she gives being missable, we just remove it entirely.
; To do this we modify the chkLetterPassed function to always return true, so she thinks you've delivered the letter.
.org 0x0259D780
	li r3, 1


; Normally whether you can use Hurricane Spin or not is determined by if the event bit for the event where Orca teaches it to you is set or not.
; But we want to separate the ability from the event so they can be randomized.
; To do this we change it to check event bit 6901 (bit 01 of byte 803C5295) instead. This bit was originally unused.
.org 0x02442248
	li r4, 0x6901
	; item func pointer modifies a relocation


; Normally Beedle checks if you've bought the Bait Bag by actually checking if you own the Bait Bag item.
; That method is problematic for many items that can get randomized into that shop slot, including progressive items.
; So we change the functions he calls to set the slot as sold out and check if it's sold out to custom functions.
; These custom functions use bit 40 of byte 803C4CBF, which was originally unused, to keep track of this.
.org 0x02215450
	bl set_shop_item_in_bait_bag_slot_sold_out
.org 0x02212C74
	bl check_shop_item_in_bait_bag_slot_sold_out

.org @NextFreeSpace
.global set_shop_item_in_bait_bag_slot_sold_out
set_shop_item_in_bait_bag_slot_sold_out:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; First call the regular SoldOutItem function with the given arguments since we overwrote a call to that in order to call this custom function.
  bl SoldOutItem
  
  ; Set event bit 6902 (bit 02 of byte 803C5295).
  ; This bit was unused in the base game, but we repurpose it to keep track of whether you've purchased whatever item is in the Bait Bag slot of Beedle's shop.
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  addi r3,r3,0x644
  li r4, 0x6902 ; Unused event bit
  bl onEventBit
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr
.global check_shop_item_in_bait_bag_slot_sold_out
check_shop_item_in_bait_bag_slot_sold_out:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; Check event bit 6902 (bit 02 of byte 803C5295), which was originally unused but we use it to keep track of whether the item in the Bait Bag slot has been purchased or not.
  lis r3,gameInfo_ptr@ha
  lwz r3,gameInfo_ptr@l(r3)
  addi r3,r3,0x644
  li r4, 0x6902 ; Unused event bit
  bl isEventBit
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


; Three items are spawned by a call to fastCreateItem:
; * The item buried under black soil that you need the pig to dig up.
; * The item given by the withered trees.
; * The item hidden in a tree on Windfall. (Not modified here since its item is not randomized.)
; This is bad since fastCreateItem doesn't load the field item model in. If the model isn't already loaded the game will crash.
; So we add a new custom function to create an item and load the model, and replace the relevant calls so they call the new function.
; Buried item
.org 0x02526490
	bl custom_createItem
	; Then remove the code that sets bit 0x4000 of the bitfield at item_entity+0x1C4.
	; This bit just seems to offset the item or something, but custom_createItem's item action does this anyway.
	; Furthermore, it's not possible to set that bit until after the item actor has been created, which doesn't happen until later with custom_createItem unlike fastCreateItem.
	nop
	nop
	nop
	nop
	nop


 ; Withered trees
.org 0x02347c34
	bl custom_createItem
	b custom_createItem_return_check
.org @NextFreeSpace
.global custom_createItem_return_check
custom_createItem_return_check:
	mr r30, r3
	cmpwi r30, -1
	beq createItem_not_success
	mr r10, r30
	stw r10, 0x3a4(r31) ; store created actor id

	b 0x02347c40
  createItem_not_success:
	b 0x02347cd4

.org 0x02347c48
	nop ; dont store parent ID to object, not created yet

.org 0x02347cb4
	nop ; dont set flags for object, not created yet

; in _create
.org 0x02346f28
	b initialize_actor_id
; initialize the item table index to -1 (repurposed as actor id)
.org @NextFreeSpace
.global initialize_actor_id
initialize_actor_id:
	li r3, -1
	stw r3, 0x3a4(r30)
	mr r3, r30
	b 0x02346f2c

; in place_heart_part
.org 0x02347abc
	bl create_item_for_withered_trees_without_setting_speeds
.org @NextFreeSpace
.global create_item_for_withered_trees_without_setting_speeds
create_item_for_withered_trees_without_setting_speeds:
	stwu sp, -0x10 (sp)
	mflr r0
	stw r0, 0x14 (sp)
  
	bl custom_createItem
	
	; HD changed how the game checks if you collected the heart piece
	; Manually store the created actor ID over the item table index (otherwise unused in the tree object)
	stw r3, 0x3a4(r28) ; store created actor id

	; We need to set our custom flag to 1 to prevent withered_tree_item_try_give_momentum from setting the speeds for this item actor. (32e and 32f were originally padding)
	li r5, 1
	stb r5, 0x32e(r28)
  
	lwz r0, 0x14 (sp)
	mtlr r0
	addi sp, sp, 0x10
	blr

.org 0x02347ac0
	cmpwi r3, -1
.org 0x02347acc
	nop

 ; We need to partially recreate a function from SD that checked if the heart was created
 ; Since custom_createItem can't store a velocity to the item, we need to check each frame to see if it exists and store the velocity to it
.org 0x02347d18
	bl withered_tree_item_try_give_momentum
.org @NextFreeSpace
.global withered_tree_item_try_give_momentum
withered_tree_item_try_give_momentum:
	stwu sp, -0x20 (sp)
	mflr r0
	stw r0, 0x24 (sp)
	stw r30, 0x18(sp)
	stw r31, 0x1C(sp)

	mr r30, r3
	lwz r3, 0x3a4(r30)
	addis r4, r3, 1
	cmplwi r4, -1
	beq withered_tree_item_try_give_momentum_end ; item not created yet
	addi r4, sp, 0x8
	bl fopAcM_SearchByID
	cmpwi r3, 0
	beq withered_tree_item_set_collected ; Item actor has already been picked up

	lwz r4, 0x8(sp)
	cmpwi r4, 0
	beq withered_tree_item_try_give_momentum_end ; Item actor was just created a few frames ago and hasn't actually been properly spawned yet

	; Update a store that could not be done earlier because the item was not fully spawned
	lwz r10, 0x4(r30)
	stw r10, 0x2e8(r4) ; store parent PcId to item

	; Now that we have the item actor pointer in r4, we need to check if the actor was just created this frame or not.
	; To do that we store a custom flag to an unused byte in the withered tree actor struct.
	lbz r5, 0x32e(r30)
	cmpwi r5, 0
	bne withered_tree_item_try_give_momentum_end ; Already set the flag, so this isn't the first frame it spawned on.

	; Since this is the first frame since the item actor was properly created, we can set its momentum.

	lis r10, withered_tree_item_speeds@ha
	addi r10, r10, withered_tree_item_speeds@l
	lfs f0, 0 (r10) ; Read forward velocity
	stfs f0, 0x370 (r4)
	lfs f0, 4 (r10) ; Read the Y velocity
	stfs f0, 0x340 (r4)
	lfs f0, 8 (r10) ; Read gravity
	stfs f0, 0x374 (r4)

	; Also set bit 0x40 in some bitfield for the item actor.
	; Apparently this bit is for allowing the actor to still move while events are going on. It doesn't seem to really matter in this specific case, but set it just to be completely safe.
	lwz r5, 0x2e0 (r4)
	ori r5, r5, 0x40
	stw r5, 0x2e0 (r4)

	; Now store the custom flag meaning that we've already set the item actor's momentum so we don't do it again.
	li r5, 1
	stb r5, 0x32e(r30)

	b withered_tree_item_try_give_momentum_end

  withered_tree_item_set_collected:
	lis r3, gameInfo_ptr@ha
	lwz r3, gameInfo_ptr@l(r3)
	addi r3, r3, 0x644
	li r4, 0x2e20
	bl onEventBit
	li r0, -1
	stw r0, 0x3a4(r30)

  withered_tree_item_try_give_momentum_end:
	mr r3, r30
	bl daObjFtree_talk_main ; replace the line we overwrote to jump here

	lwz r0, 0x24(sp)
	lwz r30, 0x18(sp)
	lwz r31, 0x1C(sp)
	mtlr r0
	addi sp, sp, 0x20
	blr

.global withered_tree_item_speeds
withered_tree_item_speeds:
	.float 1.75 ; Initial forward velocity
	.float 30 ; Initial Y velocity
	.float -2.1 ; Gravity (Y acceleration)


; Fix the Phantom Ganon from Ganon's Tower so he doesn't disappear from the maze when the player gets Light Arrows, but instead when they open the chest at the end of the maze which originally had Light Arrows.
; We replace where he calls dComIfGs_checkGetItem__FUc with a custom function that checks the appropriate treasure chest open flag.
; We only make this change for Phanton Ganon 2 (in the maze) not Phantom Ganon 3 (when you kill him with Light Arrows).
.org 0x0213B624
	bl check_ganons_tower_chest_opened
; Custom function that checks if the treasure chest in Ganon's Tower (that originally had the Light Arrows) has been opened.
; This is to make the Phantom Ganon that appears in the maze still work if you got Light Arrows beforehand.
.org @NextFreeSpace
.global check_ganons_tower_chest_opened
check_ganons_tower_chest_opened:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  li r3, 8 ; Stage ID for Ganon's Tower.
  li r4, 0 ; Chest open flag for the Light Arrows chest. Just 0 since this is the only chest in the whole dungeon.
  bl dComIfGs_isStageTbox
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr
; Then there's an issue where killing Phantom Ganon 3 first and using his sword to destroy the door makes the sword dropped by Phantom Ganon 2 also disappear, which is bad because then the player wouldn't know which way to go in the maze.
.org 0x020C1CA0
	bl check_phantom_ganons_sword_should_disappear
.org @NextFreeSpace
.global check_phantom_ganons_sword_should_disappear
check_phantom_ganons_sword_should_disappear:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; makes sword always disappear
  ; First replace the event flag check we overwrote to call this custom function.
  bl isEventBit
  
  ; If the player hasn't destroyed the door with Phantom Ganon's sword yet, we don't need to do anything different so just return.
  cmpwi r3, 0
  beq check_phantom_ganons_sword_should_disappear_end
  
  ; If the player has destroyed the door, check if the current stage is the Phantom Ganon maze, where Phantom Ganon 2 is fought.
  bl FUN_025200d4
  addi r3, r3, 0x5133
  lis r4, phantom_ganon_maze_stage_name@ha
  addi r4, r4, phantom_ganon_maze_stage_name@l
  subi r4, r4, 1
strcmp_start:
    lbzu r5, 0x1(r3)
    lbzu r6, 0x1(r4)
    cmplw r5, r6
    bne strncmp_end
    cmplwi r5, 0
    bne strcmp_start

strncmp_end:
  	subf. r3, r5, r6

  ; If the stage is the maze, strcmp will return 0, so we return that to tell Phantom Ganon's sword that it should not disappear.
  ; If the stage is anything else, strcmp will not return 0, so Phantom Ganon's sword should disappear.
check_phantom_ganons_sword_should_disappear_end:
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global phantom_ganon_maze_stage_name
phantom_ganon_maze_stage_name:
  .string "GanonJ"
  .align 2 ; Align to the next 4 bytes


; Fix some Windfall townspeople not properly keeping track of whether they've given you their quest reward item yet or not.
; Pompie/Vera, Minenco, Dampa, and Kamo give you treasure charts in the vanilla game, and they check if they've given you their item by calling checkGetItem.
; But that doesn't work for non-unique items, such as progressive items, rupees, etc.
; So we need to change their code to set and check event bits that were originally unused in the base game.
.org 0x101c399c ; For Pompie and Vera
	.short 0x6904
.org 0x101c39a0 ; For Minenco
	.short 0x6908
.org 0x101c39ac ; For Kamo
	.short 0x6910

.org 0x022C1038
	bl create_item_and_set_event_bit_for_townsperson
.org 0x022C1064
	bl create_item_and_set_event_bit_for_townsperson
.org @NextFreeSpace
.global create_item_and_set_event_bit_for_townsperson
create_item_and_set_event_bit_for_townsperson:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r4 ; Preserve argument r4, which has both the item ID and the event bit to set.

  clrlwi r4,r4,24 ; Get the lowest byte (0x000000FF), which has the item ID
  bl fopAcM_createItemForPresentDemo

  rlwinm. r4,r31,16,16,31 ; Get the upper halfword (0xFFFF0000), which has the event bit to set
  beq create_item_and_set_event_bit_for_townsperson_end ; If the event bit specified is 0000, skip to the end of the function instead
  mr r31, r3 ; Preserve the return value from createItemForPresentDemo so we can still return that
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  addi r3,r3,0x644
  bl onEventBit ; Otherwise, set that event bit
  mr r3, r31

create_item_and_set_event_bit_for_townsperson_end:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.org 0x022BE9D8 ; For Pompie and Vera
	li r3, 0x6904
	bl isEventBit_wrapper
.org 0x022BFD4C ; For Kamo
	li r3, 0x6910
	bl isEventBit_wrapper
.org 0x022C3360 ; For Kamo
	li r3, 0x6910
	bl isEventBit_wrapper
.org 0x022C6D94 ; For Minenco
	li r3, 0x6908
	bl isEventBit_wrapper
.org 0x022C7AE0 ; For Kamo
	li r3, 0x6910
.org 0x022C7AE8
	bl isEventBit_wrapper
.org 0x022C7B74 ; For Kamo
	li r3, 0x6910
	bl isEventBit_wrapper
.org 0x022BEF34 ; For Kamo
	li r3, 0x6910
	bl isEventBit_wrapper
.org 0x022BF178 ; For Kamo
	li r3, 0x6910
	bl isEventBit_wrapper
.org 0x022BF310 ; For Pompie and Vera
	li r3, 0x6904
	bl isEventBit_wrapper

.org 0x022c6440 ; For Dampa
	li r3, 0x6a80
.org 0x022c6448
	bl isEventBit_wrapper

.org 0x022CC550
	li r3, 0x6904
.org 0x022CC558
	bl isEventBit_wrapper
.org 0x022CC578
	bl isEventBit_wrapper

.org 0x022c6454 ; For Dampa
	b set_dampa_event_bit
.org @NextFreeSpace
.global set_dampa_event_bit
set_dampa_event_bit:
	lis r3, gameInfo_ptr@ha
	lwz r3, gameInfo_ptr@l(r3)
	addi r3, r3, 0x644
	li r4, 0x6a80
	bl onEventBit
	lis r12, dampa_minigame_item_id@ha
	addi r12, r12, dampa_minigame_item_id@l
	lbz r12, 0 (r12) ; Load item ID
	b 0x022c6458

.global dampa_minigame_item_id
dampa_minigame_item_id:
  .byte 0xFD
  .align 2 ; Align to the next 4 bytes

.org 0x022CFA80
	bl lenzo_set_deluxe_picto_box_event_bit
.org @NextFreeSpace
.global lenzo_set_deluxe_picto_box_event_bit ; check this runs at the right point
lenzo_set_deluxe_picto_box_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  ; First replace the line we overwrote to call this custom function.
  stb r10,0xb39(r31)
  
  ; Next set an originally-unused event bit to keep track of whether the player got the item that was the Deluxe Picto Box in vanilla.
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  addi r3,r3,0x644
  li r4, 0x6920
  bl onEventBit
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.org 0x022D05B4
	li r3, 0x6920
	bl isEventBit_wrapper


; Zunari usually checks if he's given you the item in the Magic Armor item slot by calling checkGetItem.
; That doesn't work well when the item is randomized, so we have to replace the code with code to set and check a custom unused event bit.
.org 0x022D94E8 ; Where he checks if you have own the Magic Armor by calling checkItemGet.
	; We replace this with a call to isEventBit checking our custom event bit.
	li r3, 0x6940
	nop
	bl isEventBit_wrapper
.org 0x022DA8A4
	bl zunari_give_item_and_set_magic_armor_event_bit
.org @NextFreeSpace
.global zunari_give_item_and_set_magic_armor_event_bit
zunari_give_item_and_set_magic_armor_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r4 ; Preserve argument r4, which has the item ID
  
  bl fopAcM_createItemForPresentDemo
  
  lis r4, zunari_magic_armor_slot_item_id@ha
  addi r4, r4, zunari_magic_armor_slot_item_id@l
  lbz r4, 0 (r4) ; Load what item ID is in the Magic Armor slot. This value is updated by the randomizer when it randomizes that item.
  
  cmpw r31, r4 ; Check if the item ID given is the same one from the Magic Armor slot.
  bne zunari_give_item_and_set_magic_armor_event_bit_end ; If it's not the item in the Magic Armor slot, skip to the end of the function
  mr r31, r3 ; Preserve the return value from createItemForPresentDemo so we can still return that
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  addi r3,r3,0x644
  li r4, 0x6940 ; Unused event bit that we use to keep track of whether Zunari has given the Magic Armor item
  bl onEventBit
  mr r3, r31
  
zunari_give_item_and_set_magic_armor_event_bit_end:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global zunari_magic_armor_slot_item_id
zunari_magic_armor_slot_item_id:
  .byte 0x2A ; Default item ID is Magic Armor. This value is updated by the randomizer when this item is randomized.
  .align 2 ; Align to the next 4 bytes


; Salvage Corp usually check if they gave you their item by calling checkGetItem. That doesn't work well when it's randomized.
; We replace the code so that it sets and checks a custom unused event bit.
.org 0x022E512C
	li r3, 0x6980
.org 0x022E5154
	bl isEventBit_wrapper
.org 0x022e5fb0
	bl salvage_corp_give_item_and_set_event_bit
.org 0x022e5fdc
	bl salvage_corp_give_item_and_set_event_bit	
.org @NextFreeSpace
.global salvage_corp_give_item_and_set_event_bit
salvage_corp_give_item_and_set_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  
  bl fopAcM_createItemForPresentDemo
  
  mr r31, r3 ; Preserve the return value from createItemForPresentDemo so we can still return that
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  addi r3,r3,0x644
  li r4, 0x6980 ; Unused event bit that we use to keep track of whether the Salvage Corp has given you their item yet or not
  bl onEventBit
  mr r3, r31
  
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


.org 0x0227d244
	li r3, 0x6A01
	nop
	bl isEventBit_wrapper
.org 0x0227c2b8
	bl maggie_give_item_and_set_event_bit
.org @NextFreeSpace
.global maggie_give_item_and_set_event_bit
maggie_give_item_and_set_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  
  bl fopAcM_createItemForPresentDemo
  
  mr r31, r3 ; Preserve the return value from createItemForPresentDemo so we can still return that
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  addi r3,r3,0x644
  li r4, 0x6A01 ; Unused event bit
  bl onEventBit
  mr r3, r31
  
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr
.org 0x0227d200
	b 0x0227d240


.org 0x021fccd4
	li r3, 0x6A02
  nop
.org 0x021fcce0
	bl isEventBit_wrapper
.org 0x022022f0
	li r3, 0x6A02
  nop
	bl isEventBit_wrapper
.org 0x02200d54
	b rito_cafe_postman_start_event_and_set_event_bit ; branch instead of branch + link because of call-return thing
.org @NextFreeSpace
.global rito_cafe_postman_start_event_and_set_event_bit
rito_cafe_postman_start_event_and_set_event_bit:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  stw r31, 0xC (sp)
  mr r31, r3 ; Preserve argument r3, which has the Rito postman entity
  
  bl fopAcM_orderOtherEventId
  
  lha r31, 0x9da(r31) ; Load the index of this Rito postman from the Rito postman entity
  cmpwi r31, 0 ; 0 is the one in the Windfall cafe. If it's not that one, we don't want to set the event bit.
  bne rito_cafe_postman_start_event_and_set_event_bit_end
  
  mr r31, r3 ; Preserve the return value from orderOtherEventId so we can still return that (not sure if necessary, but just to be safe)
  lis r3, gameInfo_ptr@ha
  lwz r3, gameInfo_ptr@l(r3)
  addi r3,r3,0x644
  li r4, 0x6A02 ; Unused event bit
  bl onEventBit
  mr r3, r31
  
rito_cafe_postman_start_event_and_set_event_bit_end:
  lwz r31, 0xC (sp)
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr


.org 0x02552774
	nop
	nop


.org 0x02299404
	b 0x02299470
.org 0x02299564
	b 0x0229959c
.org 0x022f42a8
	b 0x022f4358
.org 0x022f50fc
	b 0x022f5134


.org 0x023dbd88
	b check_play_special_item_get_music
.org @NextFreeSpace
.global check_play_special_item_get_music
check_play_special_item_get_music:
  
  ; We overwrite the line that branched to the "normal" item get music, it does not need to be replaced since we always branch back to the same area

  ; Check if the item ID (in r0) matches any of the items with special music.
  cmplwi r4, 0x69 ; Nayru's Pearl
  beq play_pearl_item_get_music
  cmplwi r4, 0x6A ; Din's Pearl
  beq play_pearl_item_get_music
  cmplwi r4, 0x6B ; Farore's Pearl
  beq play_pearl_item_get_music
  cmplwi r4, 0x6D ; Wind's Requiem
  beq play_song_get_music
  cmplwi r4, 0x6E ; Ballad of Gales
  beq play_song_get_music
  cmplwi r4, 0x6F ; Command Melody
  beq play_song_get_music
  cmplwi r4, 0x70 ; Earth God's Lyric
  beq play_song_get_music
  cmplwi r4, 0x71 ; Wind God's Aria
  beq play_song_get_music
  cmplwi r4, 0x72 ; Song of Passing
  beq play_song_get_music
  b 0x023dbdb0

play_pearl_item_get_music:
  lis r3, 0x8000004F@ha ; BGM ID for the pearl item get music
  addi r3, r3, 0x8000004F@l
  b 0x023dbdb0 ; Jump to the code that plays the normal item get music

play_song_get_music:
  lis r3, 0x80000027@ha ; BGM ID for the song get music
  addi r3, r3, 0x80000027@l
  b 0x023dbdb0 ; Jump to the code that plays the normal item get music


.org 0x023b6390
	bl check_tingle_statue_owned

.org 0x022e7b74
	bl check_tingle_statue_owned
.org 0x022e7b88
	bl check_tingle_statue_owned
.org 0x022e7b9c
	bl check_tingle_statue_owned
.org 0x022e7bb0
	bl check_tingle_statue_owned
.org 0x022e7bc4
	bl check_tingle_statue_owned
.org 0x022e954c
	bl check_tingle_statue_owned
.org 0x022e9748
	bl check_tingle_statue_owned
.org 0x022eaa98
	bl check_tingle_statue_owned
.org 0x022eaaac
	bl check_tingle_statue_owned
.org 0x022eaac0
	bl check_tingle_statue_owned
.org 0x022eaad4
	bl check_tingle_statue_owned
.org 0x022eaae8
	bl check_tingle_statue_owned
.org 0x022eae38
	bl check_tingle_statue_owned
.org 0x022eae4c
	bl check_tingle_statue_owned
.org 0x022eae60
	bl check_tingle_statue_owned
.org 0x022eae74
	bl check_tingle_statue_owned
.org 0x022eae88
	bl check_tingle_statue_owned
.org 0x022eb690
	bl check_tingle_statue_owned
.org 0x022eb6bc
	bl check_tingle_statue_owned
.org 0x022eb6e8
	bl check_tingle_statue_owned
.org 0x022eb714
	bl check_tingle_statue_owned
.org 0x022eb740
	bl check_tingle_statue_owned


.org 0x022313a8
	bl doc_bandam_check_new_potion_and_give_free_item
.org 0x02231414
	bl doc_bandam_check_new_potion_and_give_free_item
.org @NextFreeSpace
.global doc_bandam_check_new_potion_and_give_free_item
doc_bandam_check_new_potion_and_give_free_item:
  stwu sp, -0x10 (sp)
  mflr r0
  stw r0, 0x14 (sp)
  
  lwz r0, 0x998 (r30) ; Read the current message ID Doc Bandam is on (r28 has the Doc Bandam entity)
  cmpwi r0, 7627 ; This message ID means he just made a brand new potion for the first time
  ; Any other message ID means he's either giving you or selling you a type he already made before.
  ; So do not give a randomized item in those cases.
  bne doc_bandam_give_item
  
  ; If we're on a newly made potion we need to change the item ID in r4 to be the randomized item
  cmpwi r4, 0x52 ; Green Potion item ID
  beq doc_bandam_set_randomized_green_potion_item_id
  cmpwi r4, 0x53 ; Blue Potion item ID
  beq doc_bandam_set_randomized_blue_potion_item_id
  ; If it's not either of those something unexpected happened, so just give whatever item ID it was originally supposed to give
  b doc_bandam_give_item
  
  doc_bandam_set_randomized_green_potion_item_id:
  lis r4, doc_bandam_green_potion_slot_item_id@ha
  addi r4, r4, doc_bandam_green_potion_slot_item_id@l
  lbz r4, 0 (r4) ; Load what item ID is in the this slot. This value is updated by the randomizer when it randomizes that item.
  b doc_bandam_give_item
  
  doc_bandam_set_randomized_blue_potion_item_id:
  lis r4, doc_bandam_blue_potion_slot_item_id@ha
  addi r4, r4, doc_bandam_blue_potion_slot_item_id@l
  lbz r4, 0 (r4) ; Load what item ID is in the this slot. This value is updated by the randomizer when it randomizes that item.
  
  doc_bandam_give_item:
  bl fopAcM_createItemForPresentDemo
  
  lwz r0, 0x14 (sp)
  mtlr r0
  addi sp, sp, 0x10
  blr

.global doc_bandam_green_potion_slot_item_id
doc_bandam_green_potion_slot_item_id:
	.byte 0x52 ; Default item ID is Green Potion. This value is updated by the randomizer when this item is randomized.
.global doc_bandam_blue_potion_slot_item_id
doc_bandam_blue_potion_slot_item_id:
	.byte 0x53 ; Default item ID is Blue Potion. This value is updated by the randomizer when this item is randomized.
	.align 2 ; Align to the next 4 bytes

.org 0x022301dc
	b 0x02230340



.org 0x02341b0c
	li r4, 5
	; li r5, 0 (r5 is fortunately 0 already)
	addi r3, r12, 0x20
.org 0x02341b14
	bl isSwitch
.org 0x02341b1c
	beq 0x02341b90
