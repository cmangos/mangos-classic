
#ifndef DEF_WORLD_MAP_KALIMDOR_SCRIPTS_H
#define DEF_WORLD_MAP_KALIMDOR_SCRIPTS_H

enum
{
    // Quest 4740
    NPC_GREYMIST_COASTRUNNNER   = 2202,
    NPC_GREYMIST_WARRIOR        = 2205,
    NPC_GREYMIST_HUNTER         = 2206,
    NPC_MURKDEEP                = 10323,
    QUEST_WANTED_MURKDEEP       = 4740,

    // Quest 6134
    GO_GHOST_MAGNET             = 177746,
    NPC_MAGRAMI_SPECTRE         = 11560,
    SPELL_SPIRIT_SPAWN_OUT      = 17680,

    // Quest 8868
    NPC_MINION_OMEN             = 15466,
    NPC_OMEN                    = 15467,
    GO_ROCKET_CLUSTER           = 180875,
    SPELL_OMEN_MOONLIGHT        = 26392,
    MAX_ROCKETS                 = 30,

    // Quest 1126
    NPC_HIVE_ASHI_DRONES        = 13136,

    // Quest 6132
    NPC_HORNIZZ                 = 6019,
};

enum Encounters
{
    TYPE_OMEN               = 0, // Quest 8868
    TYPE_HIVE               = 1, // Quest 1126
    // TYPE_TETHYR          = 2, // Quest 11198 - tbc+
    TYPE_FREEDOM_CREATURES  = 3, // Quest 2969
    MAX_ENCOUNTER
};

enum SpawnIndexes
{
    POS_IDX_MURKDEEP_SPAWN      = 0,
    POS_IDX_MURKDEEP_MOVE       = 1,
    POS_IDX_OMEN_SPAWN          = 2,
    POS_IDX_OMEN_MOVE           = 3,
    POS_IDX_MINION_OMEN_START   = 4,
    POS_IDX_MINION_OMEN_STOP    = 10,
    POS_IDX_HIVE_DRONES_START   = 11,
    POS_IDX_HIVE_DRONES_STOP    = 13,
    POS_IDX_MAX
};

static const float aSpawnLocations[POS_IDX_MAX][4] =
{
    {4981.031f,    597.955f,   -1.361f, 4.82f},         // Murkdeep spawn, guesswork
    {4988.970f,    547.002f,    5.379f,  0.0f},         // Murkdeep move, guesswork
    {7564.098f,  -2835.687f,  447.374f, 3.98f},         // Omen spawn, guesswork
    {7528.430f,  -2889.838f,  459.365f,  0.0f},         // Omen move, guesswork
    {7669.430f,  -2869.980f,  464.904f, 2.60f},         // Minions of Omen
    {7621.790f,  -2861.740f,  459.043f, 4.15f},
    {7597.130f,  -2885.880f,  464.898f, 2.88f},
    {7594.170f,  -2856.490f,  459.403f, 3.17f},
    {7477.750f,  -2849.020f,  464.466f, 0.14f},
    {7507.560f,  -2818.230f,  459.486f, 4.63f},
    {7558.880f,  -2856.770f,  457.684f, 4.31f},
    {-7185.94f,     443.29f,  26.59f, 4.8458f},         // Hive'Ashi Drones spawn 1, guesswork
    {-7180.59f,     441.33f,  26.68f, 4.1798f},         // Hive'Ashi Drones spawn 2, guesswork
    {-7176.63f,     437.42f,  26.84f, 3.9348f},         // Hive'Ashi Drones spawn 3, guesswork
};

#endif