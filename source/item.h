#pragma once

struct creature;

extern bool need_update_items;

enum wearn : unsigned char {
	MeleeWeapon, MeleeWeaponOffhand, RangedWeapon, Ammunition,
	Torso, Head, Neck, Backward, Girdle, Gloves, FingerRight, FingerLeft, Elbows, Legs,
	Backpack
};
enum magicn : unsigned char {
	Mundane, Cursed, Blessed, Artifact,
};
enum itemn : unsigned char {
	CP, SP, GP,
	Staff, Spear, Axe, Mace, WarHammer, GreatMace, GreatAxe,
	Dagger, ShortSword, LongSword, Scimitar, GreatSword,
	ShortBow, LongBow, Crossbow, HeavyCrossbow,
	Robe, LeatherArmor, StuddedArmor, HideArmor, ScaleMail, ChainMail, PlateMail,
	LastItem = PlateMail,
};
struct item {
	itemn type;
	unsigned char count;
	union {
		unsigned short properties;
		struct {
			unsigned char power : 5; // Item magical power index (1-31) or 0 - if no magical power
			unsigned char broken : 3; // Charges or Broken status
			magicn magic : 2;
			unsigned char identified : 1;
		};
	};
	constexpr item() : type((itemn)0), count(0), properties(0) {}
	constexpr item(itemn v, unsigned char count = 1) : type(v), count(count), properties(0) {}
	explicit operator bool() const { return type != 0; }
	creature* owner();
	int	cost() const;
	int damage() const;
	int	weight() const;
	bool broke() const;
	void clear() { count = 0; type = (itemn)0; properties = 0; }
	bool is(magicn v) const { return magic == v; }
	bool is(wearn v) const;
	bool iscoins() const { return type == CP || type == SP || type == GP; }
	bool istwohanded() const;
	void join(item& v);
	void use();
};
struct wearable {
	typedef int (item::*fnitemget)() const;
	item wears[Legs + 1];
	int	money;
	creature* owner();
	void additem(item& v, bool try_equip = false);
	void additem(const item& v) { item it = v; additem(it); }
	bool equip(item& v);
	bool equip(const item& v) { item it = v; return equip(it); }
	bool is(itemn v) const { for(auto& e : wears) if(e && e.type==v) return true; return false; }
	bool iswear(const void* p) const { return p >= wears && p <= wears + Legs; }
	item* getwear(wearn id) { return wears + id; }
	const item* getwear(const void* data) const;
};

void add_item(short unsigned area_index, short unsigned index, item& v);
void add_item(creature* p, item& v);
void update_items();

