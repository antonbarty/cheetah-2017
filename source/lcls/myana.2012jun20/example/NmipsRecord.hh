/* $Id: NmipsRecord.hh,v 1.1 2010/12/08 11:45:39 caf Exp $ */
#ifndef __NMIPS_RECORD_H__
#define __NMIPS_RECORD_H__

#define NMIPS_MIN     0
#define NMIPS_DEFAULT 5
#define NMIPS_MAX     10

class NmipsRecord {
public:
  NmipsRecord();
  ~NmipsRecord();
public:
  int init(int nmips);
  int count();
  void insert(float value);
  float sum();
  void dump();

private:
  int _nmips;
  int _emptyCount;
#ifdef DYNAMIC_NMIPS
  float *_smallest;
  float *_largest;
#else
  float _smallest[NMIPS_MAX];
  float _largest[NMIPS_MAX];
#endif /* DYNAMIC_NMIPS */
};

#endif
