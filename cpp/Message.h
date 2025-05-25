#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include <queue>

using namespace std;

class Message
{
public:
    string prefix, ASPath, origin, community;
    pair<int, char> peer;
    int lastAS;
    char *type;
    vector<string> prefixes;

    Message();

    Message(char *type, const pair<int, char> &peer, const int &lastAS, const string &prefix, const string &ASPath, const string &origin, const string &community = "");

    Message(char *type, const pair<int, char> &peer, const int &lastAS, const string &prefix);

    Message(char *type, const int &ASN, const string &prefix);

    Message(char *type, const int &ASN);

    Message(char *type);

    Message(char *type, const int &ASN, const vector<string> &prefixes);

    ~Message();
};

#endif