#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <QtGui/QApplication>

#include "ami/qt/FeatureRegistry.hh"
#include "ami/qt/FeatureTree.hh"

#include "ami/data/Cds.hh"
#include "ami/data/Discovery.hh"
#include "ami/data/FeatureCache.hh"

using namespace Ami;

void add(FeatureCache& cache, const char* name, bool show=false)
{
  printf("========= Adding %s ========\n",name);

  cache.add(name);

  if (show) {
    Cds cds("cds");

    const unsigned niov=3;
    const unsigned bufsiz=8192;
    iovec iov[niov];
    for(unsigned i=0; i<niov; i++) {
      iov[i].iov_base = new char[bufsiz];
      iov[i].iov_len  = bufsiz;
    }

    std::vector<FeatureCache*> features;
    features.push_back(&cache);

    DiscoveryTx tx(features,cds);
    tx.serialize(iov);

    char* buff = new char[niov*bufsiz];
    char* p = buff;
    for(unsigned i=0; i<tx.niovs(); i++) {
      memcpy(p, iov[i].iov_base, iov[i].iov_len);
      p += iov[i].iov_len;
    }

    DiscoveryRx rx(buff, p-buff);

    Ami::Qt::FeatureRegistry::instance().insert(rx.features());

    Ami::Qt::FeatureTree* tree = new Ami::Qt::FeatureTree;
    tree->setWindowTitle(name);
    tree->show();
  }
}

int main(int argc, char **argv) 
{
  QApplication app(argc, argv);

  FeatureCache cache;
  add(cache,"DAQ:EVR0:P0:Delay");
  add(cache,"DAQ:EVR0:P1:Delay");
  add(cache,"DAQ:EVR0:P2:Delay");
  add(cache,"DAQ:EVR0:Evt40");
  add(cache,"DAQ:EVR0:Evt140");
  add(cache,"DAQ:EVR0:Evt162");
  add(cache,"CxiDg1-0|Ipimb-0-Ch0");
  add(cache,"CxiDg1-0|Ipimb-0-Ch1");
  add(cache,"CxiDg1-0|Ipimb-0-Ch2");
  add(cache,"CxiDg1-0|Ipimb-0-Ch3");
  add(cache,"CxiDg1-0|Ipimb-0:CH0");
  add(cache,"CxiDg1-0|Ipimb-0:CH1");
  add(cache,"CxiDg1-0|Ipimb-0:CH2");
  add(cache,"CxiDg1-0|Ipimb-0:CH3");
  add(cache,"CxiDg1-0|Ipimb-0:SUM");
  add(cache,"CxiDg1-0|Ipimb-0:XPOS");
  add(cache,"CxiDg1-0|Ipimb-0:YPOS");
  add(cache,"CxiDg2-0|Ipimb-0-Ch0");
  add(cache,"CxiDg2-0|Ipimb-0-Ch1");
  add(cache,"CxiDg2-0|Ipimb-0-Ch2");
  add(cache,"CxiDg2-0|Ipimb-0-Ch3");
  add(cache,"CxiDg2-0|Ipimb-0:CH0");
  add(cache,"CxiDg2-0|Ipimb-0:CH1");
  add(cache,"CxiDg2-0|Ipimb-0:CH2");
  add(cache,"CxiDg2-0|Ipimb-0:CH3");
  add(cache,"CxiDg2-0|Ipimb-0:SUM");
  add(cache,"CxiDg2-0|Ipimb-0:XPOS");
  add(cache,"CxiDg2-0|Ipimb-0:YPOS");
  add(cache,"CxiDg2-0|Ipimb-1-Ch0");
  add(cache,"CxiDg2-0|Ipimb-1-Ch1");
  add(cache,"CxiDg2-0|Ipimb-1-Ch2");
  add(cache,"CxiDg2-0|Ipimb-1-Ch3");
  add(cache,"CxiDg2-0|Ipimb-1:CH0");
  add(cache,"CxiDg2-0|Ipimb-1:CH1");
  add(cache,"CxiDg2-0|Ipimb-1:CH2");
  add(cache,"CxiDg2-0|Ipimb-1:CH3");
  add(cache,"CxiDg2-0|Ipimb-1:SUM");
  add(cache,"CxiDg2-0|Ipimb-1:XPOS");
  add(cache,"CxiDg2-0|Ipimb-1:YPOS");
  add(cache,"CxiDg4-0|Ipimb-0-Ch0");
  add(cache,"CxiDg4-0|Ipimb-0-Ch1");
  add(cache,"CxiDg4-0|Ipimb-0-Ch2");
  add(cache,"CxiDg4-0|Ipimb-0-Ch3");
  add(cache,"CxiDg4-0|Ipimb-0:CH0");
  add(cache,"CxiDg4-0|Ipimb-0:CH1");
  add(cache,"CxiDg4-0|Ipimb-0:CH2");
  add(cache,"CxiDg4-0|Ipimb-0:CH3");
  add(cache,"CxiDg4-0|Ipimb-0:SUM");
  add(cache,"CxiDg4-0|Ipimb-0:XPOS");
  add(cache,"CxiDg4-0|Ipimb-0:YPOS");
  add(cache,"BLD:EBEAM:Q");
  add(cache,"BLD:EBEAM:L3E");
  add(cache,"BLD:EBEAM:LTUX");
  add(cache,"BLD:EBEAM:LTUY");
  add(cache,"BLD:EBEAM:LTUXP");
  add(cache,"BLD:EBEAM:LTUYP");
  add(cache,"BLD:EBEAM:PKCURRBC2");
  add(cache,"BLD:PHASECAV:T1");
  add(cache,"BLD:PHASECAV:T2",true);
  add(cache,"BLD:PHASECAV:Q1",true);

  app.exec();

  return 0;
}
