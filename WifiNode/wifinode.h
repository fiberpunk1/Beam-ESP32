#ifndef WIFINODE_H
#define WIFINODE_H

#include "serverprocess.h"

class WifiNode
{
public:
    WifiNode();
    void init();
    void process();

private:
    ServerProcess serverprocesser;
};

#endif // WIFINODE_H
