/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptPCH.h"
#include "naxxramas.h"

enum ScriptTexts
{
    SAY_AGGRO                      = 0,             
    SAY_SUMMON                     = 1,             
    SAY_SLAY                       = 2,               
    SAY_DEATH                      = 3,
    EMOTE_SUMMON                   = 4,
    EMOTE_BALCONY                  = 5,
    EMOTE_SKELETON                 = 6,
    EMOTE_TELEPORT                 = 7,	
};

enum Spells
{
    SPELL_CURSE_PLAGUEBRINGER_10   = 29213,
    SPELL_CURSE_PLAGUEBRINGER_25   = 54835,
    SPELL_BLINK                    = 29211,
    SPELL_CRIPPLE_10               = 29212,
    SPELL_CRIPPLE_25               = 54814,
    SPELL_TELEPORT                 = 29216,
};

const Position TelePos = {2631.370f, -3529.680f, 274.040f, 6.277f};

#define MOB_WARRIOR         16984
#define MOB_CHAMPION        16983
#define MOB_GUARDIAN        16981
#define MAX_SUMMON_POS 5

const float SummonPos[MAX_SUMMON_POS][4] =
{
    {2728.12f, -3544.43f, 261.91f, 6.04f},
    {2729.05f, -3544.47f, 261.91f, 5.58f},
    {2728.24f, -3465.08f, 264.20f, 3.56f},
    {2704.11f, -3456.81f, 265.53f, 4.51f},
    {2663.56f, -3464.43f, 262.66f, 5.20f},
};

enum Events
{
    EVENT_NONE,
    EVENT_BERSERK,
    EVENT_CURSE,
    EVENT_BLINK,
    EVENT_WARRIOR,
    EVENT_BALCONY,
    EVENT_WAVE,
    EVENT_GROUND,
};

class boss_noth : public CreatureScript
{
public:
    boss_noth() : CreatureScript("boss_noth") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_nothAI (creature);
    }

    struct boss_nothAI : public BossAI
    {
        boss_nothAI(Creature* creature) : BossAI(creature, BOSS_NOTH) {}

        uint32 waveCount, balconyCount;

        void Reset()
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            _Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            balconyCount = 0;
            EnterPhaseGround();
        }

        void EnterPhaseGround()
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoZoneInCombat();
            if (me->getThreatManager().isThreatListEmpty())
                EnterEvadeMode();
            else
            {
                events.ScheduleEvent(EVENT_BALCONY, 110000);
                events.ScheduleEvent(EVENT_CURSE, 10000+rand()%15000);
                events.ScheduleEvent(EVENT_WARRIOR, 30000);
                if (GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL)
                    events.ScheduleEvent(EVENT_BLINK, urand(20000, 40000));
            }
        }

        void KilledUnit(Unit* /*victim*/)
        {
                Talk(SAY_SLAY);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            summon->setActive(true);
            summon->AI()->DoZoneInCombat();
        }

        void JustDied(Unit* /*Killer*/)
        {
            _JustDied();
            Talk(SAY_DEATH);
        }

        void SummonUndead(uint32 entry, uint32 num)
        {
            for (uint32 i = 0; i < num; ++i)
            {
                uint32 pos = rand()%MAX_SUMMON_POS;
                me->SummonCreature(entry, SummonPos[pos][0], SummonPos[pos][1], SummonPos[pos][2],
                    SummonPos[pos][3], TEMPSUMMON_CORPSE_DESPAWN, 60000);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() || !CheckInRoom())
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CURSE:
                        DoCastAOE(RAID_MODE(SPELL_CURSE_PLAGUEBRINGER_10, SPELL_CURSE_PLAGUEBRINGER_25));
                        events.ScheduleEvent(EVENT_CURSE, urand(50000, 60000));
                        return;
                    case EVENT_WARRIOR:
                        Talk(SAY_SUMMON);
                        Talk(EMOTE_SUMMON);
                        SummonUndead(MOB_WARRIOR, RAID_MODE(2, 3));
                        events.ScheduleEvent(EVENT_WARRIOR, 30000);
                        return;
                    case EVENT_BLINK:
                        Talk(EVENT_BALCONY);
                        DoCastAOE(RAID_MODE(SPELL_CRIPPLE_10, SPELL_CRIPPLE_25));
                        DoCastAOE(SPELL_BLINK);
                        DoResetThreat();
                        events.ScheduleEvent(EVENT_BLINK, 40000);
                        return;
                    case EVENT_BALCONY:
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        me->AttackStop();
                        me->RemoveAllAuras();
                        me->NearTeleportTo(TelePos.m_positionX,TelePos.m_positionY,TelePos.m_positionZ,TelePos.m_orientation);
                        events.Reset();
                        events.ScheduleEvent(EVENT_WAVE, urand(2000, 5000));
                        waveCount = 0;
                        return;
                    case EVENT_WAVE:
                        Talk(EMOTE_SKELETON);
                        switch (balconyCount)
                        {
                            case 0: SummonUndead(MOB_CHAMPION, RAID_MODE(2, 4)); break;
                            case 1: SummonUndead(MOB_CHAMPION, RAID_MODE(1, 2));
                                    SummonUndead(MOB_GUARDIAN, RAID_MODE(1, 2)); break;
                            case 2: SummonUndead(MOB_GUARDIAN, RAID_MODE(2, 4)); break;
                            default:SummonUndead(MOB_CHAMPION, RAID_MODE(5, 10));
                                    SummonUndead(MOB_GUARDIAN, RAID_MODE(5, 10));break;
                        }
                        ++waveCount;
                        events.ScheduleEvent(waveCount < 2 ? EVENT_WAVE : EVENT_GROUND, urand(30000, 45000));
                        return;
                    case EVENT_GROUND:
                    {
                        Talk(EMOTE_TELEPORT);
                        ++balconyCount;
                        float x, y, z, o;
                        me->GetHomePosition(x, y, z, o);
                        me->NearTeleportTo(x, y, z, o);
                        events.ScheduleEvent(EVENT_BALCONY, 110000);
                        EnterPhaseGround();
                        return;
                    }
                }
            }

            if (me->HasReactState(REACT_AGGRESSIVE))
                DoMeleeAttackIfReady();
        }
    };

};

void AddSC_boss_noth()
{
    new boss_noth();
}
