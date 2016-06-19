#include <iostream>
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <signal.h>
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TCutG.h"
#include "TKey.h"
#include "TStopwatch.h"
#include "CommandLineInterface.hh"
#include "FocalPlane.hh"
#include "PPAC.hh"
#include "Beam.hh"
#include "DALI.hh"
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
  cout << "\"La Desintegracion de la Persistencia de la Memoria\" (1954), Salvator Dali" << endl;
  cout << "Treesplitter for DALI" << endl;
  int LastEvent = -1;
  int Verbose = 0;
  char* InputFile = NULL;
  char* OutFile = NULL;
  char* CutFile = NULL;
  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input file", &InputFile);
  interface->Add("-o", "output file", &OutFile);    
  interface->Add("-c", "cutfile", &CutFile);
  interface->Add("-le", "last event to be read", &LastEvent);  
  interface->Add("-v", "verbose level", &Verbose);  
  interface->CheckFlags(argc, argv);
  //Complain about missing mandatory arguments
  if(InputFile == NULL){
    cout << "No input file given " << endl;
    return 1;
  }
  if(OutFile == NULL){
    cout << "No output ROOT file given " << endl;
    return 2;
  }
  if(CutFile == NULL){
    cout << "No cut file given " << endl;
    return 3;
  }
  TFile* infile = new TFile(InputFile);
  TTree* tr = (TTree*) infile->Get("tr");
  if(tr == NULL){
    cout << "could not find tree tr in file " << infile->GetName() << endl;
    return 4;
  }
  int trigbit = 0;
  tr->SetBranchAddress("trigbit",&trigbit);
  PPAC* ppac = new PPAC;
  tr->SetBranchAddress("ppacs",&ppac);
  FocalPlane* fp[NFPLANES];
  for(unsigned short f=0;f<NFPLANES;f++){
    fp[f] = new FocalPlane;
    tr->SetBranchAddress(Form("fp%d",fpID[f]),&fp[f]);
  }
  Beam* beam = new Beam;
  tr->SetBranchAddress("beam",&beam);
  DALI* dali = new DALI;
  tr->SetBranchAddress("dali",&dali);

  cout<<"output file: "<<OutFile<< endl;

  vector<TCutG*> InPartCut;
  vector<vector<TCutG*> > OutPartCut;
  vector<vector<TTree*> > splittree;
  //Read in the cuts file for incoming and outgoing particle ID
  char* Name = NULL;
  char* Name2 = NULL;
  TFile* cFile = new TFile(CutFile);
  TIter next(cFile->GetListOfKeys());
  TKey* key;
  while((key=(TKey*)next())){
    if(strcmp(key->GetClassName(),"TCutG") == 0){
      Name = (char*)key->GetName();
      if(strstr(Name,"in") && !strstr(Name,"out")){
	cout << "incoming cut found "<<Name << endl;
	InPartCut.push_back((TCutG*)cFile->Get(Name));
      }
    }      
  }
  TIter next2(cFile->GetListOfKeys());
  OutPartCut.resize(InPartCut.size());
  
  while((key=(TKey*)next2())){
    if(strcmp(key->GetClassName(),"TCutG") == 0){
      Name = (char*)key->GetName();
      if(strstr(Name,"in") && strstr(Name,"out")){
	for(unsigned short i=0;i<InPartCut.size();i++){
	  Name2 = (char*)InPartCut[i]->GetName();
	  if(strstr(Name,strstr(Name2,Name2))){
	    OutPartCut[i].push_back((TCutG*)cFile->Get(Name));
	    cout << "outgoing cut found "<<Name << endl;
	  }
	}
      }
    }
  }

  cFile->Close();
  splittree.resize(InPartCut.size());
  for(UShort_t in=0;in<InPartCut.size();in++){ // loop over incoming cuts
    splittree[in].resize(1+OutPartCut[in].size());
    splittree[in][OutPartCut[in].size()] = new TTree(Form("tr_%s",InPartCut[in]->GetName()),Form("Data Tree with cut on %s",InPartCut[in]->GetName()));
    for(UShort_t ou=0;ou<OutPartCut[in].size()+1;ou++){ // loop over PID cuts
      if(ou<OutPartCut[in].size())
	splittree[in][ou] = new TTree(Form("tr_%s",OutPartCut[in][ou]->GetName()),Form("Data Tree with cut on %s",OutPartCut[in][ou]->GetName()));
      splittree[in][ou]->Branch("trigbit",&trigbit,"trigbit/I");
      for(unsigned short f=0;f<NFPLANES;f++)
	splittree[in][ou]->Branch(Form("fp%d",fpID[f]),&fp[f],320000);
      splittree[in][ou]->Branch("ppacs",&ppac,320000);
      splittree[in][ou]->Branch("beam",&beam,320000);
      splittree[in][ou]->Branch("dali",&dali,320000);
    }
  }

  Double_t nentries = tr->GetEntries();
  cout << nentries << " entries in tree" << endl;
  if(LastEvent>0)
    nentries = LastEvent;
  
  Int_t nbytes = 0;
  Int_t status;
  for(int i=0; i<nentries;i++){
    if(signal_received){
      break;
    }
    trigbit = 0;
    for(int f=0;f<NFPLANES;f++){
      fp[f]->Clear();
    }
    ppac->Clear();
    beam->Clear();
    dali->Clear();
    if(Verbose>2)
      cout << "getting entry " << i << endl;
    status = tr->GetEvent(i);
    if(Verbose>2)
      cout << "status " << status << endl;
    if(status == -1){
      cerr<<"Error occured, couldn't read entry "<<i<<" from tree "<<tr->GetName()<<" in file "<<infile->GetName()<<endl;
      return 5;
    }
    else if(status == 0){
      cerr<<"Error occured, entry "<<i<<" in tree "<<tr->GetName()<<" in file "<<infile->GetName()<<" doesn't exist"<<endl;
      return 6;
    }
    nbytes += status;
    
    //start analysis
    for(UShort_t in=0;in<InPartCut.size();in++){ // loop over incoming cuts
      if(InPartCut[in]->IsInside(beam->GetAQ(1),beam->GetZ(1))){
	splittree[in][OutPartCut[in].size()]->Fill();
	for(UShort_t ou=0;ou<OutPartCut[in].size();ou++){ // loop over outgoing cuts
	  if(OutPartCut[in][ou]->IsInside(beam->GetAQ(5),beam->GetZ(5))){
	    splittree[in][ou]->Fill();
	  }
	}//outpartcuts
      }//inpartcuts
    }
 
    if(i%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*i)/nentries <<
	" % done\t" << (Float_t)i/(time_end - time_start) << " events/s " << 
	(nentries-i)*(time_end - time_start)/(Float_t)i << "s to go \r" << flush;
    }
  }
  cout << endl;
  cout << "creating outputfile " << endl;
  TFile* ofile = new TFile(OutFile,"recreate");
  ofile->cd();
  Long64_t filesize =0;
  for(UShort_t in=0;in<InPartCut.size();in++){ // loop over incoming cuts
    for(UShort_t ou=0;ou<OutPartCut[in].size()+1;ou++){ // loop over outgoing cuts
      splittree[in][ou]->Write("",TObject::kOverwrite);
      filesize += splittree[in][ou]->GetZipBytes();
    }
  }
  cout<<"Size of input tree  "<<setw(7)<<tr->GetZipBytes()/(1024*1024)<<" MB"<<endl
      <<"size of splitted trees "<<setw(7)<<filesize/(1024*1024)<<" MB"<<endl
      <<"=> size of splitted tree(s) is "<<setw(5)<<setiosflags(ios::fixed)<<setprecision(1)<<(100.*filesize)/tr->GetZipBytes()<<" % of size of input tree"<<endl;
  ofile->Close();
  infile->Close();
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
