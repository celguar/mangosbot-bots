#pragma once

#include "../PlayerbotAIAware.h"
#include "Action.h"
#include "Value.h"
#include "NamedObjectContext.h"
#include "Strategy.h"

namespace ai
{
    class UntypedValue;
    template<class T> class Value;
}

namespace ai
{
    class AiObjectContext : public PlayerbotAIAware
    {
    public:
        AiObjectContext(PlayerbotAI* ai);
        virtual ~AiObjectContext() {}

    public:
        virtual Strategy* GetStrategy(string name) { return strategyContexts.GetObject(name, ai); }
        virtual set<string> GetSiblingStrategy(string name) { return strategyContexts.GetSiblings(name); }
        virtual Trigger* GetTrigger(string name) { return triggerContexts.GetObject(name, ai); }
        virtual Action* GetAction(string name) { return actionContexts.GetObject(name, ai); }
        virtual UntypedValue* GetUntypedValue(string name) { return valueContexts.GetObject(name, ai); }

        template<class T>
        Value<T>* GetValue(string name)
        {
            return dynamic_cast<Value<T>*>(GetUntypedValue(name));
        }

        template<class T>
        Value<T>* GetValue(string name, string param)
        {
            return GetValue<T>((string(name) + "::" + param));
        }

        template<class T>
        Value<T>* GetValue(string name, int32 param)
        {
        	ostringstream out; out << param;
            return GetValue<T>(name, out.str());
        }

        set<string> GetValues()
        {
            return valueContexts.GetCreated();
        }

        set<string> GetSupportedStrategies()
        {
            return strategyContexts.supports();
        }

        set<string> GetSupportedTriggers()
        {
            return triggerContexts.supports();
        }

        set<string> GetSupportedActions()
        {
            return actionContexts.supports();
        }

        set<string> GetSupportedValues ()
        {
            return valueContexts.supports();
        }

        string FormatValues(string findName = "");

    public:
        virtual void Update();
        virtual void Reset();
        virtual void AddShared(NamedObjectContext<UntypedValue>* sharedValues)
        {
            valueContexts.Add(sharedValues);
        }
        list<string> Save();
        void Load(list<string> data);

        vector<string> performanceStack;
    protected:
        NamedObjectContextList<Strategy> strategyContexts;
        NamedObjectContextList<Action> actionContexts;
        NamedObjectContextList<Trigger> triggerContexts;
        NamedObjectContextList<UntypedValue> valueContexts;
    };
}
