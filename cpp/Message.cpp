#include "Message.h"

Message::Message()
{
}

Message::Message(char *type, const pair<int, char> &peer, const int &lastAS, const string &prefix, const string &ASPath, const string &origin, const string &community)
{
    this->type = type;
    this->peer = peer;
    this->lastAS = lastAS;
    this->prefix = prefix;
    this->ASPath = ASPath;
    this->origin = origin;
    this->community = community;
}

Message::Message(char *type, const pair<int, char> &peer, const int &lastAS, const string &prefix)
{
    this->type = type;
    this->peer = peer;
    this->lastAS = lastAS;
    this->prefix = prefix;
}

Message::Message(char *type, const int &ASN, const string &prefix)
{
    this->type = type;
    this->lastAS = ASN;
    this->prefix = prefix;
}

Message::Message(char *type, const int &ASN)
{
    this->type = type;
    this->lastAS = ASN;
}

Message::Message(char *type)
{
    this->type = type;
}

Message::Message(char *type, const int &ASN, const vector<string> &prefixes)
{
    this->type = type;
    this->lastAS = ASN;
    this->prefixes = prefixes;
}

Message::~Message()
{
}