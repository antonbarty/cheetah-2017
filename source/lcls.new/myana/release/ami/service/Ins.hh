/*
** ++
**  Package:
**	Services
**
**  Abstract:
**      
**  Author:
**      Michael Huffer, SLAC, (415) 926-4269
**
**  Creation Date:
**	000 - June 1,1998
**
**  Revision History:
**	None.
**
** --
*/

#ifndef PDS_INS
#define PDS_INS

#ifdef VXWORKS
#include <inetLib.h>
#else
#include <arpa/inet.h>
#endif

#if (defined _XOPEN_SOURCE && _XOPEN_SOURCE - 500 != 0)
#define explicit
#endif

namespace Ami {
class Ins
  {
  public:
    Ins();
    enum Option {DoNotInitialize};
    explicit Ins(Option);
    explicit Ins(unsigned short port);
    explicit Ins(int address);
    Ins(int address, unsigned short port);
    Ins(const Ins& ins, unsigned short port);
    explicit Ins(const sockaddr_in& sa);

    int                   address()     const; 
    void                  address(int address);
    unsigned short        portId()      const; 

    static bool           is_multicast(int address);
    static bool           is_multicast(const Ins& );

    static unsigned       parse_ip(const char*);
    static unsigned       parse_interface(const char*);

    int operator==(const Ins& that)  const;

  protected:
    int      _address;
    unsigned _port;
  };
}

/*
** ++
**
**
** --
*/

inline int Ami::Ins::operator==(const Ami::Ins& that) const{
  return (_address == that._address && _port == that._port);
}

/*
** ++
**
**
** --
*/

inline Ami::Ins::Ins(Option) 
  {
  }

/*
** ++
**
**
** --
*/

inline Ami::Ins::Ins() 
  {
    _address = INADDR_ANY;
    _port    = 0;
  }

/*
** ++
**
**
** --
*/

inline Ami::Ins::Ins(unsigned short port) 
  {
    _address = INADDR_ANY;
    _port    = port;
  }

/*
** ++
**
**
** --
*/

inline Ami::Ins::Ins(int address, unsigned short port) 
  {
    _address = address;
    _port    = port;
  }

/*
** ++
**
**
** --
*/

inline Ami::Ins::Ins(const Ami::Ins& ins, unsigned short port) 
  {
    _address = ins._address;
    _port    = port;
  }

/*
** ++
**
**
** --
*/

inline Ami::Ins::Ins(const sockaddr_in& sa) 
  {
    _address = ntohl(sa.sin_addr.s_addr);
    _port    = ntohs(sa.sin_port);
  }

/*
** ++
**
**
** --
*/

inline Ami::Ins::Ins(int address) 
  {
    _address = address;
    _port    = 0;
  }

/*
** ++
**
**
** --
*/

inline unsigned short Ami::Ins::portId() const 
  {
    return _port;  
  }

/*
** ++
**
**
** --
*/

inline void Ami::Ins::address(int address) 
  {
    _address = address;
  }

/*
** ++
**
**
** --
*/

inline int Ami::Ins::address() const 
  {
    return _address;
  }

inline bool Ami::Ins::is_multicast(int address)
  {
    const unsigned MCAST_MASK = 0xe0000000;
    return (address & MCAST_MASK) == MCAST_MASK;
  }

inline bool Ami::Ins::is_multicast(const Ins& ins)
  {
    return is_multicast(ins.address());
  }

#endif
