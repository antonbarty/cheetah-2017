/*
 * Monitor implementation (java-style synchronization).
 * Thread that locks the monitor must be same thread that unlocks.
 * Not recursive (thread cannot lock a monitor it already has locked).
 */

#define DEBUG 0

#include "Monitor.hh"
#include <iostream>

using namespace Ami;
using namespace std;

static void fail() {
  *((char *)0) = 0;
}

Monitor::Monitor(const char *name, const bool debug) :
  _name(name),
  _debug(debug),
  _owner(0) {
  pthread_cond_init(&_condition, NULL);
  pthread_mutex_init(&_mutex, NULL);
}


Monitor::~Monitor() {
  pthread_mutex_lock(&_mutex);
  if (_owner) {
    cout << "Monitor::~Monitor: " << _name << " still locked by " << hex << pthread_self() << endl;
    fail();
  }
}

void Monitor::lock() {
  pthread_mutex_lock(&_mutex);
  while (_owner) {
    if (_owner == pthread_self()) {
      cout << "Monitor::lock: " << _name << " already locked by " << hex << pthread_self() << endl;
      fail();
    }
    if (_debug) {
      cout << "Monitor::lock: " << _name << ": waiting" << endl;
    }
    pthread_cond_wait(&_condition, &_mutex);
    if (_debug) {
      cout << "Monitor::lock: " << _name << ": trying again" << endl;
    }
  }
  _owner = pthread_self();
  if (_debug) {
    cout << "Monitor::lock: " << _name << " now locked by " << hex << pthread_self() << endl;
  }
  pthread_mutex_unlock(&_mutex);
}

void Monitor::unlock() {
  pthread_mutex_lock(&_mutex);
  if (_owner != pthread_self()) {
    cout << "Monitor::unlock: " << _name << " is not locked by " << hex << pthread_self() << " but rather by " << _owner << endl;
    fail();
  }
  _owner = 0;
  if (_debug) {
    cout << "Monitor::unlock: " << _name << " unlocked by " << hex << pthread_self() << endl;
  }
  pthread_cond_broadcast(&_condition);
  pthread_mutex_unlock(&_mutex);
}
