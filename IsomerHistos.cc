#include <iostream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <signal.h>
#include "TFile.h"
#include "TChain.h"
#include "TCutG.h"
#include "TH1.h"
#include "TH2.h"
#include "TList.h"
#include "TKey.h"
#include "TStopwatch.h"
#include "CommandLineInterface.hh"
#include "Globaldefs.h"
#include "Beam.hh"
#include "EURICA.hh"
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
  cout << "Histograms for isomers" << endl;
  int Verbose = 0;
  vector<char*> InputFiles;
  char *OutFile = NULL;
  char *CutFile = NULL;
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input files", &InputFiles);
  interface->Add("-o", "output file", &OutFile);    
  interface->Add("-c", "cut file", &CutFile);    
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
  if(CutFile == NULL){
    cout << "No cut file given " << endl;
  }
  cout<<"input file(s):"<<endl;
  for(unsigned int i=0; i<InputFiles.size(); i++){
    cout<<InputFiles[i]<<endl;
  }
  TChain* tr;
  tr = new TChain("mtr");
  for(unsigned int i=0; i<InputFiles.size(); i++){
    tr->Add(InputFiles[i]);
  }
  if(tr == NULL){
    //cout << "could not find tree "<< TreeName <<" in file " << endl;
    cout << "could not find tree mtr in file " << endl;
    for(unsigned int i=0; i<InputFiles.size(); i++){
      cout<<InputFiles[i]<<endl;
    }
    return 3;
  }

  vector<TCutG*> PidCut;
  //TCutG*
  //vector<TTree*> splittree;
  //Read in the cuts file for incoming and outgoing particle ID
  char* Name = NULL;
  TFile* cFile = new TFile(CutFile);
  TIter next(cFile->GetListOfKeys());
  TKey* key;
  while((key=(TKey*)next())){
    if(strcmp(key->GetClassName(),"TCutG") == 0){
      Name = (char*)key->GetName();
      if(strstr(Name,"pid")){
	cout << "incoming cut found "<<Name << endl;
	PidCut.push_back((TCutG*)cFile->Get(Name));
      }
    }      
  }
  //splittree.resize(PidCut.size());


  
  Beam *beam = new Beam;
  unsigned long long int BRts;
  EURICA *eurica = new EURICA;
  unsigned long long int EUts;
  tr->SetBranchAddress("beam",&beam);
  tr->SetBranchAddress("brTS",&BRts);
  tr->SetBranchAddress("eurica",&eurica);
  tr->SetBranchAddress("euTS",&EUts);
  cout << "creating histograms "<< endl;
  TList *hlist = new TList();
  TH1F* TSdiff = new TH1F("TSdiff","TSdiff",400,-200,200);hlist->Add(TSdiff);
  TH2F* pid = new TH2F("pid","pid",500,2.5,2.8,500,20,28);hlist->Add(pid);
  TH1F* mult = new TH1F("mult","mult",100,0,100);hlist->Add(mult);
  TH1F* egam = new TH1F("egam","egam",4000,0,2000);hlist->Add(egam);
  vector<TH2F*> en_vs_ti_big;
  vector<TH2F*> en_vs_ti;
  vector<TH2F*> en_vs_ti_small;
  en_vs_ti_big.resize(PidCut.size()+1);
  en_vs_ti.resize(PidCut.size()+1);
  en_vs_ti_small.resize(PidCut.size()+1);

  for(unsigned short i=0; i<PidCut.size();i++){
    en_vs_ti_big[i] = new TH2F(Form("en_vs_ti_big_%s",PidCut[i]->GetName()),Form("en_vs_ti_big_%s",PidCut[i]->GetName()),2200,-1e5,1e4,2000,0,2000);hlist->Add(en_vs_ti_big[i]);
    en_vs_ti[i] = new TH2F(Form("en_vs_ti_%s",PidCut[i]->GetName()),Form("en_vs_ti_%s",PidCut[i]->GetName()),6000,-5000,1000,2000,0,2000);hlist->Add(en_vs_ti[i]);
    en_vs_ti_small[i] = new TH2F(Form("en_vs_ti_small_%s",PidCut[i]->GetName()),Form("en_vs_ti_small_%s",PidCut[i]->GetName()),2000,-5000,1000,1500,0,1500);hlist->Add(en_vs_ti_small[i]);
  }

  en_vs_ti_big[PidCut.size()] = new TH2F("en_vs_ti_big_all","en_vs_ti_big_all",2200,-1e5,1e4,2000,0,2000);hlist->Add(en_vs_ti_big[PidCut.size()]);
  en_vs_ti[PidCut.size()] = new TH2F("en_vs_ti_all","en_vs_ti_all",6000,-5000,1000,2000,0,2000);hlist->Add(en_vs_ti[PidCut.size()]);
  en_vs_ti_small[PidCut.size()] = new TH2F("en_vs_ti_small_all","en_vs_ti_small_all",2000,-5000,1000,1500,0,1500);hlist->Add(en_vs_ti_small[PidCut.size()]);

  Double_t nentries = tr->GetEntries();
  cout << nentries << " entries in tree" << endl;

  Int_t nbytes = 0;
  Int_t status;
  double time_last = get_time();
  for(int i=0; i<nentries;i++){
    if(signal_received){
      break;
    }
    BRts = 0;
    beam->Clear();
    EUts = 0;
    eurica->Clear();
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
    if(BRts<1)
      continue;
    pid->Fill(beam->GetAQ(1),beam->GetZ(1));
    
    // if(BRts<1||EUts<1)
    //   continue;
    // TSdiff->Fill(1.0*EUts-BRts);
    
    // if(1.0*EUts-BRts<-37 || 1.0*EUts-BRts>-34)
    //   continue;

    // mult->Fill(eurica->GetHits().size());
    
    // for(unsigned short j=0;j<eurica->GetHits().size();j++){
    //   EURICAHit *hit = eurica->GetHit(j);
    //   egam->Fill(hit->GetEnergy());
    //   en_vs_ti_big[PidCut.size()]->Fill(hit->GetTime(),hit->GetEnergy());
    //   en_vs_ti[PidCut.size()]->Fill(hit->GetTime(),hit->GetEnergy());
    //   en_vs_ti_small[PidCut.size()]->Fill(hit->GetTime(),hit->GetEnergy());
    //   for(unsigned short k=0; k<PidCut.size();k++){
    // 	if(PidCut[k]->IsInside(beam->GetAQ(1),beam->GetZ(1))){
    // 	  en_vs_ti_big[k]->Fill(hit->GetTime(),hit->GetEnergy());
    // 	  en_vs_ti[k]->Fill(hit->GetTime(),hit->GetEnergy());
    // 	  en_vs_ti_small[k]->Fill(hit->GetTime(),hit->GetEnergy());
    // 	}
    //   }
    // }
    
    if(i%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*i)/nentries <<
	" % done\t" << (Float_t)i/(time_end - time_start) << " events/s (average) " << 
	10000/(time_end - time_last) << " events/s (current) " << 
	(nentries-i)*(time_end - time_start)/(Float_t)i << "s to go \r" << flush;
      time_last = time_end;
    }
  }
  cout << endl;
  cout << "creating outputfile " << endl;
  TFile* ofile = new TFile(OutFile,"recreate");
  ofile->cd();
  TH1F* h1;
  TH2F* h2;
  TIter nexthist(hlist);
  while( (h1 = (TH1F*)nexthist()) ){
    if(h1->GetEntries()>0)
      h1->Write("",TObject::kOverwrite);
  }
  while( (h2 = (TH2F*)nexthist()) ){
    if(h2->GetEntries()>0)
      h2->Write("",TObject::kOverwrite);
  }
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
