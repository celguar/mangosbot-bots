#include "../../botpch.h"

#include "Engine.h"

#include "../playerbot.h"
#include "../PlayerbotAIConfig.h"
#include "../PerformanceMonitor.h"


using namespace ai;
using namespace std;

Engine::Engine(PlayerbotAI* ai, AiObjectContext *factory)
    : PlayerbotAIAware{ ai }
    , aiObjectContext{ factory }
    , lastRelevance{ 0.0f }
    , testMode{ false }
{
}

bool ActionExecutionListeners::Before(Action* action, Event event)
{
    bool result = true;
    for (const auto& ref : listeners)
    {
        result &= ref->Before(action, event);
    }
    return result;
}

void ActionExecutionListeners::After(Action* action, bool executed, Event event)
{
    for (const auto& ref : listeners)
    {
        ref->After(action, executed, event);
    }
}

bool ActionExecutionListeners::OverrideResult(Action* action, bool executed, Event event)
{
    bool result = executed;
    for (const auto& ref : listeners)
    {
        result = ref->OverrideResult(action, result, event);
    }
    return result;
}

bool ActionExecutionListeners::AllowExecution(Action* action, Event event)
{
    bool result = true;
    for (const auto& ref : listeners)
    {
        result &= ref->AllowExecution(action, event);
    }
    return result;
}

ActionExecutionListeners::~ActionExecutionListeners()
{
    for (auto ptr : listeners)
    {
        delete ptr;
    }
    listeners.clear();
}


Engine::~Engine(void)
{
    Reset();

    strategies.clear();
}

void Engine::Reset()
{
    ActionNode* action = nullptr;
    do
    {
        action = queue.Pop();
        if (!action) break;
        delete action;
    } while (true);

    for (auto ptr : triggers)
    {
        delete ptr;
    }
    triggers.clear();

    for (auto ptr : multipliers)
    {
        delete ptr;
    }
    multipliers.clear();
}

void Engine::Init()
{
    Reset();

    for (auto [index, strategy] : strategies)
    {
        strategy->InitMultipliers(multipliers);
        strategy->InitTriggers(triggers);
        Event emptyEvent;
        MultiplyAndPush(strategy->getDefaultActions(), 0.0f, false, emptyEvent, "default");
    }

	if (testMode)
	{
        FILE* file = fopen("test.log", "w");
        fprintf(file, "\n");
        fclose(file);
	}
}


bool Engine::DoNextAction(Unit* unit, int depth, bool minimal)
{
    LogAction("--- AI Tick ---");
    if (sPlayerbotAIConfig.logValuesPerTick)
        LogValues();

    bool actionExecuted = false;
    ActionBasket* basket = NULL;

    time_t currentTime = time(0);
    aiObjectContext->Update();
    ProcessTriggers(minimal);

    int iterations = 0;
    int iterationsPerTick = queue.Size() * (minimal ? (uint32)(sPlayerbotAIConfig.iterationsPerTick / 2) : sPlayerbotAIConfig.iterationsPerTick);
    do {
        basket = queue.Peek();
        if (basket) {
            float relevance = basket->getRelevance(); // just for reference
            bool skipPrerequisites = basket->isSkipPrerequisites();
            Event event = basket->getEvent();
            if (minimal && (relevance < 100))
                continue;
            // NOTE: queue.Pop() deletes basket
            ActionNode* actionNode = queue.Pop();
            Action* action = InitializeAction(actionNode);

            if(action)
                action->setRelevance(relevance);

            if (!action)
            {
                LogAction("A:%s - UNKNOWN", actionNode->getName().c_str());
            }
            else if (action->isUseful())
            {
                for (auto& multiplier : multipliers)
                {
                    relevance *= multiplier->GetValue(action);
                    action->setRelevance(relevance);
                    if (!relevance)
                    {
                        LogAction("Multiplier %s made action %s useless", multiplier->getName().c_str(), action->getName().c_str());
                        break;
                    }
                }

                if (action->isPossible() && relevance)
                {
                    if (!skipPrerequisites)
                    {
                        LogAction("A:%s - PREREQ", action->getName().c_str());
                        if (MultiplyAndPush(actionNode->getPrerequisites(), relevance + 0.02, false, event, "prereq"))
                        {
                            PushAgain(actionNode, relevance + 0.01, event);
                            continue;
                        }
                    }

                    PerformanceMonitorOperation *pmo = sPerformanceMonitor.start(PERF_MON_ACTION, action->getName(), &aiObjectContext->performanceStack);
                    actionExecuted = ListenAndExecute(action, event);
                    if (pmo) pmo->finish();

                    if (actionExecuted)
                    {
                        LogAction("A:%s - OK", action->getName().c_str());
                        MultiplyAndPush(actionNode->getContinuers(), 0, false, event, "cont");
                        lastRelevance = relevance;
                        delete actionNode;
                        break;
                    }
                    else
                    {
                        LogAction("A:%s - FAILED", action->getName().c_str());
                        MultiplyAndPush(actionNode->getAlternatives(), relevance + 0.03, false, event, "alt");
                    }
                }
                else
                {
                    if (ai->HasStrategy("debug action", BOT_STATE_NON_COMBAT))
                    {
                        ostringstream out;
                        out << "try: ";
                        out << action->getName();
                        out << " impossible (";

                        out << action->getRelevance() << ")";

                        if (!event.getSource().empty())
                            out << " [" << event.getSource() << "]";

                        ai->TellMasterNoFacing(out);
                    }
                    LogAction("A:%s - IMPOSSIBLE", action->getName().c_str());
                    MultiplyAndPush(actionNode->getAlternatives(), relevance + 0.03, false, event, "alt");
                }
            }
            else
            {
                if (ai->HasStrategy("debug action", BOT_STATE_NON_COMBAT))
                {
                    ostringstream out;
                    out << "try: ";
                    out << action->getName();
                    out << " useless (";

                    out << action->getRelevance() << ")";

                    if (!event.getSource().empty())
                        out << " [" << event.getSource() << "]";

                    ai->TellMasterNoFacing(out);
                }
                lastRelevance = relevance;
                LogAction("A:%s - USELESS", action->getName().c_str());
            }
            delete actionNode;
        }
    }
    while (basket && ++iterations <= iterationsPerTick);

    if (!basket)
    {
        lastRelevance = 0.0f;
        PushDefaultActions();
        if (queue.Peek() && depth < 2)
            return DoNextAction(unit, depth + 1, minimal);
    }

    // MEMORY FIX TEST
 /*   do {
        basket = queue.Peek();
        if (basket) {
            // NOTE: queue.Pop() deletes basket
            delete queue.Pop();
        }
    } while (basket);*/

    if (time(0) - currentTime > 1) {
        LogAction("too long execution");
    }

    if (!actionExecuted)
        LogAction("no actions executed");

    queue.RemoveExpired();
    return actionExecuted;
}

ActionNode* Engine::CreateActionNode(string name)
{
    for (auto [index, strategy] : strategies)
    {
        ActionNode* node = strategy->GetAction(name);
        if (node)
            return node;
    }
    return new ActionNode (name,
        /*P*/ NULL,
        /*A*/ NULL,
        /*C*/ NULL);
}

bool Engine::MultiplyAndPush(NextAction** actions, float forceRelevance, bool skipPrerequisites, Event event, const char* pushType)
{
    bool pushed = false;
    if (actions)
    {
        for (int j=0; actions[j]; j++)
        {
            NextAction* nextAction = actions[j];
            if (nextAction)
            {
                ActionNode* action = CreateActionNode(nextAction->getName());
                InitializeAction(action);

                float k = nextAction->getRelevance();
                if (forceRelevance > 0.0f)
                {
                    k = forceRelevance;
                }

                if (k > 0)
                {
                    LogAction("PUSH:%s - %f (%s)", action->getName().c_str(), k, pushType);
                    queue.Push(new ActionBasket(action, k, skipPrerequisites, event));
                    pushed = true;
                }
                else
                {
                    delete action;
                }

                delete nextAction;
            }
            else
                break;
        }
        delete[] actions;
    }
    return pushed;
}

ActionResult Engine::ExecuteAction(string name, Event event, string qualifier)
{
	bool result = false;

    ActionNode *actionNode = CreateActionNode(name);
    if (!actionNode)
        return ACTION_RESULT_UNKNOWN;

    Action* action = InitializeAction(actionNode);
    if (!action)
    {
        delete actionNode;
        return ACTION_RESULT_UNKNOWN;
    }



    if (!qualifier.empty())
    {
        Qualified* q = dynamic_cast<Qualified*>(action);

        if (q)
            q->Qualify(qualifier);
    }

    if (!action->isPossible())
    {
        delete actionNode;
        return ACTION_RESULT_IMPOSSIBLE;
    }

    if (!action->isUseful())
    {
        delete actionNode;
        return ACTION_RESULT_USELESS;
    }

    action->MakeVerbose();
    result = ListenAndExecute(action, event);
    MultiplyAndPush(action->getContinuers(), 0.0f, false, event, "default");
    delete actionNode;
	return result ? ACTION_RESULT_OK : ACTION_RESULT_FAILED;
}

void Engine::addStrategy(const string& name)
{
    removeStrategy(name);

    Strategy* strategy = aiObjectContext->GetStrategy(name);
    if (strategy)
    {
        set<string> siblings = aiObjectContext->GetSiblingStrategy(name);
        for (set<string>::iterator i = siblings.begin(); i != siblings.end(); i++)
            removeStrategy(*i);

        LogAction("S:+%s", strategy->getName().c_str());
        strategies[strategy->getName()] = strategy;
    }
    Init();
}

bool Engine::removeStrategy(const std::string& name)
{
    map<string, Strategy*>::iterator i = strategies.find(name);
    if (i == strategies.end())
        return false;

    LogAction("S:-%s", name);
    strategies.erase(i);
    Init();
    return true;
}

void Engine::removeAllStrategies()
{
    strategies.clear();
    Init();
}

void Engine::toggleStrategy(const std::string& name)
{
    if (!removeStrategy(name))
        addStrategy(name);
}

bool Engine::HasStrategy(const std::string& name)
{
    return strategies.find(name) != strategies.end();
}

void Engine::ProcessTriggers(bool minimal)
{
    map<Trigger*, Event> fires;
    for (auto node : triggers)
    {
        if (!node)
            continue;

        Trigger* trigger = node->getTrigger();
        if (!trigger)
        {
            trigger = aiObjectContext->GetTrigger(node->getName());
            node->setTrigger(trigger);
        }

        if (!trigger)
            continue;

        if (testMode || trigger->needCheck())
        {
            if (minimal && node->getFirstRelevance() < 100)
                continue;

            PerformanceMonitorOperation* pmo = sPerformanceMonitor.start(PERF_MON_TRIGGER, trigger->getName(), &aiObjectContext->performanceStack);
            Event event = trigger->Check();
            if (pmo) pmo->finish();
            if (!event)
                continue;
            fires[trigger] = event;
            LogAction("T:%s", trigger->getName().c_str());
        }
    }

    for (auto node : triggers)
    {
        Trigger* trigger = node->getTrigger();
        Event event = fires[trigger];
        if (!event)
            continue;

        MultiplyAndPush(node->getHandlers(), 0.0f, false, event, "trigger");
    }

    for (auto node : triggers)
    {
        Trigger* trigger = node->getTrigger();
        if (trigger) trigger->Reset();
    }
}

void Engine::PushDefaultActions()
{
    for (auto [index, strategy] : strategies)
    {
        Event emptyEvent;
        MultiplyAndPush(strategy->getDefaultActions(), 0.0f, false, emptyEvent, "default");
    }
}

const std::string Engine::ListStrategies()
{
    std::stringstream s("Strategies: ");

    if (strategies.empty())
        return s.str();

    for (auto [index, strategy] : strategies)
    {
        s << index << ", ";
    }
    return std::move(s.str().substr(0, s.str().length() - 2));
}

list<string_view> Engine::GetStrategies()
{
    list<string_view> result;
    for (const auto& ref : strategies)
    {
        result.push_back(ref.first);
    }
    return std::move(result);
}

void Engine::PushAgain(ActionNode* actionNode, float relevance, Event event)
{
    NextAction** nextAction = new NextAction*[2];
    nextAction[0] = new NextAction(actionNode->getName(), relevance);
    nextAction[1] = NULL;
    MultiplyAndPush(nextAction, relevance, true, event, "again");
    delete actionNode;
}

bool Engine::ContainsStrategy(StrategyType type)
{
    for (auto [index, strategy] : strategies)
	{
		if (strategy->GetType() & type)
			return true;
	}
	return false;
}

Action* Engine::InitializeAction(ActionNode* actionNode)
{
    Action* action = actionNode->getAction();
    if (!action)
    {
        action = aiObjectContext->GetAction(actionNode->getName());
        actionNode->setAction(action);
    }
    return action;
}

bool Engine::ListenAndExecute(Action* action, Event event)
{
    bool actionExecuted = false;

    if (actionExecutionListeners.Before(action, event))
    {
        actionExecuted = actionExecutionListeners.AllowExecution(action, event) ? action->Execute(event) : true;
    }

    if (ai->HasStrategy("debug", BOT_STATE_NON_COMBAT))
    {
        ostringstream out;
        out << "do: ";
        out << action->getName();
        if (actionExecuted)
            out << " 1 (";
        else
            out << " 0 (";

        out << action->getRelevance() << ")";

        if(!event.getSource().empty())
            out << " [" << event.getSource() << "]";

        ai->TellMasterNoFacing(out);
    }

    actionExecuted = actionExecutionListeners.OverrideResult(action, actionExecuted, event);
    actionExecutionListeners.After(action, actionExecuted, event);
    return actionExecuted;
}

template<typename ...Args>
void Engine::LogAction(Args ...args)
{
    std::stringstream buf;

    ((buf << args), ...);
    lastAction += "|";
    lastAction += buf.str();
    if (lastAction.size() > 512)
    {
        lastAction = lastAction.substr(512);
        size_t pos = lastAction.find("|");
        lastAction = (pos == string::npos ? "" : lastAction.substr(pos));
    }

    if (testMode)
    {
        std::ofstream outfile;
        outfile.open("test.txt", std::ios_base::app);
        if (outfile.is_open())
        {
            outfile << buf.str();
            outfile << std::endl;
            outfile.close();
        }
    }
    else
    {
        Player* bot = ai->GetBot();
        if (sPlayerbotAIConfig.logInGroupOnly && !bot->GetGroup())
            return;

        sLog.outDetail( "%s %s", bot->GetName(), buf.str());
    }
}

void Engine::ChangeStrategy(string names)
{
    vector<string> splitted = split(names, ',');
    for (vector<string>::iterator i = splitted.begin(); i != splitted.end(); i++)
    {
        const char* name = i->c_str();
        switch (name[0])
        {
        case '+':
            addStrategy(name+1);
            break;
        case '-':
            removeStrategy(name+1);
            break;
        case '~':
            toggleStrategy(name+1);
            break;
        case '?':
            ai->TellMaster(ListStrategies());
            break;
        }
    }
}

void Engine::LogValues()
{
    if (testMode)
        return;

    Player* bot = ai->GetBot();
    if (sPlayerbotAIConfig.logInGroupOnly && !bot->GetGroup())
        return;

    string text = ai->GetAiObjectContext()->FormatValues();
    sLog.outDebug( "Values for %s: %s", bot->GetName(), text.c_str());
}
