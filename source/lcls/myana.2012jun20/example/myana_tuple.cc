/* $Id: myana_tuple.cc,v 1.1 2010/08/24 06:28:19 cpo Exp $ */
#include <TROOT.h>
#include <TTree.h>

#include "myana.hh"
#include "main.hh"
#include "pvs_xpp.hh"

static TTree*               ntuple = NULL;

static int                  eseconds;
static int                  enanoseconds;
static double               gdetfee[4];
static double               ebeamQ, ebeamE;
static double               ebeamX, ebeamXP;
static double               ebeamY, ebeamYP;
static double               phcavQ[2];
static double               phcavT[2];
static double               pimSum;
static double               pimX, pimY;

static float                pvValues[numPv];

// This function is called once at the beginning of the analysis run,
// You can ask for detector "configuration" information here.

void beginjob() 
{
    int i;
    char bname[64];

    /*
     *  Create an N-tuple for later analysis
     */
    ntuple = new TTree("MyTuple","MyTuple");
    /*
     *  Create a branch for each detector result
     */
    ntuple->Branch("seconds",     &eseconds,    "seconds/I",     8000);
    ntuple->Branch("nanoseconds", &enanoseconds,"nanoseconds/I", 8000);

    ntuple->Branch("gdetfee", &gdetfee, "gdetfee[4]/D", 8000);
    ntuple->Branch("ebeamq" , &ebeamQ , "ebeamq/D", 8000);
    ntuple->Branch("ebeame" , &ebeamE , "ebeame/D", 8000);
    ntuple->Branch("ebeamx" , &ebeamX,  "ebeamx/D", 8000);
    ntuple->Branch("ebeamy" , &ebeamY,  "ebeamy/D", 8000);
    ntuple->Branch("ebeamxp", &ebeamXP, "ebeamxp/D", 8000);
    ntuple->Branch("ebeamyp", &ebeamYP, "ebeamyp/D", 8000);
    ntuple->Branch("phcavq" , &phcavQ , "phcavq[2]/D", 8000);
    ntuple->Branch("pimsum" , &pimSum , "pimsum/D", 8000);
    ntuple->Branch("pimx"   , &pimX   , "pimx/D", 8000);
    ntuple->Branch("pimy"   , &pimY   , "pimy/D", 8000);

    for(i=0; i<numPv; i++) {
      sprintf(bname,"%s/F",pvs[i].alias);
      ntuple->Branch(pvs[i].alias, &pvValues[i], bname, 8000);
    }
}

// This is called once every shot.  You can ask for
// the individual detector shot data here.

void event() 
{
    getTime( eseconds, enanoseconds );

    /*
     * Processing FEE Gas Detector pulse gdetfee
     */     
    if ( getFeeGasDet(gdetfee) ) {
	memset(gdetfee,0,sizeof(gdetfee));
    }

    if ( getEBeam(ebeamQ, ebeamE, ebeamX, ebeamY, ebeamXP, ebeamYP) ) {
      ebeamQ = 0;
    }

    if ( getPhaseCavity( phcavT[0], phcavT[1], phcavQ[0], phcavQ[1] ) ) {
      memset(phcavQ,0,sizeof(phcavQ));
    }

    // loop over a list of PVs
    for ( int iPv = 0; iPv < numPv; iPv++ ) {
      float value;
      if ( !getPvFloat( pvs[iPv].name, value ) )
	pvValues[iPv] = value;
    }

    unsigned short* xppSb3Image;
    int xppSb3Width, xppSb3Height;
    pimSum = pimX = pimY = 0;
    if ( !getTm6740Value(XppSb3PimCvd, xppSb3Width, xppSb3Height, xppSb3Image) ) {
      for(int j=0; j<xppSb3Height; j++)
	for(int i=0; i<xppSb3Width; i++) {
	  double v = double(*xppSb3Image++)-32.;
	  pimSum += v;
	  pimX   += double(i)*v;
	  pimY   += double(j)*v;
	}
      pimX /= pimSum;
      pimY /= pimSum;
    }

    /*
     *  Fill the ntuple with the assigned values
     */
    ntuple->Fill();
}

// This is called at the end of the analysis.

void endjob() 
{
    printf("User analysis end() routine called.\n");
}

void beginrun() {}
void endrun() {}

void begincalib() {}
void endcalib() {}
