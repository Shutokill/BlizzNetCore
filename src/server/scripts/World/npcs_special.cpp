/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Npcs_Special
SD%Complete: 100
SDComment: To be used for special NPCs that are located globally.
SDCategory: NPCs
EndScriptData
*/

/* ContentData
npc_air_force_bots       80%    support for misc (invisible) guard bots in areas where player allowed to fly. Summon guards after a preset time if tagged by spell
npc_lunaclaw_spirit      80%    support for quests 6001/6002 (Body and Heart)
npc_chicken_cluck       100%    support for quest 3861 (Cluck!)
npc_dancing_flames      100%    midsummer event NPC
npc_guardian            100%    guardianAI used to prevent players from accessing off-limits areas. Not in use by SD2
npc_garments_of_quests   80%    NPC's related to all Garments of-quests 5621, 5624, 5625, 5648, 565, FIXED SAY FOR ALL GARMENTS QUESTS
npc_injured_patient     100%    patients for triage-quests (6622 and 6624)
npc_doctor              100%    Gustaf Vanhowzen and Gregory Victor, quest 6622 and 6624 (Triage)
npc_mount_vendor        100%    Regular mount vendors all over the world. Display gossip if player doesn't meet the requirements to buy
npc_rogue_trainer        80%    Scripted trainers, so they are able to offer item 17126 for class quest 6681
npc_sayge               100%    Darkmoon event fortune teller, buff player based on answers given
npc_snake_trap_serpents  80%    AI for snakes that summoned by Snake Trap
npc_shadowfiend         100%   restore 5% of owner's mana when shadowfiend die from damage
npc_locksmith            75%    list of keys needs to be confirmed
npc_firework            100%    NPC's summoned by rockets and rocket clusters, for making them cast visual
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "World.h"

/*########
# npc_air_force_bots
#########*/

enum SpawnType
{
    SPAWNTYPE_TRIPWIRE_ROOFTOP,                             // no warning, summon Creature at smaller range
    SPAWNTYPE_ALARMBOT,                                     // cast guards mark and summon npc - if player shows up with that buff duration < 5 seconds attack
};

struct SpawnAssociation
{
    uint32 thisCreatureEntry;
    uint32 spawnedCreatureEntry;
    SpawnType spawnType;
};

enum eEnums
{
    SPELL_GUARDS_MARK               = 38067,
    AURA_DURATION_TIME_LEFT         = 5000
};

float const RANGE_TRIPWIRE          = 15.0f;
float const RANGE_GUARDS_MARK       = 50.0f;

SpawnAssociation spawnAssociations[] =
{
    {2614,  15241, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Alliance)
    {2615,  15242, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Horde)
    {21974, 21976, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Area 52)
    {21993, 15242, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Horde - Bat Rider)
    {21996, 15241, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Alliance - Gryphon)
    {21997, 21976, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Goblin - Area 52 - Zeppelin)
    {21999, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Alliance)
    {22001, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Horde)
    {22002, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Ground (Horde)
    {22003, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Ground (Alliance)
    {22063, 21976, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Goblin - Area 52)
    {22065, 22064, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Ethereal - Stormspire)
    {22066, 22067, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Scryer - Dragonhawk)
    {22068, 22064, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Ethereal - Stormspire)
    {22069, 22064, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Stormspire)
    {22070, 22067, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Scryer)
    {22071, 22067, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Scryer)
    {22078, 22077, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Aldor)
    {22079, 22077, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Aldor - Gryphon)
    {22080, 22077, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Aldor)
    {22086, 22085, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Sporeggar)
    {22087, 22085, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Sporeggar - Spore Bat)
    {22088, 22085, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Sporeggar)
    {22090, 22089, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Toshley's Station - Flying Machine)
    {22124, 22122, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Cenarion)
    {22125, 22122, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Cenarion - Stormcrow)
    {22126, 22122, SPAWNTYPE_ALARMBOT}                      //Air Force Trip Wire - Rooftop (Cenarion Expedition)
};

class npc_air_force_bots : public CreatureScript
{
public:
    npc_air_force_bots() : CreatureScript("npc_air_force_bots") { }

    struct npc_air_force_botsAI : public ScriptedAI
    {
        npc_air_force_botsAI(Creature* creature) : ScriptedAI(creature)
        {
            SpawnAssoc = NULL;
            SpawnedGUID = 0;

            // find the correct spawnhandling
            static uint32 entryCount = sizeof(spawnAssociations) / sizeof(SpawnAssociation);

            for (uint8 i = 0; i < entryCount; ++i)
            {
                if (spawnAssociations[i].thisCreatureEntry == creature->GetEntry())
                {
                    SpawnAssoc = &spawnAssociations[i];
                    break;
                }
            }

            if (!SpawnAssoc)
                sLog->outErrorDb("TCSR: Creature template entry %u has ScriptName npc_air_force_bots, but it's not handled by that script", creature->GetEntry());
            else
            {
                CreatureTemplate const* spawnedTemplate = sObjectMgr->GetCreatureTemplate(SpawnAssoc->spawnedCreatureEntry);

                if (!spawnedTemplate)
                {
                    sLog->outErrorDb("TCSR: Creature template entry %u does not exist in DB, which is required by npc_air_force_bots", SpawnAssoc->spawnedCreatureEntry);
                    SpawnAssoc = NULL;
                    return;
                }
            }
        }

        SpawnAssociation* SpawnAssoc;
        uint64 SpawnedGUID;

        void Reset() {}

        Creature* SummonGuard()
        {
            Creature* summoned = me->SummonCreature(SpawnAssoc->spawnedCreatureEntry, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 300000);

            if (summoned)
                SpawnedGUID = summoned->GetGUID();
            else
            {
                sLog->outErrorDb("TCSR: npc_air_force_bots: wasn't able to spawn Creature %u", SpawnAssoc->spawnedCreatureEntry);
                SpawnAssoc = NULL;
            }

            return summoned;
        }

        Creature* GetSummonedGuard()
        {
            Creature* creature = Unit::GetCreature(*me, SpawnedGUID);

            if (creature && creature->isAlive())
                return creature;

            return NULL;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!SpawnAssoc)
                return;

            if (me->IsValidAttackTarget(who))
            {
                Player* playerTarget = who->ToPlayer();

                // airforce guards only spawn for players
                if (!playerTarget)
                    return;

                Creature* lastSpawnedGuard = SpawnedGUID == 0 ? NULL : GetSummonedGuard();

                // prevent calling Unit::GetUnit at next MoveInLineOfSight call - speedup
                if (!lastSpawnedGuard)
                    SpawnedGUID = 0;

                switch (SpawnAssoc->spawnType)
                {
                    case SPAWNTYPE_ALARMBOT:
                    {
                        if (!who->IsWithinDistInMap(me, RANGE_GUARDS_MARK))
                            return;

                        Aura* markAura = who->GetAura(SPELL_GUARDS_MARK);
                        if (markAura)
                        {
                            // the target wasn't able to move out of our range within 25 seconds
                            if (!lastSpawnedGuard)
                            {
                                lastSpawnedGuard = SummonGuard();

                                if (!lastSpawnedGuard)
                                    return;
                            }

                            if (markAura->GetDuration() < AURA_DURATION_TIME_LEFT)
                                if (!lastSpawnedGuard->getVictim())
                                    lastSpawnedGuard->AI()->AttackStart(who);
                        }
                        else
                        {
                            if (!lastSpawnedGuard)
                                lastSpawnedGuard = SummonGuard();

                            if (!lastSpawnedGuard)
                                return;

                            lastSpawnedGuard->CastSpell(who, SPELL_GUARDS_MARK, true);
                        }
                        break;
                    }
                    case SPAWNTYPE_TRIPWIRE_ROOFTOP:
                    {
                        if (!who->IsWithinDistInMap(me, RANGE_TRIPWIRE))
                            return;

                        if (!lastSpawnedGuard)
                            lastSpawnedGuard = SummonGuard();

                        if (!lastSpawnedGuard)
                            return;

                        // ROOFTOP only triggers if the player is on the ground
                        if (!playerTarget->IsFlying() && !lastSpawnedGuard->getVictim())
                            lastSpawnedGuard->AI()->AttackStart(who);

                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_air_force_botsAI(creature);
    }
};

/*######
## npc_lunaclaw_spirit
######*/

enum
{
    QUEST_BODY_HEART_A      = 6001,
    QUEST_BODY_HEART_H      = 6002,

    TEXT_ID_DEFAULT         = 4714,
    TEXT_ID_PROGRESS        = 4715
};

#define GOSSIP_ITEM_GRANT   "You have thought well, spirit. I ask you to grant me the strength of your body and the strength of your heart."

class npc_lunaclaw_spirit : public CreatureScript
{
public:
    npc_lunaclaw_spirit() : CreatureScript("npc_lunaclaw_spirit") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_BODY_HEART_A) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(QUEST_BODY_HEART_H) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_GRANT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(TEXT_ID_DEFAULT, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->SEND_GOSSIP_MENU(TEXT_ID_PROGRESS, creature->GetGUID());
            player->AreaExploredOrEventHappens(player->GetTeam() == ALLIANCE ? QUEST_BODY_HEART_A : QUEST_BODY_HEART_H);
        }
        return true;
    }
};

/*########
# npc_chicken_cluck
#########*/

#define EMOTE_HELLO         -1070004
#define EMOTE_CLUCK_TEXT    -1070006

#define QUEST_CLUCK         3861
#define FACTION_FRIENDLY    35
#define FACTION_CHICKEN     31

class npc_chicken_cluck : public CreatureScript
{
public:
    npc_chicken_cluck() : CreatureScript("npc_chicken_cluck") { }

    struct npc_chicken_cluckAI : public ScriptedAI
    {
        npc_chicken_cluckAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 ResetFlagTimer;

        void Reset()
        {
            ResetFlagTimer = 120000;
            me->setFaction(FACTION_CHICKEN);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        }

        void EnterCombat(Unit* /*who*/) {}

        void UpdateAI(uint32 const diff)
        {
            // Reset flags after a certain time has passed so that the next player has to start the 'event' again
            if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
            {
                if (ResetFlagTimer <= diff)
                {
                    EnterEvadeMode();
                    return;
                }
                else
                    ResetFlagTimer -= diff;
            }

            if (UpdateVictim())
                DoMeleeAttackIfReady();
        }

        void ReceiveEmote(Player* player, uint32 emote)
        {
            switch (emote)
            {
                case TEXT_EMOTE_CHICKEN:
                    if (player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_NONE && rand() % 30 == 1)
                    {
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        me->setFaction(FACTION_FRIENDLY);
                        DoScriptText(EMOTE_HELLO, me);
                    }
                    break;
                case TEXT_EMOTE_CHEER:
                    if (player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_COMPLETE)
                    {
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        me->setFaction(FACTION_FRIENDLY);
                        DoScriptText(EMOTE_CLUCK_TEXT, me);
                    }
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_chicken_cluckAI(creature);
    }

    bool OnQuestAccept(Player* /*player*/, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_CLUCK)
            CAST_AI(npc_chicken_cluck::npc_chicken_cluckAI, creature->AI())->Reset();

        return true;
    }

    bool OnQuestComplete(Player* /*player*/, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_CLUCK)
            CAST_AI(npc_chicken_cluck::npc_chicken_cluckAI, creature->AI())->Reset();

        return true;
    }
};

/*######
## npc_dancing_flames
######*/

#define SPELL_BRAZIER       45423
#define SPELL_SEDUCTION     47057
#define SPELL_FIERY_AURA    45427

class npc_dancing_flames : public CreatureScript
{
public:
    npc_dancing_flames() : CreatureScript("npc_dancing_flames") { }

    struct npc_dancing_flamesAI : public ScriptedAI
    {
        npc_dancing_flamesAI(Creature* creature) : ScriptedAI(creature) {}

        bool Active;
        uint32 CanIteract;

        void Reset()
        {
            Active = true;
            CanIteract = 3500;
            DoCast(me, SPELL_BRAZIER, true);
            DoCast(me, SPELL_FIERY_AURA, false);
            float x, y, z;
            me->GetPosition(x, y, z);
            me->Relocate(x, y, z + 0.94f);
            me->SetDisableGravity(true);
            me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
            WorldPacket data;                       //send update position to client
            me->BuildHeartBeatMsg(&data);
            me->SendMessageToSet(&data, true);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!Active)
            {
                if (CanIteract <= diff)
                {
                    Active = true;
                    CanIteract = 3500;
                    me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
                }
                else
                    CanIteract -= diff;
            }
        }

        void EnterCombat(Unit* /*who*/){}

        void ReceiveEmote(Player* player, uint32 emote)
        {
            if (me->IsWithinLOS(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ()) && me->IsWithinDistInMap(player, 30.0f))
            {
                me->SetInFront(player);
                Active = false;

                WorldPacket data;
                me->BuildHeartBeatMsg(&data);
                me->SendMessageToSet(&data, true);
                switch (emote)
                {
                    case TEXT_EMOTE_KISS:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SHY);
                        break;
                    case TEXT_EMOTE_WAVE:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                        break;
                    case TEXT_EMOTE_BOW:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                        break;
                    case TEXT_EMOTE_JOKE:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                        break;
                    case TEXT_EMOTE_DANCE:
                        if (!player->HasAura(SPELL_SEDUCTION))
                            DoCast(player, SPELL_SEDUCTION, true);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dancing_flamesAI(creature);
    }
};

/*######
## Triage quest
######*/

//signed for 9623
#define SAY_DOC1    -1000201
#define SAY_DOC2    -1000202
#define SAY_DOC3    -1000203

#define DOCTOR_ALLIANCE     12939
#define DOCTOR_HORDE        12920
#define ALLIANCE_COORDS     7
#define HORDE_COORDS        6

struct Location
{
    float x, y, z, o;
};

static Location AllianceCoords[]=
{
    {-3757.38f, -4533.05f, 14.16f, 3.62f},                      // Top-far-right bunk as seen from entrance
    {-3754.36f, -4539.13f, 14.16f, 5.13f},                      // Top-far-left bunk
    {-3749.54f, -4540.25f, 14.28f, 3.34f},                      // Far-right bunk
    {-3742.10f, -4536.85f, 14.28f, 3.64f},                      // Right bunk near entrance
    {-3755.89f, -4529.07f, 14.05f, 0.57f},                      // Far-left bunk
    {-3749.51f, -4527.08f, 14.07f, 5.26f},                      // Mid-left bunk
    {-3746.37f, -4525.35f, 14.16f, 5.22f},                      // Left bunk near entrance
};

//alliance run to where
#define A_RUNTOX -3742.96f
#define A_RUNTOY -4531.52f
#define A_RUNTOZ 11.91f

static Location HordeCoords[]=
{
    {-1013.75f, -3492.59f, 62.62f, 4.34f},                      // Left, Behind
    {-1017.72f, -3490.92f, 62.62f, 4.34f},                      // Right, Behind
    {-1015.77f, -3497.15f, 62.82f, 4.34f},                      // Left, Mid
    {-1019.51f, -3495.49f, 62.82f, 4.34f},                      // Right, Mid
    {-1017.25f, -3500.85f, 62.98f, 4.34f},                      // Left, front
    {-1020.95f, -3499.21f, 62.98f, 4.34f}                       // Right, Front
};

//horde run to where
#define H_RUNTOX -1016.44f
#define H_RUNTOY -3508.48f
#define H_RUNTOZ 62.96f

uint32 const AllianceSoldierId[3] =
{
    12938,                                                  // 12938 Injured Alliance Soldier
    12936,                                                  // 12936 Badly injured Alliance Soldier
    12937                                                   // 12937 Critically injured Alliance Soldier
};

uint32 const HordeSoldierId[3] =
{
    12923,                                                  //12923 Injured Soldier
    12924,                                                  //12924 Badly injured Soldier
    12925                                                   //12925 Critically injured Soldier
};

/*######
## npc_doctor (handles both Gustaf Vanhowzen and Gregory Victor)
######*/
class npc_doctor : public CreatureScript
{
public:
    npc_doctor() : CreatureScript("npc_doctor") {}

    struct npc_doctorAI : public ScriptedAI
    {
        npc_doctorAI(Creature* creature) : ScriptedAI(creature) {}

        uint64 PlayerGUID;

        uint32 SummonPatientTimer;
        uint32 SummonPatientCount;
        uint32 PatientDiedCount;
        uint32 PatientSavedCount;

        bool Event;

        std::list<uint64> Patients;
        std::vector<Location*> Coordinates;

        void Reset()
        {
            PlayerGUID = 0;

            SummonPatientTimer = 10000;
            SummonPatientCount = 0;
            PatientDiedCount = 0;
            PatientSavedCount = 0;

            Patients.clear();
            Coordinates.clear();

            Event = false;

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void BeginEvent(Player* player)
        {
            PlayerGUID = player->GetGUID();

            SummonPatientTimer = 10000;
            SummonPatientCount = 0;
            PatientDiedCount = 0;
            PatientSavedCount = 0;

            switch (me->GetEntry())
            {
                case DOCTOR_ALLIANCE:
                    for (uint8 i = 0; i < ALLIANCE_COORDS; ++i)
                        Coordinates.push_back(&AllianceCoords[i]);
                    break;
                case DOCTOR_HORDE:
                    for (uint8 i = 0; i < HORDE_COORDS; ++i)
                        Coordinates.push_back(&HordeCoords[i]);
                    break;
            }

            Event = true;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void PatientDied(Location* point)
        {
            Player* player = Unit::GetPlayer(*me, PlayerGUID);
            if (player && ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)))
            {
                ++PatientDiedCount;

                if (PatientDiedCount > 5 && Event)
                {
                    if (player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                        player->FailQuest(6624);
                    else if (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                        player->FailQuest(6622);

                    Reset();
                    return;
                }

                Coordinates.push_back(point);
            }
            else
                // If no player or player abandon quest in progress
                Reset();
        }

        void PatientSaved(Creature* /*soldier*/, Player* player, Location* point)
        {
            if (player && PlayerGUID == player->GetGUID())
            {
                if ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                {
                    ++PatientSavedCount;

                    if (PatientSavedCount == 15)
                    {
                        if (!Patients.empty())
                        {
                            std::list<uint64>::const_iterator itr;
                            for (itr = Patients.begin(); itr != Patients.end(); ++itr)
                            {
                                if (Creature* patient = Unit::GetCreature((*me), *itr))
                                    patient->setDeathState(JUST_DIED);
                            }
                        }

                        if (player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                            player->AreaExploredOrEventHappens(6624);
                        else if (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                            player->AreaExploredOrEventHappens(6622);

                        Reset();
                        return;
                    }

                    Coordinates.push_back(point);
                }
            }
        }

        void UpdateAI(uint32 const diff);

        void EnterCombat(Unit* /*who*/){}
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if ((quest->GetQuestId() == 6624) || (quest->GetQuestId() == 6622))
            CAST_AI(npc_doctor::npc_doctorAI, creature->AI())->BeginEvent(player);

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_doctorAI(creature);
    }
};

/*#####
## npc_injured_patient (handles all the patients, no matter Horde or Alliance)
#####*/

class npc_injured_patient : public CreatureScript
{
public:
    npc_injured_patient() : CreatureScript("npc_injured_patient") { }

    struct npc_injured_patientAI : public ScriptedAI
    {
        npc_injured_patientAI(Creature* creature) : ScriptedAI(creature) {}

        uint64 DoctorGUID;
        Location* Coord;

        void Reset()
        {
            DoctorGUID = 0;
            Coord = NULL;

            //no select
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            //no regen health
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

            //to make them lay with face down
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_DEAD);

            uint32 mobId = me->GetEntry();

            switch (mobId)
            {                                                   //lower max health
                case 12923:
                case 12938:                                     //Injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(75));
                    break;
                case 12924:
                case 12936:                                     //Badly injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(50));
                    break;
                case 12925:
                case 12937:                                     //Critically injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(25));
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/){}

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (caster->GetTypeId() == TYPEID_PLAYER && me->isAlive() && spell->Id == 20804)
            {
                if ((CAST_PLR(caster)->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (CAST_PLR(caster)->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                    if (DoctorGUID)
                        if (Creature* doctor = Unit::GetCreature(*me, DoctorGUID))
                            CAST_AI(npc_doctor::npc_doctorAI, doctor->AI())->PatientSaved(me, CAST_PLR(caster), Coord);

                //make not selectable
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                //regen health
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

                //stand up
                me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);

                DoScriptText(RAND(SAY_DOC1, SAY_DOC2, SAY_DOC3), me);

                uint32 mobId = me->GetEntry();
                me->SetWalk(false);

                switch (mobId)
                {
                    case 12923:
                    case 12924:
                    case 12925:
                        me->GetMotionMaster()->MovePoint(0, H_RUNTOX, H_RUNTOY, H_RUNTOZ);
                        break;
                    case 12936:
                    case 12937:
                    case 12938:
                        me->GetMotionMaster()->MovePoint(0, A_RUNTOX, A_RUNTOY, A_RUNTOZ);
                        break;
                }
            }
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            //lower HP on every world tick makes it a useful counter, not officlone though
            if (me->isAlive() && me->GetHealth() > 6)
                me->ModifyHealth(-5);

            if (me->isAlive() && me->GetHealth() <= 6)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->setDeathState(JUST_DIED);
                me->SetFlag(UNIT_DYNAMIC_FLAGS, 32);

                if (DoctorGUID)
                    if (Creature* doctor = Unit::GetCreature((*me), DoctorGUID))
                        CAST_AI(npc_doctor::npc_doctorAI, doctor->AI())->PatientDied(Coord);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_injured_patientAI(creature);
    }
};

void npc_doctor::npc_doctorAI::UpdateAI(uint32 const diff)
{
    if (Event && SummonPatientCount >= 20)
    {
        Reset();
        return;
    }

    if (Event)
    {
        if (SummonPatientTimer <= diff)
        {
            if (Coordinates.empty())
                return;

            std::vector<Location*>::iterator itr = Coordinates.begin() + rand() % Coordinates.size();
            uint32 patientEntry = 0;

            switch (me->GetEntry())
            {
                case DOCTOR_ALLIANCE:
                    patientEntry = AllianceSoldierId[rand() % 3];
                    break;
                case DOCTOR_HORDE:
                    patientEntry = HordeSoldierId[rand() % 3];
                    break;
                default:
                    sLog->outError("TSCR: Invalid entry for Triage doctor. Please check your database");
                    return;
            }

            if (Location* point = *itr)
            {
                if (Creature* Patient = me->SummonCreature(patientEntry, point->x, point->y, point->z, point->o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000))
                {
                    //303, this flag appear to be required for client side item->spell to work (TARGET_SINGLE_FRIEND)
                    Patient->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

                    Patients.push_back(Patient->GetGUID());
                    CAST_AI(npc_injured_patient::npc_injured_patientAI, Patient->AI())->DoctorGUID = me->GetGUID();
                    CAST_AI(npc_injured_patient::npc_injured_patientAI, Patient->AI())->Coord = point;

                    Coordinates.erase(itr);
                }
            }
            SummonPatientTimer = 10000;
            ++SummonPatientCount;
        }
        else
            SummonPatientTimer -= diff;
    }
}

/*######
## npc_garments_of_quests
######*/

//TODO: get text for each NPC

enum eGarments
{
    SPELL_LESSER_HEAL_R2    = 2052,
    SPELL_FORTITUDE_R1      = 1243,

    QUEST_MOON              = 5621,
    QUEST_LIGHT_1           = 5624,
    QUEST_LIGHT_2           = 5625,
    QUEST_SPIRIT            = 5648,
    QUEST_DARKNESS          = 5650,

    ENTRY_SHAYA             = 12429,
    ENTRY_ROBERTS           = 12423,
    ENTRY_DOLF              = 12427,
    ENTRY_KORJA             = 12430,
    ENTRY_DG_KEL            = 12428,

    //used by 12429, 12423, 12427, 12430, 12428, but signed for 12429
    
	SAY_COMMON_HEALED       = -1000231,
    SAY_DG_KEL_THANKS       = -1000232,
    SAY_DG_KEL_GOODBYE      = -1000233,
    SAY_ROBERTS_THANKS      = -1000256,
    SAY_ROBERTS_GOODBYE     = -1000257,
    SAY_KORJA_THANKS        = -1000258,
    SAY_KORJA_GOODBYE       = -1000259,
    SAY_DOLF_THANKS         = -1000260,
    SAY_DOLF_GOODBYE        = -1000261,
    SAY_SHAYA_THANKS        = -1000262,
    SAY_SHAYA_GOODBYE       = -1000263,//signed for 21469
};

class npc_garments_of_quests : public CreatureScript
{
public:
    npc_garments_of_quests() : CreatureScript("npc_garments_of_quests") { }

    struct npc_garments_of_questsAI : public npc_escortAI
    {
        npc_garments_of_questsAI(Creature* creature) : npc_escortAI(creature) {Reset();}

        uint64 CasterGUID;

        bool IsHealed;
        bool CanRun;

        uint32 RunAwayTimer;

        void Reset()
        {
            CasterGUID = 0;

            IsHealed = false;
            CanRun = false;

            RunAwayTimer = 5000;

            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            //expect database to have RegenHealth=0
            me->SetHealth(me->CountPctFromMaxHealth(70));
        }

        void EnterCombat(Unit* /*who*/) {}

        void SpellHit(Unit* caster, SpellInfo const* Spell)
        {
            if (Spell->Id == SPELL_LESSER_HEAL_R2 || Spell->Id == SPELL_FORTITUDE_R1)
            {
                //not while in combat
                if (me->isInCombat())
                    return;

                //nothing to be done now
                if (IsHealed && CanRun)
                    return;

                if (Player* player = caster->ToPlayer())
                {
                    switch (me->GetEntry())
                    {
                        case ENTRY_SHAYA:
                            if (player->GetQuestStatus(QUEST_MOON) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_SHAYA_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_ROBERTS:
                            if (player->GetQuestStatus(QUEST_LIGHT_1) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_ROBERTS_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_DOLF:
                            if (player->GetQuestStatus(QUEST_LIGHT_2) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_DOLF_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_KORJA:
                            if (player->GetQuestStatus(QUEST_SPIRIT) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_KORJA_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_DG_KEL:
                            if (player->GetQuestStatus(QUEST_DARKNESS) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_DG_KEL_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                    }

                    //give quest credit, not expect any special quest objectives
                    if (CanRun)
                        player->TalkedToCreature(me->GetEntry(), me->GetGUID());
                }
            }
        }

        void WaypointReached(uint32 /*point*/)
        {
        }

        void UpdateAI(uint32 const diff)
        {
            if (CanRun && !me->isInCombat())
            {
                if (RunAwayTimer <= diff)
                {
                    if (Unit* unit = Unit::GetUnit(*me, CasterGUID))
                    {
                        switch (me->GetEntry())
                        {
                            case ENTRY_SHAYA:
                                DoScriptText(SAY_SHAYA_GOODBYE, me, unit);
                                break;
                            case ENTRY_ROBERTS:
                                DoScriptText(SAY_ROBERTS_GOODBYE, me, unit);
                                break;
                            case ENTRY_DOLF:
                                DoScriptText(SAY_DOLF_GOODBYE, me, unit);
                                break;
                            case ENTRY_KORJA:
                                DoScriptText(SAY_KORJA_GOODBYE, me, unit);
                                break;
                            case ENTRY_DG_KEL:
                                DoScriptText(SAY_DG_KEL_GOODBYE, me, unit);
                                break;
                        }

                        Start(false, true, true);
                    }
                    else
                        EnterEvadeMode();                       //something went wrong

                    RunAwayTimer = 30000;
                }
                else
                    RunAwayTimer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_garments_of_questsAI(creature);
    }
};

/*######
## npc_guardian
######*/

#define SPELL_DEATHTOUCH                5

class npc_guardian : public CreatureScript
{
public:
    npc_guardian() : CreatureScript("npc_guardian") { }

    struct npc_guardianAI : public ScriptedAI
    {
        npc_guardianAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        void EnterCombat(Unit* /*who*/)
        {
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            if (!UpdateVictim())
                return;

            if (me->isAttackReady())
            {
                DoCast(me->getVictim(), SPELL_DEATHTOUCH, true);
                me->resetAttackTimer();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_guardianAI(creature);
    }
};

/*######
## npc_mount_vendor
######*/

class npc_mount_vendor : public CreatureScript
{
public:
    npc_mount_vendor() : CreatureScript("npc_mount_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        bool canBuy = false;
        uint32 vendor = creature->GetEntry();
        uint8 race = player->getRace();

        switch (vendor)
        {
            case 384:                                           //Katie Hunter
            case 1460:                                          //Unger Statforth
            case 2357:                                          //Merideth Carlson
            case 4885:                                          //Gregor MacVince
                if (player->GetReputationRank(72) != REP_EXALTED && race != RACE_HUMAN)
                    player->SEND_GOSSIP_MENU(5855, creature->GetGUID());
                else canBuy = true;
                break;
            case 1261:                                          //Veron Amberstill
                if (player->GetReputationRank(47) != REP_EXALTED && race != RACE_DWARF)
                    player->SEND_GOSSIP_MENU(5856, creature->GetGUID());
                else canBuy = true;
                break;
            case 3362:                                          //Ogunaro Wolfrunner
                if (player->GetReputationRank(76) != REP_EXALTED && race != RACE_ORC)
                    player->SEND_GOSSIP_MENU(5841, creature->GetGUID());
                else canBuy = true;
                break;
            case 3685:                                          //Harb Clawhoof
                if (player->GetReputationRank(81) != REP_EXALTED && race != RACE_TAUREN)
                    player->SEND_GOSSIP_MENU(5843, creature->GetGUID());
                else canBuy = true;
                break;
            case 4730:                                          //Lelanai
                if (player->GetReputationRank(69) != REP_EXALTED && race != RACE_NIGHTELF)
                    player->SEND_GOSSIP_MENU(5844, creature->GetGUID());
                else canBuy = true;
                break;
            case 4731:                                          //Zachariah Post
                if (player->GetReputationRank(68) != REP_EXALTED && race != RACE_UNDEAD_PLAYER)
                    player->SEND_GOSSIP_MENU(5840, creature->GetGUID());
                else canBuy = true;
                break;
            case 7952:                                          //Zjolnir
                if (player->GetReputationRank(530) != REP_EXALTED && race != RACE_TROLL)
                    player->SEND_GOSSIP_MENU(5842, creature->GetGUID());
                else canBuy = true;
                break;
            case 7955:                                          //Milli Featherwhistle
                if (player->GetReputationRank(54) != REP_EXALTED && race != RACE_GNOME)
                    player->SEND_GOSSIP_MENU(5857, creature->GetGUID());
                else canBuy = true;
                break;
            case 16264:                                         //Winaestra
                if (player->GetReputationRank(911) != REP_EXALTED && race != RACE_BLOODELF)
                    player->SEND_GOSSIP_MENU(10305, creature->GetGUID());
                else canBuy = true;
                break;
            case 17584:                                         //Torallius the Pack Handler
                if (player->GetReputationRank(930) != REP_EXALTED && race != RACE_DRAENEI)
                    player->SEND_GOSSIP_MENU(10239, creature->GetGUID());
                else canBuy = true;
                break;
        }

        if (canBuy)
        {
            if (creature->isVendor())
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_TRADE)
            player->GetSession()->SendListInventory(creature->GetGUID());

        return true;
    }
};

/*######
## npc_rogue_trainer
######*/

#define GOSSIP_HELLO_ROGUE1 "I wish to unlearn my talents"
#define GOSSIP_HELLO_ROGUE2 "<Take the letter>"
#define GOSSIP_HELLO_ROGUE3 "Purchase a Dual Talent Specialization."

class npc_rogue_trainer : public CreatureScript
{
public:
    npc_rogue_trainer() : CreatureScript("npc_rogue_trainer") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (creature->isTrainer())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_TEXT_TRAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

        if (creature->isCanTrainingAndResetTalentsOf(player))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_HELLO_ROGUE1, GOSSIP_SENDER_MAIN, GOSSIP_OPTION_UNLEARNTALENTS);

        if (player->GetSpecsCount() == 1 && creature->isCanTrainingAndResetTalentsOf(player) && !(player->getLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL)))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_HELLO_ROGUE3, GOSSIP_SENDER_MAIN, GOSSIP_OPTION_LEARNDUALSPEC);

        if (player->getClass() == CLASS_ROGUE && player->getLevel() >= 24 && !player->HasItemCount(17126, 1) && !player->GetQuestRewardStatus(6681))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_ROGUE2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(5996, creature->GetGUID());
        } else
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, 21100, false);
                break;
            case GOSSIP_ACTION_TRAIN:
                player->GetSession()->SendTrainerList(creature->GetGUID());
                break;
            case GOSSIP_OPTION_UNLEARNTALENTS:
                player->CLOSE_GOSSIP_MENU();
                player->SendTalentWipeConfirm(creature->GetGUID());
                break;
            case GOSSIP_OPTION_LEARNDUALSPEC:
                if (player->GetSpecsCount() == 1 && !(player->getLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL)))
                {
                    if (!player->HasEnoughMoney(10000000))
                    {
                        player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
                        player->CLOSE_GOSSIP_MENU();
                        break;
                    }
                    else
                    {
                        player->ModifyMoney(-10000000);

                        // Cast spells that teach dual spec
                        // Both are also ImplicitTarget self and must be cast by player
                        player->CastSpell(player, 63680, true, NULL, NULL, player->GetGUID());
                        player->CastSpell(player, 63624, true, NULL, NULL, player->GetGUID());

                        // Should show another Gossip text with "Congratulations..."
                        player->CLOSE_GOSSIP_MENU();
                    }
                }
                break;
        }
        return true;
    }
};

/*######
## npc_sayge
######*/

#define SPELL_DMG       23768                               //dmg
#define SPELL_RES       23769                               //res
#define SPELL_ARM       23767                               //arm
#define SPELL_SPI       23738                               //spi
#define SPELL_INT       23766                               //int
#define SPELL_STM       23737                               //stm
#define SPELL_STR       23735                               //str
#define SPELL_AGI       23736                               //agi
#define SPELL_FORTUNE   23765                               //faire fortune

#define GOSSIP_HELLO_SAYGE  "Yes"
#define GOSSIP_SENDACTION_SAYGE1    "Slay the Man"
#define GOSSIP_SENDACTION_SAYGE2    "Turn him over to liege"
#define GOSSIP_SENDACTION_SAYGE3    "Confiscate the corn"
#define GOSSIP_SENDACTION_SAYGE4    "Let him go and have the corn"
#define GOSSIP_SENDACTION_SAYGE5    "Execute your friend painfully"
#define GOSSIP_SENDACTION_SAYGE6    "Execute your friend painlessly"
#define GOSSIP_SENDACTION_SAYGE7    "Let your friend go"
#define GOSSIP_SENDACTION_SAYGE8    "Confront the diplomat"
#define GOSSIP_SENDACTION_SAYGE9    "Show not so quiet defiance"
#define GOSSIP_SENDACTION_SAYGE10   "Remain quiet"
#define GOSSIP_SENDACTION_SAYGE11   "Speak against your brother openly"
#define GOSSIP_SENDACTION_SAYGE12   "Help your brother in"
#define GOSSIP_SENDACTION_SAYGE13   "Keep your brother out without letting him know"
#define GOSSIP_SENDACTION_SAYGE14   "Take credit, keep gold"
#define GOSSIP_SENDACTION_SAYGE15   "Take credit, share the gold"
#define GOSSIP_SENDACTION_SAYGE16   "Let the knight take credit"
#define GOSSIP_SENDACTION_SAYGE17   "Thanks"

class npc_sayge : public CreatureScript
{
public:
    npc_sayge() : CreatureScript("npc_sayge") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->HasSpellCooldown(SPELL_INT) ||
            player->HasSpellCooldown(SPELL_ARM) ||
            player->HasSpellCooldown(SPELL_DMG) ||
            player->HasSpellCooldown(SPELL_RES) ||
            player->HasSpellCooldown(SPELL_STR) ||
            player->HasSpellCooldown(SPELL_AGI) ||
            player->HasSpellCooldown(SPELL_STM) ||
            player->HasSpellCooldown(SPELL_SPI))
            player->SEND_GOSSIP_MENU(7393, creature->GetGUID());
        else
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_SAYGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(7339, creature->GetGUID());
        }

        return true;
    }

    void SendAction(Player* player, Creature* creature, uint32 action)
    {
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE1,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE2,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE3,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE4,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                player->SEND_GOSSIP_MENU(7340, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE5,            GOSSIP_SENDER_MAIN + 1, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE6,            GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE7,            GOSSIP_SENDER_MAIN + 3, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(7341, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE8,            GOSSIP_SENDER_MAIN + 4, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE9,            GOSSIP_SENDER_MAIN + 5, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE10,           GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(7361, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE11,           GOSSIP_SENDER_MAIN + 6, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE12,           GOSSIP_SENDER_MAIN + 7, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE13,           GOSSIP_SENDER_MAIN + 8, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(7362, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE14,           GOSSIP_SENDER_MAIN + 5, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE15,           GOSSIP_SENDER_MAIN + 4, GOSSIP_ACTION_INFO_DEF);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE16,           GOSSIP_SENDER_MAIN + 3, GOSSIP_ACTION_INFO_DEF);
                player->SEND_GOSSIP_MENU(7363, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE17,           GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                player->SEND_GOSSIP_MENU(7364, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                creature->CastSpell(player, SPELL_FORTUNE, false);
                player->SEND_GOSSIP_MENU(7365, creature->GetGUID());
                break;
        }
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (sender)
        {
            case GOSSIP_SENDER_MAIN:
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 1:
                creature->CastSpell(player, SPELL_DMG, false);
                player->AddSpellCooldown(SPELL_DMG, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 2:
                creature->CastSpell(player, SPELL_RES, false);
                player->AddSpellCooldown(SPELL_RES, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 3:
                creature->CastSpell(player, SPELL_ARM, false);
                player->AddSpellCooldown(SPELL_ARM, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 4:
                creature->CastSpell(player, SPELL_SPI, false);
                player->AddSpellCooldown(SPELL_SPI, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 5:
                creature->CastSpell(player, SPELL_INT, false);
                player->AddSpellCooldown(SPELL_INT, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 6:
                creature->CastSpell(player, SPELL_STM, false);
                player->AddSpellCooldown(SPELL_STM, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 7:
                creature->CastSpell(player, SPELL_STR, false);
                player->AddSpellCooldown(SPELL_STR, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
            case GOSSIP_SENDER_MAIN + 8:
                creature->CastSpell(player, SPELL_AGI, false);
                player->AddSpellCooldown(SPELL_AGI, 0, time(NULL) + 7200);
                SendAction(player, creature, action);
                break;
        }
        return true;
    }
};

class npc_steam_tonk : public CreatureScript
{
public:
    npc_steam_tonk() : CreatureScript("npc_steam_tonk") { }

    struct npc_steam_tonkAI : public ScriptedAI
    {
        npc_steam_tonkAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() {}
        void EnterCombat(Unit* /*who*/) {}

        void OnPossess(bool apply)
        {
            if (apply)
            {
                // Initialize the action bar without the melee attack command
                me->InitCharmInfo();
                me->GetCharmInfo()->InitEmptyActionBar(false);

                me->SetReactState(REACT_PASSIVE);
            }
            else
                me->SetReactState(REACT_AGGRESSIVE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_steam_tonkAI(creature);
    }
};

#define SPELL_TONK_MINE_DETONATE 25099

class npc_tonk_mine : public CreatureScript
{
public:
    npc_tonk_mine() : CreatureScript("npc_tonk_mine") { }

    struct npc_tonk_mineAI : public ScriptedAI
    {
        npc_tonk_mineAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        uint32 ExplosionTimer;

        void Reset()
        {
            ExplosionTimer = 3000;
        }

        void EnterCombat(Unit* /*who*/) {}
        void AttackStart(Unit* /*who*/) {}
        void MoveInLineOfSight(Unit* /*who*/) {}

        void UpdateAI(uint32 const diff)
        {
            if (ExplosionTimer <= diff)
            {
                DoCast(me, SPELL_TONK_MINE_DETONATE, true);
                me->setDeathState(DEAD); // unsummon it
            }
            else
                ExplosionTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tonk_mineAI(creature);
    }
};

/*####
## npc_brewfest_reveler
####*/

class npc_brewfest_reveler : public CreatureScript
{
public:
    npc_brewfest_reveler() : CreatureScript("npc_brewfest_reveler") { }

    struct npc_brewfest_revelerAI : public ScriptedAI
    {
        npc_brewfest_revelerAI(Creature* creature) : ScriptedAI(creature) {}
        void ReceiveEmote(Player* player, uint32 emote)
        {
            if (!IsHolidayActive(HOLIDAY_BREWFEST))
                return;

            if (emote == TEXT_EMOTE_DANCE)
                me->CastSpell(player, 41586, false);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_brewfest_revelerAI(creature);
    }
};

/*####
## npc_winter_reveler
####*/

class npc_winter_reveler : public CreatureScript
{
public:
    npc_winter_reveler() : CreatureScript("npc_winter_reveler") { }

    struct npc_winter_revelerAI : public ScriptedAI
    {
        npc_winter_revelerAI(Creature* c) : ScriptedAI(c) {}
        void ReceiveEmote(Player* player, uint32 emote)
        {
            if (!IsHolidayActive(HOLIDAY_FEAST_OF_WINTER_VEIL))
                return;
            //TODO: check auralist.
            if (player->HasAura(26218))
                return;

            if (emote == TEXT_EMOTE_KISS)
            {
                me->CastSpell(me, 26218, false);
                player->CastSpell(player, 26218, false);
                switch (urand(0, 2))
                {
                    case 0:
                        me->CastSpell(player, 26207, false);
                        break;
                    case 1:
                        me->CastSpell(player, 26206, false);
                        break;
                    case 2:
                        me->CastSpell(player, 45036, false);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_winter_revelerAI(creature);
    }
};

/*####
## npc_snake_trap_serpents
####*/

#define SPELL_MIND_NUMBING_POISON    25810   //Viper
#define SPELL_DEADLY_POISON          34655   //Venomous Snake
#define SPELL_CRIPPLING_POISON       30981   //Viper

#define VENOMOUS_SNAKE_TIMER 1500
#define VIPER_TIMER 3000

#define C_VIPER 19921

#define RAND 5

class npc_snake_trap : public CreatureScript
{
public:
    npc_snake_trap() : CreatureScript("npc_snake_trap_serpents") { }

    struct npc_snake_trap_serpentsAI : public ScriptedAI
    {
        npc_snake_trap_serpentsAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 SpellTimer;
        bool IsViper;

        void EnterCombat(Unit* /*who*/) {}

        void Reset()
        {
            SpellTimer = 0;

            CreatureTemplate const* Info = me->GetCreatureTemplate();

            IsViper = Info->Entry == C_VIPER ? true : false;

            me->SetMaxHealth(uint32(107 * (me->getLevel() - 40) * 0.025f));
            //Add delta to make them not all hit the same time
            uint32 delta = (rand() % 7) * 100;
            me->SetStatFloatValue(UNIT_FIELD_BASEATTACKTIME, float(Info->baseattacktime + delta));
            me->SetStatFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER, float(Info->attackpower));

            // Start attacking attacker of owner on first ai update after spawn - move in line of sight may choose better target
            if (!me->getVictim() && me->isSummon())
                if (Unit* Owner = me->ToTempSummon()->GetSummoner())
                    if (Owner->getAttackerForHelper())
                        AttackStart(Owner->getAttackerForHelper());
        }

        //Redefined for random target selection:
        void MoveInLineOfSight(Unit* who)
        {
            if (!me->getVictim() && me->canCreatureAttack(who))
            {
                if (me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                    return;

                float attackRadius = me->GetAttackDistance(who);
                if (me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who))
                {
                    if (!(rand() % RAND))
                    {
                        me->setAttackTimer(BASE_ATTACK, (rand() % 10) * 100);
                        SpellTimer = (rand() % 10) * 100;
                        AttackStart(who);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (me->getVictim()->HasBreakableByDamageCrowdControlAura(me))
            {
                me->InterruptNonMeleeSpells(false);
                return;
            }

            if (SpellTimer <= diff)
            {
                if (IsViper) //Viper
                {
                    if (urand(0, 2) == 0) //33% chance to cast
                    {
                        uint32 spell;
                        if (urand(0, 1) == 0)
                            spell = SPELL_MIND_NUMBING_POISON;
                        else
                            spell = SPELL_CRIPPLING_POISON;

                        DoCast(me->getVictim(), spell);
                    }

                    SpellTimer = VIPER_TIMER;
                }
                else //Venomous Snake
                {
                    if (urand(0, 2) == 0) //33% chance to cast
                        DoCast(me->getVictim(), SPELL_DEADLY_POISON);
                    SpellTimer = VENOMOUS_SNAKE_TIMER + (rand() % 5) * 100;
                }
            }
            else
                SpellTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_snake_trap_serpentsAI(creature);
    }
};

#define SAY_RANDOM_MOJO0    "Now that's what I call froggy-style!"
#define SAY_RANDOM_MOJO1    "Your lily pad or mine?"
#define SAY_RANDOM_MOJO2    "This won't take long, did it?"
#define SAY_RANDOM_MOJO3    "I thought you'd never ask!"
#define SAY_RANDOM_MOJO4    "I promise not to give you warts..."
#define SAY_RANDOM_MOJO5    "Feelin' a little froggy, are ya?"
#define SAY_RANDOM_MOJO6a   "Listen, "
#define SAY_RANDOM_MOJO6b   ", I know of a little swamp not too far from here...."
#define SAY_RANDOM_MOJO7    "There's just never enough Mojo to go around..."

class mob_mojo : public CreatureScript
{
public:
    mob_mojo() : CreatureScript("mob_mojo") { }

    struct mob_mojoAI : public ScriptedAI
    {
        mob_mojoAI(Creature* creature) : ScriptedAI(creature) {Reset();}
        uint32 hearts;
        uint64 victimGUID;
        void Reset()
        {
            victimGUID = 0;
            hearts = 15000;
            if (Unit* own = me->GetOwner())
                me->GetMotionMaster()->MoveFollow(own, 0, 0);
        }

        void EnterCombat(Unit* /*who*/){}

        void UpdateAI(uint32 const diff)
        {
            if (me->HasAura(20372))
            {
                if (hearts <= diff)
                {
                    me->RemoveAurasDueToSpell(20372);
                    hearts = 15000;
                } hearts -= diff;
            }
        }

        void ReceiveEmote(Player* player, uint32 emote)
        {
            me->HandleEmoteCommand(emote);
            Unit* own = me->GetOwner();
            if (!own || own->GetTypeId() != TYPEID_PLAYER || CAST_PLR(own)->GetTeam() != player->GetTeam())
                return;
            if (emote == TEXT_EMOTE_KISS)
            {
                std::string whisp = "";
                switch (rand() % 8)
                {
                    case 0:
                        whisp.append(SAY_RANDOM_MOJO0);
                        break;
                    case 1:
                        whisp.append(SAY_RANDOM_MOJO1);
                        break;
                    case 2:
                        whisp.append(SAY_RANDOM_MOJO2);
                        break;
                    case 3:
                        whisp.append(SAY_RANDOM_MOJO3);
                        break;
                    case 4:
                        whisp.append(SAY_RANDOM_MOJO4);
                        break;
                    case 5:
                        whisp.append(SAY_RANDOM_MOJO5);
                        break;
                    case 6:
                        whisp.append(SAY_RANDOM_MOJO6a);
                        whisp.append(player->GetName());
                        whisp.append(SAY_RANDOM_MOJO6b);
                        break;
                    case 7:
                        whisp.append(SAY_RANDOM_MOJO7);
                        break;
                }

                me->MonsterWhisper(whisp.c_str(), player->GetGUID());
                if (victimGUID)
                    if (Player* victim = Unit::GetPlayer(*me, victimGUID))
                        victim->RemoveAura(43906);//remove polymorph frog thing
                me->AddAura(43906, player);//add polymorph frog thing
                victimGUID = player->GetGUID();
                DoCast(me, 20372, true);//tag.hearts
                me->GetMotionMaster()->MoveFollow(player, 0, 0);
                hearts = 15000;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_mojoAI(creature);
    }
};

/*#####
# npc_spring_rabbit
#####*/

enum rabbitSpells
{
    SPELL_SPRING_FLING          = 61875,
    SPELL_SPRING_RABBIT_JUMP    = 61724,
    SPELL_SPRING_RABBIT_WANDER  = 61726,
    SPELL_SUMMON_BABY_BUNNY     = 61727,
    SPELL_SPRING_RABBIT_IN_LOVE = 61728,
    NPC_SPRING_RABBIT           = 32791
};

class npc_spring_rabbit : public CreatureScript
{
public:
    npc_spring_rabbit() : CreatureScript("npc_spring_rabbit") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_spring_rabbitAI(creature);
    }

    struct npc_spring_rabbitAI : public ScriptedAI
    {
        npc_spring_rabbitAI(Creature* c) : ScriptedAI(c) { }

        bool inLove;
        uint32 jumpTimer;
        uint32 bunnyTimer;
        uint32 searchTimer;
        uint64 rabbitGUID;

        void Reset()
        {
            inLove = false;
            rabbitGUID = 0;
            jumpTimer = urand(5000, 10000);
            bunnyTimer = urand(10000, 20000);
            searchTimer = urand(5000, 10000);
            if (Unit* owner = me->GetOwner())
                me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        }

        void EnterCombat(Unit * /*who*/) { }

        void DoAction(const int32 /*param*/)
        {
            inLove = true;
            if (Unit* owner = me->GetOwner())
                owner->CastSpell(owner, SPELL_SPRING_FLING, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (inLove)
            {
                if (jumpTimer <= diff)
                {
                    if (Unit* rabbit = Unit::GetUnit(*me, rabbitGUID))
                        DoCast(rabbit, SPELL_SPRING_RABBIT_JUMP);
                    jumpTimer = urand(5000, 10000);
                }else jumpTimer -= diff;

                if (bunnyTimer <= diff)
                {
                    DoCast(SPELL_SUMMON_BABY_BUNNY);
                    bunnyTimer = urand(20000, 40000);
                }
                else 
                    bunnyTimer -= diff;
            }
            else
            {
                if (searchTimer <= diff)
                {
                    if (Creature* rabbit = me->FindNearestCreature(NPC_SPRING_RABBIT, 10.0f))
                    {
                        if (rabbit == me || rabbit->HasAura(SPELL_SPRING_RABBIT_IN_LOVE))
                            return;

                        me->AddAura(SPELL_SPRING_RABBIT_IN_LOVE, me);
                        DoAction(1);
                        rabbit->AddAura(SPELL_SPRING_RABBIT_IN_LOVE, rabbit);
                        rabbit->AI()->DoAction(1);
                        rabbit->CastSpell(rabbit, SPELL_SPRING_RABBIT_JUMP, true);
                        rabbitGUID = rabbit->GetGUID();
                    }
                    searchTimer = urand(5000, 10000);
                }else searchTimer -= diff;
            }
        }
    };
};

class npc_mirror_image : public CreatureScript
{
public:
    npc_mirror_image() : CreatureScript("npc_mirror_image") { }

    struct npc_mirror_imageAI : CasterAI
    {
        npc_mirror_imageAI(Creature* creature) : CasterAI(creature) {}

        void InitializeAI()
        {
            CasterAI::InitializeAI();
            Unit* owner = me->GetOwner();
            if (!owner)
                return;
            // Inherit Master's Threat List (not yet implemented)
            owner->CastSpell((Unit*)NULL, 58838, true);
            // here mirror image casts on summoner spell (not present in client dbc) 49866
            // here should be auras (not present in client dbc): 35657, 35658, 35659, 35660 selfcasted by mirror images (stats related?)
            // Clone Me!
            owner->CastSpell(me, 45204, false);
        }

        // Do not reload Creature templates on evade mode enter - prevent visual lost
        void EnterEvadeMode()
        {
            if (me->IsInEvadeMode() || !me->isAlive())
                return;

            Unit* owner = me->GetCharmerOrOwner();

            me->CombatStop(true);
            if (owner && !me->HasUnitState(UNIT_STATE_FOLLOW))
            {
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle(), MOTION_SLOT_ACTIVE);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mirror_imageAI(creature);
    }
};

class npc_ebon_gargoyle : public CreatureScript
{
public:
    npc_ebon_gargoyle() : CreatureScript("npc_ebon_gargoyle") { }

    struct npc_ebon_gargoyleAI : CasterAI
    {
        npc_ebon_gargoyleAI(Creature* creature) : CasterAI(creature) {}

        uint32 despawnTimer;

        void InitializeAI()
        {
            CasterAI::InitializeAI();
            uint64 ownerGuid = me->GetOwnerGUID();
            if (!ownerGuid)
                return;
            // Not needed to be despawned now
            despawnTimer = 0;
            // Find victim of Summon Gargoyle spell
            std::list<Unit*> targets;
            Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 30);
            Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
            me->VisitNearbyObject(30, searcher);
            for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                if ((*iter)->GetAura(49206, ownerGuid))
                {
                    me->Attack((*iter), false);
                    break;
                }
        }

        void JustDied(Unit* /*killer*/)
        {
            // Stop Feeding Gargoyle when it dies
            if (Unit* owner = me->GetOwner())
                owner->RemoveAurasDueToSpell(50514);
        }

        // Fly away when dismissed
        void SpellHit(Unit* source, SpellInfo const* spell)
        {
            if (spell->Id != 50515 || !me->isAlive())
                return;

            Unit* owner = me->GetOwner();

            if (!owner || owner != source)
                return;

            // Stop Fighting
            me->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, true);
            // Sanctuary
            me->CastSpell(me, 54661, true);
            me->SetReactState(REACT_PASSIVE);

            //! HACK: Creature's can't have MOVEMENTFLAG_FLYING
            // Fly Away
            me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY|MOVEMENTFLAG_ASCENDING|MOVEMENTFLAG_FLYING);
            me->SetSpeed(MOVE_FLIGHT, 0.75f, true);
            me->SetSpeed(MOVE_RUN, 0.75f, true);
            float x = me->GetPositionX() + 20 * cos(me->GetOrientation());
            float y = me->GetPositionY() + 20 * sin(me->GetOrientation());
            float z = me->GetPositionZ() + 40;
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MovePoint(0, x, y, z);

            // Despawn as soon as possible
            despawnTimer = 4 * IN_MILLISECONDS;
        }

        void UpdateAI(const uint32 diff)
        {
            if (despawnTimer > 0)
            {
                if (despawnTimer > diff)
                    despawnTimer -= diff;
                else
                    me->DespawnOrUnsummon();
                return;
            }
            CasterAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ebon_gargoyleAI(creature);
    }
};

class npc_lightwell : public CreatureScript
{
public:
    npc_lightwell() : CreatureScript("npc_lightwell") { }

    struct npc_lightwellAI : public PassiveAI
    {
        npc_lightwellAI(Creature* creature) : PassiveAI(creature) {}

        void Reset()
        {
            DoCast(me, 59907, false); // Spell for Lightwell Charges
        }

        void EnterEvadeMode()
        {
            if (!me->isAlive())
                return;

            me->DeleteThreatList();
            me->CombatStop(true);
            me->ResetPlayerDamageReq();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lightwellAI(creature);
    }
};

enum eTrainingDummy
{
    NPC_ADVANCED_TARGET_DUMMY                  = 2674,
    NPC_TARGET_DUMMY                           = 2673
};

class npc_training_dummy : public CreatureScript
{
public:
    npc_training_dummy() : CreatureScript("npc_training_dummy") { }

    struct npc_training_dummyAI : Scripted_NoMovementAI
    {
        npc_training_dummyAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            entry = creature->GetEntry();
        }

        uint32 entry;
        uint32 resetTimer;
        uint32 despawnTimer;

        void Reset()
        {
            me->SetControlled(true, UNIT_STATE_STUNNED);//disable rotate

            resetTimer = 5000;
            despawnTimer = 15000;
        }

        void EnterEvadeMode()
        {
            if (!_EnterEvadeMode())
                return;

            Reset();
        }

        void DamageTaken(Unit* /*doneBy*/, uint32& damage)
        {
            resetTimer = 5000;
            damage = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (entry != NPC_ADVANCED_TARGET_DUMMY && entry != NPC_TARGET_DUMMY)
                return;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (!me->HasUnitState(UNIT_STATE_STUNNED))
                me->SetControlled(true, UNIT_STATE_STUNNED);//disable rotate

            if (entry != NPC_ADVANCED_TARGET_DUMMY && entry != NPC_TARGET_DUMMY)
            {
                if (resetTimer <= diff)
                {
                    EnterEvadeMode();
                    resetTimer = 5000;
                }
                else
                    resetTimer -= diff;
                return;
            }
            else
            {
                if (despawnTimer <= diff)
                    me->DespawnOrUnsummon();
                else
                    despawnTimer -= diff;
            }
        }
        void MoveInLineOfSight(Unit* /*who*/){return;}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_training_dummyAI(creature);
    }
};

/*######
# npc_shadowfiend
######*/
#define GLYPH_OF_SHADOWFIEND_MANA         58227
#define GLYPH_OF_SHADOWFIEND              58228

class npc_shadowfiend : public CreatureScript
{
public:
    npc_shadowfiend() : CreatureScript("npc_shadowfiend") { }

    struct npc_shadowfiendAI : public ScriptedAI
    {
        npc_shadowfiendAI(Creature* creature) : ScriptedAI(creature) {}

        void DamageTaken(Unit* /*killer*/, uint32& damage)
        {
            if (me->isSummon())
                if (Unit* owner = me->ToTempSummon()->GetSummoner())
                    if (owner->HasAura(GLYPH_OF_SHADOWFIEND) && damage >= me->GetHealth())
                        owner->CastSpell(owner, GLYPH_OF_SHADOWFIEND_MANA, true);
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shadowfiendAI(creature);
    }
};

/*######
# npc_fire_elemental
######*/
#define SPELL_FIRENOVA        12470
#define SPELL_FIRESHIELD      13376
#define SPELL_FIREBLAST       57984

class npc_fire_elemental : public CreatureScript
{
public:
    npc_fire_elemental() : CreatureScript("npc_fire_elemental") { }

    struct npc_fire_elementalAI : public ScriptedAI
    {
        npc_fire_elementalAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 FireNova_Timer;
        uint32 FireShield_Timer;
        uint32 FireBlast_Timer;

        void Reset()
        {
            FireNova_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            FireBlast_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            FireShield_Timer = 0;
            me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (FireShield_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_FIRESHIELD);
                FireShield_Timer = 2 * IN_MILLISECONDS;
            }
            else
                FireShield_Timer -= diff;

            if (FireBlast_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_FIREBLAST);
                FireBlast_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            }
            else
                FireBlast_Timer -= diff;

            if (FireNova_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_FIRENOVA);
                FireNova_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            }
            else
                FireNova_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI *GetAI(Creature* creature) const
    {
        return new npc_fire_elementalAI(creature);
    }
};

/*######
# npc_earth_elemental
######*/
#define SPELL_ANGEREDEARTH        36213

class npc_earth_elemental : public CreatureScript
{
public:
    npc_earth_elemental() : CreatureScript("npc_earth_elemental") { }

    struct npc_earth_elementalAI : public ScriptedAI
    {
        npc_earth_elementalAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 AngeredEarth_Timer;

        void Reset()
        {
            AngeredEarth_Timer = 0;
            me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (AngeredEarth_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_ANGEREDEARTH);
                AngeredEarth_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            }
            else
                AngeredEarth_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI *GetAI(Creature* creature) const
    {
        return new npc_earth_elementalAI(creature);
    }
};

/*######
# npc_wormhole
######*/

#define GOSSIP_ENGINEERING1   "Borean Tundra."
#define GOSSIP_ENGINEERING2   "Howling Fjord."
#define GOSSIP_ENGINEERING3   "Sholazar Basin."
#define GOSSIP_ENGINEERING4   "Icecrown."
#define GOSSIP_ENGINEERING5   "Storm Peaks."
#define GOSSIP_ENGINEERING6   "The Underground."

enum eWormhole
{
    SPELL_HOWLING_FJORD         = 67838,
    SPELL_SHOLAZAR_BASIN        = 67835,
    SPELL_ICECROWN              = 67836,
    SPELL_STORM_PEAKS           = 67837,
    SPELL_UNDERGROUND           = 68081,

    TEXT_WORMHOLE               = 907,
    DATA_UNDERGROUND            = 1
};

class npc_wormhole : public CreatureScript
{
    public:
        npc_wormhole() : CreatureScript("npc_wormhole") { }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (player == creature->ToTempSummon()->GetSummoner())
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

                if (creature->AI()->GetData(DATA_UNDERGROUND) == 1)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            }

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();
            bool roll = urand(0, 1);

            switch(action)
            {
                case GOSSIP_ACTION_INFO_DEF + 1: // Borean Tundra
                    player->CLOSE_GOSSIP_MENU();
                    if (roll) // At the moment we don't have chance on spell_target_position table so we hack this
                        player->TeleportTo(571, 4305.505859f, 5450.839844f, 63.005806f, 0.627286f);
                    else
                        player->TeleportTo(571, 3201.936279f, 5630.123535f, 133.658798f, 3.855272f);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 2: // Howling Fjord
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_HOWLING_FJORD, true);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 3: // Sholazar Basin
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_SHOLAZAR_BASIN, true);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 4: // Icecrown
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_ICECROWN, true);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 5: // Storm peaks
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_STORM_PEAKS, true);
                    break;
                case GOSSIP_ACTION_INFO_DEF + 6: // Underground
                    player->CLOSE_GOSSIP_MENU();
                    player->CastSpell(player, SPELL_UNDERGROUND, true);
                    break;
            }
            return true;
        }

        struct npc_wormholeAI : PassiveAI
        {
            npc_wormholeAI(Creature* c) : PassiveAI(c)
            {
                _random = urand(0, 9);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            }

            uint32 GetData(uint32 type)
            {
                if (type == DATA_UNDERGROUND)
                    return (_random > 0) ? 0 : 1;
                return 0;
            }

        private:
            uint8 _random;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_wormholeAI(creature);
        }
};

/*######
## npc_pet_trainer
######*/

enum ePetTrainer
{
    TEXT_ISHUNTER               = 5838,
    TEXT_NOTHUNTER              = 5839,
    TEXT_PETINFO                = 13474,
    TEXT_CONFIRM                = 7722
};

#define GOSSIP_PET1             "How do I train my pet?"
#define GOSSIP_PET2             "I wish to untrain my pet."
#define GOSSIP_PET_CONFIRM      "Yes, please do."

class npc_pet_trainer : public CreatureScript
{
public:
    npc_pet_trainer() : CreatureScript("npc_pet_trainer") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->getClass() == CLASS_HUNTER)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            if (player->GetPet() && player->GetPet()->getPetType() == HUNTER_PET)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

            player->PlayerTalkClass->SendGossipMenu(TEXT_ISHUNTER, creature->GetGUID());
            return true;
        }
        player->PlayerTalkClass->SendGossipMenu(TEXT_NOTHUNTER, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->PlayerTalkClass->SendGossipMenu(TEXT_PETINFO, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET_CONFIRM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->PlayerTalkClass->SendGossipMenu(TEXT_CONFIRM, creature->GetGUID());
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                {
                    player->ResetPetTalents();
                    player->CLOSE_GOSSIP_MENU();
                }
                break;
        }
        return true;
    }
};

/*######
## npc_locksmith
######*/

enum eLockSmith
{
    QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ = 10704,
    QUEST_DARK_IRON_LEGACY                = 3802,
    QUEST_THE_KEY_TO_SCHOLOMANCE_A        = 5505,
    QUEST_THE_KEY_TO_SCHOLOMANCE_H        = 5511,
    QUEST_HOTTER_THAN_HELL_A              = 10758,
    QUEST_HOTTER_THAN_HELL_H              = 10764,
    QUEST_RETURN_TO_KHAGDAR               = 9837,
    QUEST_CONTAINMENT                     = 13159,

    ITEM_ARCATRAZ_KEY                     = 31084,
    ITEM_SHADOWFORGE_KEY                  = 11000,
    ITEM_SKELETON_KEY                     = 13704,
    ITEM_SHATTERED_HALLS_KEY              = 28395,
    ITEM_THE_MASTERS_KEY                  = 24490,
    ITEM_VIOLET_HOLD_KEY                  = 42482,

    SPELL_ARCATRAZ_KEY                    = 54881,
    SPELL_SHADOWFORGE_KEY                 = 54882,
    SPELL_SKELETON_KEY                    = 54883,
    SPELL_SHATTERED_HALLS_KEY             = 54884,
    SPELL_THE_MASTERS_KEY                 = 54885,
    SPELL_VIOLET_HOLD_KEY                 = 67253
};

#define GOSSIP_LOST_ARCATRAZ_KEY         "I've lost my key to the Arcatraz."
#define GOSSIP_LOST_SHADOWFORGE_KEY      "I've lost my key to the Blackrock Depths."
#define GOSSIP_LOST_SKELETON_KEY         "I've lost my key to the Scholomance."
#define GOSSIP_LOST_SHATTERED_HALLS_KEY  "I've lost my key to the Shattered Halls."
#define GOSSIP_LOST_THE_MASTERS_KEY      "I've lost my key to the Karazhan."
#define GOSSIP_LOST_VIOLET_HOLD_KEY      "I've lost my key to the Violet Hold."

class npc_locksmith : public CreatureScript
{
public:
    npc_locksmith() : CreatureScript("npc_locksmith") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        // Arcatraz Key
        if (player->GetQuestRewardStatus(QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ) && !player->HasItemCount(ITEM_ARCATRAZ_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_ARCATRAZ_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        // Shadowforge Key
        if (player->GetQuestRewardStatus(QUEST_DARK_IRON_LEGACY) && !player->HasItemCount(ITEM_SHADOWFORGE_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHADOWFORGE_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        // Skeleton Key
        if ((player->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_A) || player->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_H)) &&
            !player->HasItemCount(ITEM_SKELETON_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SKELETON_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        // Shatered Halls Key
        if ((player->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_A) || player->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_H)) &&
            !player->HasItemCount(ITEM_SHATTERED_HALLS_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHATTERED_HALLS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);

        // Master's Key
        if (player->GetQuestRewardStatus(QUEST_RETURN_TO_KHAGDAR) && !player->HasItemCount(ITEM_THE_MASTERS_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_THE_MASTERS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

        // Violet Hold Key
        if (player->GetQuestRewardStatus(QUEST_CONTAINMENT) && !player->HasItemCount(ITEM_VIOLET_HOLD_KEY, 1, true))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_VIOLET_HOLD_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_ARCATRAZ_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_SHADOWFORGE_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_SKELETON_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_SHATTERED_HALLS_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_THE_MASTERS_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_VIOLET_HOLD_KEY, false);
                break;
        }
        return true;
    }
};

/*######
## npc_tabard_vendor
######*/

enum
{
    QUEST_TRUE_MASTERS_OF_LIGHT = 9737,
    QUEST_THE_UNWRITTEN_PROPHECY = 9762,
    QUEST_INTO_THE_BREACH = 10259,
    QUEST_BATTLE_OF_THE_CRIMSON_WATCH = 10781,
    QUEST_SHARDS_OF_AHUNE = 11972,

    ACHIEVEMENT_EXPLORE_NORTHREND = 45,
    ACHIEVEMENT_TWENTYFIVE_TABARDS = 1021,
    ACHIEVEMENT_THE_LOREMASTER_A = 1681,
    ACHIEVEMENT_THE_LOREMASTER_H = 1682,

    ITEM_TABARD_OF_THE_HAND = 24344,
    ITEM_TABARD_OF_THE_BLOOD_KNIGHT = 25549,
    ITEM_TABARD_OF_THE_PROTECTOR = 28788,
    ITEM_OFFERING_OF_THE_SHATAR = 31408,
    ITEM_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI = 31404,
    ITEM_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI = 31405,
    ITEM_TABARD_OF_THE_SUMMER_SKIES = 35279,
    ITEM_TABARD_OF_THE_SUMMER_FLAMES = 35280,
    ITEM_TABARD_OF_THE_ACHIEVER = 40643,
    ITEM_LOREMASTERS_COLORS = 43300,
    ITEM_TABARD_OF_THE_EXPLORER = 43348,

    SPELL_TABARD_OF_THE_BLOOD_KNIGHT = 54974,
    SPELL_TABARD_OF_THE_HAND = 54976,
    SPELL_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI = 54977,
    SPELL_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI = 54982,
    SPELL_TABARD_OF_THE_ACHIEVER = 55006,
    SPELL_TABARD_OF_THE_PROTECTOR = 55008,
    SPELL_LOREMASTERS_COLORS = 58194,
    SPELL_TABARD_OF_THE_EXPLORER = 58224,
    SPELL_TABARD_OF_SUMMER_SKIES = 62768,
    SPELL_TABARD_OF_SUMMER_FLAMES = 62769
};

#define GOSSIP_LOST_TABARD_OF_BLOOD_KNIGHT "I've lost my Tabard of Blood Knight."
#define GOSSIP_LOST_TABARD_OF_THE_HAND "I've lost my Tabard of the Hand."
#define GOSSIP_LOST_TABARD_OF_THE_PROTECTOR "I've lost my Tabard of the Protector."
#define GOSSIP_LOST_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI "I've lost my Green Trophy Tabard of the Illidari."
#define GOSSIP_LOST_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI "I've lost my Purple Trophy Tabard of the Illidari."
#define GOSSIP_LOST_TABARD_OF_SUMMER_SKIES "I've lost my Tabard of Summer Skies."
#define GOSSIP_LOST_TABARD_OF_SUMMER_FLAMES "I've lost my Tabard of Summer Flames."
#define GOSSIP_LOST_LOREMASTERS_COLORS "I've lost my Loremaster's Colors."
#define GOSSIP_LOST_TABARD_OF_THE_EXPLORER "I've lost my Tabard of the Explorer."
#define GOSSIP_LOST_TABARD_OF_THE_ACHIEVER "I've lost my Tabard of the Achiever."

class npc_tabard_vendor : public CreatureScript
{
public:
    npc_tabard_vendor() : CreatureScript("npc_tabard_vendor") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        bool lostBloodKnight = false;
        bool lostHand = false;
        bool lostProtector = false;
        bool lostIllidari = false;
        bool lostSummer = false;
        bool lostExplorer = false;

        // Tabard of the Blood Knight
        if (player->GetQuestRewardStatus(QUEST_TRUE_MASTERS_OF_LIGHT) && !player->HasItemCount(ITEM_TABARD_OF_THE_BLOOD_KNIGHT, 1, true))
            lostBloodKnight = true;

        // Tabard of the Hand
        if (player->GetQuestRewardStatus(QUEST_THE_UNWRITTEN_PROPHECY) && !player->HasItemCount(ITEM_TABARD_OF_THE_HAND, 1, true))
            lostHand = true;

        // Tabard of the Protector
        if (player->GetQuestRewardStatus(QUEST_INTO_THE_BREACH) && !player->HasItemCount(ITEM_TABARD_OF_THE_PROTECTOR, 1, true))
            lostProtector = true;

        // Green Trophy Tabard of the Illidari
        // Purple Trophy Tabard of the Illidari
        if (player->GetQuestRewardStatus(QUEST_BATTLE_OF_THE_CRIMSON_WATCH) &&
            (!player->HasItemCount(ITEM_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, 1, true) &&
            !player->HasItemCount(ITEM_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, 1, true) &&
            !player->HasItemCount(ITEM_OFFERING_OF_THE_SHATAR, 1, true)))
            lostIllidari = true;

        // Tabard of Summer Skies
        // Tabard of Summer Flames
        if (player->GetQuestRewardStatus(QUEST_SHARDS_OF_AHUNE) &&
            !player->HasItemCount(ITEM_TABARD_OF_THE_SUMMER_SKIES, 1, true) &&
            !player->HasItemCount(ITEM_TABARD_OF_THE_SUMMER_FLAMES, 1, true))
            lostSummer = true;

        if (player->HasAchieved(ACHIEVEMENT_EXPLORE_NORTHREND) &&
            !player->HasItemCount(ITEM_TABARD_OF_THE_EXPLORER, 1, true))
            lostExplorer = true;

        if (lostBloodKnight || lostHand || lostProtector || lostIllidari || lostSummer || lostExplorer)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

            if (lostBloodKnight)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_BLOOD_KNIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if (lostHand)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_HAND, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

            if (lostProtector)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_PROTECTOR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

            if (lostIllidari)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            }

            if (lostSummer)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_SUMMER_SKIES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_SUMMER_FLAMES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            }

            if (lostExplorer)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_EXPLORER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        }
        else
            player->GetSession()->SendListInventory(creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
            case GOSSIP_ACTION_TRADE:
                player->GetSession()->SendListInventory(creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_THE_BLOOD_KNIGHT, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_THE_HAND, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_THE_PROTECTOR, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_SUMMER_SKIES, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 7:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_SUMMER_FLAMES, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 8:
                player->CLOSE_GOSSIP_MENU();
                player->CastSpell(player, SPELL_TABARD_OF_THE_EXPLORER, false);
                break;
        }
        return true;
    }
};

/*######
## npc_experience
######*/

#define EXP_COST                100000 //10 00 00 copper (10golds)
#define GOSSIP_TEXT_EXP         14736
#define GOSSIP_XP_OFF           "I no longer wish to gain experience."
#define GOSSIP_XP_ON            "I wish to start gaining experience again."

class npc_experience : public CreatureScript
{
public:
    npc_experience() : CreatureScript("npc_experience") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_OFF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_ON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_EXP, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        bool noXPGain = player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        bool doSwitch = false;

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1://xp off
                {
                    if (!noXPGain)//does gain xp
                        doSwitch = true;//switch to don't gain xp
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 2://xp on
                {
                    if (noXPGain)//doesn't gain xp
                        doSwitch = true;//switch to gain xp
                }
                break;
        }
        if (doSwitch)
        {
            if (!player->HasEnoughMoney(EXP_COST))
                player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
            else if (noXPGain)
            {
                player->ModifyMoney(-EXP_COST);
                player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
            }
            else if (!noXPGain)
            {
                player->ModifyMoney(-EXP_COST);
                player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
            }
        }
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

/*######
## npc_torch_tossing_bunny
######*/

enum
{
    SPELL_TORCH_TOSSING_COMPLETE_A = 45719,
    SPELL_TORCH_TOSSING_COMPLETE_H = 46651,
    SPELL_TORCH_TOSSING_TRAINING   = 45716,
    SPELL_TORCH_TOSSING_PRACTICE   = 46630,
    SPELL_TORCH_TOSS               = 46054,
    SPELL_TARGET_INDICATOR         = 45723,
    SPELL_BRAZIERS_HIT             = 45724
};

class npc_torch_tossing_bunny : public CreatureScript
{
    public:
        npc_torch_tossing_bunny() : CreatureScript("npc_torch_tossing_bunny") { }

        struct npc_torch_tossing_bunnyAI : public ScriptedAI
        {
            npc_torch_tossing_bunnyAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                _targetTimer = urand(5000, 20000);
                _validTarget = false;
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_TORCH_TOSS && _validTarget)
                {
                    uint8 neededHits;

                    if (caster->HasAura(SPELL_TORCH_TOSSING_TRAINING))
                        neededHits = 8;
                    else if (caster->HasAura(SPELL_TORCH_TOSSING_PRACTICE))
                        neededHits = 20;
                    else
                        return;

                    DoCast(me, SPELL_BRAZIERS_HIT, true);
                    caster->AddAura(SPELL_BRAZIERS_HIT, caster);

                    if (caster->GetAuraCount(SPELL_BRAZIERS_HIT) >= neededHits)
                    {
                        // complete quest
                        caster->CastSpell(caster, SPELL_TORCH_TOSSING_COMPLETE_A, true);
                        caster->CastSpell(caster, SPELL_TORCH_TOSSING_COMPLETE_H, true);
                        caster->RemoveAurasDueToSpell(SPELL_BRAZIERS_HIT);
                        caster->RemoveAurasDueToSpell(neededHits == 8 ? SPELL_TORCH_TOSSING_TRAINING : SPELL_TORCH_TOSSING_PRACTICE);
                    }
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (_targetTimer <= diff)
                {
                    if (!_validTarget)
                    {
                        _validTarget = true;
                        DoCast(SPELL_TARGET_INDICATOR);
                        _targetTimer = 5000;
                    }
                    else
                    {
                        _validTarget = false;
                        _targetTimer = urand(5000, 15000);
                    }
                }
                else
                    _targetTimer -= diff;
            }

        private:
            uint32 _targetTimer;
            bool _validTarget;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_torch_tossing_bunnyAI(creature);
        }
};

/*######
## Brewfest
######*/

enum Brewfest
{
    SPELL_APPLE_TRAP             = 43450,
    SPELL_RACING_RAM             = 42146,
    SPELL_RAM_FATIGUE            = 43052,
    SPELL_CREATE_KEG             = 42414,
    SPELL_HAS_KEG                = 44066,
    SPELL_THROW_KEG              = 43660,
    SPELL_THROW_KEG_PLAYER       = 43662,
    SPELL_WORKING_FOR_THE_MAN    = 43534,
    SPELL_RELAY_RACE_DEBUFF      = 44689,
    SPELL_RENTAL_RACING_RAM      = 43883,
    SPELL_CREATE_TICKETS         = 44501,

    QUEST_THERE_AND_BACK_AGAIN_A = 11122,
    QUEST_THERE_AND_BACK_AGAIN_H = 11412,
    QUEST_BARK_FOR_THE_1         = 11293,
    QUEST_BARK_FOR_THE_2         = 11294,
    QUEST_BARK_FOR_THE_3         = 11407,
    QUEST_BARK_FOR_THE_4         = 11408,

    ITEM_PORTABLE_BREWFEST_KEG   = 33797,

    NPC_DELIVERY_CREDIT          = 24337, // TODO: use spell
    NPC_FLYNN_FIREBREW           = 24364,
    NPC_BOK_DROPCERTAIN          = 24527,
    NPC_RAM_MASTER_RAY           = 24497,
    NPC_NEILL_RAMSTEIN           = 23558,

    ACHIEV_BREW_OF_THE_MONTH     = 2796
};

class npc_apple_trap_bunny : public CreatureScript
{
    public:
        npc_apple_trap_bunny() : CreatureScript("npc_apple_trap_bunny") { }

        struct npc_apple_trap_bunnyAI : public ScriptedAI
        {
            npc_apple_trap_bunnyAI(Creature* creature) : ScriptedAI(creature) { }

            void MoveInLineOfSight(Unit* who)
            {
                if (who && who->ToPlayer() && who->HasAura(SPELL_RACING_RAM) && !who->HasAura(SPELL_APPLE_TRAP) && me->GetDistance(who) < 9.0f)
                {
                    who->RemoveAurasDueToSpell(SPELL_RAM_FATIGUE);
                    who->CastSpell(who, SPELL_APPLE_TRAP, true);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_apple_trap_bunnyAI(creature);
        }
};

class npc_keg_delivery : public CreatureScript
{
    public:
        npc_keg_delivery() : CreatureScript("npc_keg_delivery") { }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (creature->isQuestGiver())
                player->PrepareQuestMenu(creature->GetGUID());

            if ((player->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_A) ||
                player->GetQuestRewardStatus(QUEST_THERE_AND_BACK_AGAIN_H)) && !player->HasAura(SPELL_RELAY_RACE_DEBUFF))
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Gibt es noch mehr zu tun?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
        {
            if (action == GOSSIP_ACTION_INFO_DEF + 1)
            {
                player->CastSpell(player, SPELL_RENTAL_RACING_RAM, true);
                player->CastSpell(player, SPELL_WORKING_FOR_THE_MAN, true);
                creature->AddAura(SPELL_RELAY_RACE_DEBUFF, player);
                player->CLOSE_GOSSIP_MENU();
            }
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        struct npc_keg_deliveryAI : public ScriptedAI
        {
            npc_keg_deliveryAI(Creature* creature) : ScriptedAI(creature) { }

            void MoveInLineOfSight(Unit* who)
            {
                if (who && who->ToPlayer() && who->HasAura(SPELL_RACING_RAM) && me->GetDistance(who) < 15.0f &&
                   (who->ToPlayer()->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_A) == QUEST_STATUS_INCOMPLETE ||
                    who->ToPlayer()->GetQuestStatus(QUEST_THERE_AND_BACK_AGAIN_H) == QUEST_STATUS_INCOMPLETE ||
                    who->HasAura(SPELL_WORKING_FOR_THE_MAN)))
                {
                    switch (me->GetEntry())
                    {
                        case NPC_FLYNN_FIREBREW:
                        case NPC_BOK_DROPCERTAIN:
                            if (!who->HasAura(SPELL_HAS_KEG))
                            {
                                me->CastSpell(who, SPELL_CREATE_KEG, true);
                                me->CastSpell(who, SPELL_THROW_KEG, true); // visual
                            }
                            break;
                        case NPC_RAM_MASTER_RAY:
                        case NPC_NEILL_RAMSTEIN:
                            if (who->HasAura(SPELL_HAS_KEG))
                            {
                                who->CastSpell(me, SPELL_THROW_KEG_PLAYER, true);
                                who->ToPlayer()->DestroyItemCount(ITEM_PORTABLE_BREWFEST_KEG, 1, true);

                                // rewards
                                if (!who->HasAura(SPELL_WORKING_FOR_THE_MAN))
                                    who->ToPlayer()->KilledMonsterCredit(NPC_DELIVERY_CREDIT, 0);
                                else
                                {
                                    // give 2 tickets
                                    who->CastSpell(who, SPELL_CREATE_TICKETS, true);

                                    // plus 30s ram duration
                                    if (Aura* aura = who->GetAura(SPELL_RENTAL_RACING_RAM))
                                        aura->SetDuration(aura->GetDuration() + 30*IN_MILLISECONDS);
                                }
                            }
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_keg_deliveryAI(creature);
        }
};

class npc_bark_bunny : public CreatureScript
{
    public:
        npc_bark_bunny() : CreatureScript("npc_bark_bunny") { }

        struct npc_bark_bunnyAI : public ScriptedAI
        {
            npc_bark_bunnyAI(Creature* creature) : ScriptedAI(creature) { }

            void MoveInLineOfSight(Unit* who)
            {
                if (who && who->ToPlayer() && who->HasAura(SPELL_RACING_RAM) && me->GetDistance(who) < 20.0f &&
                   (who->ToPlayer()->GetQuestStatus(QUEST_BARK_FOR_THE_1) == QUEST_STATUS_INCOMPLETE ||
                    who->ToPlayer()->GetQuestStatus(QUEST_BARK_FOR_THE_2) == QUEST_STATUS_INCOMPLETE ||
                    who->ToPlayer()->GetQuestStatus(QUEST_BARK_FOR_THE_3) == QUEST_STATUS_INCOMPLETE ||
                    who->ToPlayer()->GetQuestStatus(QUEST_BARK_FOR_THE_4) == QUEST_STATUS_INCOMPLETE))
                {
                    who->ToPlayer()->KilledMonsterCredit(me->GetEntry(), 0);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_bark_bunnyAI(creature);
        }
};

class npc_brew_vendor : public CreatureScript
{
    public:
        npc_brew_vendor() : CreatureScript("npc_brew_vendor") { }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            if (player->HasAchieved(ACHIEV_BREW_OF_THE_MONTH))
                player->GetSession()->SendListInventory(creature->GetGUID());
            else
                player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

            return true;
        }
};

enum DarkIronAttack
{
    GO_FESTIVE_KEG             = 186183, // .. 186187
    GO_MOLE_MACHINE_WRECKAGE_A = 189989,
    GO_MOLE_MACHINE_WRECKAGE_H = 189990,

    NPC_DARK_IRON_GUZZLER      = 23709,
    NPC_DARK_IRON_HERALD       = 24536,

    SPELL_BREWFEST_STUN        = 42435,
    SPELL_MOLE_MACHINE_SPAWN   = 43563
};

class npc_dark_iron_herald : public CreatureScript
{
    public:
        npc_dark_iron_herald() : CreatureScript("npc_dark_iron_herald") { }

        struct npc_dark_iron_heraldAI : public ScriptedAI
        {
            npc_dark_iron_heraldAI(Creature* creature) : ScriptedAI(creature), _summons(me)
            {
                me->setActive(true);
                if (me->isDead())
                    me->Respawn();
            }

            void Reset()
            {
                _eventTimer = 5*MINUTE*IN_MILLISECONDS;
                _spawnTimer = 15*IN_MILLISECONDS;
            }

            void ResetKegs()
            {
                for (uint32 i = GO_FESTIVE_KEG; i < GO_FESTIVE_KEG+5; ++i)
                {
                    GameObject* keg = me->FindNearestGameObject(i, 100.0f);
                    if (keg && keg->GetGoState() == GO_STATE_ACTIVE)
                        keg->SetGoState(GO_STATE_READY);
                }
            }

            GameObject* GetKeg() const
            {
                std::list<GameObject*> tempList;

                // get all valid near kegs
                for (uint32 i = GO_FESTIVE_KEG; i < GO_FESTIVE_KEG+5; ++i)
                {
                    GameObject* keg = me->FindNearestGameObject(i, 100.0f);
                    if (keg && keg->GetGoState() != GO_STATE_ACTIVE)
                        tempList.push_back(keg);
                }

                // select a random one
                if (!tempList.empty())
                {
                    std::list<GameObject*>::iterator itr = tempList.begin();
                    std::advance(itr, urand(0, tempList.size() - 1));
                    if (GameObject* keg = *itr)
                        return keg;
                }

                return NULL;
            }

            void JustSummoned(Creature* summon)
            {
                _summons.Summon(summon);
            }

            void UpdateAI(uint32 const diff)
            {
                if (_eventTimer <= diff)
                {
                    float x, y, z;
                    me->GetPosition(x, y, z);
                    uint32 area = me->GetAreaId();
                    me->SummonGameObject((area == 1) ? GO_MOLE_MACHINE_WRECKAGE_A : GO_MOLE_MACHINE_WRECKAGE_H, x, y, z, 0, 0, 0, 0, 0, 90);

                    _summons.DespawnAll();
                    ResetKegs();
                    me->DisappearAndDie();
                    return;
                }
                else
                    _eventTimer -= diff;

                if (_spawnTimer <= diff)
                {
                    Position spawn;
                    me->GetRandomNearPosition(spawn, 20.0f);

                    if (Creature* guzzler = me->SummonCreature(NPC_DARK_IRON_GUZZLER, spawn))
                    {
                        guzzler->SetReactState(REACT_PASSIVE);
                        guzzler->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                        guzzler->CastSpell(guzzler, SPELL_MOLE_MACHINE_SPAWN, true);
                        guzzler->SetVisible(false);

                        if (GameObject* keg = GetKeg())
                        {
                            Position pos;
                            keg->GetNearPosition(pos, 3.0f, keg->GetAngle(me->GetPositionX(), me->GetPositionZ()) - float(M_PI*rand_norm()));
                            guzzler->GetMotionMaster()->MovePoint(1, pos);
                            guzzler->AI()->SetGUID(keg->GetGUID());
                        }
                        else
                        {
                            _summons.DespawnAll();
                            ResetKegs();
                            me->DisappearAndDie();
                        }
                    }
                    _spawnTimer = urand(1, 4)*IN_MILLISECONDS;
                }
                else
                    _spawnTimer -= diff;
            }

        private:
            SummonList _summons;
            uint32 _eventTimer;
            uint32 _spawnTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dark_iron_heraldAI(creature);
        }
};

class npc_dark_iron_guzzler : public CreatureScript
{
    public:
        npc_dark_iron_guzzler() : CreatureScript("npc_dark_iron_guzzler") { }

        struct npc_dark_iron_guzzlerAI : public ScriptedAI
        {
            npc_dark_iron_guzzlerAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                _kegGUID = 0;
                _waitTimer = 2*IN_MILLISECONDS;
                _destroyTimer = 20*IN_MILLISECONDS;
                _kegReached = false;
                _waiting = true;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_BREWFEST_STUN)
                {
                    me->GetMotionMaster()->Clear();
                    me->Kill(me);
                    me->DespawnOrUnsummon(10*IN_MILLISECONDS);
                    _kegReached = false;
                }
            }

            void SetGUID(uint64 guid, int32 /*id*/ = 0)
            {
                _kegGUID = guid;
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (GameObject* keg = ObjectAccessor::GetGameObject(*me, _kegGUID))
                {
                    _kegReached = true;
                    me->SetFacingToObject(keg);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_ATTACK2H_LOOSE);
                }
                else
                    me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 const diff)
            {
                if (_waiting)
                {
                    if (_waitTimer <= diff)
                    {
                        _waiting = false;
                        me->SetVisible(true);
                    }
                    else
                        _waitTimer -= diff;
                }

                if (_kegReached)
                {
                    GameObject* keg = ObjectAccessor::GetGameObject(*me, _kegGUID);
                    if (!keg || (keg && keg->GetGoState() == GO_STATE_ACTIVE))
                    {
                        me->DespawnOrUnsummon();
                        _kegReached = false;
                        return;
                    }

                    if (_destroyTimer <= diff)
                    {
                        keg->SetGoState(GO_STATE_ACTIVE);
                        me->DespawnOrUnsummon();
                        _kegReached = false;
                    }
                    else
                        _destroyTimer -= diff;
                }
            }

        private:
            uint64 _kegGUID;
            uint32 _destroyTimer;
            uint32 _waitTimer;
            bool _kegReached;
            bool _waiting;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dark_iron_guzzlerAI(creature);
        }
};

enum WildTurkey
{
    SPELL_TURKEY_TRACKER = 62014
};

class npc_wild_turkey : public CreatureScript
{
    public:
        npc_wild_turkey() : CreatureScript("npc_wild_turkey") { }

        struct npc_wild_turkeyAI : public CritterAI
        {
            npc_wild_turkeyAI(Creature* creature) : CritterAI(creature) { }

            void JustDied(Unit* killer)
            {
                if (killer && killer->GetTypeId() == TYPEID_PLAYER)
                    killer->CastSpell(killer, SPELL_TURKEY_TRACKER, true);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_wild_turkeyAI(creature);
        }
};

enum Fireworks
{
    NPC_OMEN                = 15467,
    NPC_MINION_OF_OMEN      = 15466,
    NPC_FIREWORK_BLUE       = 15879,
    NPC_FIREWORK_GREEN      = 15880,
    NPC_FIREWORK_PURPLE     = 15881,
    NPC_FIREWORK_RED        = 15882,
    NPC_FIREWORK_YELLOW     = 15883,
    NPC_FIREWORK_WHITE      = 15884,
    NPC_FIREWORK_BIG_BLUE   = 15885,
    NPC_FIREWORK_BIG_GREEN  = 15886,
    NPC_FIREWORK_BIG_PURPLE = 15887,
    NPC_FIREWORK_BIG_RED    = 15888,
    NPC_FIREWORK_BIG_YELLOW = 15889,
    NPC_FIREWORK_BIG_WHITE  = 15890,

    NPC_CLUSTER_BLUE        = 15872,
    NPC_CLUSTER_RED         = 15873,
    NPC_CLUSTER_GREEN       = 15874,
    NPC_CLUSTER_PURPLE      = 15875,
    NPC_CLUSTER_WHITE       = 15876,
    NPC_CLUSTER_YELLOW      = 15877,
    NPC_CLUSTER_BIG_BLUE    = 15911,
    NPC_CLUSTER_BIG_GREEN   = 15912,
    NPC_CLUSTER_BIG_PURPLE  = 15913,
    NPC_CLUSTER_BIG_RED     = 15914,
    NPC_CLUSTER_BIG_WHITE   = 15915,
    NPC_CLUSTER_BIG_YELLOW  = 15916,
    NPC_CLUSTER_ELUNE       = 15918,

    GO_FIREWORK_LAUNCHER_1  = 180771,
    GO_FIREWORK_LAUNCHER_2  = 180868,
    GO_FIREWORK_LAUNCHER_3  = 180850,
    GO_CLUSTER_LAUNCHER_1   = 180772,
    GO_CLUSTER_LAUNCHER_2   = 180859,
    GO_CLUSTER_LAUNCHER_3   = 180869,
    GO_CLUSTER_LAUNCHER_4   = 180874,

    SPELL_ROCKET_BLUE       = 26344,
    SPELL_ROCKET_GREEN      = 26345,
    SPELL_ROCKET_PURPLE     = 26346,
    SPELL_ROCKET_RED        = 26347,
    SPELL_ROCKET_WHITE      = 26348,
    SPELL_ROCKET_YELLOW     = 26349,
    SPELL_ROCKET_BIG_BLUE   = 26351,
    SPELL_ROCKET_BIG_GREEN  = 26352,
    SPELL_ROCKET_BIG_PURPLE = 26353,
    SPELL_ROCKET_BIG_RED    = 26354,
    SPELL_ROCKET_BIG_WHITE  = 26355,
    SPELL_ROCKET_BIG_YELLOW = 26356,
    SPELL_LUNAR_FORTUNE     = 26522,

    ANIM_GO_LAUNCH_FIREWORK = 3,
    ZONE_MOONGLADE          = 493,
};

Position omenSummonPos = {7558.993f, -2839.999f, 450.0214f, 4.46f};

class npc_firework : public CreatureScript
{
public:
    npc_firework() : CreatureScript("npc_firework") { }

    struct npc_fireworkAI : public ScriptedAI
    {
        npc_fireworkAI(Creature* creature) : ScriptedAI(creature) {}

        bool isCluster()
        {
            switch (me->GetEntry())
            {
                case NPC_FIREWORK_BLUE:
                case NPC_FIREWORK_GREEN:
                case NPC_FIREWORK_PURPLE:
                case NPC_FIREWORK_RED:
                case NPC_FIREWORK_YELLOW:
                case NPC_FIREWORK_WHITE:
                case NPC_FIREWORK_BIG_BLUE:
                case NPC_FIREWORK_BIG_GREEN:
                case NPC_FIREWORK_BIG_PURPLE:
                case NPC_FIREWORK_BIG_RED:
                case NPC_FIREWORK_BIG_YELLOW:
                case NPC_FIREWORK_BIG_WHITE:
                    return false;
                case NPC_CLUSTER_BLUE:
                case NPC_CLUSTER_GREEN:
                case NPC_CLUSTER_PURPLE:
                case NPC_CLUSTER_RED:
                case NPC_CLUSTER_YELLOW:
                case NPC_CLUSTER_WHITE:
                case NPC_CLUSTER_BIG_BLUE:
                case NPC_CLUSTER_BIG_GREEN:
                case NPC_CLUSTER_BIG_PURPLE:
                case NPC_CLUSTER_BIG_RED:
                case NPC_CLUSTER_BIG_YELLOW:
                case NPC_CLUSTER_BIG_WHITE:
                case NPC_CLUSTER_ELUNE:
                default:
                    return true;
            }
        }

        GameObject* FindNearestLauncher()
        {
            GameObject* launcher = NULL;

            if (isCluster())
            {
                GameObject* launcher1 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_1, 0.5f);
                GameObject* launcher2 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_2, 0.5f);
                GameObject* launcher3 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_3, 0.5f);
                GameObject* launcher4 = GetClosestGameObjectWithEntry(me, GO_CLUSTER_LAUNCHER_4, 0.5f);

                if (launcher1)
                    launcher = launcher1;
                else if (launcher2)
                    launcher = launcher2;
                else if (launcher3)
                    launcher = launcher3;
                else if (launcher4)
                    launcher = launcher4;
            }
            else
            {
                GameObject* launcher1 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_1, 0.5f);
                GameObject* launcher2 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_2, 0.5f);
                GameObject* launcher3 = GetClosestGameObjectWithEntry(me, GO_FIREWORK_LAUNCHER_3, 0.5f);

                if (launcher1)
                    launcher = launcher1;
                else if (launcher2)
                    launcher = launcher2;
                else if (launcher3)
                    launcher = launcher3;
            }

            return launcher;
        }

        uint32 GetFireworkSpell(uint32 entry)
        {
            switch (entry)
            {
                case NPC_FIREWORK_BLUE:
                    return SPELL_ROCKET_BLUE;
                case NPC_FIREWORK_GREEN:
                    return SPELL_ROCKET_GREEN;
                case NPC_FIREWORK_PURPLE:
                    return SPELL_ROCKET_PURPLE;
                case NPC_FIREWORK_RED:
                    return SPELL_ROCKET_RED;
                case NPC_FIREWORK_YELLOW:
                    return SPELL_ROCKET_YELLOW;
                case NPC_FIREWORK_WHITE:
                    return SPELL_ROCKET_WHITE;
                case NPC_FIREWORK_BIG_BLUE:
                    return SPELL_ROCKET_BIG_BLUE;
                case NPC_FIREWORK_BIG_GREEN:
                    return SPELL_ROCKET_BIG_GREEN;
                case NPC_FIREWORK_BIG_PURPLE:
                    return SPELL_ROCKET_BIG_PURPLE;
                case NPC_FIREWORK_BIG_RED:
                    return SPELL_ROCKET_BIG_RED;
                case NPC_FIREWORK_BIG_YELLOW:
                    return SPELL_ROCKET_BIG_YELLOW;
                case NPC_FIREWORK_BIG_WHITE:
                    return SPELL_ROCKET_BIG_WHITE;
                default:
                    return 0;
            }
        }

        uint32 GetFireworkGameObjectId()
        {
            uint32 spellId = 0;

            switch (me->GetEntry())
            {
                case NPC_CLUSTER_BLUE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BLUE);
                    break;
                case NPC_CLUSTER_GREEN:
                    spellId = GetFireworkSpell(NPC_FIREWORK_GREEN);
                    break;
                case NPC_CLUSTER_PURPLE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_PURPLE);
                    break;
                case NPC_CLUSTER_RED:
                    spellId = GetFireworkSpell(NPC_FIREWORK_RED);
                    break;
                case NPC_CLUSTER_YELLOW:
                    spellId = GetFireworkSpell(NPC_FIREWORK_YELLOW);
                    break;
                case NPC_CLUSTER_WHITE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_WHITE);
                    break;
                case NPC_CLUSTER_BIG_BLUE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_BLUE);
                    break;
                case NPC_CLUSTER_BIG_GREEN:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_GREEN);
                    break;
                case NPC_CLUSTER_BIG_PURPLE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_PURPLE);
                    break;
                case NPC_CLUSTER_BIG_RED:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_RED);
                    break;
                case NPC_CLUSTER_BIG_YELLOW:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_YELLOW);
                    break;
                case NPC_CLUSTER_BIG_WHITE:
                    spellId = GetFireworkSpell(NPC_FIREWORK_BIG_WHITE);
                    break;
                case NPC_CLUSTER_ELUNE:
                    spellId = GetFireworkSpell(urand(NPC_FIREWORK_BLUE, NPC_FIREWORK_WHITE));
                    break;
            }

            const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);

            if (spellInfo && spellInfo->Effects[0].Effect == SPELL_EFFECT_SUMMON_OBJECT_WILD)
                return spellInfo->Effects[0].MiscValue;

            return 0;
        }

        void Reset()
        {
            if (GameObject* launcher = FindNearestLauncher())
            {
                launcher->SendCustomAnim(ANIM_GO_LAUNCH_FIREWORK);
                me->SetOrientation(launcher->GetOrientation() + M_PI/2);
            }
            else
                return;

            if (isCluster())
            {
                // Check if we are near Elune'ara lake south, if so try to summon Omen or a minion
                if (me->GetZoneId() == ZONE_MOONGLADE)
                {
                    if (!me->FindNearestCreature(NPC_OMEN, 100.0f, false) && me->GetDistance2d(omenSummonPos.GetPositionX(), omenSummonPos.GetPositionY()) <= 100.0f)
                    {
                        switch (urand(0,9))
                        {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                if (Creature* minion = me->SummonCreature(NPC_MINION_OF_OMEN, me->GetPositionX()+frand(-5.0f, 5.0f), me->GetPositionY()+frand(-5.0f, 5.0f), me->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                                    minion->AI()->AttackStart(me->SelectNearestPlayer(20.0f));
                                break;
                            case 9:
                                me->SummonCreature(NPC_OMEN, omenSummonPos);
                                break;
                        }
                    }
                }
                if (me->GetEntry() == NPC_CLUSTER_ELUNE)
                    DoCast(SPELL_LUNAR_FORTUNE);

                float displacement = 0.7f;
                for (uint8 i = 0; i < 4; i++)
                    me->SummonGameObject(GetFireworkGameObjectId(), me->GetPositionX() + (i%2 == 0 ? displacement : -displacement), me->GetPositionY() + (i > 1 ? displacement : -displacement), me->GetPositionZ() + 4.0f, me->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 1);
            }
            else
                //me->CastSpell(me, GetFireworkSpell(me->GetEntry()), true);
                me->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), GetFireworkSpell(me->GetEntry()), true);
        }
    };

    CreatureAI *GetAI(Creature* creature) const
    {
        return new npc_fireworkAI(creature);
    }
};

/*#####
# npc_train_wrecker
#####*/

enum TrainWrecker
{
    GO_TOY_TRAIN_SET    = 193963,
    SPELL_TRAIN_WRECKER = 62943,
    POINT_JUMP          = 1,
    EVENT_SEARCH        = 1,
    EVENT_JUMP          = 2,
    EVENT_WRECK         = 3,
    EVENT_DANCE         = 4
};

class npc_train_wrecker : public CreatureScript
{
    public:
        npc_train_wrecker() : CreatureScript("npc_train_wrecker") { }

        struct npc_train_wreckerAI : public ScriptedAI
        {
            npc_train_wreckerAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset()
            {
                _events.ScheduleEvent(EVENT_SEARCH, 3000);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == POINT_JUMP)
                    _events.ScheduleEvent(EVENT_JUMP, 500);
            }

            void UpdateAI(uint32 const diff)
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SEARCH:
                            if (GameObject* train = me->FindNearestGameObject(GO_TOY_TRAIN_SET, 20.0f))
                            {
                                if (me->GetDistance(train) > 1.5f)
                                {
                                    float x, y, z;
                                    me->GetNearPoint(me, x, y, z, 0.0f, me->GetDistance(train) - 1.5f, me->GetAngle(train));
                                    me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                                    me->GetMotionMaster()->MovePoint(POINT_JUMP, x, y, z);
                                }
                                else
                                    _events.ScheduleEvent(EVENT_JUMP, 500);
                            }
                            else
                                _events.ScheduleEvent(EVENT_SEARCH, 3000);
                            break;
                        case EVENT_JUMP:
                            if (GameObject* train = me->FindNearestGameObject(GO_TOY_TRAIN_SET, 5.0f))
                                me->GetMotionMaster()->MoveJump(train->GetPositionX(), train->GetPositionY(), train->GetPositionZ(), 4.0f, 6.0f);
                            _events.ScheduleEvent(EVENT_WRECK, 2500);
                            break;
                        case EVENT_WRECK:
                            if (GameObject* train = me->FindNearestGameObject(GO_TOY_TRAIN_SET, 5.0f))
                            {
                                DoCast(SPELL_TRAIN_WRECKER);
                                train->SetLootState(GO_JUST_DEACTIVATED); // TODO: fix SPELL_TRAIN_WRECKER's effect
                                _events.ScheduleEvent(EVENT_DANCE, 2500);
                            }
                            else
                                me->DespawnOrUnsummon(3000);
                            break;
                        case EVENT_DANCE:
                            me->HandleEmoteCommand(EMOTE_STATE_DANCE);
                            me->DespawnOrUnsummon(10000);
                            break;
                    }
                }
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_train_wreckerAI(creature);
        }
};

enum hallowen
{
    QUEST_INCOMING_GUMDROP  = 8358,
    EMOTE_TRAIN             = 264,
};

class npc_kali_remik : public CreatureScript
{
public:
    npc_kali_remik() : CreatureScript("npc_kali_remik") { }

    CreatureAI *GetAI(Creature *creature) const
    {
        return new npc_kali_remikAI(creature);
    }

    struct npc_kali_remikAI : public ScriptedAI
    {
        npc_kali_remikAI(Creature* creature) : ScriptedAI(creature) { }

        void ReceiveEmote(Player* player, uint32 emote)
        {
            if (!IsHolidayActive(HOLIDAY_HALLOWS_END))
                return;

            if (player->GetQuestStatus(QUEST_INCOMING_GUMDROP) == QUEST_STATUS_INCOMPLETE && emote == EMOTE_TRAIN)
                player->KilledMonsterCredit(me->GetEntry(),0);
        }
    };
};


struct PostionEventoHallowend
{
    uint32 Area;
    uint8 Area_Count;
    bool AlreadyFired;
    Position SpawnPosition;
}

PostionEventoHallowends[] =
{
     87 ,  34 , false ,  -9456.598633f, 39.966614f, 62.195915f, 0.0f  ,
     87 ,  0 ,  false ,  -9456.162109f, 41.880970f, 62.011547f, 0.0f  ,
     87 ,  0 ,  false ,  -9455.980469f, 43.709557f, 62.056351f, 0.0f  ,
     87 ,  0 ,  false ,  -9461.942383f, 44.572384f, 62.474407f, 0.0f  ,
     87 ,  0 ,  false ,  -9461.984375f, 43.089458f, 62.598511f, 0.0f  ,
     87 ,  0 ,  false ,  -9462.203125f, 40.574947f, 62.672440f, 0.0f  ,
     87 ,  0 ,  false ,  -9475.374023f, 41.268730f, 64.304977f, 0.0f  ,
     87 ,  0 ,  false ,  -9471.819336f, 40.876884f, 64.311752f, 0.0f  ,
     87 ,  0 ,  false ,  -9466.619141f, 40.237621f, 64.321869f, 0.0f  ,
     87 ,  0 ,  false ,  -9462.197266f, 39.733025f, 64.328857f, 0.0f  ,
     87 ,  0 ,  false ,  -9457.063477f, 39.105618f, 64.323448f, 0.0f  ,
     87 ,  0 ,  false ,  -9452.693359f, 36.618862f, 69.778679f, 0.0f  ,
     87 ,  0 ,  false ,  -9456.606445f, 37.412331f, 69.696899f, 0.0f  ,
     87 ,  0 ,  false ,  -9462.572266f, 38.191925f, 69.859924f, 0.0f  ,
     87 ,  0 ,  false ,  -9476.938477f, 42.137085f, 71.250961f, 0.0f  ,
     87 ,  0 ,  false ,  -9479.271484f, 30.612669f, 70.269409f, 0.0f  ,
     87 ,  0 ,  false ,  -9478.808594f, 36.716347f, 64.324905f, 0.0f  ,
     87 ,  0 ,  false ,  -9479.466797f, 31.536119f, 64.346130f, 0.0f  ,
     87 ,  0 ,  false ,  -9479.929688f, 27.957096f, 64.348228f, 0.0f  ,
     87 ,  0 ,  false ,  -9480.361328f, 24.283165f, 64.418297f, 0.0f  ,
     87 ,  0 ,  false ,  -9452.841797f, 35.504967f, 64.328300f, 0.0f  ,
     87 ,  0 ,  false ,  -9453.877930f, 26.877312f, 64.328369f, 0.0f  ,
     87 ,  0 ,  false ,  -9453.395508f, 31.103748f, 64.328857f, 0.0f  ,
     87 ,  0 ,  false ,  -9465.262695f, 83.267693f, 66.926437f, 0.0f  ,
     87 ,  0 ,  false ,  -9466.190430f, 88.508972f, 66.678841f, 0.0f  ,
     87 ,  0 ,  false ,  -9461.586914f, 87.903183f, 68.835785f, 0.0f  ,
     87 ,  0 ,  false ,  -9461.333008f, 83.322281f, 68.954262f, 0.0f  ,
     87 ,  0 ,  false ,  -9457.314453f, 83.163277f, 68.416641f, 0.0f  ,
     87 ,  0 ,  false ,  -9457.511719f, 87.481514f, 68.503716f, 0.0f  ,
     87 ,  0 ,  false ,  -9454.168945f, 87.684364f, 66.910706f, 0.0f  ,
     87 ,  0 ,  false ,  -9453.876953f, 84.042046f, 66.780327f, 0.0f  ,
     87 ,  0 ,  false ,  -9452.500977f, 90.110939f, 66.963150f, 0.0f  ,
     87 ,  0 ,  false ,  -9453.014648f, 97.334198f, 67.853394f, 0.0f  ,
     87 ,  0 ,  false ,  -9466.933594f, 97.318192f, 67.665657f, 0.0f  ,
     131 , 27 , false ,  -5578.172852f, -508.167419f, 404.096893f, 0.0f  ,
     131 ,  0 ,  false ,  -5577.978027f, -512.376343f, 404.096893f, 0.0f  ,
     131 ,  0 ,  false ,  -5578.129395f, -516.255676f, 404.097656f, 0.0f  ,
     131 ,  0 ,  false ,  -5578.432129f, -522.842163f, 404.096008f, 0.0f  ,
     131 ,  0 ,  false ,  -5579.825684f, -505.745697f, 404.096344f, 0.0f  ,
     131 ,  0 ,  false ,  -5584.710449f, -505.517365f, 404.096344f, 0.0f  ,
     131 ,  0 ,  false ,  -5585.525879f, -503.660217f, 413.178284f, 0.0f  ,
     131 ,  0 ,  false ,  -5582.077637f, -503.811554f, 413.373688f, 0.0f  ,
     131 ,  0 ,  false ,  -5577.631836f, -503.804688f, 413.265656f, 0.0f  ,
     131 ,  0 ,  false ,  -5576.027344f, -510.544647f, 413.286865f, 0.0f  ,
     131 ,  0 ,  false ,  -5576.213867f, -517.652893f, 413.300690f, 0.0f  ,
     131 ,  0 ,  false ,  -5592.137695f, -510.345764f, 413.121124f, 0.0f  ,
     131 ,  0 ,  false ,  -5596.859375f, -513.450806f, 413.181976f, 0.0f  ,
     131 ,  0 ,  false ,  -5602.766602f, -513.240051f, 413.275360f, 0.0f  ,
     131 ,  0 ,  false ,  -5609.043457f, -513.394104f, 413.381927f, 0.0f  ,
     131 ,  0 ,  false ,  -5588.285156f, -462.058838f, 414.503693f, 0.0f  ,
     131 ,  0 ,  false ,  -5588.110352f, -458.775116f, 414.375275f, 0.0f  ,
     131 ,  0 ,  false ,  -5588.123535f, -454.830292f, 414.161285f, 0.0f  ,
     131 ,  0 ,  false ,  -5583.522461f, -461.959167f, 414.408997f, 0.0f  ,
     131 ,  0 ,  false ,  -5577.516113f, -462.162292f, 414.318024f, 0.0f  ,
     131 ,  0 ,  false ,  -5572.210938f, -462.232544f, 414.228027f, 0.0f  ,
     131 ,  0 ,  false ,  -5570.547363f, -459.389496f, 414.081726f, 0.0f  ,
     131 ,  0 ,  false ,  -5570.349121f, -454.491058f, 413.896912f, 0.0f  ,
     131 ,  0 ,  false ,  -5573.891113f, -456.134583f, 403.206482f, 0.0f  ,
     131 ,  0 ,  false ,  -5584.926270f, -456.245667f, 403.206238f, 0.0f  ,
     131 ,  0 ,  false ,  -5582.571289f, -461.105408f, 402.603882f, 0.0f  ,
     131 ,  0 ,  false ,  -5576.388672f, -460.653290f, 402.603882f, 0.0f  ,
     3576 ,  28 ,  false ,  -4213.185547f, -12522.341797f, 49.787685f, 0.0f  ,
     3576 ,  0 ,  false ,  -4214.963867f, -12525.010742f, 50.883205f, 0.0f  ,
     3576 ,  0 ,  false ,  -4219.444336f, -12527.609375f, 50.422890f, 0.0f  ,
     3576 ,  0 ,  false ,  -4222.613281f, -12527.093750f, 54.527485f, 0.0f  ,
     3576 ,  0 ,  false ,  -4219.314941f, -12522.365234f, 55.642303f, 0.0f  ,
     3576 ,  0 ,  false ,  -4216.854492f, -12517.195313f, 55.820114f, 0.0f  ,
     3576 ,  0 ,  false ,  -4218.576660f, -12512.222656f, 53.843594f, 0.0f  ,
     3576 ,  0 ,  false ,  -4218.751465f, -12508.384766f, 48.981907f, 0.0f  ,
     3576 ,  0 ,  false ,  -4222.914063f, -12507.796875f, 49.336563f, 0.0f  ,
     3576 ,  0 ,  false ,  -4223.995117f, -12511.844727f, 53.319275f, 0.0f  ,
     3576 ,  0 ,  false ,  -4223.807129f, -12515.146484f, 56.371929f, 0.0f  ,
     3576 ,  0 ,  false ,  -4220.514160f, -12519.626953f, 57.187836f, 0.0f  ,
     3576 ,  0 ,  false ,  -4213.557129f, -12516.336914f, 54.507278f, 0.0f  ,
     3576 ,  0 ,  false ,  -4150.668457f, -12483.409180f, 51.244488f, 0.0f  ,
     3576 ,  0 ,  false ,  -4146.772461f, -12487.382813f, 51.631248f, 0.0f  ,
     3576 ,  0 ,  false ,  -4143.254883f, -12488.479492f, 48.431683f, 0.0f  ,
     3576 ,  0 ,  false ,  -4144.871094f, -12485.075195f, 55.429489f, 0.0f  ,
     3576 ,  0 ,  false ,  -4147.387207f, -12481.426758f, 57.274059f, 0.0f  ,
     3576 ,  0 ,  false ,  -4150.745605f, -12479.188477f, 48.477905f, 0.0f  ,
     3576 ,  0 ,  false ,  -4152.065430f, -12472.902344f, 50.862717f, 0.0f  ,
     3576 ,  0 ,  false ,  -4154.771973f, -12469.408203f, 52.474388f, 0.0f  ,
     3576 ,  0 ,  false ,  -4146.585938f, -12472.744141f, 54.687321f, 0.0f  ,
     3576 ,  0 ,  false ,  -4143.433105f, -12475.541992f, 58.953270f, 0.0f  ,
     3576 ,  0 ,  false ,  -4141.323242f, -12481.124023f, 59.377552f, 0.0f  ,
     3576 ,  0 ,  false ,  -4133.463379f, -12485.035156f, 53.983967f, 0.0f  ,
     3576 ,  0 ,  false ,  -4135.257813f, -12495.936523f, 51.745071f, 0.0f  ,
     3576 ,  0 ,  false ,  -4135.921387f, -12492.143555f, 48.833118f, 0.0f  ,
     3576 ,  0 ,  false ,  -4154.284180f, -12472.077148f, 51.846313f, 0.0f  ,
     362 ,  40 ,  false ,  336.829926f, -4706.948730f, 16.889151f, 0.0f  ,
     362 ,  0 ,  false ,  339.642456f, -4708.543945f, 18.006519f, 0.0f  ,
     362 ,  0 ,  false ,  342.224396f, -4710.393066f, 16.818239f, 0.0f  ,
     362 ,  0 ,  false ,  344.960907f, -4709.553711f, 16.890581f, 0.0f  ,
     362 ,  0 ,  false ,  347.107666f, -4707.392090f, 16.872906f, 0.0f  ,
     362 ,  0 ,  false ,  349.523041f, -4705.116699f, 16.835857f, 0.0f  ,
     362 ,  0 ,  false ,  351.904236f, -4702.746094f, 16.814392f, 0.0f  ,
     362 ,  0 ,  false ,  354.097717f, -4700.604004f, 16.788067f, 0.0f  ,
     362 ,  0 ,  false ,  356.199799f, -4698.549805f, 16.764526f, 0.0f  ,
     362 ,  0 ,  false ,  358.804993f, -4696.005371f, 16.734446f, 0.0f  ,
     362 ,  0 ,  false ,  360.704559f, -4694.149902f, 16.711086f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  332.730408f, -4712.049805f, 15.277740f, 0.0f  ,
     362 ,  0 ,  false ,  317.190308f, -4772.762695f, 27.700096f, 0.0f  ,
     362 ,  0 ,  false ,  313.450867f, -4773.237793f, 27.700096f, 0.0f  ,
     362 ,  0 ,  false ,  310.523895f, -4773.609863f, 27.700096f, 0.0f  ,
     362 ,  0 ,  false ,  307.617767f, -4773.979004f, 27.700096f, 0.0f  ,
     362 ,  0 ,  false ,  303.871460f, -4774.482422f, 27.700096f, 0.0f  ,
     362 ,  0 ,  false ,  300.950226f, -4774.166504f, 27.700096f, 0.0f  ,
     362 ,  0 ,  false ,  297.887360f, -4774.443848f, 27.699116f, 0.0f  ,
     362 ,  0 ,  false ,  310.588226f, -4777.041016f, 23.035875f, 0.0f  ,
     362 ,  0 ,  false ,  302.890533f, -4777.734375f, 22.160141f, 0.0f  ,
     362 ,  0 ,  false ,  303.526398f, -4783.147949f, 22.091099f, 0.0f  ,
     362 ,  0 ,  false ,  311.356537f, -4782.477051f, 22.357243f, 0.0f  ,
     362 ,  0 ,  false ,  311.864349f, -4786.790527f, 22.512373f, 0.0f  ,
     362 ,  0 ,  false ,  303.852966f, -4787.567871f, 22.226116f, 0.0f  ,
     362 ,  0 ,  false ,  304.246216f, -4792.124512f, 22.298147f, 0.0f  ,
     362 ,  0 ,  false ,  312.697449f, -4791.288574f, 22.231749f, 0.0f  ,
     362 ,  0 ,  false ,  284.312897f, -4721.923828f, 13.982411f, 0.0f  ,
     362 ,  0 ,  false ,  282.126221f, -4720.415527f, 14.315186f, 0.0f  ,
     362 ,  0 ,  false ,  279.237915f, -4718.960938f, 14.767025f, 0.0f  ,
     362 ,  0 ,  false ,  277.069824f, -4717.780273f, 16.000343f, 0.0f  ,
     362 ,  0 ,  false ,  276.343201f, -4715.700195f, 17.616024f, 0.0f  ,
     362 ,  0 ,  false ,  276.362915f, -4713.054688f, 18.524426f, 0.0f  ,
     362 ,  0 ,  false ,  278.565430f, -4710.165527f, 18.451063f, 0.0f  ,
     362 ,  0 ,  false ,  280.094330f, -4707.666504f, 18.503857f, 0.0f  ,
     362 ,  0 ,  false ,  282.835510f, -4707.406738f, 15.029071f, 0.0f  ,
     362 ,  0 ,  false ,  284.558929f, -4709.244141f, 14.899632f, 0.0f  ,
     362 ,  0 ,  false ,  287.173279f, -4711.354004f, 14.433277f, 0.0f  ,
     362 ,  0 ,  false ,  289.262390f, -4713.011719f, 14.084586f, 0.0f  ,
     362 ,  0 ,  false ,  283.003418f, -4701.726563f, 17.097305f, 0.0f  ,
     362 ,  0 ,  false ,  280.778656f, -4701.861816f, 19.083368f, 0.0f  ,
     362 ,  0 ,  false ,  270.202362f, -4720.975586f, 15.897130f, 0.0f  ,
     362 ,  0 ,  false ,  270.199463f, -4716.922363f, 19.472425f, 0.0f  ,
     362 ,  0 ,  false ,  274.486603f, -4719.081055f, 15.034330f, 0.0f  ,
     159 ,  59 ,  false ,  2274.941895f, 252.668365f, 41.622330f, 0.0f  ,
     159 ,  0 ,  false ,  2271.454346f, 253.745346f, 45.455288f, 0.0f  ,
     159 ,  0 ,  false ,  2267.237305f, 254.750198f, 45.298695f, 0.0f  ,
     159 ,  0 ,  false ,  2264.763184f, 254.484863f, 41.623123f, 0.0f  ,
     159 ,  0 ,  false ,  2262.695313f, 257.412903f, 41.721722f, 0.0f  ,
     159 ,  0 ,  false ,  2258.168701f, 258.099304f, 41.689083f, 0.0f  ,
     159 ,  0 ,  false ,  2256.925537f, 255.833481f, 41.622211f, 0.0f  ,
     159 ,  0 ,  false ,  2254.367920f, 256.765656f, 45.932198f, 0.0f  ,
     159 ,  0 ,  false ,  2250.811279f, 257.452820f, 45.191032f, 0.0f  ,
     159 ,  0 ,  false ,  2247.595703f, 257.532532f, 41.623463f, 0.0f  ,
     159 ,  0 ,  false ,  2244.849854f, 253.516251f, 41.308601f, 0.0f  ,
     159 ,  0 ,  false ,  2243.304199f, 252.771896f, 40.534016f, 0.0f  ,
     159 ,  0 ,  false ,  2240.819336f, 253.191071f, 40.551826f, 0.0f  ,
     159 ,  0 ,  false ,  2239.991211f, 249.324326f, 40.504696f, 0.0f  ,
     159 ,  0 ,  false ,  2242.890625f, 248.904922f, 40.592785f, 0.0f  ,
     159 ,  0 ,  false ,  2244.179443f, 248.268082f, 41.625122f, 0.0f  ,
     159 ,  0 ,  false ,  2243.473633f, 244.102371f, 41.616474f, 0.0f  ,
     159 ,  0 ,  false ,  2242.311768f, 240.774567f, 45.481342f, 0.0f  ,
     159 ,  0 ,  false ,  2241.650391f, 236.973892f, 45.417168f, 0.0f  ,
     159 ,  0 ,  false ,  2241.770020f, 234.239273f, 41.600002f, 0.0f  ,
     159 ,  0 ,  false ,  2240.255127f, 230.403503f, 46.864922f, 0.0f  ,
     159 ,  0 ,  false ,  2244.803223f, 229.592178f, 46.864922f, 0.0f  ,
     159 ,  0 ,  false ,  2251.706787f, 228.477722f, 46.864922f, 0.0f  ,
     159 ,  0 ,  false ,  2252.351074f, 232.585495f, 49.898830f, 0.0f  ,
     159 ,  0 ,  false ,  2241.279785f, 234.531128f, 49.734783f, 0.0f  ,
     159 ,  0 ,  false ,  2243.065918f, 243.725128f, 49.198402f, 0.0f  ,
     159 ,  0 ,  false ,  2247.495361f, 245.338654f, 48.451134f, 0.0f  ,
     159 ,  0 ,  false ,  2247.403320f, 249.132950f, 47.998882f, 0.0f  ,
     159 ,  0 ,  false ,  2248.053223f, 257.497223f, 47.268879f, 0.0f  ,
     159 ,  0 ,  false ,  2282.821289f, 275.469238f, 48.475018f, 0.0f  ,
     159 ,  0 ,  false ,  2287.594482f, 273.995117f, 48.269260f, 0.0f  ,
     159 ,  0 ,  false ,  2294.277100f, 272.058746f, 48.804775f, 0.0f  ,
     159 ,  0 ,  false ,  2295.394043f, 266.628235f, 50.173767f, 0.0f  ,
     159 ,  0 ,  false ,  2293.394531f, 261.493561f, 49.654194f, 0.0f  ,
     159 ,  0 ,  false ,  2285.579102f, 281.685547f, 49.144688f, 0.0f  ,
     159 ,  0 ,  false ,  2286.483154f, 288.617188f, 47.933754f, 0.0f  ,
     159 ,  0 ,  false ,  2290.901367f, 286.164734f, 49.143620f, 0.0f  ,
     159 ,  0 ,  false ,  2300.947021f, 296.703705f, 48.676743f, 0.0f  ,
     159 ,  0 ,  false ,  2302.537354f, 301.321167f, 48.746048f, 0.0f  ,
     159 ,  0 ,  false ,  2304.679688f, 307.534210f, 49.174549f, 0.0f  ,
     159 ,  0 ,  false ,  2262.712646f, 322.167633f, 40.868729f, 0.0f  ,
     159 ,  0 ,  false ,  2259.568115f, 324.951965f, 42.353420f, 0.0f  ,
     159 ,  0 ,  false ,  2256.455322f, 319.867920f, 42.218792f, 0.0f  ,
     159 ,  0 ,  false ,  2258.497559f, 318.102112f, 41.262493f, 0.0f  ,
     159 ,  0 ,  false ,  2251.768066f, 321.050323f, 45.584774f, 0.0f  ,
     159 ,  0 ,  false ,  2248.273682f, 318.227020f, 45.933731f, 0.0f  ,
     159 ,  0 ,  false ,  2244.188965f, 313.004700f, 44.313381f, 0.0f  ,
     159 ,  0 ,  false ,  2240.801514f, 314.274811f, 44.793324f, 0.0f  ,
     159 ,  0 ,  false ,  2240.160645f, 309.061005f, 46.732929f, 0.0f  ,
     159 ,  0 ,  false ,  2237.880615f, 304.928070f, 46.984474f, 0.0f  ,
     159 ,  0 ,  false ,  2235.998047f, 307.378815f, 47.289276f, 0.0f  ,
     159 ,  0 ,  false ,  2232.723389f, 308.740234f, 46.884659f, 0.0f  ,
     159 ,  0 ,  false ,  2231.217529f, 305.549561f, 45.195446f, 0.0f  ,
     159 ,  0 ,  false ,  2235.197998f, 300.963928f, 44.766743f, 0.0f  ,
     159 ,  0 ,  false ,  2258.797119f, 332.812012f, 45.993340f, 0.0f  ,
     159 ,  0 ,  false ,  2261.823242f, 338.828125f, 44.637466f, 0.0f  ,
     159 ,  0 ,  false ,  2266.796143f, 339.480469f, 42.030144f, 0.0f  ,
     159 ,  0 ,  false ,  2269.283691f, 344.173828f, 41.916660f, 0.0f  ,
     159 ,  0 ,  false ,  2267.883789f, 348.258240f, 43.713596f, 0.0f  ,
     3665 ,  32 ,  false ,  9538.890625f, -6864.580566f, 32.180435f, 0.0f  ,
     3665 ,  0 ,  false ,  9545.317383f, -6864.680664f, 24.290354f, 0.0f  ,
     3665 ,  0 ,  false ,  9539.849609f, -6863.418945f, 21.899117f, 0.0f  ,
     3665 ,  0 ,  false ,  9534.633789f, -6860.602539f, 20.969927f, 0.0f  ,
     3665 ,  0 ,  false ,  9536.678711f, -6862.365234f, 32.320389f, 0.0f  ,
     3665 ,  0 ,  false ,  9531.854492f, -6862.107910f, 33.342640f, 0.0f  ,
     3665 ,  0 ,  false ,  9527.215820f, -6861.431641f, 32.332523f, 0.0f  ,
     3665 ,  0 ,  false ,  9521.734375f, -6861.298340f, 33.342361f, 0.0f  ,
     3665 ,  0 ,  false ,  9516.587891f, -6860.852539f, 32.162258f, 0.0f  ,
     3665 ,  0 ,  false ,  9513.078125f, -6864.157227f, 32.287502f, 0.0f  ,
     3665 ,  0 ,  false ,  9511.090820f, -6862.834473f, 22.868782f, 0.0f  ,
     3665 ,  0 ,  false ,  9517.139648f, -6861.322266f, 20.973946f, 0.0f  ,
     3665 ,  0 ,  false ,  9512.208008f, -6855.778809f, 23.205914f, 0.0f  ,
     3665 ,  0 ,  false ,  9510.145508f, -6851.798828f, 23.008202f, 0.0f  ,
     3665 ,  0 ,  false ,  9505.134766f, -6848.893066f, 23.350080f, 0.0f  ,
     3665 ,  0 ,  false ,  9497.835938f, -6848.266113f, 23.375212f, 0.0f  ,
     3665 ,  0 ,  false ,  9495.145508f, -6849.114746f, 31.737856f, 0.0f  ,
     3665 ,  0 ,  false ,  9494.801758f, -6845.622559f, 32.778965f, 0.0f  ,
     3665 ,  0 ,  false ,  9492.223633f, -6842.419922f, 34.687244f, 0.0f  ,
     3665 ,  0 ,  false ,  9488.187500f, -6838.931152f, 32.722904f, 0.0f  ,
     3665 ,  0 ,  false ,  9485.586914f, -6836.182617f, 33.134155f, 0.0f  ,
     3665 ,  0 ,  false ,  9483.424805f, -6833.842285f, 33.123482f, 0.0f  ,
     3665 ,  0 ,  false ,  9478.472656f, -6833.402832f, 31.652319f, 0.0f  ,
     3665 ,  0 ,  false ,  9479.270508f, -6833.205566f, 23.415434f, 0.0f  ,
     3665 ,  0 ,  false ,  9480.656250f, -6829.862793f, 23.495510f, 0.0f  ,
     3665 ,  0 ,  false ,  9480.732422f, -6824.777832f, 23.296629f, 0.0f  ,
     3665 ,  0 ,  false ,  9478.640625f, -6820.736328f, 23.355219f, 0.0f  ,
     3665 ,  0 ,  false ,  9475.358398f, -6818.134277f, 23.355270f, 0.0f  ,
     3665 ,  0 ,  false ,  9472.142578f, -6816.050781f, 23.065981f, 0.0f  ,
     3665 ,  0 ,  false ,  9482.250977f, -6833.041992f, 17.442104f, 0.0f  ,
     3665 ,  0 ,  false ,  9485.652344f, -6835.646973f, 17.438766f, 0.0f  ,
     3665 ,  0 ,  false ,  9491.399414f, -6842.145508f, 17.439112f, 0.0f  ,
     3665 ,  0 ,  false ,  9495.438477f, -6846.233398f, 17.441633f, 0.0f  ,
     0 ,  0 ,  false ,  0.0f, 0.0f, 0.0f, 0.0f  ,
};

enum HallowendFire
{
    NPC_HEADLESS_HORSEMAN_FIRE_DND          = 23537,
    NPC_SHADE_OF_THE_HORSEMAN               = 23543,
    SPELL_HEADLES_HORSEMAN_QUEST_CREDIT     = 42242,
    SPELL_HEADLESS_HORSEMAN_START_FIRE      = 42132,
    SPELL_BUCKET_LANDS                      = 42339,
    SPELL_HEADLESS_HORSEMAN_FIRE_EXTINGUISH = 42348,
    SPELL_HEADLESS_HORSEMAN_LARGE_JACK      = 44231,
    // RangoFire
    SPELL_HEADLESS_HORSEMAN_BURNING         = 42971,
    SPELL_HEADLESS_HORSEMAN_FIRE            = 42074,
    SPELL_HEADLESS_HORSEMAN_VISUAL_FIRE     = 42075,
    // http://old.wowhead.com/object=186887
    GO_LARGE_JACK                           = 186887,
};

#define ACTION_FAIL_EVENT       1
#define ACTION_START_EVENT      2
#define ACTION_PASS_EVENT       2
#define ACTION_SAY_1            3
#define ACTION_SAY_2            4
#define HALLOWEND_FIRE_ADD      1
#define HALLOWEND_FIRE_REMOVE   2

class npc_hallowend : public CreatureScript
{
public:
    npc_hallowend() : CreatureScript("npc_hallowend") { }
    
    CreatureAI *GetAI(Creature *creature) const
    {
        return new npc_hallowendAI(creature);
    }

    struct npc_hallowendAI : public ScriptedAI
    {
        npc_hallowendAI(Creature* c) : ScriptedAI(c), Fires(c) 
        {
            Area = me->GetAreaId();
            AreaFire = 0;
            while (PostionEventoHallowends[AreaFire].Area && PostionEventoHallowends[AreaFire].Area != Area)
                AreaFire++;
            me->SetVisible(false);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
            for (uint8 j = 0; j < PostionEventoHallowends[AreaFire].Area_Count; j++)
            {
                Creature *summon = me->SummonCreature(NPC_HEADLESS_HORSEMAN_FIRE_DND,PostionEventoHallowends[AreaFire + j].SpawnPosition)->ToCreature();
                if (summon)
                {
                    Fires.Summon(summon);
                    summon->AI()->SetData(0,AreaFire + j);
                }
            }
        }
        
        uint32 TimerBegin;
        uint32 TimerDuration;
        uint32 Area;
        uint32 CountPlayersEvent;
        uint8 AreaFire;
        SummonList Fires;
        bool EventProgress;
        bool SaidPhrase[2];

        void Reset()
        {
            CountPlayersEvent = 0;
            EventProgress = false;
        }
        
        uint32 GetData(uint32 )
        {
            return CountPlayersEvent;
        }

        uint32 PlayersCountRange(float dist) const
        {
            std::list<Player*> players;
            Trinity::AnyPlayerInObjectRangeCheck checker(me, dist);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
            me->VisitNearbyWorldObject(dist, searcher);

            return players.size();
        }
        
        void EventComplete(float dist) const
        {
            std::list<Player*> players;
            Trinity::AnyPlayerInObjectRangeCheck checker(me, dist);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
            me->VisitNearbyWorldObject(dist, searcher);
            me->CastSpell(me->GetPositionX(),me->GetPositionY(), me->GetPositionZ()+1,SPELL_HEADLESS_HORSEMAN_LARGE_JACK,true);
            for (std::list<Player*>::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                (*itr)->CastSpell((*itr),SPELL_HEADLES_HORSEMAN_QUEST_CREDIT,true);
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            switch (uiType)
            {
                case HALLOWEND_FIRE_REMOVE:
                    PostionEventoHallowends[uiData].AlreadyFired = false;
                    bool EventPassed = true;
                    for (uint8 j = 0; j < PostionEventoHallowends[AreaFire].Area_Count; j++) 
                        if (PostionEventoHallowends[AreaFire + j].AlreadyFired)
                            EventPassed = false;
                    if (EventPassed)
                        EventEnd(EventPassed);
                    break;
            }
        }

        void EventBegin()
        {
            CountPlayersEvent = PlayersCountRange(100.0f);

            if (!CountPlayersEvent)
                return;
            EventProgress = true;
            TimerDuration = 400*IN_MILLISECONDS;
            for (uint8 j = 0; j < PostionEventoHallowends[AreaFire].Area_Count; j++) 
                PostionEventoHallowends[AreaFire + j].AlreadyFired = false;
            for (uint8 i = 0; i < 2; i++)
                SaidPhrase[i] = false;
            Fires.DoAction(NPC_HEADLESS_HORSEMAN_FIRE_DND,ACTION_START_EVENT);
            Creature *summon = me->SummonCreature(NPC_SHADE_OF_THE_HORSEMAN,0,0,0)->ToCreature();
            if (summon) 
                Fires.Summon(summon);
        }
        
        void EventEnd(bool EventPassed = false) 
        {
            if (!EventPassed) 
            {
                Fires.DoAction(NPC_HEADLESS_HORSEMAN_FIRE_DND,ACTION_FAIL_EVENT);
                Fires.DoAction(NPC_SHADE_OF_THE_HORSEMAN,ACTION_FAIL_EVENT);
            }
            else 
            {
                EventComplete(100.0f);
                Fires.DoAction(NPC_SHADE_OF_THE_HORSEMAN,ACTION_PASS_EVENT);
            }
            EventProgress = false;
        }
        
        void UpdateAI(const uint32 diff) 
        {
            if (!IsHolidayActive(HOLIDAY_HALLOWS_END))
                return;

            // El evento comienza cada 1/4 de hora. si durante este proceso no hay nadie cerca (10 segs), el evento no comenzara
            // Datos de como funciona Time.  http://www.cs.cf.ac.uk/Dave/C/node21.html   http://pubs.opengroup.org/onlinepubs/009604499/functions/time.html
            // ejemplo una hora son 60 minutos queremos que empieze cada 15 minutos se divide en 4 (60/4) y esto esta en segundos por ende se multiplica por 60 ((60/4)*60)
            if (!EventProgress && (time(NULL)%900 < 5 || (900 - time(NULL)%900) < 5))
                EventBegin();

            if (EventProgress) 
            {
                if (!SaidPhrase[0])
                    if (TimerDuration <= 280*IN_MILLISECONDS)
                    {
                        Fires.DoAction(NPC_SHADE_OF_THE_HORSEMAN,ACTION_SAY_1);
                        SaidPhrase[0] = true;
                    } else 
                        TimerDuration -= diff;
                else 
                    if (!SaidPhrase[1])
                        if (TimerDuration <= 130*IN_MILLISECONDS)
                        {
                            Fires.DoAction(NPC_SHADE_OF_THE_HORSEMAN,ACTION_SAY_2);
                            SaidPhrase[1] = true;
                        } else 
                            TimerDuration -= diff;
                    else 
                        if (TimerDuration <= diff)
                        { 
                            EventEnd();
                        } else TimerDuration -= diff;
            }
        }
    }; 
};

// http://www.wowhead.com/npc=23537
// Headless Horseman - Fire (DND)
class npc_headless_horseman_fire : public CreatureScript
{
public:
    npc_headless_horseman_fire() : CreatureScript("npc_headless_horseman_fire") { }

    CreatureAI *GetAI(Creature *creature) const
    {
        return new npc_headless_horseman_fireAI(creature);
    }

    struct npc_headless_horseman_fireAI : public ScriptedAI
    {
        npc_headless_horseman_fireAI(Creature* c) : ScriptedAI(c)
        {
            RangoFire[0] = SPELL_HEADLESS_HORSEMAN_BURNING;
            RangoFire[1] = SPELL_HEADLESS_HORSEMAN_FIRE;
            RangoFire[2] = SPELL_HEADLESS_HORSEMAN_VISUAL_FIRE;
            Immuned = true;
            Fire = false;
        }

        uint32 RangoFire[3];
        uint32 Ui_ID;
        uint32 RateFire;
        uint32 IncreaseFireTimer;
        bool Fire;
        bool Immuned;
        uint8 curRangoFire;

        void SetData(uint32 type,uint32 uiId)
        {
            Ui_ID = uiId;
        }

        uint32 GetData(uint32 )
        {
            return Ui_ID;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_FAIL_EVENT:
                    Immuned = true;
                    break;
                case ACTION_START_EVENT:
                    Immuned = false;
                    Fire = false;
                    RateFire = 0;
                    curRangoFire = 0;
                    me->RemoveAllAuras();
                    break;
            }
        }

        uint32 PlayersCountRange(float dist) const
        {
            std::list<Player*> players;
            Trinity::AnyPlayerInObjectRangeCheck checker(me, dist);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
            me->VisitNearbyWorldObject(dist, searcher);

            return players.size();
        }

        Creature * FireNodeNext()
        {
           std::list<Creature*> FireNodsList;
           GetCreatureListWithEntryInGrid(FireNodsList, me, NPC_HEADLESS_HORSEMAN_FIRE_DND, 15.0f);
           
           if (!FireNodsList.empty())
           {
               FireNodsList.sort(Trinity::ObjectDistanceOrderPred(me));

               for (std::list<Creature*>::const_iterator itr = FireNodsList.begin(); itr != FireNodsList.end(); ++itr)
                   if (Creature* pNodeFire = *itr) 
                       if (!PostionEventoHallowends[pNodeFire->AI()->GetData(0)].AlreadyFired)
                           return pNodeFire;
           }
           return NULL;
        }

        void SpellHit(Unit* /*caster*/, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_BUCKET_LANDS && Fire && !Immuned) 
            {

                if (PlayersCountRange(5.0f))
                return;

                me->CastSpell(me,SPELL_HEADLESS_HORSEMAN_FIRE_EXTINGUISH,true);
                me->RemoveAura(RangoFire[curRangoFire]);

                if (curRangoFire) 
                {
                    curRangoFire--;
                    me->AddAura(RangoFire[curRangoFire],me);
                }
                else 
                {
                    if (me->isSummon())
                    if (Unit * pEventKeeper =  me->ToTempSummon()->GetSummoner())
                        pEventKeeper->ToCreature()->AI()->SetData(HALLOWEND_FIRE_REMOVE,Ui_ID);
                    Fire = false;
                }
                return;
            }
            if (spell->Id == SPELL_HEADLESS_HORSEMAN_START_FIRE) 
            {
                me->AddAura(RangoFire[0],me);
                PostionEventoHallowends[Ui_ID].AlreadyFired = true;
            }
        }
   
        void UpdateAI(uint32 const diff)
        {
            if (me->HasAura(RangoFire[0]) || Fire)
            {
                if (me->HasAura(RangoFire[0]) && !Fire)
                {
                    if (!RateFire)
                        if (Unit * Owner = me->ToTempSummon()->GetSummoner())
                            RateFire = Owner->ToCreature()->AI()->GetData(0);
                        else 
                            return;
                    if (!RateFire)
                        return;
                    Fire = true;
                    IncreaseFireTimer = 60000 / RateFire;
                } else
                    if (IncreaseFireTimer <= diff)
                    {
                        if (curRangoFire < 2)
                        {
                            me->RemoveAura(RangoFire[curRangoFire]);
                            curRangoFire++;
                            me->AddAura(RangoFire[curRangoFire],me);
                        } else
                            if (Creature * nextFireNode = FireNodeNext())
                            {
                                nextFireNode->AddAura(RangoFire[0],nextFireNode);
                                PostionEventoHallowends[nextFireNode->AI()->GetData(0)].AlreadyFired = true;
                            }
                            IncreaseFireTimer = 60000 / RateFire;
                    } else
                        IncreaseFireTimer -= diff;
            }
        }
    };
};

// http://www.wowhead.com/npc=23543
// Shade of the Horseman

struct WaypointsShadeOfTheHorsemans
{
    uint32 area;
    bool CastPoint;
    Position waypoint;
}

WaypointsShadeOfTheHorsemans[] =
{
      87 ,  false ,  -9449.220703f, 99.542000f, 80.433723f, 0.0f  ,
      87 ,  false ,  -9487.250977f, 96.086182f, 76.224045f, 0.0f  ,
      87 ,  false ,  -9505.733398f, 85.762619f, 78.615189f, 0.0f  ,
      87 ,  false ,  -9515.541016f, 69.045929f, 78.518990f, 0.0f  ,
      87 ,  false ,  -9506.210938f, 49.220688f, 74.766258f, 0.0f  ,
      87 ,  false ,  -9493.707031f, 52.980251f, 73.841759f, 0.0f  ,
      87 ,  false ,  -9466.496094f, 59.418045f, 68.635506f, 0.0f  ,
      87 ,  true ,  -9451.565430f, 60.701469f, 69.817856f, 0.0f  ,
      87 ,  false ,  -9433.534180f, 56.966469f, 69.535995f, 0.0f  ,
      87 ,  false ,  -9426.815430f, 37.612507f, 69.721375f, 0.0f  ,
      87 ,  false ,  -9428.989258f, 20.016560f, 71.739769f, 0.0f  ,
      131 ,  false ,  -5549.735352f, -515.127075f, 417.435242f, 0.0f  ,
      131 ,  false ,  -5563.625977f, -494.025787f, 408.276031f, 0.0f  ,
      131 ,  false ,  -5576.229492f, -484.620575f, 407.339783f, 0.0f  ,
      131 ,  false ,  -5598.328613f, -481.467041f, 405.901550f, 0.0f  ,
      131 ,  false ,  -5618.585449f, -484.472595f, 405.008942f, 0.0f  ,
      131 ,  false ,  -5638.980469f, -484.686829f, 403.672089f, 0.0f  ,
      131 ,  false ,  -5653.769531f, -490.112793f, 404.059021f, 0.0f  ,
      131 ,  false ,  -5648.979492f, -507.761230f, 413.465271f, 0.0f  ,
      131 ,  false ,  -5633.484863f, -515.791504f, 415.236389f, 0.0f  ,
      131 ,  false ,  -5619.904297f, -504.694641f, 412.783020f, 0.0f  ,
      131 ,  true ,  -5599.156738f, -492.679260f, 408.886597f, 0.0f  ,
      131 ,  false ,  -5581.895020f, -482.131866f, 411.817047f, 0.0f  ,
      131 ,  false ,  -5551.732910f, -464.940369f, 418.337616f, 0.0f  ,
      3576 ,  false ,  -4181.044922f, -12545.703125f, 58.406044f, 0.0f  ,
      3576 ,  false ,  -4184.859863f, -12545.989258f, 66.531494f, 0.0f  ,
      3576 ,  false ,  -4162.623535f, -12534.581055f, 60.958271f, 0.0f  ,
      3576 ,  false ,  -4138.957031f, -12509.273438f, 58.346157f, 0.0f  ,
      3576 ,  false ,  -4145.405762f, -12485.465820f, 61.571053f, 0.0f  ,
      3576 ,  false ,  -4170.041016f, -12472.727539f, 62.661823f, 0.0f  ,
      3576 ,  false ,  -4177.905273f, -12487.194336f, 60.646717f, 0.0f  ,
      3576 ,  true ,  -4179.975098f, -12502.522461f, 55.405643f, 0.0f  ,
      3576 ,  false ,  -4187.343750f, -12508.174805f, 56.180138f, 0.0f  ,
      3576 ,  false ,  -4208.107422f, -12512.329102f, 59.904762f, 0.0f  ,
      362 ,  false ,  275.410431f, -4664.126465f, 31.811525f, 0.0f  ,
      362 ,  false ,  289.195679f, -4688.268066f, 28.616646f, 0.0f  ,
      362 ,  true ,  313.071777f, -4734.405762f, 27.163692f, 0.0f  ,
      362 ,  false ,  336.558136f, -4735.292480f, 25.536400f, 0.0f  ,
      362 ,  false ,  364.464661f, -4734.240723f, 23.702780f, 0.0f  ,
      159 ,  false ,  2288.239014f, 370.882111f, 52.932304f, 0.0f  ,
      159 ,  false ,  2283.253174f, 357.807556f, 52.963966f, 0.0f  ,
      159 ,  false ,  2266.459473f, 319.605682f, 52.030453f, 0.0f  ,
      159 ,  true ,  2247.472656f, 284.688538f, 47.141811f, 0.0f  ,
      159 ,  false ,  2241.526123f, 277.526276f, 47.969597f, 0.0f  ,
      159 ,  false ,  2201.554688f, 251.503479f, 52.721680f, 0.0f  ,
      3665 ,  false ,  9553.414063f, -6814.344727f, 48.652557f, 0.0f  ,
      3665 ,  false ,  9545.339844f, -6787.827637f, 44.812103f, 0.0f  ,
      3665 ,  false ,  9524.579102f, -6766.849609f, 41.753448f, 0.0f  ,
      3665 ,  false ,  9502.037109f, -6770.908203f, 40.224274f, 0.0f  ,
      3665 ,  false ,  9486.791992f, -6789.646973f, 38.460819f, 0.0f  ,
      3665 ,  true ,  9505.909180f, -6822.906250f, 36.336525f, 0.0f  ,
      3665 ,  false ,  9530.319336f, -6826.905273f, 33.300465f, 0.0f  ,
      3665 ,  false ,  9546.786133f, -6818.094727f, 34.542740f, 0.0f  ,
      3665 ,  false ,  9550.537109f, -6816.340332f, 34.288609f, 0.0f  ,
      0 ,  false ,  0.0f, 0.0f, 0.0f, 0.0f  
};

class npc_shade_of_the_horseman : public CreatureScript
{
public:
    npc_shade_of_the_horseman() : CreatureScript("npc_shade_of_the_horseman") { }

    CreatureAI *GetAI(Creature *creature) const
    {
        return new npc_shade_of_the_horsemanAI(creature);
    }
    
    struct npc_shade_of_the_horsemanAI : public ScriptedAI
    {
        npc_shade_of_the_horsemanAI(Creature* c) : ScriptedAI(c)
        {
            PointFisrtArea = 0;
            area = me->GetAreaId();
            while (WaypointsShadeOfTheHorsemans[PointFisrtArea].area && WaypointsShadeOfTheHorsemans[PointFisrtArea].area != area)
                PointFisrtArea++;
            TimerEventUI = 400*IN_MILLISECONDS;
            wp_reached = true;
            FlyMode();
            MovementFinished = false;
        }

        uint32 TimerEventUI;
        uint32 CountFire;
        uint32 PointCur;
        uint32 area;
        uint32 PointFisrtArea;
        bool wp_reached;
        bool MovementFinished;

        void FlyMode()
        {
            me->SetVisible(false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT | MOVEMENTFLAG_DISABLE_GRAVITY);
            me->Mount(25159);
            me->SetSpeed(MOVE_WALK,5.0f,true);
            PointCur = 0;
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_FAIL_EVENT:
                    me->PlayDirectSound(11967);
                    me->DisappearAndDie();
                    break;
                case ACTION_PASS_EVENT:
                    me->PlayDirectSound(11968);
                    me->DisappearAndDie();
                    break;
                case ACTION_SAY_1:
                    me->PlayDirectSound(12570);
                    break;
                case ACTION_SAY_2:
                    me->PlayDirectSound(12571);
                    break;
            }
        }

        void CreateFires()
        {
            std::list<Creature*> FireNodsList;
            FireNodsList.clear();
            if (Unit * Owner = me->ToTempSummon()->GetSummoner())
                CountFire = Owner->ToCreature()->AI()->GetData(0);
            GetCreatureListWithEntryInGrid(FireNodsList, me, NPC_HEADLESS_HORSEMAN_FIRE_DND, 150.0f);
            uint32 CountFireer = 0;
            if (!FireNodsList.empty())
            {
                while (CountFireer <= CountFire)
                {
                    std::list<Creature*>::iterator itr = FireNodsList.begin();
                    std::advance(itr, urand(0, FireNodsList.size()-1));
                    me->CastSpell((*itr),SPELL_HEADLESS_HORSEMAN_START_FIRE,true);
                    CountFireer++;
                    FireNodsList.erase(itr);
                }
            }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;
            wp_reached = true;
            if (id == 0) 
                me->SetVisible(true);

            if (WaypointsShadeOfTheHorsemans[PointFisrtArea + PointCur].CastPoint) 
            {
                //CreateFires();
                me->PlayDirectSound(11966);
            }

            PointCur++;
            if (WaypointsShadeOfTheHorsemans[PointFisrtArea + PointCur].area != area) 
            {
                MovementFinished = true;
                me->SetVisible(false);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (wp_reached && !MovementFinished)
            {
                wp_reached = false;
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MovePoint(PointCur,WaypointsShadeOfTheHorsemans[PointFisrtArea + PointCur].waypoint);
            }
        }
    };
};

enum WickermanGuardian
{
    SPELL_KNOCKDOWN                = 19128,
    SPELL_STRIKE                   = 18368,
    SPELL_WICKERMAN_GUARDIAN_EMBER = 25007
};

class npc_wickerman_guardian : public CreatureScript
{
public:
    npc_wickerman_guardian() : CreatureScript("npc_wickerman_guardian") { }

    CreatureAI *GetAI(Creature *creature) const
    {
        return new npc_wickerman_guardianAI(creature);
    }

    struct npc_wickerman_guardianAI : public ScriptedAI
    {
        npc_wickerman_guardianAI(Creature* creature) : ScriptedAI(creature) { }
 
        uint32 KnockdownTimer;
        uint32 StrikeTimer;
 
        void Reset()
        {
            KnockdownTimer = 7000;
            StrikeTimer = 10000;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (KnockdownTimer <= diff)
            {
                DoCast(me->getVictim(),SPELL_KNOCKDOWN);
                KnockdownTimer = 7000;
            }else KnockdownTimer -= diff;

            if (StrikeTimer <= diff)
            {
                DoCast(me->getVictim(),SPELL_STRIKE);
                StrikeTimer = 10000;
            }else StrikeTimer -= diff;
            
            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* killer)
        {
            DoCast(killer, SPELL_WICKERMAN_GUARDIAN_EMBER, true);
        }
    };
};

void AddSC_npcs_special()
{
    new npc_air_force_bots();
    new npc_lunaclaw_spirit();
    new npc_chicken_cluck();
    new npc_dancing_flames();
    new npc_doctor();
    new npc_injured_patient();
    new npc_garments_of_quests();
    new npc_guardian();
    new npc_mount_vendor();
    new npc_rogue_trainer();
    new npc_sayge();
    new npc_steam_tonk();
    new npc_tonk_mine();
    new npc_winter_reveler();
    new npc_brewfest_reveler();
    new npc_snake_trap();
    new npc_mirror_image();
    new npc_ebon_gargoyle();
    new npc_lightwell();
    new mob_mojo();
    new npc_training_dummy();
    new npc_shadowfiend();
    new npc_wormhole();
    new npc_pet_trainer();
    new npc_locksmith();
    new npc_tabard_vendor();
    new npc_experience();
    new npc_torch_tossing_bunny();
    new npc_apple_trap_bunny();
    new npc_keg_delivery();
    new npc_bark_bunny();
    new npc_brew_vendor();
    new npc_dark_iron_herald();
    new npc_dark_iron_guzzler();
    new npc_wild_turkey();
    new npc_firework();
    new npc_spring_rabbit();
    new npc_train_wrecker();
    new npc_kali_remik;
    new npc_hallowend(); 
    new npc_headless_horseman_fire();
    new npc_shade_of_the_horseman();
    new npc_wickerman_guardian;
}