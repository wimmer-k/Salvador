#include <iostream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <signal.h>
#include <set>
#include "TMath.h"
#include "TFile.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCutG.h"
#include "TKey.h"
#include "TStopwatch.h"
#include "CommandLineInterface.hh"
#include "WASABI.hh"
#include "EURICA.hh"
#include "Beam.hh"
#include "FocalPlane.hh"

#include "Globaldefs.h"

using namespace TMath;
using namespace std;
bool signal_received = false;
void signalhandler(int sig);
double get_time();
int main(int argc, char* argv[]){
  double time_start = get_time();  
  TStopwatch timer;
  timer.Start();
  signal(SIGINT,signalhandler);
  cout << "\"Swans Reflecting Elephants\" (1937), Salvador Dali" << endl;
  cout << "Analyzer for WASABI" << endl;
  int LastEvent =-1;
  int Verbose =0;
  vector<char*> InputFiles;
  char *OutFile = NULL;
  char* CutFile = NULL;
  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input files", &InputFiles);
  interface->Add("-o", "output file", &OutFile);    
  interface->Add("-le", "last event to be read", &LastEvent);  
  interface->Add("-v", "verbose level", &Verbose);  
  interface->Add("-c", "cutfile", &CutFile);  

  interface->CheckFlags(argc, argv);
  //Complain about missing mandatory arguments
  if(InputFiles.size() == 0){
    cout << "No input file given " << endl;
    return 1;
  }
  if(OutFile == NULL){
    cout << "No output ROOT file given " << endl;
    OutFile = (char*)"test.root";
  }
  cout<<"input file(s):"<<endl;
  for(unsigned int i=0; i<InputFiles.size(); i++){
    cout<<InputFiles[i]<<endl;
  }
  TChain* tr;
  tr = new TChain("tr");
  for(unsigned int i=0; i<InputFiles.size(); i++){
    tr->Add(InputFiles[i]);
  }
  if(tr == NULL){
    cout << "could not find tree tr in file " << endl;
    for(unsigned int i=0; i<InputFiles.size(); i++){
      cout<<InputFiles[i]<<endl;
    }
    return 3;
  }
  // int trigbit = 0;
  // tr->SetBranchAddress("trigbit",&trigbit);
  WASABIRaw* wasabiRAW = new WASABIRaw;
  tr->SetBranchAddress("wasabiRAW",&wasabiRAW);
  WASABI* wasabi = new WASABI;
  tr->SetBranchAddress("wasabi",&wasabi);
  EURICA *eurica = new EURICA;
  tr->SetBranchAddress("eurica",&eurica);
  Beam* beam = new Beam;
  tr->SetBranchAddress("beam",&beam);
  FocalPlane *fp[NFPLANES];
  for(unsigned short f=0;f<NFPLANES;f++){
    fp[f] = new FocalPlane;
    tr->SetBranchAddress(Form("fp%d",fpID[f]),&fp[f]);
  }

  vector<TCutG*> PartCut;
  //Read in the cuts file for incoming and outgoing particle ID
  char* Name = NULL;
  char* Name2 = NULL;
  TFile* cFile = new TFile(CutFile);
  TIter nextcut(cFile->GetListOfKeys());
  TKey* key;
  while((key=(TKey*)nextcut())){
    if(strcmp(key->GetClassName(),"TCutG") == 0){
      Name = (char*)key->GetName();
      if(strstr(Name,"cut")){
	cout << "cut found "<<Name << endl;
	PartCut.push_back((TCutG*)cFile->Get(Name));
      }
    }      
  }

  set<int> badstrips = {13,14,28,29,30,31,44,45,46,47};
  
  TList *hlist = new TList();

  //histograms
  TH2F* bigrips = new TH2F("bigrips","bigrips",1000,1.8,2.2,1000,30,40);hlist->Add(bigrips);
  TH2F* zerodeg = new TH2F("zerodeg","zerodeg",1000,1.8,2.2,1000,30,40);hlist->Add(zerodeg);
  TH1F* f11ppacX = new TH1F("f11ppacX","f11ppacX",200,-100,100);hlist->Add(f11ppacX);
  //TH1F* trigger = new TH1F("trigger","trigger",10,0,10);hlist->Add(trigger);
  TH1F* adc[NADCS][NADCCH];
  TH2F* adcthresh = new TH2F("adcthresh","adcthresh",NADCS*NADCCH,0,NADCS*NADCCH,2000,0,2000);hlist->Add(adcthresh);
  TH2F* tdcoffset = new TH2F("tdcoffset","tdcoffset",NTDCS*NTDCCH,0,NTDCS*NTDCCH,200,0,20000);hlist->Add(tdcoffset);
//  for(int i=0;i<NADCS;i++){
//    for(int j=0;j<NADCCH;j++){
//      adc[i][j] = new TH1F(Form("adc_%d_%d",i,j),Form("adc_%d_%d",i,j),4096,0,4096);hlist->Add(adc[i][j]);
//    }
//  }
  TH2F* implantmap[NDSSSD];
  TH1F* multstripX[NDSSSD];
  TH1F* multstripY[NDSSSD];
  TH2F* en_stripX[NDSSSD];
  TH2F* en_stripY[NDSSSD];
  TH2F* en_multX[NDSSSD];
  TH2F* en_multY[NDSSSD];
  TH2F* ti_stripX[NDSSSD];
  TH2F* ti_stripY[NDSSSD];
  TH2F* ti_cal_stripX[NDSSSD];
  TH2F* ti_cal_stripY[NDSSSD];
  // TH2F* ti_cut_stripX[NDSSSD];
  // TH2F* ti_cut_stripY[NDSSSD];
  TH1F* multstripX_veto[NDSSSD];
  TH1F* multstripY_veto[NDSSSD];
  TH2F* en_stripX_veto[NDSSSD];
  TH2F* en_stripY_veto[NDSSSD];
  TH2F* en_multX_veto[NDSSSD];
  TH2F* en_multY_veto[NDSSSD];
  TH2F* en_XY[NDSSSD];
  TH2F* en_XY_veto[NDSSSD];
  TH2F* strip_XY[NDSSSD];




  TH1F* multstripABX[NDSSSD];
  TH1F* multstripABY[NDSSSD];
  TH1F* multstripABX_veto[NDSSSD];
  TH1F* multstripABY_veto[NDSSSD];
  TH2F* en_stripABX[NDSSSD];
  TH2F* en_stripABY[NDSSSD];
  TH2F* en_stripABX_veto[NDSSSD];
  TH2F* en_stripABY_veto[NDSSSD];
  TH2F* en_multABX_veto[NDSSSD];
  TH2F* en_multABY_veto[NDSSSD];
 
  // TH2F* cal_stripX[NDSSSD][NXSTRIPS];
  // TH2F* cal_stripY[NDSSSD][NYSTRIPS];

  // TH2F* cocal_stripX[NDSSSD][NXSTRIPS];
  // TH2F* cocal_stripY[NDSSSD][NXSTRIPS];
  
  TH2F* egamID = new TH2F("egamID","egamID",17*5,0,17*5,2000,0,2000);;hlist->Add(egamID);
  
  for(int i=0;i<NDSSSD;i++){
    implantmap[i] = new TH2F(Form("implantmap_%d",i),Form("implantmap_%d",i),60,0,60,40,0,40);hlist->Add(implantmap[i]);
    multstripX[i] = new TH1F(Form("multstripX_%d",i),Form("multstripX_%d",i),100,0,100);hlist->Add(multstripX[i]);
    multstripY[i] = new TH1F(Form("multstripY_%d",i),Form("multstripY_%d",i),100,0,100);hlist->Add(multstripY[i]);
    en_stripX[i] = new TH2F(Form("en_stripX_%d",i),Form("en_stripX_%d",i),NXSTRIPS,0,NXSTRIPS,1000,0,4000);hlist->Add(en_stripX[i]);
    en_stripY[i] = new TH2F(Form("en_stripY_%d",i),Form("en_stripY_%d",i),NYSTRIPS,0,NYSTRIPS,1000,0,4000);hlist->Add(en_stripY[i]);
    en_multX[i] = new TH2F(Form("en_multX_%d",i),Form("en_multX_%d",i),NXSTRIPS,0,NXSTRIPS,1000,0,4000);hlist->Add(en_multX[i]);
    en_multY[i] = new TH2F(Form("en_multY_%d",i),Form("en_multY_%d",i),NYSTRIPS,0,NYSTRIPS,1000,0,4000);hlist->Add(en_multY[i]);
    ti_stripX[i] = new TH2F(Form("ti_stripX_%d",i),Form("ti_stripX_%d",i),NXSTRIPS,0,NXSTRIPS,1000,-50000,50000);hlist->Add(ti_stripX[i]);
    ti_stripY[i] = new TH2F(Form("ti_stripY_%d",i),Form("ti_stripY_%d",i),NYSTRIPS,0,NYSTRIPS,1000,-50000,50000);hlist->Add(ti_stripY[i]);
    ti_cal_stripX[i] = new TH2F(Form("ti_cal_stripX_%d",i),Form("ti_cal_stripX_%d",i),NXSTRIPS,0,NXSTRIPS,4000,-2000,2000);hlist->Add(ti_cal_stripX[i]);
    ti_cal_stripY[i] = new TH2F(Form("ti_cal_stripY_%d",i),Form("ti_cal_stripY_%d",i),NYSTRIPS,0,NYSTRIPS,4000,-2000,2000);hlist->Add(ti_cal_stripY[i]);
    // ti_cut_stripX[i] = new TH2F(Form("ti_cut_stripX_%d",i),Form("ti_cut_stripX_%d",i),NXSTRIPS,0,NXSTRIPS,1000,-50000,50000);hlist->Add(ti_cut_stripX[i]);
    // ti_cut_stripY[i] = new TH2F(Form("ti_cut_stripY_%d",i),Form("ti_cut_stripY_%d",i),NYSTRIPS,0,NYSTRIPS,1000,-50000,50000);hlist->Add(ti_cut_stripY[i]);
    en_stripX_veto[i] = new TH2F(Form("en_stripX_veto_%d",i),Form("en_stripX_veto_%d",i),NXSTRIPS,0,NXSTRIPS,1000,0,2000);hlist->Add(en_stripX_veto[i]);
    en_stripY_veto[i] = new TH2F(Form("en_stripY_veto_%d",i),Form("en_stripY_veto_%d",i),NYSTRIPS,0,NYSTRIPS,1000,0,2000);hlist->Add(en_stripY_veto[i]);
    en_multX_veto[i] = new TH2F(Form("en_multX_veto_%d",i),Form("en_multX_veto_%d",i),NXSTRIPS,0,NXSTRIPS,1000,0,4000);hlist->Add(en_multX_veto[i]);
    en_multY_veto[i] = new TH2F(Form("en_multY_veto_%d",i),Form("en_multY_veto_%d",i),NYSTRIPS,0,NYSTRIPS,1000,0,4000);hlist->Add(en_multY_veto[i]);
    multstripX_veto[i] = new TH1F(Form("multstripX_veto_%d",i),Form("multstripX_veto_%d",i),100,0,100);hlist->Add(multstripX_veto[i]);
    multstripY_veto[i] = new TH1F(Form("multstripY_veto_%d",i),Form("multstripY_veto_%d",i),100,0,100);hlist->Add(multstripY_veto[i]);
    en_XY_veto[i] = new TH2F(Form("en_XY_veto_%d",i),Form("en_XY_veto_%d",i),1000,0,2000,1000,0,2000);hlist->Add(en_XY_veto[i]);

    en_XY[i] = new TH2F(Form("en_XY_%d",i),Form("en_XY_%d",i),2000,0,4000,2000,0,4000);hlist->Add(en_XY[i]);
    strip_XY[i] = new TH2F(Form("strip_XY_%d",i),Form("strip_XY_%d",i),NXSTRIPS,0,NXSTRIPS,NYSTRIPS,0,NYSTRIPS);hlist->Add(strip_XY[i]);


    multstripABX[i] = new TH1F(Form("multstripABX_%d",i),Form("multstripABX_%d",i),100,0,100);hlist->Add(multstripABX[i]);
    multstripABY[i] = new TH1F(Form("multstripABY_%d",i),Form("multstripABY_%d",i),100,0,100);hlist->Add(multstripABY[i]);
    multstripABX_veto[i] = new TH1F(Form("multstripABX_veto_%d",i),Form("multstripABX_veto_%d",i),100,0,100);hlist->Add(multstripABX_veto[i]);
    multstripABY_veto[i] = new TH1F(Form("multstripABY_veto_%d",i),Form("multstripABY_veto_%d",i),100,0,100);hlist->Add(multstripABY_veto[i]);
    en_stripABX[i] = new TH2F(Form("en_stripABX_%d",i),Form("en_stripABX_%d",i),NXSTRIPS,0,NXSTRIPS,1000,0,4000);hlist->Add(en_stripABX[i]);
    en_stripABY[i] = new TH2F(Form("en_stripABY_%d",i),Form("en_stripABY_%d",i),NYSTRIPS,0,NYSTRIPS,1000,0,4000);hlist->Add(en_stripABY[i]);
    en_stripABX_veto[i] = new TH2F(Form("en_stripABX_veto_%d",i),Form("en_stripABX_veto_%d",i),NXSTRIPS,0,NXSTRIPS,1000,0,2000);hlist->Add(en_stripABX_veto[i]);
    en_stripABY_veto[i] = new TH2F(Form("en_stripABY_veto_%d",i),Form("en_stripABY_veto_%d",i),NYSTRIPS,0,NYSTRIPS,1000,0,2000);hlist->Add(en_stripABY_veto[i]);
    en_multABX_veto[i] = new TH2F(Form("en_multABX_veto_%d",i),Form("en_multABX_veto_%d",i),NXSTRIPS,0,NXSTRIPS,1000,0,4000);hlist->Add(en_multABX_veto[i]);
    en_multABY_veto[i] = new TH2F(Form("en_multABY_veto_%d",i),Form("en_multABY_veto_%d",i),NYSTRIPS,0,NYSTRIPS,1000,0,4000);hlist->Add(en_multABY_veto[i]);

    // for(int j=0;j<NXSTRIPS;j++){
    //   cal_stripX[i][j] = new TH2F(Form("cal_stripX_%d_%d",i,j),Form("cal_stripX_%d_%d",i,j),2000,0,4000,2000,0,4000);hlist->Add(cal_stripX[i][j]);
    //   cocal_stripX[i][j] = new TH2F(Form("cocal_stripX_%d_%d",i,j),Form("cocal_stripX_%d_%d",i,j),1000,0,2000,1000,0,2000);hlist->Add(cocal_stripX[i][j]);
    // }
    // for(int j=0;j<NYSTRIPS;j++){
    //   cal_stripY[i][j] = new TH2F(Form("cal_stripY_%d_%d",i,j),Form("cal_stripY_%d_%d",i,j),2000,0,4000,2000,0,4000);hlist->Add(cal_stripY[i][j]);
    //   cocal_stripY[i][j] = new TH2F(Form("cocal_stripY_%d_%d",i,j),Form("cocal_stripY_%d_%d",i,j),1000,0,2000,1000,0,2000);hlist->Add(cocal_stripY[i][j]);
    // }
  }

  vector< vector<TH2F*> > implantmap_cut;
  vector< vector<TH2F*> > en_stripX_cut;
  vector< vector<TH2F*> > en_multX_cut;
  vector< vector<TH2F*> > en_F11X_cut;	  
  vector< vector<TH2F*> > en_stripABX_cut;
  vector< vector<TH2F*> > en_multABX_cut;
  vector< vector<TH2F*> > en_F11ABX_cut;	  

  implantmap_cut.resize(NDSSSD);
  en_stripX_cut.resize(NDSSSD);
  en_multX_cut.resize(NDSSSD);
  en_F11X_cut.resize(NDSSSD);  
  en_stripABX_cut.resize(NDSSSD);
  en_multABX_cut.resize(NDSSSD);
  en_F11ABX_cut.resize(NDSSSD);  

  for(int i=0;i<NDSSSD;i++){
    implantmap_cut[i].resize(PartCut.size());
    en_stripX_cut[i].resize(PartCut.size());
    en_multX_cut[i].resize(PartCut.size());
    en_F11X_cut[i].resize(PartCut.size());
    en_stripABX_cut[i].resize(PartCut.size());
    en_multABX_cut[i].resize(PartCut.size());
    en_F11ABX_cut[i].resize(PartCut.size());
    for(unsigned short j=0; j<PartCut.size();j++){
      implantmap_cut[i][j] = new TH2F(Form("implantmap_%s_%d",PartCut[j]->GetName(),i),Form("implantmap_%s_%d",PartCut[j]->GetName(),i),60,0,60,40,0,40);hlist->Add(implantmap_cut[i][j]);
      
      en_stripX_cut[i][j] =  new TH2F(Form("en_stripX_%s_%d",PartCut[j]->GetName(),i),Form("en_stripX_%s_%d",PartCut[j]->GetName(),i),NXSTRIPS,0,NXSTRIPS,2000,0,4000);hlist->Add(en_stripX_cut[i][j]);
      en_multX_cut[i][j] =  new TH2F(Form("en_multX_%s_%d",PartCut[j]->GetName(),i),Form("en_multX_%s_%d",PartCut[j]->GetName(),i),NXSTRIPS,0,NXSTRIPS,2000,0,4000);hlist->Add(en_multX_cut[i][j]);
      en_F11X_cut[i][j] =  new TH2F(Form("en_F11X_%s_%d",PartCut[j]->GetName(),i),Form("en_F11X_%s_%d",PartCut[j]->GetName(),i),200,-100,100,2000,0,4000);hlist->Add(en_F11X_cut[i][j]);
      en_stripABX_cut[i][j] =  new TH2F(Form("en_stripABX_%s_%d",PartCut[j]->GetName(),i),Form("en_stripABX_%s_%d",PartCut[j]->GetName(),i),NXSTRIPS,0,NXSTRIPS,2000,0,4000);hlist->Add(en_stripABX_cut[i][j]);
      en_multABX_cut[i][j] =  new TH2F(Form("en_multABX_%s_%d",PartCut[j]->GetName(),i),Form("en_multABX_%s_%d",PartCut[j]->GetName(),i),NXSTRIPS,0,NXSTRIPS,2000,0,4000);hlist->Add(en_multABX_cut[i][j]);
      en_F11ABX_cut[i][j] =  new TH2F(Form("en_F11ABX_%s_%d",PartCut[j]->GetName(),i),Form("en_F11ABX_%s_%d",PartCut[j]->GetName(),i),200,-100,100,2000,0,4000);hlist->Add(en_F11ABX_cut[i][j]);
    }
  }

  vector<TH2F*> cen_stripX_cut;
  vector<TH2F*> cen_multX_cut;
  vector<TH2F*> cen_F11X_cut;	  
  vector<TH2F*> cen_stripABX_cut;
  vector<TH2F*> cen_multABX_cut;
  vector<TH2F*> cen_F11ABX_cut;	  
  cen_stripX_cut.resize(PartCut.size());
  cen_multX_cut.resize(PartCut.size());
  cen_F11X_cut.resize(PartCut.size());
  cen_stripABX_cut.resize(PartCut.size());
  cen_multABX_cut.resize(PartCut.size());
  cen_F11ABX_cut.resize(PartCut.size());
  for(unsigned short j=0; j<PartCut.size();j++){
    cen_stripX_cut[j] =  new TH2F(Form("cen_stripX_%s",PartCut[j]->GetName()),Form("cen_stripX_%s",PartCut[j]->GetName()),NXSTRIPS,0,NXSTRIPS,2000,0,4000);hlist->Add(cen_stripX_cut[j]);
    cen_multX_cut[j] =  new TH2F(Form("cen_multX_%s",PartCut[j]->GetName()),Form("cen_multX_%s",PartCut[j]->GetName()),NXSTRIPS,0,NXSTRIPS,2000,0,4000);hlist->Add(cen_multX_cut[j]);
    cen_F11X_cut[j] =  new TH2F(Form("cen_F11X_%s",PartCut[j]->GetName()),Form("cen_F11X_%s",PartCut[j]->GetName()),200,-100,100,2000,0,4000);hlist->Add(cen_F11X_cut[j]);
    cen_stripABX_cut[j] =  new TH2F(Form("cen_stripABX_%s",PartCut[j]->GetName()),Form("cen_stripABX_%s",PartCut[j]->GetName()),NXSTRIPS,0,NXSTRIPS,2000,0,4000);hlist->Add(cen_stripABX_cut[j]);
    cen_multABX_cut[j] =  new TH2F(Form("cen_multABX_%s",PartCut[j]->GetName()),Form("cen_multABX_%s",PartCut[j]->GetName()),NXSTRIPS,0,NXSTRIPS,2000,0,4000);hlist->Add(cen_multABX_cut[j]);
    cen_F11ABX_cut[j] =  new TH2F(Form("cen_F11ABX_%s",PartCut[j]->GetName()),Form("cen_F11ABX_%s",PartCut[j]->GetName()),200,-100,100,2000,0,4000);hlist->Add(cen_F11ABX_cut[j]);
  }



  Double_t nentries = tr->GetEntries();
  cout << nentries << " entries in tree" << endl;
  if(nentries<1)
    return 4;
  if(LastEvent>0)
    nentries = LastEvent;
  
  Int_t nbytes = 0;
  Int_t status;
  for(int i=0; i<nentries;i++){
    if(signal_received){
      break;
    }
    wasabiRAW->Clear();
    wasabi->Clear();
    eurica->Clear();
    beam->Clear();
    for(int f=0;f<NFPLANES;f++){
      fp[f]->Clear();
    }
    if(Verbose>2)
      cout << "getting entry " << i << endl;
    status = tr->GetEvent(i);
    if(Verbose>2)
      cout << "status " << status << endl;
    if(status == -1){
      cerr<<"Error occured, couldn't read entry "<<i<<" from tree "<<tr->GetName()<<" in file "<<tr->GetFile()->GetName()<<endl;
      return 5;
    }
    else if(status == 0){
      cerr<<"Error occured, entry "<<i<<" in tree "<<tr->GetName()<<" in file "<<tr->GetFile()->GetName()<<" doesn't exist"<<endl;
      return 6;
    }
    nbytes += status;

    bigrips->Fill(beam->GetAQ(1),beam->GetZ(1));
    zerodeg->Fill(beam->GetAQ(5),beam->GetZ(5));
    f11ppacX->Fill(fp[fpNr(11)]->GetTrack()->GetX());


    vector<EURICAHit*> ghits = eurica->GetHits();
    for(vector<EURICAHit*>::iterator ghit=ghits.begin(); ghit!=ghits.end(); ghit++){
      egamID->Fill((*ghit)->GetID(),(*ghit)->GetEnergy());
    }
    
    vector<WASABIRawADC*> adcs = wasabiRAW->GetADCs();
    for(vector<WASABIRawADC*>::iterator hit=adcs.begin(); hit!=adcs.end(); hit++){
      //adc[(*hit)->GetADC()][(*hit)->GetChan()]->Fill((*hit)->GetVal());
      adcthresh->Fill((*hit)->GetADC()*NADCCH+(*hit)->GetChan(),(*hit)->GetVal());
    }
    vector<WASABIRawTDC*> tdcs = wasabiRAW->GetTDCs();
    for(vector<WASABIRawTDC*>::iterator hit=tdcs.begin(); hit!=tdcs.end(); hit++){
      for(unsigned short v = 0; v<(*hit)->GetVal().size(); v++){
	tdcoffset->Fill((*hit)->GetTDC()*NTDCCH+(*hit)->GetChan(),(*hit)->GetVal().at(v));
      }
    }
    
    for(int d=0;d<NDSSSD;d++){
      DSSSD *dsssd = wasabi->GetDSSSD(d);
      vector<WASABIHit*> hitsX = dsssd->GetHitsX();
      vector<WASABIHit*> hitsY = dsssd->GetHitsY();
      multstripX[d]->Fill(dsssd->GetMultX());
      multstripY[d]->Fill(dsssd->GetMultY());
      vector<WASABIHit*> hitsABX = dsssd->GetHitsABX();
      vector<WASABIHit*> hitsABY = dsssd->GetHitsABY();
      multstripABX[d]->Fill(dsssd->GetMultABX());
      multstripABY[d]->Fill(dsssd->GetMultABY());
      
      bool vetoX = dsssd->IsVetoX();
      bool vetoY = dsssd->IsVetoY();
      if(!vetoX){
	multstripX_veto[d]->Fill(dsssd->GetMultX());
	multstripABX_veto[d]->Fill(dsssd->GetMultABX());
      }
      if(!vetoY){
	multstripY_veto[d]->Fill(dsssd->GetMultY());
	multstripABY_veto[d]->Fill(dsssd->GetMultABY());
      }

      if(dsssd->ImplantX()>-1 && dsssd->ImplantY()>-1){
	implantmap[d]->Fill(dsssd->ImplantX(),dsssd->ImplantY());
        for(unsigned short j=0; j<PartCut.size();j++){
	  if(PartCut[j]->IsInside(beam->GetAQ(1),beam->GetZ(1))){
	    implantmap_cut[d][j]->Fill(dsssd->ImplantX(),dsssd->ImplantY());
	  }
	}
      }
     
      for(vector<WASABIHit*>::iterator hit=hitsX.begin(); hit!=hitsX.end(); hit++){
	en_stripX[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	ti_stripX[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	if(d==0 && (*hit)->GetEn()>4000)
	  ti_cal_stripX[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	if(d>0 && (*hit)->GetEn()>3000)
	  ti_cal_stripX[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	//if(!vetoX && (*hit)->GetEn()> 500 && (*hit)->GetEn()< 900)
	//  ti_cut_stripX[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	if(!vetoX){
	  en_stripX_veto[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	  en_multX_veto[d]->Fill(dsssd->GetMultX(), (*hit)->GetEn());
	}
	// if((*hit)->GetEn()>100&&dsssd->GetMultX()<3){
	//   for(vector<EURICAHit*>::iterator ghit=ghits.begin(); ghit!=ghits.end(); ghit++){
	//     cocal_stripX[d][(*hit)->GetStrip()]->Fill((*hit)->GetEn(), (*ghit)->GetEnergy());
	//     if((*ghit)->GetEnergy()>100)
	//       ti_cut_stripX[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	//   }
	// }
	for(unsigned short j=0; j<PartCut.size();j++){
	  if(!vetoX && PartCut[j]->IsInside(beam->GetAQ(1),beam->GetZ(1))){
	    en_stripX_cut[d][j]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	    en_multX_cut[d][j]->Fill(dsssd->GetMultX(), (*hit)->GetEn());
	    en_F11X_cut[d][j]->Fill(fp[fpNr(11)]->GetTrack()->GetX(), (*hit)->GetEn());
	  }
	}
	for(vector<WASABIHit*>::iterator hity=hitsY.begin(); hity!=hitsY.end(); hity++){
	  if((*hit)->IsCal()){
	    if((*hity)->IsCal()){
	      en_XY[d]->Fill((*hit)->GetEn(),(*hity)->GetEn());
	      if(!vetoX){
		en_XY_veto[d]->Fill((*hit)->GetEn(),(*hity)->GetEn());
	      }
	      strip_XY[d]->Fill((*hit)->GetStrip(),(*hity)->GetStrip());
	    }
	  }
	}
      }
      for(vector<WASABIHit*>::iterator hit=hitsABX.begin(); hit!=hitsABX.end(); hit++){
	en_stripABX[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	if(!vetoX){
	  en_stripABX_veto[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	  en_multABX_veto[d]->Fill(dsssd->GetMultABX(), (*hit)->GetEn());
	}
	for(unsigned short j=0; j<PartCut.size();j++){
	  if(!vetoX && PartCut[j]->IsInside(beam->GetAQ(1),beam->GetZ(1))){
	    en_stripABX_cut[d][j]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	    en_multABX_cut[d][j]->Fill(dsssd->GetMultABX(), (*hit)->GetEn());
	    en_F11ABX_cut[d][j]->Fill(fp[fpNr(11)]->GetTrack()->GetX(), (*hit)->GetEn());
	  }
	}
      }
      
      for(vector<WASABIHit*>::iterator hit=hitsY.begin(); hit!=hitsY.end(); hit++){
	en_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	ti_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	if(d==0 && (*hit)->GetEn()>4000)
	  ti_cal_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	if(d==1 && (*hit)->GetStrip()<20 && (*hit)->GetEn()>2000)
	  ti_cal_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	if(d==1 && (*hit)->GetStrip()>19 && (*hit)->GetEn()>4000)
	  ti_cal_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());	
	if(d==2 && (*hit)->GetEn()>3000)
	  ti_cal_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());	
	// if(!vetoY && (*hit)->GetEn()> 500 && (*hit)->GetEn()< 900)
	//   ti_cut_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	if(!vetoY){
	  en_stripY_veto[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	  en_multY_veto[d]->Fill(dsssd->GetMultY(), (*hit)->GetEn());
	}
	// if((*hit)->GetEn()>100&&dsssd->GetMultY()<3){
	//   for(vector<EURICAHit*>::iterator ghit=ghits.begin(); ghit!=ghits.end(); ghit++){
	//     cocal_stripY[d][(*hit)->GetStrip()]->Fill((*hit)->GetEn(), (*ghit)->GetEnergy());
	//     if((*ghit)->GetEnergy()>100)
	//       ti_cut_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetTime0());
	//   }
	// }
      }
      //mult one hits front back correlation
      // if(dsssd->GetMultX()==1 && dsssd->GetMultY()==1){
      // 	if(!hitsX.at(0)->IsCal() && hitsY.at(0)->IsCal())
      // 	  cal_stripX[d][hitsX.at(0)->GetStrip()]->Fill(hitsX.at(0)->GetEn() , hitsY.at(0)->GetEn());
      // 	if(hitsX.at(0)->IsCal() && !hitsY.at(0)->IsCal())
      // 	  cal_stripY[d][hitsY.at(0)->GetStrip()]->Fill(hitsY.at(0)->GetEn() , hitsX.at(0)->GetEn());
      // }
      for(vector<WASABIHit*>::iterator hit=hitsABY.begin(); hit!=hitsABY.end(); hit++){
	en_stripABY[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	if(!vetoY){
	  en_stripABY_veto[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	  en_multABY_veto[d]->Fill(dsssd->GetMultABY(), (*hit)->GetEn());
	}
      }
    }//loop dsssds


    //special histos
    if(wasabi->GetDSSSD(0)->ImplantX()>-1 && wasabi->GetDSSSD(0)->ImplantY()>-1 && !wasabi->GetDSSSD(1)->IsVetoX()){
      vector<WASABIHit*> hitsX = wasabi->GetDSSSD(1)->GetHitsX();
      wasabi->GetDSSSD(1)->ClearAddback();
      vector<WASABIHit*> output;
      output.clear();
      for(vector<WASABIHit*>::iterator hit=hitsX.begin(); hit!=hitsX.end(); hit++){
	// search for the iterator of given string in set
	set<int>::iterator it = badstrips.find((*hit)->GetStrip());
	if(it == badstrips.end())
	  output.push_back((*hit));
      }
      wasabi->GetDSSSD(1)->SetHitsX(output);
      wasabi->GetDSSSD(1)->Addback();

      vector<WASABIHit*> chitsX = wasabi->GetDSSSD(1)->GetHitsX();
      vector<WASABIHit*> chitsABX = wasabi->GetDSSSD(1)->GetHitsABX();
      
      for(unsigned short j=0; j<PartCut.size();j++){
	if(PartCut[j]->IsInside(beam->GetAQ(1),beam->GetZ(1))){
	  for(vector<WASABIHit*>::iterator hit=chitsX.begin(); hit!=chitsX.end(); hit++){
	    cen_stripX_cut[j]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	    cen_multX_cut[j]->Fill(wasabi->GetDSSSD(1)->GetMultX(), (*hit)->GetEn());
	    cen_F11X_cut[j]->Fill(fp[fpNr(11)]->GetTrack()->GetX(), (*hit)->GetEn());
	  }
	  for(vector<WASABIHit*>::iterator hit=chitsABX.begin(); hit!=chitsABX.end(); hit++){
	    cen_stripABX_cut[j]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	    cen_multABX_cut[j]->Fill(wasabi->GetDSSSD(1)->GetMultABX(), (*hit)->GetEn());
	    cen_F11ABX_cut[j]->Fill(fp[fpNr(11)]->GetTrack()->GetX(), (*hit)->GetEn());
	  }
	}
      }

    }
      

    
    if(i%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*i)/nentries <<
	" % done\t" << (Float_t)i/(time_end - time_start) << " events/s " << 
	(nentries-i)*(time_end - time_start)/(Float_t)i << "s to go \r" << flush;
    }
  }


  cout << endl;
  cout << "creating outputfile " << OutFile << endl;
  TFile* ofile = new TFile(OutFile,"recreate");
  ofile->cd();
  TH1F* h1;
  TH2F* h2;
  TIter next(hlist);
  while( (h1 = (TH1F*)next()) ){
    if(h1->GetEntries()>0)
      h1->Write("",TObject::kOverwrite);
  }
  while( (h2 = (TH2F*)next()) ){
    if(h2->GetEntries()>0)
      h2->Write("",TObject::kOverwrite);
  }
  ofile->Close();
  double time_end = get_time();
  cout << "Program Run time: " << time_end - time_start << " s." << endl;
  timer.Stop();
  cout << "CPU time: " << timer.CpuTime() << "\tReal time: " << timer.RealTime() << endl;
  return 0;
}
void signalhandler(int sig){
  if (sig == SIGINT){
    signal_received = true;
  }
}

double get_time(){  
    struct timeval t;  
    gettimeofday(&t, NULL);  
    double d = t.tv_sec + (double) t.tv_usec/1000000;  
    return d;  
}  
