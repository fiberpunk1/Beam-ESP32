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
    void setHeaderTitil();

private:
    ServerProcess serverprocesser;
};

#endif // WIFINODE_H
