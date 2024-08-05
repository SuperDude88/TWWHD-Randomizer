; Makes functions that play music just return instantly instead.
.org 0x0202204C ; JAIZelBasic::bgmStart
	blr
.org 0x0202879C ; JAIZelBasic::bgmNowBattle
	blr
.org 0x0201DE00 ; JAIZelBasic::subBgmStart
	blr
.org 0x02027F04 ; JAIZelBasic::bgmStreamPlay
	blr
.org 0x02897F48 ; Something related to streamed sounds
	li r3, 0x2 ; always pretend the sound id was invalid
	blr
.org 0x02030F1C ; Checks if some of the credits music is loaded/playing
	li r3, 0x1 ; Pretend it finished loading (next scene won't load otherwise)
	blr
