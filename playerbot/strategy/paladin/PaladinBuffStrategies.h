#pragma once

#include "GenericPaladinStrategy.h"

namespace ai
{
    class PaladinBuffWisdomStrategy : public Strategy
    {
    public:
		PaladinBuffWisdomStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "bwisdom"; }
    };

    class PaladinBuffMightStrategy : public Strategy
    {
    public:
		PaladinBuffMightStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "bmight"; }
    };

	class PaladinBuffArmorStrategy : public Strategy
	{
	public:
		PaladinBuffArmorStrategy(PlayerbotAI* ai) : Strategy(ai) {}

	public:
		virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
		virtual string getName() { return "barmor"; }
	};

	class PaladinBuffConcentrationStrategy : public Strategy
	{
	public:
		PaladinBuffConcentrationStrategy(PlayerbotAI* ai) : Strategy(ai) {}

	public:
		virtual void InitTriggers(std::vector<TriggerNode*> & triggers);
		virtual string getName() { return "bconcentration"; }
	};

	class PaladinBuffSanctityStrategy : public Strategy
	{
	public:
		PaladinBuffSanctityStrategy(PlayerbotAI* ai) : Strategy(ai) {}

	public:
		virtual void InitTriggers(std::vector<TriggerNode*> & triggers);
		virtual string getName() { return "bsanctity"; }
	};

    class PaladinBuffAoeStrategy : public Strategy
    {
    public:
        PaladinBuffAoeStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "baoe"; }
    };

	class PaladinBuffThreatStrategy : public Strategy
	{
	public:
		PaladinBuffThreatStrategy(PlayerbotAI* ai) : Strategy(ai) {}

	public:
		virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
		virtual string getName() { return "bthreat"; }
	};

	class PaladinBuffSpeedStrategy : public Strategy
	{
	public:
		PaladinBuffSpeedStrategy(PlayerbotAI* ai) : Strategy(ai) {}

	public:
		virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
		virtual string getName() { return "crusader"; }
	};

    class PaladinBuffKingsStrategy : public Strategy
    {
    public:
		PaladinBuffKingsStrategy(PlayerbotAI* ai) : Strategy(ai) {}

    public:
        virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
        virtual string getName() { return "bkings"; }
    };

	class PaladinShadowResistanceStrategy : public Strategy
	{
	public:
		PaladinShadowResistanceStrategy(PlayerbotAI* ai) : Strategy(ai) {}

	public:
		virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
		virtual string getName() { return "rshadow"; }
	};

	class PaladinFrostResistanceStrategy : public Strategy
	{
	public:
		PaladinFrostResistanceStrategy(PlayerbotAI* ai) : Strategy(ai) {}

	public:
		virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
		virtual string getName() { return "rfrost"; }
	};

	class PaladinFireResistanceStrategy : public Strategy
	{
	public:
		PaladinFireResistanceStrategy(PlayerbotAI* ai) : Strategy(ai) {}

	public:
		virtual void InitTriggers(std::vector<TriggerNode*> &triggers);
		virtual string getName() { return "rfire"; }
	};
}
