#ifndef WIFINODE_H
#define WIFINODE_H

#include "serverprocess.h"

class WifiNode
{
public:
    WifiNode();
    void init();
    void process();
    void checkwifi();

private:
    ServerProcess serverprocesser;
};

#endif // WIFINODE_H
