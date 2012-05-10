#ifndef PDS_TASK_OBJECT_H
#define PDS_TASK_OBJECT_H

#include <string.h>
#include <pthread.h>

namespace Ami {

class Task;

/*
 *
 * Note on priority: 0xffffffff is a sentinal value meaning use the
 *     default priority for the thread. The value of the priority is
 *     entered as though it were for a vxWorks task ( 0 highest to 127
 *     lowest ). This also limits the vxWorks range to 0 to 127 instead
 *     of 0 to 255. The sense of this priority is opposite that for
 *     threads so the value is renormalized to the unix version inside
 *     the constructor.
 */

class TaskObject {
 public:
  TaskObject( const char* name,
		 int priority=127,
		 int stackSize=20*1024, char* stackBase=NULL);
  TaskObject(const TaskObject& tparam );
  TaskObject();
  void operator= (const TaskObject&);
  ~TaskObject();

  char* name() const { return _name;}
  int priority() const { return _priority;}
  //  int flags() const {return _flags;}
  char* stackBase() const {return (char*)_stackbase;}
  int stackSize() const {return _stacksize;}
  int taskID() const {return (int)_threadID;}

  friend class Task;
 private:
  char* _name;
  size_t _stacksize;
  void* _stackbase;
  int _priority;
  pthread_attr_t _flags;
  pthread_t _threadID;
};

}
#endif
