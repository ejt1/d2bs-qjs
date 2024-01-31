#ifndef D2SKILLS_H
#define D2SKILLS_H
// Added by lord2800 - English skill names all in a nice and complete struct.
// TODO: Deprecate this and look it up via the MPQ tables

struct Skill_t {
  char name[64];
  unsigned short skillID;
};

static Skill_t Game_Skills[] = {
    // Skill,					Id
    {"Attack", 0},
    {"Kick", 1},
    {"Throw", 2},
    {"Unsummon", 3},
    {"Left Hand Throw", 4},
    {"Left Hand Swing", 5},
    {"Magic Arrow", 6},
    {"Fire Arrow", 7},
    {"Inner Sight", 8},
    {"Critical Strike", 9},
    {"Jab", 10},
    {"Cold Arrow", 11},
    {"Multiple Shot", 12},
    {"Dodge", 13},
    {"Power Strike", 14},
    {"Poison Javelin", 15},
    {"Exploding Arrow", 16},
    {"Slow Missiles", 17},
    {"Avoid", 18},
    {"Impale", 19},
    {"Lightning Bolt", 20},
    {"Ice Arrow", 21},
    {"Guided Arrow", 22},
    {"Penetrate", 23},
    {"Charged Strike", 24},
    {"Plague Javelin", 25},
    {"Strafe", 26},
    {"Immolation Arrow", 27},
    {"Decoy", 28},
    {"Evade", 29},
    {"Fend", 30},
    {"Freezing Arrow", 31},
    {"Valkyrie", 32},
    {"Pierce", 33},
    {"Lightning Strike", 34},
    {"Lightning Fury", 35},
    {"Fire Bolt", 36},
    {"Warmth", 37},
    {"Charged Bolt", 38},
    {"Ice Bolt", 39},
    {"Frozen Armor", 40},
    {"Inferno", 41},
    {"Static Field", 42},
    {"Telekinesis", 43},
    {"Frost Nova", 44},
    {"Ice Blast", 45},
    {"Blaze", 46},
    {"Fire Ball", 47},
    {"Nova", 48},
    {"Lightning", 49},
    {"Shiver Armor", 50},
    {"Fire Wall", 51},
    {"Enchant", 52},
    {"Chain Lightning", 53},
    {"Teleport", 54},
    {"Glacial Spike", 55},
    {"Meteor", 56},
    {"Thunder Storm", 57},
    {"Energy Shield", 58},
    {"Blizzard", 59},
    {"Chilling Armor", 60},
    {"Fire Mastery", 61},
    {"Hydra", 62},
    {"Lightning Mastery", 63},
    {"Frozen Orb", 64},
    {"Cold Mastery", 65},
    {"Amplify Damage", 66},
    {"Teeth", 67},
    {"Bone Armor", 68},
    {"Skeleton Mastery", 69},
    {"Raise Skeleton", 70},
    {"Dim Vision", 71},
    {"Weaken", 72},
    {"Poison Dagger", 73},
    {"Corpse Explosion", 74},
    {"Clay Golem", 75},
    {"Iron Maiden", 76},
    {"Terror", 77},
    {"Bone Wall", 78},
    {"Golem Mastery", 79},
    {"Raise Skeletal Mage", 80},
    {"Confuse", 81},
    {"Life Tap", 82},
    {"Poison Explosion", 83},
    {"Bone Spear", 84},
    {"Blood Golem", 85},
    {"Attract", 86},
    {"Decrepify", 87},
    {"Bone Prison", 88},
    {"Summon Resist", 89},
    {"Iron Golem", 90},
    {"Lower Resist", 91},
    {"Poison Nova", 92},
    {"Bone Spirit", 93},
    {"Fire Golem", 94},
    {"Revive", 95},
    {"Sacrifice", 96},
    {"Smite", 97},
    {"Might", 98},
    {"Prayer", 99},
    {"Resist Fire", 100},
    {"Holy Bolt", 101},
    {"Holy Fire", 102},
    {"Thorns", 103},
    {"Defiance", 104},
    {"Resist Cold", 105},
    {"Zeal", 106},
    {"Charge", 107},
    {"Blessed Aim", 108},
    {"Cleansing", 109},
    {"Resist Lightning", 110},
    {"Vengeance", 111},
    {"Blessed Hammer", 112},
    {"Concentration", 113},
    {"Holy Freeze", 114},
    {"Vigor", 115},
    {"Conversion", 116},
    {"Holy Shield", 117},
    {"Holy Shock", 118},
    {"Sanctuary", 119},
    {"Meditation", 120},
    {"Fist of the Heavens", 121},
    {"Fanaticism", 122},
    {"Conviction", 123},
    {"Redemption", 124},
    {"Salvation", 125},
    {"Bash", 126},
    {"Sword Mastery", 127},
    {"Axe Mastery", 128},
    {"Mace Mastery", 129},
    {"Howl", 130},
    {"Find Potion", 131},
    {"Leap", 132},
    {"Double Swing", 133},
    {"Pole Arm Mastery", 134},
    {"Throwing Mastery", 135},
    {"Spear Mastery", 136},
    {"Taunt", 137},
    {"Shout", 138},
    {"Stun", 139},
    {"Double Throw", 140},
    {"Increased Stamina", 141},
    {"Find Item", 142},
    {"Leap Attack", 143},
    {"Concentrate", 144},
    {"Iron Skin", 145},
    {"Battle Cry", 146},
    {"Frenzy", 147},
    {"Increased Speed", 148},
    {"Battle Orders", 149},
    {"Grim Ward", 150},
    {"Whirlwind", 151},
    {"Berserk", 152},
    {"Natural Resistance", 153},
    {"War Cry", 154},
    {"Battle Command", 155},
    {"Scroll of Townportal", 219},
    {"Book of Townportal", 220},
    {"Raven", 221},
    {"Poison Creeper", 222},
    {"Werewolf", 223},
    {"Shape Shifting", 224},
    {"Firestorm", 225},
    {"Oak Sage", 226},
    {"Summon Spirit Wolf", 227},
    {"Werebear", 228},
    {"Molten Boulder", 229},
    {"Arctic Blast", 230},
    {"Carrion Vine", 231},
    {"Feral Rage", 232},
    {"Maul", 233},
    {"Fissure", 234},
    {"Cyclone Armor", 235},
    {"Heart of Wolverine", 236},
    {"Summon Dire Wolf", 237},
    {"Rabies", 238},
    {"Fire Claws", 239},
    {"Twister", 240},
    {"Solar Creeper", 241},
    {"Hunger", 242},
    {"Shock Wave", 243},
    {"Volcano", 244},
    {"Tornado", 245},
    {"Spirit of barbs", 246},
    {"Summon Grizzly", 247},
    {"Fury", 248},
    {"Armageddon", 249},
    {"Hurricane", 250},
    {"Fire Blast", 251},
    {"Claw Mastery", 252},
    {"Psychic Hammer", 253},
    {"Tiger Strike", 254},
    {"Dragon Talon", 255},
    {"Shock Web", 256},
    {"Blade Sentinel", 257},
    {"Burst of Speed", 258},
    {"Fists of Fire", 259},
    {"Dragon Claw", 260},
    {"Charged Bolt Sentry", 261},
    {"Wake of Fire", 262},
    {"Weapon Block", 263},
    {"Cloak of Shadows", 264},
    {"Cobra Strike", 265},
    {"Blade Fury", 266},
    {"Fade", 267},
    {"Shadow Warrior", 268},
    {"Claws of Thunder", 269},
    {"Dragon Tail", 270},
    {"Lightning Sentry", 271},
    {"Wake of Inferno", 272},
    {"Mind Blast", 273},
    {"Blades of Ice", 274},
    {"Dragon Flight", 275},
    {"Death Sentry", 276},
    {"Blade Shield", 277},
    {"Venom", 278},
    {"Shadow Master", 279},
    {"Phoenix Strike", 280}};

#endif
