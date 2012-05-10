#ifndef AmiPython_pyami_hh
#define AmiPython_pyami_hh

typedef struct {
  PyObject_HEAD
  unsigned             phy;
  unsigned             channel;
  Ami::Python::Client* client;
} amientry;

#endif
