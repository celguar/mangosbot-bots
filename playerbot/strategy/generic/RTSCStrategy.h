#pragma once

namespace ai
{
    class RTSCStrategy : public Strategy
    {
    public:
        RTSCStrategy(PlayerbotAI* ai);
        virtual string getName() { return "RTSC"; }

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
    };

}
