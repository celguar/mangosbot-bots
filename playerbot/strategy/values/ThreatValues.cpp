#include "botpch.h"
#include "../../playerbot.h"
#include "ThreatValues.h"

#include "../../ServerFacade.h"
#include "ThreatManager.h"

using namespace ai;

float MyThreatValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
      
    return ThreatValue::GetThreat(bot, target);
}

float TankThreatValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);

    return ThreatValue::GetTankThreat(ai, target);
}

uint8 ThreatValue::Calculate()
{
    if (qualifier == "aoe")
    {
        uint8 maxThreat = 0;
        list<ObjectGuid> attackers = context->GetValue<list<ObjectGuid>>("possible attack targets")->Get();
        for (list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); i++)
        {
            Unit* unit = ai->GetUnit(*i);
            if (!unit || !sServerFacade.IsAlive(unit))
                continue;

            uint8 threat = Calculate(unit);
            if (!maxThreat || threat > maxThreat)
                maxThreat = threat;
        }

        return maxThreat;
    }

    Unit* target = AI_VALUE(Unit*, qualifier);

    if (target && target->IsFriend(bot))
        target = target->GetTarget();

    return Calculate(target);
}

float ThreatValue::GetThreat(Player* player, Unit* target)
{
    if (!target || target->GetMapId() != player->GetMapId())
        return 0;

    if (dynamic_cast<Player*>(target) && (dynamic_cast<Player*>(target))->IsBeingTeleported())
        return 0;

    if (target->IsFriend(player))
        target = target->GetTarget();

    if (target->GetObjectGuid().IsPlayer())
        return 0;

    float botThreat = sServerFacade.GetThreatManager(target).getThreat(player);

    return botThreat;
}

float ThreatValue::GetTankThreat(PlayerbotAI* ai, Unit* target)
{
    if (!target)
        return 0;

    if (target->IsFriend(ai->GetBot()))
        target = target->GetTarget();

    if (target->GetObjectGuid().IsPlayer())
        return 0;

    Group* group = ai->GetBot()->GetGroup();
    if (!group)
        return 0;

    float maxThreat = -1.0f;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->getSource();
        if (!player || !sServerFacade.IsAlive(player) || !ai->IsSafe(player) || player == ai->GetBot())
            continue;

        if (ai->IsTank(player))
        {
            float threat = sServerFacade.GetThreatManager(target).getThreat(player);
            if (maxThreat < threat)
                maxThreat = threat;
        }
    }

    return maxThreat;
}

uint8 ThreatValue::Calculate(Unit* target)
{
    if (!target)
        return 0;

    if (target->IsFriend(bot))
        target = target->GetTarget();

    if (!target)
        return 0;

    if (target->GetObjectGuid().IsPlayer())
        return 0;

    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    float botThreat, maxThreat;
    if (qualifier == "aoe")
    {
        botThreat = GetThreat(bot, target);
        maxThreat = GetTankThreat(ai, target);
    }
    else
    {
        botThreat = AI_VALUE2(float, "my threat", qualifier);
        maxThreat = AI_VALUE2(float, "tank threat", qualifier);
    }

    if (maxThreat < 0)
        return 0;

    // calculate normal threat for fleeing targets
    bool fleeing = target->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLEEING_MOTION_TYPE ||
        target->GetMotionMaster()->GetCurrentMovementGeneratorType() == TIMED_FLEEING_MOTION_TYPE;

    // return high threat if tank has no threat
    if (target->IsInCombat() && maxThreat == 0 && !fleeing)
        return 100;

    // return low threat if mob if fleeing
    if (maxThreat > 0 && fleeing)
        return 0;

    return botThreat * 100 / maxThreat;
}

/*
bool SpellThreatValue::IsEffectWithImplementedMultiplier(uint32 effectId) const
{
    // TODO: extend this for all effects that do damage and healing
    switch (effectId)
    {
    case SPELL_EFFECT_SCHOOL_DAMAGE:
    case SPELL_EFFECT_HEAL:
    case SPELL_EFFECT_POWER_BURN:
    case SPELL_EFFECT_HEAL_MECHANICAL:
        // weapon based
    case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
    case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
    case SPELL_EFFECT_WEAPON_DAMAGE:
    case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
        return true;
    default:
        return false;
    }
}


int32 SpellThreatValue::SpellDamage(SpellEntry const* spellInfo, Unit* target)
{
    int32 m_damagePerEffect[MAX_EFFECT_INDEX], damagePerEffect[MAX_EFFECT_INDEX];
    float m_damageMultipliers[MAX_EFFECT_INDEX];
    int32 damage;
    Spell* spell;

    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        uint8 eff = spellInfo->Effect[i];

        if (spell->IsEffectWithImplementedMultiplier(eff))
        {
            m_damagePerEffect[i] = 0;
            damage = spell->CalculateSpellEffectValue(i, target);
        }
        else
            damage = int32(spell->CalculateSpellEffectValue(i, target) * m_damageMultipliers[i]);

        if (eff < MAX_SPELL_EFFECTS)
        {
            damagePerEffect[i] = damage;
                damagePerEffect[i] = damage;
                m_damagePerEffect[i] = spell->CalculateSpellEffectDamage(target, damage);
            }
        }
        else
            sLog.outError("WORLD: Spell FX %d > MAX_SPELL_EFFECTS ", eff);

        if (IsEffectWithImplementedMultiplier(eff))
        {
            if (m_damagePerEffect[i] > 0)
                m_damage += int32(m_damagePerEffect[i] * DamageMultiplier);
        }

        // Get multiplier
        float multiplier = spellInfo->DmgMultiplier[i];
        // Apply multiplier mods
        if (bot)
            if (Player* modOwner = bot->GetSpellModOwner())
                modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_EFFECT_PAST_FIRST, multiplier);
        m_damageMultipliers[i] *= multiplier;
    }
}


float SpellThreatValue::Calculate()
{
    int32 spellId = stoi(qualifier);

    Unit* target = AI_VALUE(Unit*, "current target");

    if (!target)
        return 0.0f;

    if (target->GetObjectGuid().IsPlayer())
        return 0.0f;

    //Lifted from Spell::HandleThreatSpells()
    SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(spellId);

    Spell* spell = new Spell(bot, spellInfo, false);  
    spell->m_targets.setUnitTarget(target);
    
    
    float threat = spell->GetDamage();

    SpellThreatEntry const* threatEntry = sSpellMgr.GetSpellThreatEntry(spellInfo->Id);
    if (threatEntry && threatEntry->threat && threatEntry->ap_bonus > 0.0f)
        threat = threatEntry->threat;

    if (threatEntry->ap_bonus != 0.0f)
        threat += threatEntry->ap_bonus * bot->GetTotalAttackPowerValue(GetWeaponAttackType(spellInfo));

    float calculatedThreat = ThreatCalcHelper::CalcThreat(target, bot, threat, false, GetSpellSchoolMask(spellInfo), spellInfo);

    float botThreat = sServerFacade.GetThreatManager(target).getThreat(bot);

    return botThreat;
}
*/