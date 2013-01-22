ALTER TABLE db_version CHANGE COLUMN required_z1108_s0481_03_mangos_spell_bonus_data required_z1135_s0540_01_mangos_spell_learn_spell bit;

/* All form passives */
DELETE FROM spell_learn_spell WHERE SpellID in (
1178,  /* Bear Form (Passive) */
3025,  /* Cat Form (Passive) */
3122,  /* Tree of Life _passive_ */
5419,  /* Travel Form (Passive) */
5420,  /* Tree of Life _passive_ */
5421,  /* Aquatic Form (Passive) */
7376,  /* Defensive Stance Passive */
7381,  /* Berserker Stance Passive */
9635,  /* Dire Bear Form (Passive) */
21156, /* Battle Stance Passive */
21178, /* Bear Form (Passive2) */
24905  /* Moonkin Form (Passive) */
);
