#include "Filter.h"

Filter::Filter()
{
    nextFt = NULL;
}

Filter::Filter(const string &match, string &action)
{
    if (match == "any")
    {
        matchType = ANY;
    }
    else if (match.find("community contains") != string::npos)
    {
        matchType = COMMUNITY_CONTAINS;
    }
    else if (match.find("prefix is") != string::npos)
    {
        matchType = PREFIX_IS;
    }
    else if (match.find("prefix in") != string::npos)
    {
        matchType = PREFIX_IN;
    }
    else if (match.find("path") != string::npos)
    {
        matchType = PATH;
    }
    else if (match.find("AS-SET") != string::npos)
    {
        matchType = AS_SET;
    }

    if (matchType != ANY)
        matchValue = match.substr(match.find_last_of(" ") + 1);

    if (matchType == PATH)
    {
        matchValue = matchValue.substr(1, matchValue.length() - 2);
    }

    string subAction = "";
    if (action[action.length() - 1] != ';')
        action += ";";

    for (char c : action)
    {
        if (c == ';')
        {
            if (subAction == "accept")
            {
                actions.push_front(make_pair(ACCEPT, ""));
            }
            else if (subAction == "deny")
            {
                actions.push_front(make_pair(DENY, ""));
            }
            else if (subAction.find("local-pref") != string::npos)
            {
                actions.push_front(make_pair(LOCAL_PREF, subAction.substr(subAction.find_last_of(" ") + 1)));
            }
            else if (subAction.find("community add") != string::npos)
            {
                actions.push_front(make_pair(COMMUNITY_ADD, subAction.substr(subAction.find_last_of(" ") + 1)));
            }
            else if (subAction.find("community strip") != string::npos)
            {
                actions.push_front(make_pair(COMMUNITY_STRIP, ""));
            }
            else if (subAction.find("community remove") != string::npos)
            {
                actions.push_front(make_pair(COMMUNITY_REMOVE, subAction.substr(subAction.find_last_of(" ") + 1)));
            }
            else if (subAction.find("prepend") != string::npos)
            {
                actions.push_front(make_pair(PREPEND, subAction.substr(subAction.find_last_of(" ") + 1)));
            }
            subAction = "";
        }
        else
        {
            subAction += c;
        }
    }

    nextFt = NULL;
}

Filter::~Filter()
{
    actions.clear();
}