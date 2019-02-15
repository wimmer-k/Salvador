#include "BuildEvents.hh"
using namespace std;

/*!
  Initialyze the event building
  \param brtr tree with input bigrips data
  \param watr tree with input wasabi data
  \param eutr tree with input eurica data
*/
void BuildEvents::Init(TTree* brtr, TTree* watr, TTree* eutr){
  flastevent = -1;
  fhasBR = false;
  fhasWA = false;
  fhasEU = false;
  fBRentries = 0;
  fWAentries = 0;
  fEUentries = 0;
  
  if(brtr!=NULL)
    fhasBR = true;
  if(watr!=NULL)
    fhasWA = true;
  if(eutr!=NULL)
    fhasEU = true;
  
  fBRtr = brtr;
  fWAtr = watr;
  fEUtr = eutr;

  fBRentry = 0;
  fWAentry = 0;
  fEUentry = 0;
  fnbytes = 0;

  fBRts = 0;
  fbeam = new Beam;
  for(unsigned short f=0;f<NFPLANES;f++){
    ffp[f] = new FocalPlane;
  }
  fWAts = 0;
  fwasabi = new WASABI;
  fEUts = 0;
  feurica = new EURICA;
  fEUeventinfo = NULL;
  fEUcluster = NULL;
  fEUclusterAB = NULL;

  //local copy for intermediate storing
  flocalBRts = 0;
  flocalbeam = new Beam;
  for(unsigned short f=0;f<NFPLANES;f++){
    flocalfp[f] = new FocalPlane;
  }
  flocalWAts = 0;
  flocalwasabi = new WASABI;
  flocalEUts = 0;
  flocaleurica = new EURICA;

  if(fhasBR){
    fBRtr->SetBranchAddress("timestamp",&flocalBRts);
    fBRtr->SetBranchAddress("beam",&flocalbeam);
    for(unsigned short f=0;f<NFPLANES;f++){
      fBRtr->SetBranchAddress(Form("fp%d",fpID[f]),&flocalfp[f]);
    }
    fBRentries = fBRtr->GetEntries();
    cout << fBRentries << " entries in BigRIPS tree" << endl;
  }
  if(fhasWA){
    fWAtr->SetBranchAddress("timestamp",&flocalWAts);
    fWAtr->SetBranchAddress("wasabi",&flocalwasabi);
    fWAentries = fWAtr->GetEntries();
    cout << fWAentries << " entries in WASABI tree" << endl;
  }
  if(fhasEU){
    fEUtr->SetBranchAddress("EventInfo",&fEUeventinfo);
    fEUtr->SetBranchAddress("GeCluster",&fEUcluster);
    fEUtr->SetBranchAddress("GeAddback",&fEUclusterAB);
    fEUentries = fEUtr->GetEntries();
    cout << fEUentries << " entries in EURICA tree" << endl;
  }
    
  fmtr = new TTree("tr","merged tree");
  fmtr->Branch("beam",&fbeam,320000);
  for(unsigned short f=0;f<NFPLANES;f++){
    fmtr->Branch(Form("fp%d",fpID[f]),&ffp[f],320000);
  }
  fmtr->Branch("brTS",&fBRts,320000);
  fmtr->Branch("brentry",&fBRentry,320000);
  fmtr->Branch("wasabi",&fwasabi,320000);
  fmtr->Branch("waTS",&fWAts,320000);
  fmtr->Branch("waentry",&fWAentry,320000);
  fmtr->Branch("eurica",&feurica,320000);
  fmtr->Branch("euTS",&fEUts,320000);
  fmtr->Branch("euentry",&fEUentry,320000);
  fmtr->BranchRef();


  if(fverbose>-1){
    Int_t status = 0;
    if(fhasBR){
      status = fBRtr->GetEvent(0);
      if(status<0)
	cout << "first BigRIPS entry faulty!" << endl;
      cout << "first BigRIPS timestamp: " << flocalBRts << endl;
      status = fBRtr->GetEvent(fBRentries-1);
      if(status<0)
	cout << "last BigRIPS entry faulty!" << endl;
      cout << "last BigRIPS timestamp: " << flocalBRts << endl;
    }
    if(fhasWA){
      status = fWAtr->GetEvent(0);
      if(status<0)
	cout << "first WASABI entry faulty!" << endl;
      cout << "first WASABI timestamp: " << flocalWAts << endl;
      status = fWAtr->GetEvent(fWAentries-1);
      if(status<0)
	cout << "last WASABI entry faulty!" << endl;
      cout << "last WASABI timestamp: " << flocalWAts << endl;;
    }
    if(fhasEU){
      status = fEUtr->GetEvent(0);
      if(status<0)
	cout << "first EURICA entry faulty!" << endl;
      flocalEUts =((TArtEventInfo*) fEUeventinfo->At(0))->GetTimeStamp();
      cout << "first EURICA timestamp: " << flocalEUts << endl;
      status = fEUtr->GetEvent(fEUentries-1);
      if(status<0)
	cout << "last EURICA entry faulty!" << endl;
      flocalEUts =((TArtEventInfo*) fEUeventinfo->At(0))->GetTimeStamp();
      cout << "last EURICA timestamp: " << flocalEUts << endl;
    }
  }
  flocalBRts = 0;
  flocalWAts = 0;
  flocalEUts = 0;
  flocalbeam->Clear();
  for(unsigned short f=0;f<NFPLANES;f++){
    flocalfp[f]->Clear();
  }
  flocalwasabi->Clear();
  flocaleurica->Clear();

  flastBRts = 0;
  flastWAts = 0;
  flastEUts = 0;
  fBRtsjump = false;
  fWAtsjump = false;
  fEUtsjump = false;

}
bool BuildEvents::ReadBigRIPS(){
  if(fverbose>1)
    cout << __PRETTY_FUNCTION__ << endl;

  flocalbeam->Clear();
  for(unsigned short f=0;f<NFPLANES;f++){
    flocalfp[f]->Clear();
  }
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
  fnbytes += status;
  if(flocalBRts<flastBRts){
    cout << endl << "BigRIPS timestamp jump detected. this = " << flocalBRts << ", last = " << flastBRts << endl;
    fBRtsjump = true;
    return false;
  }

  if(fverbose>0)
    cout << "read new bigrips with TS = " << flocalBRts << " tof = "<< flocalbeam->GetTOF(0)+48<< endl;
  fBRentry++;

  detector* det = new detector;
  det->TS = flocalBRts;
  det->ID = 0;
  fdetectors.push_back(det);

  flastBRts = flocalBRts;
  return true;
}
bool BuildEvents::ReadWASABI(){
  if(fverbose>1)
    cout << __PRETTY_FUNCTION__ << endl;
  flocalwasabi->Clear();
  flocalWAts = 0;
  if(fWAentry==fWAentries){
    return false;
  }
  Int_t status = fWAtr->GetEvent(fWAentry);
  if(fverbose>2)
    cout << "status " << status << endl;
  if(status == -1){
    cerr<<"Error occured, couldn't read entry "<<fWAentry<<" from tree "<<fWAtr->GetName()<<endl;
    return false;
  }
  else if(status == 0){
    cerr<<"Error occured, entry "<<fWAentry<<" in tree "<<fWAtr->GetName()<<" in file doesn't exist"<<endl;
    return false;
  }
  fnbytes += status;
  
  if(flocalWAts<flastWAts){
    cout <<"WASABI timestamp jump detected. this = " << flocalWAts << ", last = " << flastWAts << endl;
    fWAtsjump = true;
    return false;
  }
  if(fverbose>0)
    cout << "read new wasabi with TS = " << flocalWAts << endl;
  fWAentry++;

  detector* det = new detector;
  det->TS = flocalWAts;
  det->ID = 1;
  fdetectors.push_back(det);

  flastWAts = flocalWAts;
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
  if(flocalEUts<flastEUts){
    cout <<"EURICA timestamp jump detected. this = " << flocalEUts << ", last = " << flastEUts << endl;
    fEUtsjump = true;
    return false;
  }
  if(fverbose>0)
    cout << "read new eurica with TS = " << flocalEUts << endl;

  //cout << "fEUcluster->GetEntries() " << fEUcluster->GetEntries() << "\tfEUclusterAB->GetEntries() " << fEUclusterAB->GetEntries() << endl; 
  for(int i=0;i<fEUcluster->GetEntries();i++){
    TArtGeCluster *hit =(TArtGeCluster*) fEUcluster->At(i);
    if(hit->GetEnergy()>1){
      if(fverbose>2)
	cout << fEUentry << "\t" << i <<"\t" << hit->GetEnergy() << "\t" << hit->GetTiming() << endl;
      
      EURICAHit *euhit = new EURICAHit(hit->GetChannel(),hit->GetEnergy(),hit->GetTiming(),1,flocalEUts);
      flocaleurica->AddHit(euhit);
    }
  }
  for(int i=0;i<fEUclusterAB->GetEntries();i++){
    TArtGeCluster *hit =(TArtGeCluster*) fEUclusterAB->At(i);
    if(hit->GetEnergy()>1){
      if(fverbose>2)
	cout << fEUentry << "\t" << i <<"\t" << hit->GetEnergy() << "\t" << hit->GetTiming() << endl;
      
      EURICAHit *euhit = new EURICAHit(hit->GetChannel(),hit->GetEnergy(),hit->GetTiming(),1,flocalEUts);
      flocaleurica->AddHitAB(euhit);
    }
  }
  fEUentry++;

  detector* det = new detector;
  det->TS = flocalEUts;
  det->ID = 2;
  fdetectors.push_back(det);

  flastEUts = flocalEUts;
  return true;
}

bool BuildEvents::ReadEach(){
  flastBRts = 0;
  flastWAts = 0;
  flastEUts = 0;
  bool success = false;
  if(fhasEU)
    success += ReadEURICA();
  if(fhasWA)
    success += ReadWASABI();
  if(fhasBR)
    success += ReadBigRIPS();
  if(success)
    return true;
  else{
    cout << "failed to read from all WASABI, EURICA, and BigRIPS." << endl;
    return false;
  }
}
void BuildEvents::CloseEvent(){
  if(fverbose>0)
    cout << __PRETTY_FUNCTION__ << endl;
  // // bool printme = false;
  // if(fBRts>0 && fWAts >0){
  //   //printme = true;
  //   cout << "fBRentry = " << fBRentry << "\tfBRts = " << fBRts << ", fWAentry = " << fWAentry << "\tfWAts = " << fWAts << "\tmult = " << fwasabi->GetMult()<< endl;
  //   cout << "BR AoQ = " << fbeam->GetAQ(1) << " Z = " << fbeam->GetZ(1) << endl;
  //   fwasabi->Print();
  // }
  if(fverbose>0 && fBRts>0){
    cout << "closing event with local TS = " << flocalBRts << " tof = "<< flocalbeam->GetTOF(0)+48<< endl;
    cout << "closing event with set TS = " << fBRts << " tof = "<< fbeam->GetTOF(0)+48<< endl;
  }
  switch(fmode){
  default:
  case 0: //write all events
    fmtr->Fill();
    break;
  case 1://isomer data BR and WA coincidence
    if(fBRts>0 && fWAts >0)
      fmtr->Fill();
    break;
  }

  fBRts = 0;
  fbeam->Clear();
  for(unsigned short f=0;f<NFPLANES;f++){
    ffp[f]->Clear();
  }
  fWAts = 0;
  fwasabi->Clear();
  fEUts = 0;
  feurica->Clear();
  // if(printme){
  //   cout << "after clearing " << endl;
  //   fwasabi->Print();
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
    fbeam = (Beam*)flocalbeam->Clone();
    for(unsigned short f=0;f<NFPLANES;f++){
      ffp[f] = (FocalPlane*)flocalfp[f]->Clone(Form("fp_%d",f));
    }
    fcurrentts = fBRts;
    fdetectors.erase(fdetectors.begin());
    if(!ReadBigRIPS()&&fBRtsjump==false)
      cout << "failed to read BigRIPS, end of file" << endl;
    break;
  case 1: //WASABI
    if(flocalWAts - fcurrentts > fwindow){
      if(fverbose>0)
	cout << "WA larger than window" << endl;
      CloseEvent();
    }
    fWAts = flocalWAts;
    fwasabi = (WASABI*)flocalwasabi->Clone();
    flocalwasabi->Clear();
    fcurrentts = fWAts;
    fdetectors.erase(fdetectors.begin());
    if(!ReadWASABI()&&fWAtsjump==false)
      cout << "failed to read WASABI, end of file" << endl;
    break;
  case 2: //EURICA
    if(flocalEUts - fcurrentts > fwindow){
      if(fverbose>0)
	cout << "EU larger than window" << endl;
      CloseEvent();
    }
    fEUts = flocalEUts;
    feurica->AddHits(flocaleurica->GetHits());
    feurica->AddHitsAB(flocaleurica->GetHitsAB());
    flocaleurica->Clear();
    fcurrentts = fEUts;
    fdetectors.erase(fdetectors.begin());
    if(!ReadEURICA()&&fEUtsjump==false)
      cout << "failed to read EURICA, end of file" << endl;
    break;
         
  default:
    break;
  }
  if(flastevent>0 && flastevent == (int)(fBRentry + fWAentry + fEUentry)){
    cout << "last event reached " << endl;
    return false;
  }

  if(fdetectors.size()==0){
    if(fhasBR && fhasEU && fhasWA && fEUtsjump==true && fWAtsjump==true && fBRtsjump==true){
      cout << "all timestamps jumped" << endl;
      fEUtsjump = false;
      fWAtsjump = false;
      fBRtsjump = false;
      return ReadEach();
    }
    if(fhasBR & fhasWA && fWAtsjump==true && fBRtsjump==true){
      cout << "all timestamps jumped" << endl;
      fWAtsjump = false;
      fBRtsjump = false;
      return ReadEach();
    }
    cout << "all files finished " << endl;
    return false;
  }
  return true;
}
