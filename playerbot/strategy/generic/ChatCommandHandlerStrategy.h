#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class ChatCommandHandlerStrategy : public PassTroughStrategy
    {
    public:
        ChatCommandHandlerStrategy(PlayerbotAI* ai);

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "chat"; }
    };
}
