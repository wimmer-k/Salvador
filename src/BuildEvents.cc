#include "BuildEvents.hh"
using namespace std;

/*!
  Initialyze the event building
  \param brtr tree with input bigrips data
  \param eutr tree with input eurica data
*/
void BuildEvents::Init(TTree* brtr, TTree* eutr){
  flastevent = -1;

  fBRtr = brtr;
  fEUtr = eutr;

  fBRentry = 0;
  fEUentry = 0;
  fnbytes = 0;

  fBRts = 0;
  fbeam = new Beam;
  fEUts = 0;
  feurica = new EURICA;
  fEUeventinfo = NULL;
  fEUcluster = NULL;

  //local copy for intermediate storing
  flocalBRts = 0;
  flocalbeam = new Beam;
  flocalEUts = 0;
  flocaleurica = new EURICA;

  fBRtr->SetBranchAddress("timestamp",&flocalBRts);
  fBRtr->SetBranchAddress("beam",&flocalbeam);
  fEUtr->SetBranchAddress("EventInfo",&fEUeventinfo);
  fEUtr->SetBranchAddress("GeCluster",&fEUcluster);

  fmtr = new TTree("mtr","merged tree");
  fmtr->Branch("beam",&fbeam,320000);
  fmtr->Branch("brTS",&fBRts,320000);
  fmtr->Branch("eurica",&feurica,320000);
  fmtr->Branch("euTS",&fEUts,320000);
  fmtr->BranchRef();

  fBRentries = fBRtr->GetEntries();
  cout << fBRentries << " entries in BigRIPS tree" << endl;
  fEUentries = fEUtr->GetEntries();
  cout << fEUentries << " entries in EURICA tree" << endl;

  if(fverbose>0){
    Int_t status = fBRtr->GetEvent(0);
    if(status<0)
      cout << "first BigRIPS entry faulty!" << endl;
    cout << "first BigRIPS timestamp: " << flocalBRts << endl;;
    status = fEUtr->GetEvent(0);
    if(status<0)
      cout << "first EURICA entry faulty!" << endl;
    flocalEUts =((TArtEventInfo*) fEUeventinfo->At(0))->GetTimeStamp();
    cout << "first EURICA timestamp: " << flocalEUts << endl;;
    
    status = fBRtr->GetEvent(fBRentries-1);
    if(status<0)
      cout << "last BigRIPS entry faulty!" << endl;
    cout << "last BigRIPS timestamp: " << flocalBRts << endl;;
    status = fEUtr->GetEvent(fEUentries-1);
    if(status<0)
      cout << "last EURICA entry faulty!" << endl;
    flocalEUts =((TArtEventInfo*) fEUeventinfo->At(0))->GetTimeStamp();
    cout << "last EURICA timestamp: " << flocalEUts << endl;;
  }
  flocalBRts = 0;
  flocalEUts = 0;
  flocalbeam->Clear();

  fEUeventinfo->Clear();
  fEUcluster->Clear();  

}
bool BuildEvents::ReadBigRIPS(){
  if(fverbose>1)
    cout << __PRETTY_FUNCTION__ << endl;

  flocalbeam->Clear();
  flocalBRts = 0;
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
  if(fverbose>0)
    cout << "read new bigrips with TS = " << flocalBRts << endl;
  fnbytes += status;
  fBRentry++;

  detector* det = new detector;
  det->TS = flocalBRts;
  det->ID = 0;
  fdetectors.push_back(det);

  return true;
}
bool BuildEvents::ReadEURICA(){
  if(fverbose>1)
    cout << __PRETTY_FUNCTION__ << endl;
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
  
  flocalEUts = ((TArtEventInfo*) fEUeventinfo->At(0))->GetTimeStamp();
  if(fverbose>0)
    cout << "read new eurica with TS = " << flocalEUts << endl;

  for(int i=0;i<fEUcluster->GetEntries();i++){
    TArtGeCluster *hit =(TArtGeCluster*) fEUcluster->At(i);
    if(hit->GetEnergy()>1){
      if(fverbose>2)
	cout << fEUentry << "\t" << i <<"\t" << hit->GetEnergy() << "\t" << hit->GetTiming() << endl;
      
      EURICAHit *euhit = new EURICAHit(hit->GetChannel(),hit->GetEnergy(),hit->GetTiming(),1,flocalEUts);
      flocaleurica->AddHit(euhit);
    }
  }
  fEUentry++;

  detector* det = new detector;
  det->TS = flocalEUts;
  det->ID = 1;
  fdetectors.push_back(det);

  return true;
}
bool BuildEvents::ReadEach(){
  bool success = ReadEURICA();
  success += ReadBigRIPS();
  if(success)
    return true;
  else{
    cout << "failed to read from both EURICA and BigRIPS." << endl;
    return false;
  }
}
void BuildEvents::CloseEvent(){
  if(fverbose>0)
    cout << __PRETTY_FUNCTION__ << endl;
  // bool printme = false;
  // if(fBRts>0 && fEUts >0){
  //   printme = true;
  //   cout << "fBRentry = " << fBRentry << "\tfBRts = " << fBRts << ", fEUentry = " << fEUentry << "\tfEUts = " << fEUts << "\tmult = " << feurica->GetMult()<< endl;
  //   cout << "BR AoQ = " << fbeam->GetAQ(1) << " Z = " << fbeam->GetZ(1) << endl;
  //   feurica->Print();
  // }
  switch(fmode){
  default:
  case 0: //write all events
    fmtr->Fill();
    break;
  case 1://isomer data BR and EU coincidence
    if(fBRts>0 && fEUts >0)
      fmtr->Fill();
    break;
  }

  fBRts = 0;
  fbeam->Clear();
  fEUts = 0;
  feurica->Clear();
  // if(printme){
  //   cout << "after clearing " << endl;
  //   feurica->Print();
  // }

  if(fverbose>0)
    cout << "end "<< __PRETTY_FUNCTION__ << endl;
}
bool BuildEvents::Merge(){
  if(fverbose>1)
    cout << __PRETTY_FUNCTION__ << endl;

  if(fverbose>1){
    cout << "before sorting" << endl;
    for(vector<detector*>::iterator det=fdetectors.begin(); det!=fdetectors.end(); det++){
      cout << "ID = " << (*det)->ID << ", TS = " << (*det)->TS << endl;
    }
  }
  sort(fdetectors.begin(), fdetectors.end(),TSComparer());
  if(fverbose>1){
    cout << "after sorting" << endl;
    for(vector<detector*>::iterator det=fdetectors.begin(); det!=fdetectors.end(); det++){
      cout << "ID = " << (*det)->ID << ", TS = " << (*det)->TS << endl;
    }
  }

  

  switch(fdetectors.at(0)->ID){
  case 0: //BigRIPS
    if(fBRts>0){
      if(fverbose>1)
	cout << "has already BigRIPS" << endl;
      CloseEvent();
    }
    else if(flocalBRts - fcurrentts > fwindow){
      if(fverbose>0)
	cout << "BR larger than window" << endl;
      CloseEvent();
    }
    fBRts = flocalBRts;
    fbeam = flocalbeam;
    fcurrentts = fBRts;
    fdetectors.erase(fdetectors.begin());
    if(!ReadBigRIPS())
      cout << "failed to read BigRIPS, end of file" << endl;
    break;
  case 1: //EURICA
    if(flocalEUts - fcurrentts > fwindow){
      if(fverbose>0)
	cout << "EU larger than window" << endl;
      CloseEvent();
    }
    fEUts = flocalEUts;
    feurica->AddHits(flocaleurica->GetHits());
    flocaleurica->Clear();
    fcurrentts = fEUts;
    fdetectors.erase(fdetectors.begin());
    if(!ReadEURICA())
      cout << "failed to read EURICA, end of file" << endl;
    break;
    
  default:
    break;
  }
  if(flastevent>0 && flastevent == (int)(fBRentry + fEUentry)){
    cout << "last event reached " << endl;
    return false;
  }

  if(fdetectors.size()==0){
    cout << "all files finished " << endl;
    return false;
  }
  return true;
}
