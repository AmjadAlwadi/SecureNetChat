#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>


struct Group_Secret {

    std::string nickname;
    std::string key;
};

struct Group {

    std::string name;
    std::string topic;
    std::string owner_nickname;
    std::vector<std::string> members_nicknames;
    EVP_PKEY* pkey;
    std::vector<Group_Secret> secrets;
};

#endif