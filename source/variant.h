#pragma once

enum abilityn : unsigned char;
enum featn : unsigned char;
enum gendern : unsigned char;
enum itemn : unsigned char;
enum monstern : unsigned char;
enum spelln : unsigned char;

enum variantn : unsigned char {
	Variant,
	Ability, Feat, Gender, Item, Monster, Spell
};
union variant {
	struct {
		variantn type;
		unsigned char value;
	};
	short unsigned u;
	constexpr variant() : u(0) {}
	constexpr variant(abilityn v) : type(Ability), value(v) {}
	constexpr variant(featn v) : type(Feat), value(v) {}
	constexpr variant(gendern v) : type(Gender), value(v) {}
	constexpr variant(itemn v) : type(Item), value(v) {}
	constexpr variant(monstern v) : type(Monster), value(v) {}
	constexpr variant(spelln v) : type(Spell), value(v) {}
	constexpr variant(variantn v) : type(Variant), value(v) {}
	constexpr bool operator==(const variant& v) const { return u == v.u; }
};