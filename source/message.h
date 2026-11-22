#pragma once

enum glown : unsigned char {
	GlowGray, GlowBlack, GlowWhite,
	GlowRed, GlowGreen, GlowBlue,
	GlowBrown, GlowYellow,
	LastGlow = GlowYellow,
};
enum messagen : unsigned char {
	ApplicationTitle, Cancel, Apply, PickUp, DropDown, Inventory,
	Coins, Rounds,
	LevelUp, MsgMultiplyChoose,
	MsgHeal, MsgCurePoison, MsgTwoHands,
	TomeLearnNothing, TomeLearnSkill, TomeLearnSpell, TomeReaded,
	NotEnoughMana, MsgNoTargets, MsgNoRangedWeapon, MsgNoAmmo,
	TasteBad, TasteNormal, TasteGood,
	PlayerEat, PlayerReadTome, PlayerKilled, PlayerFoundSecretDoor, PlayerFoundTrap,
	PlayerBlock, PlayerDodge,
	PlayerPickUpItem, PlayerDropItem,
	PlayerUseLastRodCharge,
	MsgSkillFooter,
	ItemGrowColor, ItemWearTotal, ItemDecoy,
	ItemDamaged, ItemBurnAway, ItemMeldDown, ItemCrumbleToDust, ItemCrushingOut,
	LastMessage = ItemCrushingOut,
};