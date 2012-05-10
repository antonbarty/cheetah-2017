#ifndef PDS_QUEUE
#define PDS_QUEUE

/*
** ++
**  Package:
**	Service
**
**  Abstract:
**	Various classes to maniplate doubly-lined lists
**
**  Author:
**      Michael Huffer, SLAC, (415) 926-4269
**
**  Creation Date:
**	000 - June 20 1,1997
**
**  Revision History:
**	November 18, 1998 - (RiC) Fixed problem with calling some of these
**                          routines from IPL where Lock is ignored.  As a
**                          work-around, the Locks have been replaced with
**                          intLock() and intUnlock() statements, making these
**                          functions unfit for inter-task communication on
**                          VxWorks.  On UNIX, the behaviour is unchanged.
**
** --
*/

#include <stdio.h>

#ifdef VXWORKS
#  include "Interrupt.hh"
#else
#  include "Lock.hh"
#endif

namespace Ami {

class QEntry
  {
  public:
    QEntry();
    QEntry* insert(QEntry* after);
    QEntry* insertList(QEntry* after);
    QEntry* remove();
    QEntry* next()     const;
    QEntry* previous() const;
  private:
    QEntry* _flink;
    QEntry* _blink;
  };

class List
  {
  public:
    List();
    QEntry* empty() const;
    QEntry* insert(QEntry*);
    QEntry* jam(QEntry*);
    QEntry* remove();
    QEntry* remove(QEntry*);
    QEntry* atHead() const;
    QEntry* atTail() const;
  private:
    QEntry* _flink;
    QEntry* _blink;
#ifdef VXWORKS
    class qLock {
    public:
      qLock() {};
     ~qLock() {};

      unsigned set()               { return Interrupt::off(); }
      void     clear(unsigned key) { Interrupt::on(key);      }
    } _lock;
#else
    class qLock : public Lock {
    public:
      qLock() : Lock(_lockRetries) {};
     ~qLock() {};

      unsigned set()               { get(); return 0; }
      void     clear(unsigned key) { release();       }

      void     cantLock() {printf("List: Unable to obtain queue lock\n");}
    private:
      enum {_lockRetries = 3};
    } _lock;
#endif
  };
}

/*
** ++
**
** Return the queue's label (usefull for checking for an empty queue)
**
** --
*/

inline Ami::QEntry* Ami::List::empty() const
  {
  return (QEntry*)&_flink;
  }

/*
** ++
**
** constructor sets up the queue's listhead to point to itself...
**
** --
*/

inline Ami::List::List() : _flink(empty()), _blink(empty()) {}


/*
** ++
**
** Return the entry at either the head or tail of the queue...
**
** --
*/

inline Ami::QEntry* Ami::List::atHead() const {return _flink;}
inline Ami::QEntry* Ami::List::atTail() const {return _blink;}

/*
** ++
**
** constructor sets up the queue's listhead to point to itself...
**
** --
*/

inline Ami::QEntry::QEntry() :
_flink((QEntry*) &_flink), _blink((QEntry*)&_flink) {}

/*
** ++
**
** insert an item on a doubly-linked list. The method has one argument:
**   after - A pointer to the item AFTER which the entry is to be inserted.
**
** --
*/

// WARNING: if `this' and `after' point to the same entry, the
// following function will destroy the list consistency. Last entry
// will point to itself instead of pointing to the list label and
// looping on the list will result in a infinite loop

inline Ami::QEntry* Ami::QEntry::insert(QEntry* after)
  {
  Ami::QEntry* next = after->_flink;
  _flink         = next;
  _blink         = after;
  next->_blink   = this;
  after->_flink  = this;
  return after;
  }

/*
** ++
**
** insert a list into a doubly-linked list. The method has one argument:
**   after - A pointer to the item AFTER which the list is to be inserted.
** 'this' is treated as being the head of a linked list.
**
** --
*/

inline Ami::QEntry* Ami::QEntry::insertList(QEntry* after)
  {
  Ami::QEntry* next = after->_flink;
  _blink->_flink = next;
  next->_blink   = _blink;
  _blink         = after;
  after->_flink  = this;
  return after;
  }

/*
** ++
**
** remove the entry from a doubly linked list...
**
** --
*/

inline Ami::QEntry* Ami::QEntry::remove()
  {
  register Ami::QEntry* next = _flink;
  register Ami::QEntry* prev = _blink;
  prev->_flink            = next;
  next->_blink            = prev;
  _flink                  = (QEntry*) &_flink;
  _blink                  = (QEntry*) &_flink;
  return this;
  }

/*
** ++
**
** insert an item on a doubly-linked list. The method has one argument:
**   after - A pointer to the item AFTER which the entry is to be inserted.
**
** --
*/

// WARNING: see note before Ami::QEntry::insert(QEntry* after)

inline Ami::QEntry* Ami::List::insert(QEntry* entry)
  {
  unsigned key = _lock.set();
  Ami::QEntry* afterentry = entry->insert(atTail());
  _lock.clear(key);
  return afterentry;
  }

/*
** ++
**
** insert an item on a doubly-linked list. The method has one argument:
**   after - A pointer to the item AFTER which the entry is to be inserted.
**
** --
*/

inline Ami::QEntry* Ami::List::jam(QEntry* entry)
  {
  unsigned key = _lock.set();
  Ami::QEntry* afterentry = entry->insert(atHead());
  _lock.clear(key);
  return afterentry;
  }

/*
** ++
**
**
** --
*/

inline Ami::QEntry* Ami::QEntry::next()     const {return _flink;}
inline Ami::QEntry* Ami::QEntry::previous() const {return _blink;}

/*
** ++
**
** remove the entry from a doubly linked list...
**
** --
*/

inline Ami::QEntry* Ami::List::remove()
  {
  unsigned key = _lock.set();
  Ami::QEntry* entry = atHead()->remove();
  _lock.clear(key);
  return entry;
  }

/*
** ++
**
** remove a specific entry from a doubly linked list...
**
** --
*/

inline Ami::QEntry* Ami::List::remove(QEntry* entry)
  {
  unsigned key = _lock.set();
  Ami::QEntry* theQEntry = entry->remove();
  _lock.clear(key);
  return theQEntry;
  }


namespace Ami {
template<class T>
class Queue : private List
  {
  public:
    ~Queue()                {}
    Queue()                 {}
    T* empty() const           {return (T*) List::empty();}
    T* insert(T* entry)        {return (T*) List::insert((QEntry*)entry);}
    T* jam(T* entry)           {return (T*) List::jam((QEntry*)entry);}
    T* remove()                {return (T*) List::remove();}
    T* remove(T* entry)        {return (T*) List::remove((QEntry*)entry);}
    T* atHead() const          {return (T*) List::atHead();}
    T* atTail() const          {return (T*) List::atTail();}
  };
}
#endif

