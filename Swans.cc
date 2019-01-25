#include <iostream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <signal.h>
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
//#include "WASABIdefs.h"
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
  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input files", &InputFiles);
  interface->Add("-o", "output file", &OutFile);    
  interface->Add("-le", "last event to be read", &LastEvent);  
  interface->Add("-v", "verbose level", &Verbose);  

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

  TList *hlist = new TList();

  //histograms
  //TH1F* trigger = new TH1F("trigger","trigger",10,0,10);hlist->Add(trigger);
  TH1F* adc[NADCS][NADCCH];
  TH2F* adcthresh = new TH2F("adcthresh","adcthresh",NADCS*NADCCH,0,NADCS*NADCCH,200,0,200);;hlist->Add(adcthresh);
//  for(int i=0;i<NADCS;i++){
//    for(int j=0;j<NADCCH;j++){
//      adc[i][j] = new TH1F(Form("adc_%d_%d",i,j),Form("adc_%d_%d",i,j),4096,0,4096);hlist->Add(adc[i][j]);
//    }
//  }

  TH1F* multstripX[NDSSSD];
  TH1F* multstripY[NDSSSD];
  TH2F* en_stripX[NDSSSD];
  TH2F* en_stripY[NDSSSD];
  TH2F* en_XY[NDSSSD];
  TH2F* strip_XY[NDSSSD];
  for(int i=0;i<NDSSSD;i++){
    multstripX[i] = new TH1F(Form("multstripX_%d",i),Form("multstripX_%d",i),100,0,100);hlist->Add(multstripX[i]);
    multstripY[i] = new TH1F(Form("multstripY_%d",i),Form("multstripY_%d",i),100,0,100);hlist->Add(multstripY[i]);
    en_stripX[i] = new TH2F(Form("en_stripX_%d",i),Form("en_stripX_%d",i),NXSTRIPS,0,NXSTRIPS,2000,0,2000);hlist->Add(en_stripX[i]);
    en_stripY[i] = new TH2F(Form("en_stripY_%d",i),Form("en_stripY_%d",i),NYSTRIPS,0,NYSTRIPS,2000,0,2000);hlist->Add(en_stripY[i]);

    en_XY[i] = new TH2F(Form("en_XY_%d",i),Form("en_XY_%d",i),2000,0,2000,2000,0,2000);hlist->Add(en_XY[i]);
    strip_XY[i] = new TH2F(Form("strip_XY_%d",i),Form("strip_XY_%d",i),NXSTRIPS,0,NXSTRIPS,NYSTRIPS,0,NYSTRIPS);hlist->Add(strip_XY[i]);
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
    
    vector<WASABIRawADC*> adcs = wasabiRAW->GetADCs();
    for(vector<WASABIRawADC*>::iterator hit=adcs.begin(); hit!=adcs.end(); hit++){
      //adc[(*hit)->GetADC()][(*hit)->GetChan()]->Fill((*hit)->GetVal());
      adcthresh->Fill((*hit)->GetADC()*32+(*hit)->GetChan(),(*hit)->GetVal());
    }

    
    for(int d=0;d<NDSSSD;d++){
      vector<WASABIHit*> hitsX = wasabi->GetDSSSD(d)->GetHitsX();
      vector<WASABIHit*> hitsY = wasabi->GetDSSSD(d)->GetHitsY();
      multstripX[d]->Fill(wasabi->GetDSSSD(d)->GetMultX());
      multstripY[d]->Fill(wasabi->GetDSSSD(d)->GetMultY());
      
      for(vector<WASABIHit*>::iterator hit=hitsX.begin(); hit!=hitsX.end(); hit++){
	en_stripX[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
	for(vector<WASABIHit*>::iterator hity=hitsY.begin(); hity!=hitsY.end(); hity++){
	  if((*hit)->IsCal()){
	    if((*hity)->IsCal()){
	      en_XY[d]->Fill((*hit)->GetEn(),(*hity)->GetEn());
	      strip_XY[d]->Fill((*hit)->GetStrip(),(*hity)->GetStrip());
	    }
	  }
	}
      }
      for(vector<WASABIHit*>::iterator hit=hitsY.begin(); hit!=hitsY.end(); hit++){
	en_stripY[d]->Fill((*hit)->GetStrip(), (*hit)->GetEn());
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
