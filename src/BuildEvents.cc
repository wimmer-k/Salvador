#include "BuildEvents.hh"
using namespace std;

/*!
  Initialyze the event building
  \param brtr tree with input bigrips data
  \param eutr tree with input eurica data
  \param mtr tree with merged output data
*/
void BuildEvents::Init(TTree* brtr, TTree* eutr, TTree* mtr){
  fBRtr = brtr;
  fEUtr = eutr;
  fmtr = mtr;

  fBRentry = 0;
  fEUentry = 0;
  fnbytes = 0;

  fBRts = 0;
  fEUts = 0;
  fbeam = new Beam;
  feurica = new EURICA;
  fEUeventinfo = NULL;
  fEUcluster = NULL;

  fBRtr->SetBranchAddress("timestamp",&fBRts);
  fBRtr->SetBranchAddress("beam",&fbeam);
  fEUtr->SetBranchAddress("EventInfo",&fEUeventinfo);
  fEUtr->SetBranchAddress("GeCluster",&fEUcluster);

  fmtr = new TTree("mtr","merged tree");
  fmtr->Branch("beam",&fbeam,320000);
  fmtr->Branch("brTS",&fBRts,320000);
  fmtr->Branch("eurica",&feurica,320000);
  fmtr->Branch("euTS",&fEUts,320000);

  fBRentries = fBRtr->GetEntries();
  cout << fBRentries << " entries in BigRIPS tree" << endl;
  fEUentries = fEUtr->GetEntries();
  cout << fEUentries << " entries in EURICA tree" << endl;

  if(fverbose>0){
    Int_t status = fBRtr->GetEvent(0);
    if(status<0)
      cout << "first BigRIPS entry faulty!" << endl;
    cout << "first BigRIPS timestamp: " << fBRts << endl;;
    status = fEUtr->GetEvent(0);
    if(status<0)
      cout << "first EURICA entry faulty!" << endl;
    fEUts =((TArtEventInfo*) fEUeventinfo->At(0))->GetTimeStamp();
    cout << "first EURICA timestamp: " << fEUts << endl;;
    
    status = fBRtr->GetEvent(fBRentries-1);
    if(status<0)
      cout << "last BigRIPS entry faulty!" << endl;
    cout << "last BigRIPS timestamp: " << fBRts << endl;;
    status = fEUtr->GetEvent(fEUentries-1);
    if(status<0)
      cout << "last EURICA entry faulty!" << endl;
    fEUts =((TArtEventInfo*) fEUeventinfo->At(0))->GetTimeStamp();
    cout << "last EURICA timestamp: " << fEUts << endl;;
  }
  fBRts = 0;
  fEUts = 0;
  fbeam->Clear();
  feurica->Clear();
}
bool BuildEvents::ReadBigRIPS(){
  if(fverbose>0)
    cout << __PRETTY_FUNCTION__ << endl;
  if(fBRts>0){
    if(fverbose>0){
      cout << "reading another BigRIPS, closing event" << endl;
    }
    CloseEvent();
  }
  fbeam->Clear();
  fBRts = 0;
  if(fBRentry==fBRentries){
    return false;
  }
  Int_t status = fBRtr->GetEvent(fBRentry);
  if(fverbose>2)
    cout << "status " << status << endl;
  if(status == -1){
    cerr<<"Error occured, couldn't read entry "<<fBRentry<<" from tree "<<fBRtr->GetName()<<endl;
    return false;
  }
  else if(status == 0){
    cerr<<"Error occured, entry "<<fBRentry<<" in tree "<<fBRtr->GetName()<<" in file doesn't exist"<<endl;
    return false;
  }
  fnbytes += status;
  fBRentry++;
  return true;
}
bool BuildEvents::ReadEURICA(){
  //if(fverbose>0)
  //  cout << __PRETTY_FUNCTION__ << endl;
  if(fEUentry==fEUentries){
    return false;
  }
  Int_t status = fEUtr->GetEvent(fEUentry);
  if(fverbose>2)
    cout << "status " << status << endl;
  if(status == -1){
    cerr<<"Error occured, couldn't read entry "<<fEUentry<<" from tree "<<fEUtr->GetName()<<endl;
    return false;
  }
  else if(status == 0){
    cerr<<"Error occured, entry "<<fEUentry<<" in tree "<<fEUtr->GetName()<<" in file doesn't exist"<<endl;
    return false;
  }
  fnbytes += status;
  
  //cout << "fEUentry: " << fEUentry << "\tfEUcluster->GetEntries() " << fEUcluster->GetEntries() << endl;
  for(unsigned int i=0;i<fEUcluster->GetEntries();i++){
    TArtGeCluster *hit =(TArtGeCluster*) fEUcluster->At(i);
    if(hit->GetEnergy()>1)
      cout << fEUentry << "\t" << i <<"\t" << hit->GetEnergy() << "\t" << hit->GetTiming() << endl;
  }
  fEUentry++;
  return true;
}
void BuildEvents::CloseEvent(){
  fmtr->Fill();
  fBRts = 0;
  fEUts = 0;
  fbeam->Clear();
  feurica->Clear();
 
}
