#ifndef ODF_TASK_H
#define ODF_TASK_H

#include "QueueSlowSafe.hh"
#include "Semaphore.hh"
#include "TaskObject.hh"
#include "Routine.hh"


namespace Ami {

extern "C" {
  typedef void* (*TaskFunction)(void*);
  void* TaskMainLoop(void*);
}


/*
 * an Task specific class that is used privately by Task so that
 * it can properly delete itself.
 *
 */

class Task;
class TaskDelete : public Routine {
 public:
  TaskDelete(Task* t, Semaphore* s=0) { _taskToKill = t; _sem=s; }
  void routine(void);
 private:
  Task* _taskToKill;
  Semaphore* _sem;
};

/*
 * The Task callable interface. Customization of the code 
 *
 *
 */

class Task {
 public:

  Task(const TaskObject&);
  Task(const Task&);

  // this ctor makes current task the Task.  make c++ signature
  // distinct so it isn't used by accident.
  enum MakeThisATaskFlag {MakeThisATask};
  Task(MakeThisATaskFlag);

  void operator= (const Task&);

  const TaskObject& parameters() const;
  void call(Routine*);
  void destroy();
  void destroy_b();
  void mainLoop();
  void signal(int signal);

  bool is_self() const;

  friend class TaskDelete;
  friend void* TaskMainLoop(void*);

 private:
  Task() {}
  ~Task();
  int createTask( TaskObject&, TaskFunction );
  void deleteTask();

  TaskObject*        _taskObj;
  int*                  _refCount;
  Queue<Routine>* _jobs;
  Semaphore*         _pending;
  Routine*           _destroyRoutine;
};
}

#endif
