#include <staticPara.h>

CxiFileHandlar * CxiFileHandlar::_instance = NULL;

CxiFileHandlar * CxiFileHandlar::Instance()
{
	if( NULL == _instance)
        {
            _instance = new CxiFileHandlar;
        }
        return _instance;
}
