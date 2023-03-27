#include "botpch.h"
#include "../../playerbot.h"
#include "CheatAction.h"

using namespace ai;



bool CheatAction::Execute(Event& event)
{
    string param = event.getParam();

    uint32 cheatMask = (uint32)ai->GetCheat();

    vector<string> splitted = split(param, ',');
    for (vector<string>::iterator i = splitted.begin(); i != splitted.end(); i++)
    {
        const char* name = i->c_str();
        switch (name[0])
        {
        case '+':
            cheatMask |= (uint32)GetCheatMask(name + 1);
            break;
        case '-':
            cheatMask &= ~(uint32)GetCheatMask(name + 1);
            break;
        case '~':
            cheatMask ^= (uint32)GetCheatMask(name + 1);
            break;
        case '?':
            ListCheats();
            return true;
        }
    }

    ai->SetCheat(BotCheatMask(cheatMask));

    return true;
}

BotCheatMask CheatAction::GetCheatMask(string cheat)
{
    vector<string> cheatName = { "taxi", "gold", "health", "mana", "power", "item", "cooldown", "repair", "maxMask" };
    for (int i = 0; i < log2((uint32)BotCheatMask::maxMask); i++)
    {
        if (cheatName[i] == cheat)
            return BotCheatMask(1 << i);
    }

    return BotCheatMask::none;
}

string CheatAction::GetCheatName(BotCheatMask cheatMask)
{
    vector<string> cheatName = { "none", "taxi", "gold", "health", "mana", "power", "item", "cooldown", "repair", "maxMask" };
    return cheatName[log2((uint32)cheatMask)];
}

void CheatAction::ListCheats()
{
    ostringstream out;
    for (int i = 0; i < log2((uint32)BotCheatMask::maxMask); i++)
    {
        BotCheatMask cheatMask = BotCheatMask(1 << i);
       if ((uint32)cheatMask & (uint32)sPlayerbotAIConfig.botCheatMask)
           out << "[conf:" << GetCheatName(BotCheatMask(cheatMask)) << "]";
       else if (ai->HasCheat(cheatMask))
           out << "[" << GetCheatName(BotCheatMask(cheatMask)) << "]";
    }

    ai->TellMasterNoFacing(out);
}