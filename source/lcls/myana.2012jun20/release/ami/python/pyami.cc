#include <Python.h>
#include <structmember.h>

#include "ami/python/Client.hh"
#include "ami/python/Discovery.hh"
#include "ami/python/pyami.hh"

#include "ami/data/RawFilter.hh"
#include "ami/data/FeatureRange.hh"
#include "ami/data/LogicAnd.hh"
#include "ami/data/LogicOr.hh"

#include "ami/data/DescScalar.hh"
#include "ami/data/EntryScalar.hh"

#include "ami/data/DescTH1F.hh"
#include "ami/data/EntryTH1F.hh"

#include "ami/data/DescWaveform.hh"
#include "ami/data/EntryWaveform.hh"

#include "ami/data/DescImage.hh"
#include "ami/data/EntryImage.hh"

#include "ami/data/DescScan.hh"
#include "ami/data/EntryScan.hh"

#include "ami/data/EnvPlot.hh"
#include "ami/data/Single.hh"
#include "ami/data/Average.hh"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <string>

static const unsigned CDS_SUBNET_LO = 37;
static const unsigned CDS_SUBNET_HI = 44;

static const unsigned FEZ_SUBNET_LO = 20;
static const unsigned FEZ_SUBNET_HI = 27;

static PyObject* AmiError;

static Ami::Python::Discovery* _discovery;

static Ami::AbsFilter* parse_filter(std::string str)
{
  if (str[0]!='(') {
    int m1 = str.find_first_of('<');
    int m2 = str.find_last_of ('<');
    return new Ami::FeatureRange(str.substr(m1+1,m2-m1-1).c_str(),
				 strtod(str.substr(0,m1).c_str(),0),
				 strtod(str.substr(m2+1).c_str(),0));
  }
  else {
    //  break into binary operator and its operands
    int level=0;
    int pos=0;
    while(level) {
      pos=str.find_first_of("()",pos);
      if (str[pos++]=='(') 
	level++;
      else
	level--;
    }
    Ami::AbsFilter* a = parse_filter(str.substr(0,pos));
    Ami::AbsFilter* b = parse_filter(str.substr(pos+1));
    if (str[pos]=='&')
      return new Ami::LogicAnd(*a,*b);
    else
      return new Ami::LogicOr (*a,*b);
  }
}

static Ami::AbsFilter* parse_filter(const char* str)
{
  if (str==0)
    return new Ami::RawFilter;

  return parse_filter(std::string(str));
}

//
//  Object methods
//

static void amientry_dealloc(amientry* self)
{
  if (self->client) {
    delete self->client;
    self->client = 0;
  }

  self->ob_type->tp_free((PyObject*)self);
}

static PyObject* amientry_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
  amientry* self;

  self = (amientry*)type->tp_alloc(type,0);
  if (self != NULL) {
    self->phy     = 0;
    self->channel = 0;
    self->client  = 0;
  }

  return (PyObject*)self;
}

class Info_pyami : public Pds::DetInfo {
public:
  Info_pyami(unsigned phy) { _log=Pds::Level::Source<<24; _phy=phy; }
  ~Info_pyami() {}
};

static int amientry_init(amientry* self, PyObject* args, PyObject* kwds)
{
  unsigned index=0, index_n=0;
  PyObject* t;
  int sts;

  static const unsigned channel_default=0;
  unsigned phy=0, channel=channel_default;
  Info_pyami info(0x100);
  Ami::AbsOperator* op = 0;
  Ami::AbsFilter*   filter = 0;
  
  while(1) {
    { char* kwlist[] = {"name","entry_type",NULL};
      const char *name = 0, *entry_type=0;
      t = PyTuple_GetSlice(args,index,index_n=2);
      sts = PyArg_ParseTupleAndKeywords(t,kwds,"s|s",kwlist,
					&name, &entry_type);

      Py_DECREF(t);
      //
      //  Handle scalar variables (like diodes and BLD)
      //
      if (sts) {
	index = index_n;
	if (entry_type==0 || strcmp(entry_type,"Scalar")==0) {
	  op = new Ami::EnvPlot(Ami::DescScalar(name,"mean"));
	  break;
	}
        else if (strcmp(entry_type,"Scan")==0) {
	  unsigned nbins = 0;
          char* xtitle = 0;
	  char* ekwlist[] = {"xtitle","nbins",NULL};
	  t = PyTuple_GetSlice(args,index,index_n+=2);
	  sts = PyArg_ParseTupleAndKeywords(t,kwds,"sI",ekwlist,
					    &xtitle, &nbins);
          if (sts) {
            printf( "Creating Scan %s, %s, %d\n" , name,xtitle,nbins);
            op = new Ami::EnvPlot(Ami::DescScan(name,xtitle,"mean",nbins));
            break;
          }
          return -1;
        }
	else if (strcmp(entry_type,"TH1F")==0) {
	  unsigned nbins = 0;
	  float    range_lo = 0;
	  float    range_hi = 0;
	  char* ekwlist[] = {"nbins","range_lo","range_hi",NULL};
	  t = PyTuple_GetSlice(args,index,index_n+=3);
	  sts = PyArg_ParseTupleAndKeywords(t,kwds,"Iff",ekwlist,
					    &nbins, &range_lo, &range_hi);
	  Py_DECREF(t);
	  if (sts) {
	    index = index_n;
	    op = new Ami::EnvPlot(Ami::DescTH1F(name,name,"events",nbins,range_lo,range_hi));
	    break;
	  }
	}
	return -1;
      }
    }
    { char* kwlist[] = {"det_id","channel",NULL};
      t = PyTuple_GetSlice(args,index,index_n=2);
      sts = PyArg_ParseTupleAndKeywords(t,kwds,"II",kwlist,
					&phy, &channel);
      Py_DECREF(t);
      if (sts) {
        index = index_n;

	info = Info_pyami(phy);
	op   = new Ami::Average;
	break;
      }
    }
    return -1;
  }

  { char* kwlist[] = {"filter",NULL};
    const char* filter_str = 0;
    t = PyTuple_GetSlice(args,index,index_n+=1);
    sts = PyArg_ParseTupleAndKeywords(t,kwds,"|s",kwlist,&filter_str);
    Py_DECREF(t);
    filter = parse_filter(filter_str);
    printf("parsed filter from %s\n",filter_str);
  }

  PyErr_Clear();

  Ami::Python::Client* cl = new Ami::Python::Client(info, 
						    channel,
						    filter,
						    op);

  int result = cl->initialize(*_discovery->allocate(*cl));

  if (result == Ami::Python::Client::Success) {
    self->client = cl;
    return 0;
  }
  else if (result == Ami::Python::Client::TimedOut) {
    PyErr_SetString(PyExc_RuntimeError,"Ami configure timeout");
  }
  else if (result == Ami::Python::Client::NoEntry) {
    PyErr_SetString(PyExc_RuntimeError,"Detector entry not found");
  }
  return -1;
}

static PyObject* get(PyObject* self, PyObject* args)
{
  amientry* e = reinterpret_cast<amientry*>(self);

  int sts = e->client->request_payload();
  if (sts==0) {
    const Ami::Entry* entry = e->client->payload();
    switch(entry->desc().type()) {
    case Ami::DescEntry::Scalar:
      { const Ami::EntryScalar* s = static_cast<const Ami::EntryScalar*>(entry);
        PyObject* o = PyDict_New();
        PyDict_SetItemString(o,"type"   ,PyString_FromString("Scalar"));
        PyDict_SetItemString(o,"entries",PyLong_FromDouble (s->entries()));
        PyDict_SetItemString(o,"mean"   ,PyFloat_FromDouble(s->mean()));
        PyDict_SetItemString(o,"rms"    ,PyFloat_FromDouble(s->rms()));
        return o; }
    case Ami::DescEntry::TH1F:
      { const Ami::EntryTH1F* s = static_cast<const Ami::EntryTH1F*>(entry);
	PyObject* t = PyList_New(s->desc().nbins());
	for(unsigned i=0; i<s->desc().nbins();i++)
	  PyList_SetItem(t,i,PyFloat_FromDouble(s->content(i)));

	PyObject* o = PyDict_New();
        PyDict_SetItemString(o,"type",   PyString_FromString("TH1F"));
        PyDict_SetItemString(o,"uflow",  PyLong_FromDouble(s->info(Ami::EntryTH1F::Underflow)));
        PyDict_SetItemString(o,"oflow",  PyLong_FromDouble(s->info(Ami::EntryTH1F::Overflow)));
        PyDict_SetItemString(o,"data",   t);

	Py_DECREF(t);
	return o;
      }
    case Ami::DescEntry::Waveform:
      { const Ami::EntryWaveform* s = static_cast<const Ami::EntryWaveform*>(entry);
	PyObject* t = PyList_New(s->desc().nbins());
	for(unsigned i=0; i<s->desc().nbins();i++)
	  PyList_SetItem(t,i,PyFloat_FromDouble(s->content(i)));

	PyObject* o = PyDict_New();
        PyDict_SetItemString(o,"type",   PyString_FromString("Waveform"));
        PyDict_SetItemString(o,"entries",PyLong_FromDouble(s->info(Ami::EntryWaveform::Normalization)));
        PyDict_SetItemString(o,"xlow",   PyFloat_FromDouble(s->desc().xlow()));
        PyDict_SetItemString(o,"xhigh",  PyFloat_FromDouble(s->desc().xup ()));
        PyDict_SetItemString(o,"data",   t);

	Py_DECREF(t);
	return o;
      }
    case Ami::DescEntry::Image:
      { const Ami::EntryImage* s = static_cast<const Ami::EntryImage*>(entry);
	PyObject* t;
	PyObject* o;

	if (s->desc().nframes()<=1) {
	  t = PyList_New(s->desc().nbinsy());  // rows
	  for(unsigned i=0; i<s->desc().nbinsy();i++) {
	    PyObject* col = PyList_New(s->desc().nbinsx());
	    for(unsigned j=0; j<s->desc().nbinsx();j++)
	      PyList_SetItem(col,j,PyFloat_FromDouble(s->content(j,i)));
	    PyList_SetItem(t,i,col);
	  }
          o = PyDict_New();
          PyDict_SetItemString(o,"type",   PyString_FromString("Image"));
          PyDict_SetItemString(o,"entries",PyLong_FromDouble (s->info(Ami::EntryImage::Normalization)));
          PyDict_SetItemString(o,"offset", PyFloat_FromDouble(s->info(Ami::EntryImage::Pedestal)));
          PyDict_SetItemString(o,"ppxbin", PyLong_FromLong(s->desc().ppxbin()));
          PyDict_SetItemString(o,"ppybin", PyLong_FromLong(s->desc().ppybin()));
          PyDict_SetItemString(o,"data",   t);
	}
	else {
	  t = PyList_New(s->desc().nframes());
	  for(unsigned k=0; k<s->desc().nframes(); k++) {
	    const Ami::SubFrame& f = s->desc().frame(k);
	    PyObject* rows = PyList_New(f.ny);
	    for(unsigned i=0; i<f.ny;i++) {
	      PyObject* col = PyList_New(f.nx);
	      for(unsigned j=0; j<f.nx;j++)
		PyList_SetItem(col,j,PyFloat_FromDouble(s->content(f.x+j,f.y+i)));
	      PyList_SetItem(rows,i,col);
	    }
	    PyList_SetItem(t,k,Py_BuildValue("ddO",f.x,f.y,rows));
	  }
          o = PyDict_New();
          PyDict_SetItemString(o,"type",   PyString_FromString("ImageArray"));
          PyDict_SetItemString(o,"entries",PyLong_FromDouble (s->info(Ami::EntryImage::Normalization)));
          PyDict_SetItemString(o,"offset", PyFloat_FromDouble(s->info(Ami::EntryImage::Pedestal)));
          PyDict_SetItemString(o,"ppxbin", PyLong_FromLong(s->desc().ppxbin()));
          PyDict_SetItemString(o,"ppybin", PyLong_FromLong(s->desc().ppybin()));
          PyDict_SetItemString(o,"data",   t);

	}
        Py_DECREF(t);
        return o;
      }
    case Ami::DescEntry::Scan:
      { const Ami::EntryScan* s = static_cast<const Ami::EntryScan*>(entry);

	PyObject* o = PyDict_New();
        PyDict_SetItemString(o,"type",   PyString_FromString("Scan"));
        PyDict_SetItemString(o,"nbins",  PyLong_FromDouble(s->desc().nbins()));
        PyDict_SetItemString(o,"current",PyLong_FromDouble(s->info(Ami::EntryScan::Current)));
	{ PyObject* t = PyList_New(s->desc().nbins());
          for(unsigned i=0; i<s->desc().nbins();i++)
            PyList_SetItem(t,i,PyFloat_FromDouble(s->xbin(i)));
          PyDict_SetItemString(o,"xbins",   t);
          Py_DECREF(t);
        }
	{ PyObject* t = PyList_New(s->desc().nbins());
          for(unsigned i=0; i<s->desc().nbins();i++)
            PyList_SetItem(t,i,PyFloat_FromDouble(s->nentries(i)));
          PyDict_SetItemString(o,"yentries",   t);
          Py_DECREF(t);
        }
	{ PyObject* t = PyList_New(s->desc().nbins());
          for(unsigned i=0; i<s->desc().nbins();i++)
            PyList_SetItem(t,i,PyFloat_FromDouble(s->ysum(i)));
          PyDict_SetItemString(o,"ysum",   t);
          Py_DECREF(t);
        }
	{ PyObject* t = PyList_New(s->desc().nbins());
          for(unsigned i=0; i<s->desc().nbins();i++)
            PyList_SetItem(t,i,PyFloat_FromDouble(s->y2sum(i)));
          PyDict_SetItemString(o,"y2sum",   t);
          Py_DECREF(t);
        }
	return o;
      }
    default:
      break;
    }
  }

  PyErr_SetString(PyExc_RuntimeError,"get timedout");
  return NULL;
}

static PyObject* clear(PyObject* self, PyObject* args)
{
  amientry* e = reinterpret_cast<amientry*>(self);
  e->client->reset();
  Py_INCREF(Py_None);
  return Py_None;
}

//
//  Register amientry methods
//
static PyMethodDef amientry_methods[] = {
  {"get"   , get   , METH_VARARGS, "Return the accumulated data"},
  {"clear" , clear , METH_VARARGS, "Clear the accumulated data"},
  {NULL},
};

//
//  Register amientry members
//
static PyMemberDef amientry_members[] = {
  {"phy"    , T_INT, offsetof(amientry, phy    ), 0, "detector identifier"},
  {"channel", T_INT, offsetof(amientry, channel), 0, "detector channel"},
  {NULL} 
};

static PyTypeObject amientry_type = {
  PyObject_HEAD_INIT(NULL)
    0,                         /* ob_size */
    "pyami.Entry",             /* tp_name */
    sizeof(amientry),          /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)amientry_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "pyami Entry objects",     /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    amientry_methods,          /* tp_methods */
    amientry_members,          /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)amientry_init,   /* tp_init */
    0,                         /* tp_alloc */
    amientry_new,              /* tp_new */
};
    
//
//  Module methods
//
//    Setup multicast interface, point-to-point interface
//    Setup server group
//    Connect to servers
//

static PyObject*
pyami_connect(PyObject *self, PyObject *args)
{
  unsigned ppinterface(0), mcinterface(0), servergroup;

  if (!PyArg_ParseTuple(args, "I|II", &servergroup,
                        &ppinterface, &mcinterface))
    return NULL;

  if (ppinterface==0 || mcinterface==0) {
    //
    //  Lookup these parameters from the available network interfaces
    //
    int fd;  
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      PyErr_SetString(PyExc_RuntimeError,"failed to lookup host interface");
      return NULL;
    }

    const int MaxRoutes = 5;
    struct ifreq ifrarray[MaxRoutes];
    memset(ifrarray, 0, sizeof(ifreq)*MaxRoutes);

    struct ifconf ifc;
    ifc.ifc_len = MaxRoutes * sizeof(struct ifreq);
    ifc.ifc_buf = (char*)ifrarray;
  
    if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
      PyErr_SetString(PyExc_RuntimeError,"failed to lookup host interfaces");
      close(fd);
      return NULL;
    }

    for (int i=0; i<MaxRoutes; i++) {
      struct ifreq* ifr = ifrarray+i;
      if (!ifr || !(((sockaddr_in&)ifr->ifr_addr).sin_addr.s_addr)) break;

      struct ifreq ifreq_flags = *ifr;
      if (ioctl(fd, SIOCGIFFLAGS, &ifreq_flags) < 0) {
        PyErr_SetString(PyExc_RuntimeError,"failed to lookup host interfaces");
        close(fd);
        return NULL;
      }

      int flags = ifreq_flags.ifr_flags;
      if ((flags & IFF_UP) && (flags & IFF_BROADCAST)) {
        struct ifreq ifreq_hwaddr = *ifr;
        if (ioctl(fd, SIOCGIFHWADDR, &ifreq_hwaddr) < 0) 
          continue;

        unsigned addr = htonl(((sockaddr_in&)ifr->ifr_addr).sin_addr.s_addr);
        unsigned subn = (addr>>8)&0xff;
        //      printf("Found addr %08x  subn %d\n",addr,subn);
        if (subn>=CDS_SUBNET_LO &&
            subn<=CDS_SUBNET_HI)
          ppinterface = addr;
        else if (subn>=FEZ_SUBNET_LO && 
                 subn<=FEZ_SUBNET_HI)
          mcinterface = addr;
      }
    }
  }

  if (ppinterface==0) {
    PyErr_SetString(PyExc_RuntimeError,"failed to lookup host interface");
    return NULL;
  }
  if (mcinterface==0) {
    PyErr_SetString(PyExc_RuntimeError,"failed to lookup group interface");
    return NULL;
  }

  if (_discovery)
    delete _discovery;

  _discovery = new Ami::Python::Discovery(ppinterface,
					  mcinterface,
					  servergroup);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef PyamiMethods[] = {
    {"connect",  pyami_connect, METH_VARARGS, "Connect to servers."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

//
//  Module initialization
//
PyMODINIT_FUNC
initpyami(void)
{
  if (PyType_Ready(&amientry_type) < 0) {
    return; 
  }

  PyObject *m = Py_InitModule("pyami", PyamiMethods);
  if (m == NULL)
    return;

  _discovery = 0;

  Py_INCREF(&amientry_type);
  PyModule_AddObject(m, "Entry", (PyObject*)&amientry_type);

  AmiError = PyErr_NewException("pyami.error", NULL, NULL);
  Py_INCREF(AmiError);
  PyModule_AddObject(m, "error", AmiError);
}

