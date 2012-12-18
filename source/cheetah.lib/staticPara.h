#ifndef STATICPARA_H
#define STATICPARA_H
#include "cxiPara.h"
#include <iostream>
#include <memory>
using namespace std;

class CxiFileHandlar{
public:
       static CxiFileHandlar * Instance();
       Cxi_para para;

private:
       CxiFileHandlar(){};
       static CxiFileHandlar * _instance;
};


#endif // STATICPARA_H
