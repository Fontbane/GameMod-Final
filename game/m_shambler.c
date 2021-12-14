/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
==============================================================================

SHAMBLER

==============================================================================
*/

#include "g_local.h"
#include "m_shambler.h"

int sound_cock;


// STAND

void shambler_stand (edict_t *self);

mframe_t shambler_frames_stand1 [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t shambler_move_stand1 = {FRAME_stand101, FRAME_stand116, shambler_frames_stand1, shambler_stand};


void shambler_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &shambler_move_stand1;
}


//
// WALK
//

void shambler_walk1_random (edict_t *self)
{
	if (random() > 0.1)
		self->monsterinfo.nextframe = FRAME_walk101;
}

mframe_t shambler_frames_walk1 [] =
{
	ai_walk, 3,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 2,  NULL,
	ai_walk, 1,  NULL,
	ai_walk, 6,  NULL,
	ai_walk, 5,  NULL,
	ai_walk, 3,  NULL,
	ai_walk, -1, shambler_walk1_random,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL
};
mmove_t shambler_move_walk1 = {FRAME_walk101, FRAME_walk113, shambler_frames_walk1, NULL};

void shambler_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &shambler_move_walk1;
}


//
// RUN
//

void shambler_run (edict_t *self);

mframe_t shambler_frames_start_run [] =
{
	ai_run, 7,  NULL,
	ai_run, 5,  NULL
};
mmove_t shambler_move_start_run = {FRAME_run01, FRAME_run02, shambler_frames_start_run, shambler_run};

mframe_t shambler_frames_run [] =
{
	ai_run, 10, NULL,
	ai_run, 11, NULL,
	ai_run, 11, NULL,
	ai_run, 16, NULL
};
mmove_t shambler_move_run = {FRAME_run03, FRAME_run06, shambler_frames_run, NULL};

void shambler_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = &shambler_move_stand1;
		return;
	}

	if (self->monsterinfo.currentmove == &shambler_move_walk1 ||
		self->monsterinfo.currentmove == &shambler_move_start_run)
	{
		self->monsterinfo.currentmove = &shambler_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = &shambler_move_start_run;
	}
}


//
// PAIN
//

mframe_t shambler_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 1,  NULL,
};
mmove_t shambler_move_pain1 = {FRAME_pain101, FRAME_pain104, shambler_frames_pain1, shambler_run};

mframe_t shambler_frames_pain2 [] =
{
	ai_move, -13, NULL,
	ai_move, -1,  NULL,
	ai_move, 2,   NULL,
	ai_move, 4,   NULL,
	ai_move, 2,   NULL,
	ai_move, 3,   NULL,
};
mmove_t shambler_move_pain2 = {FRAME_pain201, FRAME_pain206, shambler_frames_pain2, shambler_run};

void shambler_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;
	int		n;

	if (self->health < (self->max_health / 2))
			self->s.skinnum |= 1;

	self->pain_debounce_time = level.time + 3;

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	r = random();

	if (r < 0.5)
		self->monsterinfo.currentmove = &shambler_move_pain1;
	else
		self->monsterinfo.currentmove = &shambler_move_pain2;
}


//
// ATTACK
//

void shambler_zap(edict_t* self)
{
	vec3_t	forward, right;
	vec3_t	origin;
	vec3_t	dir;
	vec3_t	offset;

	VectorSubtract(self->enemy->s.origin, self->s.origin, dir);

	AngleVectors(self->s.angles, forward, right, NULL);
	//FIXME use a flash and replace these two lines with the commented one
	VectorSet(offset, 18.5, -0.9, 10);
	G_ProjectSource(self->s.origin, offset, forward, right, origin);
	//	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, origin);

	gi.sound(self, CHAN_WEAPON, sound_cock, 1, ATTN_NORM, 0);

	//FIXME use the flash, Luke
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPLASH);
	gi.WriteByte(32);
	gi.WritePosition(origin);
	gi.WriteDir(dir);
	gi.WriteByte(1);	//sparks
	gi.multicast(origin, MULTICAST_PVS);

	T_Damage(self->enemy, self, self, dir, self->enemy->s.origin, vec3_origin, 5 + rand() % 6, -10, DAMAGE_ENERGY+DAMAGE_LIGHTNING, MOD_UNKNOWN);
}

// ATTACK1 (blaster/shotgun)

void shambler_fire1 (edict_t *self)
{
	shambler_fire (self, 0);
}

void shambler_attack1_refire1 (edict_t *self)
{
	if (self->s.skinnum > 1)
		return;

	if (self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attak102;
	else
		self->monsterinfo.nextframe = FRAME_attak110;
}

void shambler_attack1_refire2 (edict_t *self)
{
	if (self->s.skinnum < 2)
		return;

	if (self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attak102;
}

mframe_t shambler_frames_zap [] =
{
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  shambler_zap,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,  NULL
};
mmove_t shambler_move_zap = {FRAME_attak101, FRAME_attak112, shambler_frames_zap, shambler_run};

// ATTACK2 (blaster/shotgun)

void shambler_fire2 (edict_t *self)
{
	shambler_fire (self, 1);
}
void shambler_attackleft(edict_t* self)
{
	static	vec3_t	aim = { MELEE_DISTANCE, 0, -24 };
	fire_hit(self, aim, (15 + (rand() % 6)), 400);		//	Faster attack -- upwards and backwards
}
void shambler_attackright(edict_t* self)
{
	static	vec3_t	aim = { MELEE_DISTANCE, 0, -24 };
	fire_hit(self, aim, (15 + (rand() % 6)), 400);		//	Faster attack -- upwards and backwards
}

mframe_t shambler_frames_attack_swing[] =
{
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, shambler_swing,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
};
mmove_t shambler_move_attack_swing = { FRAME_attak101, FRAME_attak112, shambler_frames_attackleft, shambler_run };


void shambler_swing(edict_t* self)
{
	vec3_t	aim;

	VectorSet(aim, MELEE_DISTANCE, self->mins[0], -4);
	fire_hit(self, aim, (5 + (rand() % 6)), 400);		// Slower attack
}

mframe_t shambler_frames_attackleft[] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, shambler_attackright,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, shambler_attackleft,
};

mmove_t shambler_move_attackleft = { FRAME_attak201, FRAME_attak208, shambler_frames_attackleft, shambler_run };

mframe_t shambler_frames_attackright[] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, shambler_attackleft,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, shambler_attackright,
	ai_move, 0, NULL
};

mmove_t shambler_move_attackright = { FRAME_attak301, FRAME_attak309, shambler_frames_attackright, shambler_run };


void berserk_melee(edict_t* self)
{
	if ((rand() % 2) == 0)
		self->monsterinfo.currentmove = &shambler_swing;
	else
		self->monsterinfo.currentmove = &shambler_move_attackleft;
}
#if 0
// ATTACK5 (prone)

void shambler_fire5 (edict_t *self)
{
	shambler_fire (self, 4);
}

void shambler_attack5_refire (edict_t *self)
{
	if (self->enemy->health <= 0)
		return;

	if ( ((skill->value == 3) && (random() < 0.5)) || (range(self, self->enemy) == RANGE_MELEE) )
		self->monsterinfo.nextframe = FRAME_attak505;
}

mframe_t shambler_frames_attack5 [] =
{
	ai_charge, 8, NULL,
	ai_charge, 8, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, shambler_fire5,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, shambler_attack5_refire
};
mmove_t shambler_move_attack5 = {FRAME_attak501, FRAME_attak508, shambler_frames_attack5, shambler_run};
#endif

void shambler_attack(edict_t *self)
{
	
	self->monsterinfo.currentmove = &shambler_move_zap;
}


//
// SIGHT
//

void shambler_sight(edict_t *self, edict_t *other)
{
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_sight2, 1, ATTN_NORM, 0);

	if ((skill->value > 0) && (range(self, self->enemy) >= RANGE_MID))
	{
		if (random() > 0.5)
			self->monsterinfo.currentmove = &shambler_move_attack6;
	}
}

//
// DUCK
//

void shambler_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

mframe_t shambler_frames_duck [] =
{
	ai_move, 5, shambler_duck_down,
	ai_move, -1, shambler_duck_hold,
	ai_move, 1,  NULL,
	ai_move, 0,  shambler_duck_up,
	ai_move, 5,  NULL
};
mmove_t shambler_move_duck = {FRAME_duck01, FRAME_duck05, shambler_frames_duck, shambler_run};

void shambler_dodge (edict_t *self, edict_t *attacker, float eta)
{
	float	r;

	r = random();
	if (r > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	if (skill->value == 0)
	{
		self->monsterinfo.currentmove = &shambler_move_duck;
		return;
	}

	self->monsterinfo.pausetime = level.time + eta + 0.3;
	r = random();

	if (skill->value == 1)
	{
		if (r > 0.33)
			self->monsterinfo.currentmove = &shambler_move_duck;
		else
			self->monsterinfo.currentmove = &shambler_move_attack3;
		return;
	}

	if (skill->value >= 2)
	{
		if (r > 0.66)
			self->monsterinfo.currentmove = &shambler_move_duck;
		else
			self->monsterinfo.currentmove = &shambler_move_attack3;
		return;
	}

	self->monsterinfo.currentmove = &shambler_move_attack3;
}


//
// DEATH
//

void shambler_fire6 (edict_t *self)
{
	shambler_fire (self, 5);
}

void shambler_fire7 (edict_t *self)
{
	shambler_fire (self, 6);
}

void shambler_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t shambler_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -10, NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   shambler_fire6,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   shambler_fire7,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t shambler_move_death1 = {FRAME_death101, FRAME_death136, shambler_frames_death1, shambler_dead};

mframe_t shambler_frames_death2 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t shambler_move_death2 = {FRAME_death201, FRAME_death235, shambler_frames_death2, shambler_dead};

mframe_t shambler_frames_death3 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
};
mmove_t shambler_move_death3 = {FRAME_death301, FRAME_death345, shambler_frames_death3, shambler_dead};

mframe_t shambler_frames_death4 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t shambler_move_death4 = {FRAME_death401, FRAME_death453, shambler_frames_death4, shambler_dead};

mframe_t shambler_frames_death5 [] =
{
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, -5,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,

	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t shambler_move_death5 = {FRAME_death501, FRAME_death524, shambler_frames_death5, shambler_dead};

mframe_t shambler_frames_death6 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL
};
mmove_t shambler_move_death6 = {FRAME_death601, FRAME_death610, shambler_frames_death6, shambler_dead};

void shambler_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 3; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->s.skinnum |= 1;

	if (self->s.skinnum == 1)
		gi.sound (self, CHAN_VOICE, sound_death_light, 1, ATTN_NORM, 0);
	else if (self->s.skinnum == 3)
		gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	else // (self->s.skinnum == 5)
		gi.sound (self, CHAN_VOICE, sound_death_ss, 1, ATTN_NORM, 0);

	if (fabs((self->s.origin[2] + self->viewheight) - point[2]) <= 4)
	{
		// head shot
		self->monsterinfo.currentmove = &shambler_move_death3;
		return;
	}

	n = rand() % 5;
	if (n == 0)
		self->monsterinfo.currentmove = &shambler_move_death1;
	else if (n == 1)
		self->monsterinfo.currentmove = &shambler_move_death2;
	else if (n == 2)
		self->monsterinfo.currentmove = &shambler_move_death4;
	else if (n == 3)
		self->monsterinfo.currentmove = &shambler_move_death5;
	else
		self->monsterinfo.currentmove = &shambler_move_death6;
}


//
// SPAWN
//

void SP_monster_shambler_x (edict_t *self)
{

	self->s.modelindex = gi.modelindex ("models/monsters/shambler/tris.md2");
	self->monsterinfo.scale = MODEL_SCALE;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	sound_idle =	gi.soundindex ("shambler/solidle1.wav");
	sound_sight1 =	gi.soundindex ("shambler/solsght1.wav");
	sound_sight2 =	gi.soundindex ("shambler/solsrch1.wav");
	sound_cock =	gi.soundindex ("infantry/infatck3.wav");

	self->mass = 100;

	self->pain = shambler_pain;
	self->die = shambler_die;
	self->element = ELEMENT_GRASS;

	self->monsterinfo.stand = shambler_stand;
	self->monsterinfo.walk = shambler_walk;
	self->monsterinfo.run = shambler_run;
	self->monsterinfo.dodge = shambler_dodge;
	self->monsterinfo.attack = shambler_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = shambler_sight;

	gi.linkentity (self);

	self->monsterinfo.stand (self);

	walkmonster_start (self);
}


/*QUAKED monster_shambler_light (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_shambler_light (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	SP_monster_shambler_x (self);

	sound_pain_light = gi.soundindex ("shambler/solpain2.wav");
	sound_death_light =	gi.soundindex ("shambler/soldeth2.wav");
	gi.modelindex ("models/objects/laser/tris.md2");
	gi.soundindex ("misc/lasfly.wav");
	gi.soundindex ("shambler/solatck2.wav");

	self->s.skinnum = 0;
	self->health = 20;
	self->gib_health = -30;
}

/*QUAKED monster_shambler (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_shambler (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	SP_monster_shambler_x (self);

	sound_pain = gi.soundindex ("shambler/solpain1.wav");
	sound_death = gi.soundindex ("shambler/soldeth1.wav");
	gi.soundindex ("shambler/solatck1.wav");

	self->s.skinnum = 2;
	self->health = 30;
	self->gib_health = -30;
}

/*QUAKED monster_shambler_ss (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_shambler_ss (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	SP_monster_shambler_x (self);

	sound_pain_ss = gi.soundindex ("shambler/solpain3.wav");
	sound_death_ss = gi.soundindex ("shambler/soldeth3.wav");
	gi.soundindex ("shambler/solatck3.wav");

	self->s.skinnum = 4;
	self->health = 40;
	self->gib_health = -30;
}
