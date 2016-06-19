#include "BuildEvents.hh"
using namespace std;

void BuildEvents::Init(TTree* brtr, TTree* eutr, TTree* mtr){
  fBRtr = brtr;
  fEUtr = eutr;
  fmtr = mtr;

  fBRts = 0;
  fEUts = 0;
  fbeam = new Beam;
  feurica = new EURICA;
  fEUeventinfo = NULL;
  fBRtr->SetBranchAddress("timestamp",&fBRts);
  fBRtr->SetBranchAddress("beam",&fbeam);
  
  fEUtr->SetBranchAddress("EventInfo",&fEUeventinfo);
  fmtr = new TTree("mtr","merged tree");
  fmtr->Branch("beam",&fbeam,320000);
  fmtr->Branch("brTS",&fBRts,320000);
  fmtr->Branch("eurica",&feurica,320000);
  fmtr->Branch("euTS",&fEUts,320000);
}
