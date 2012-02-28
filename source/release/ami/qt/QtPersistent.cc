#include "ami/qt/QtPersistent.hh"

using namespace Ami::Qt;

const int StringSize = 64;
const int IntSize = 16;
const int DoubleSize = 16;

static unsigned indent   = 0;
static bool     tag_open = false;

static char buff[8*1024];

Ami::XML::TagIterator::TagIterator(const char*& p) :
  _p (p)
{
  _tag = QtPersistent::extract_tag(p);
}

bool  Ami::XML::TagIterator::end() const
{
  return _tag.name.empty();
}

Ami::XML::TagIterator::operator const Ami::XML::StartTag*() const
{
  return &_tag;
}

Ami::XML::TagIterator& Ami::XML::TagIterator::operator++(int)
{
  if (!end()) {
    std::string stop_tag = QtPersistent::extract_tag(_p).element.substr(1);
    if (stop_tag != _tag.element)
      printf("Mismatch tags %s/%s\n",_tag.element.c_str(),stop_tag.c_str());
    const char* p = _p;
    _tag = QtPersistent::extract_tag(p);
    if (!end())
      _p = p;
  }
    
  return *this;
}

void QtPersistent::insert(char*& p, const Ami::XML::StartTag& tag)
{
  *p++ = '\n';
  for(unsigned i=0; i<2*indent; i++)
    *p++ = ' ';

  p += sprintf(p, "<%s name=\"%s\">", tag.element.c_str(), tag.name.c_str());

  indent++;
  tag_open = true;
}

void QtPersistent::insert(char*& p, const Ami::XML::StopTag& tag)
{
  indent--;
  if (!tag_open) {
    *p++ = '\n';
    for(unsigned i=0; i<2*indent; i++)
      *p++ = ' ';
  }

  p += sprintf(p, "</%s>", tag.element.c_str());

  tag_open = false;
}

void QtPersistent::insert(char*& p, const QString& s)
{
  //  printf("Inserting %s @ %p\n",qPrintable(s),p);
  int sz = s.size();
  for(int i=0; i<sz; i++) {
    const char c = s[i].toAscii();
    if (c && c!='<')
      *p++ = c;
    else
      p += sprintf(p,"&#x%04x",s[i].unicode());
  }
}

void QtPersistent::insert(char*& p, int s)
{
  p += sprintf(p,"%d",s);
}

void QtPersistent::insert(char*& p, unsigned s)
{
  p += sprintf(p,"%d",s);
}

void QtPersistent::insert(char*& p, double s)
{
  p += sprintf(p,"%g",s);
}

void QtPersistent::insert(char*& p, bool s)
{
  p += sprintf(p,"%s",s?"True":"False");
}

void QtPersistent::insert(char*& p, void* b, int len)
{
  char* end = (char*)b + len;
  for(char* bp = (char*)b; bp<end; bp++)
    p += sprintf(p, "%02hhx", *bp);
}

Ami::XML::StartTag QtPersistent::extract_tag(const char*& p)
{
  while( *p++ != '<' ) ;

  Ami::XML::StartTag tag;
  while( *p != ' ' && *p != '>')
    tag.element.push_back(*p++);
  if (*p++ != '>') {
    while( *p++ != '\"') ;
    while( *p != '\"' )
      tag.name.push_back(*p++);
    while( *p++ != '>') ;
  }

  if (*p == '\n') p++;

  return tag;
}

QString QtPersistent::extract_s(const char*& p)
{
  QString v;
  while(*p != '<') {
    if (*p != '&')
      v.append(*p++);
    else {
      p += 3; // skip &#x
      uint16_t u;
      sscanf(p, "%04hx", &u);
      v.append(u);
      p += 4;
    }
  }
  return v;
}

int QtPersistent::extract_i(const char*& p)
{
  char* endPtr;
  return strtol(p, &endPtr, 0);
  p = endPtr;
}

double QtPersistent::extract_d(const char*& p)
{
  char* endPtr;
  return strtod(p, &endPtr);
  p = endPtr;
}

bool QtPersistent::extract_b(const char*& p)
{
  QString v = QtPersistent::extract_s(p);
  return v.compare("True",::Qt::CaseInsensitive)==0;
}

void* QtPersistent::extract_op(const char*& p)
{
  char* b = buff;
  while(*p != '<') {
    sscanf(p, "%02hhx", b);
    p += 2;
    b++;
  }
  return buff;
}
