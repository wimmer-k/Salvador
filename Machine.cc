#include <iostream>
#include <fstream>
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
#include "FocalPlane.hh"
#include "Beam.hh"
#include "PPAC.hh"
#include "Reconstruction.hh"
#include "Globaldefs.h"


using namespace TMath;
using namespace std;
bool signal_received = false;
void signalhandler(int sig);
double get_time();
double deg2rad = TMath::Pi()/180.;
double rad2deg = 180./TMath::Pi();
int main(int argc, char* argv[]){
  double time_start = get_time();  
  TStopwatch timer;
  timer.Start();
  signal(SIGINT,signalhandler);
  cout << "\"Machine a coudre avec parapluies dans un paysage surrealiste\" (1941), Salvador Dali" << endl;
  cout << "Extracter for Machine Learning" << endl;
  int LastEvent =-1;
  int Verbose =0;
  vector<char*> InputFiles;
  char *OutFile = NULL;
  char *SetFile = NULL;
  char* TreeName = (char*)"tr";
  //Read in the command line arguments
  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input files", &InputFiles);
  interface->Add("-o", "output file", &OutFile);    
  interface->Add("-s", "settings file", &SetFile);    
  interface->Add("-tn", "name of the tree", &TreeName);
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
  if(SetFile == NULL){
    cout << "No settings file given " << endl;
    return 2;
  }
  cout<<"input file(s):"<<endl;
  for(unsigned int i=0; i<InputFiles.size(); i++){
    cout<<InputFiles[i]<<endl;
  }
  TChain* tr;
  tr = new TChain(TreeName);
  for(unsigned int i=0; i<InputFiles.size(); i++){
    tr->Add(InputFiles[i]);
  }
  if(tr == NULL){
    cout << "could not find tree "<< TreeName <<" in file " << endl;
    for(unsigned int i=0; i<InputFiles.size(); i++){
      cout<<InputFiles[i]<<endl;
    }
    return 3;
  }
  int trigbit = 0;
  tr->SetBranchAddress("trigbit",&trigbit);
  PPAC* ppac = new PPAC;
  tr->SetBranchAddress("ppacs",&ppac);
  Beam* beam = new Beam;
  tr->SetBranchAddress("beam",&beam);
  FocalPlane *fp[NFPLANES];
  for(unsigned short f=0;f<NFPLANES;f++){
    fp[f] = new FocalPlane;
    tr->SetBranchAddress(Form("fp%d",fpID[f]),&fp[f]);
  }

  cout<<"settings file: " << SetFile <<endl;
  Settings* setcor = new Settings(SetFile);
  Reconstruction *rec = new Reconstruction(SetFile);
  if(Verbose>0){
    rec->GetSettings()->SetVerboseLevel(Verbose);
    rec->GetSettings()->PrintSettings();
  }

  int incuts = 0;
  int oucuts = 0;

  vector<TCutG*> InCut;
  vector<TCutG*> OuCut;
  vector<double> InrealAoQ;
  vector<double> OurealAoQ;
  
  int BR_AoQ = 2;
  int ZD_AoQ = 5;
 
  double zrange[2] = {10,30};
  double aoqrange[2] = {2.2,2.8};
 
  TEnv* set = new TEnv("settings/learn.dat");
  char* cfilename = (char*)set->GetValue("CutFile","file");
  incuts = set->GetValue("NInCuts",0);
  oucuts = set->GetValue("NOuCuts",0);
  BR_AoQ = set->GetValue("UseBRAoQ",2);
  ZD_AoQ = set->GetValue("UseZDAoQ",5);

  zrange[0] = set->GetValue("Z.Range.Min",10);
  zrange[1] = set->GetValue("Z.Range.Max",30);
  aoqrange[0] = set->GetValue("AoQ.Range.Min",1.8);
  aoqrange[1] = set->GetValue("AoQ.Range.Max",2.2);

  TFile* cFile = new TFile(cfilename);
  for(int i=0;i<incuts;i++){
    char* iname = (char*)set->GetValue(Form("InCut.%d",i),"file");
    if(cFile->GetListOfKeys()->Contains(iname)){
      TCutG* icg = (TCutG*)cFile->Get(iname);
      InCut.push_back(icg);
      double a = (double)set->GetValue(Form("InRealA.%d",i),0.0);
      double z = (double)set->GetValue(Form("InRealZ.%d",i),0.0);
      InrealAoQ.push_back(a/z);
      cout << "incoming cut found "<< iname << " with A/Q " << InrealAoQ.back() << endl;
    }
    else{
      cout << "Incoming cut " << iname << " does not exist! Error in settings file" << endl;
      return 77;
    }
  }
  for(int i=0;i<oucuts;i++){
    char* ouame = (char*)set->GetValue(Form("OuCut.%d",i),"file");
    if(cFile->GetListOfKeys()->Contains(ouame)){
      TCutG* icg = (TCutG*)cFile->Get(ouame);
      OuCut.push_back(icg);
      double a = (double)set->GetValue(Form("OuRealA.%d",i),0.0);
      double z = (double)set->GetValue(Form("OuRealZ.%d",i),0.0);
      OurealAoQ.push_back(a/z);
      cout << "outgoing cut found "<< ouame << " with A/Q " << OurealAoQ.back() << endl;
    }
    else{
      cout << "Outgoing cut " << ouame << " does not exist! Error in settings file" << endl;
      return 77;
    }
  }
 

  
  TList *hlist = new TList();

  //histograms
  TH1F* trigger = new TH1F("trigger","trigger",10,0,10);hlist->Add(trigger);
  TH2F* bigrips = new TH2F("bigrips","bigrips",1000,aoqrange[0],aoqrange[1],1000,zrange[0],zrange[1]);hlist->Add(bigrips);
  TH2F* zerodeg = new TH2F("zerodeg","zerodeg",1000,aoqrange[0],aoqrange[1],1000,zrange[0],zrange[1]);hlist->Add(zerodeg);
  TH2F* bigripsC = new TH2F("bigripsC","bigripsC",1000,aoqrange[0],aoqrange[1],1000,zrange[0],zrange[1]);hlist->Add(bigripsC);
  TH2F* zerodegC = new TH2F("zerodegC","zerodegC",1000,aoqrange[0],aoqrange[1],1000,zrange[0],zrange[1]);hlist->Add(zerodegC);
  
  TH1F* delta[4];
  for(unsigned short b=0;b<4;b++){
    delta[b] = new TH1F(Form("delta_%d",b),Form("delta_%d",b),1000,-10,10);hlist->Add(delta[b]);
  }
  TH1F* deltadiff[2];
  for(unsigned short b=0;b<2;b++){
    deltadiff[b] = new TH1F(Form("deltadiff_%d",b),Form("deltadiff_%d",b),1000,-10,10);hlist->Add(deltadiff[b]);
  }
  //background inspection
  TH2F* PL3PPAC3   = new TH2F("PL3PPAC3","PL3PPAC3",    1000,-5,5,1000,-150,150);hlist->Add(PL3PPAC3);
  TH2F* PL7PPAC7   = new TH2F("PL7PPAC7","PL7PPAC7",    1000,-5,5,1000,-150,150);hlist->Add(PL7PPAC7);
  TH2F* PL9PPAC9   = new TH2F("PL9PPAC9","PL9PPAC9",    1000,-5,5,1000,-150,150);hlist->Add(PL9PPAC9);
  TH2F* PL11PPAC11 = new TH2F("PL11PPAC11","PL11PPAC11",1000,-5,5,1000,-150,150);hlist->Add(PL11PPAC11);
  TH2F* PL3PL3     = new TH2F("PL3PL3","PL3PL3",        1000,-5,5,1000,-5,5);hlist->Add(PL3PL3);
  TH2F* PL7PL7     = new TH2F("PL7PL7","PL7PL7",        1000,-5,5,1000,-5,5);hlist->Add(PL7PL7);
  TH2F* PL9PL9     = new TH2F("PL9PL9","PL9PL9",        1000,-5,5,1000,-5,5);hlist->Add(PL9PL9);
  TH2F* PL11PL11   = new TH2F("PL11PL11","PL11PL11",    1000,-5,5,1000,-5,5);hlist->Add(PL11PL11);


  char* date = "20211203";
  ofstream brout(Form("python/pid_data_BR_%s.dat", date));
  ofstream zdout(Form("python/pid_data_ZD_%s.dat", date));

  brout << "z\taoq\t";
  brout << "f3a\tf3x\tf3q\t";
  brout << "f5a\tf5x\t";
  brout << "f7a\tf7x\tf7q\t";
  brout << "raoq" << endl;
  zdout << "z\taoq\t";
  zdout << "f8a\tf8x\tf8q\t";
  zdout << "f9a\tf9x\t";
  zdout << "f11a\tf11x\tf11q\t";
  zdout << "raoq" << endl;
  
  

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
    trigbit = 0;
    for(int f=0;f<NFPLANES;f++){
      fp[f]->Clear();
    }
    ppac->Clear();
    beam->Clear();
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

    for(unsigned short b=0;b<3;b++){
      double corr = 0;
      corr = setcor->GetBRAoQCorrection_F3X()*fp[fpNr(3)]->GetTrack()->GetX() +
	setcor->GetBRAoQCorrection_F3A()*fp[fpNr(3)]->GetTrack()->GetA() +
	setcor->GetBRAoQCorrection_F3Q()*sqrt(fp[fpNr(3)]->GetPlastic()->GetChargeL() * fp[fpNr(3)]->GetPlastic()->GetChargeR()) +
	setcor->GetBRAoQCorrection_F5X()*fp[fpNr(5)]->GetTrack()->GetX() +
	setcor->GetBRAoQCorrection_F5A()*fp[fpNr(5)]->GetTrack()->GetA() +
	setcor->GetBRAoQCorrection_F7X()*fp[fpNr(7)]->GetTrack()->GetX() +
	setcor->GetBRAoQCorrection_F7A()*fp[fpNr(7)]->GetTrack()->GetA() +
	setcor->GetBRAoQCorrection_F7Q()*sqrt(fp[fpNr(7)]->GetPlastic()->GetChargeL() * fp[fpNr(7)]->GetPlastic()->GetChargeR());
      beam->CorrectAQ(b,corr);
      beam->ScaleAQ(b,setcor->GetBRAoQCorrection_gain(),setcor->GetBRAoQCorrection_offs());

      corr = setcor->GetZDAoQCorrection_F8X()*fp[fpNr(8)]->GetTrack()->GetX() +
	setcor->GetZDAoQCorrection_F8A()*fp[fpNr(8)]->GetTrack()->GetA() +
	setcor->GetZDAoQCorrection_F8Q()*sqrt(fp[fpNr(8)]->GetPlastic()->GetChargeL() * fp[fpNr(8)]->GetPlastic()->GetChargeR()) +
	setcor->GetZDAoQCorrection_F9X()*fp[fpNr(9)]->GetTrack()->GetX() +
	setcor->GetZDAoQCorrection_F9A()*fp[fpNr(9)]->GetTrack()->GetA() +
	setcor->GetZDAoQCorrection_F11X()*fp[fpNr(11)]->GetTrack()->GetX() +
	setcor->GetZDAoQCorrection_F11A()*fp[fpNr(11)]->GetTrack()->GetA() +
	setcor->GetZDAoQCorrection_F11Q()*sqrt(fp[fpNr(11)]->GetPlastic()->GetChargeL() * fp[fpNr(11)]->GetPlastic()->GetChargeR());
      beam->CorrectAQ(b+3,corr);
      beam->ScaleAQ(b+3,setcor->GetZDAoQCorrection_gain(),setcor->GetZDAoQCorrection_offs());
    }
    
    //gate on F5X position 
    if(!rec->F5XGate(fp[fpNr(5)]->GetTrack()->GetX()))
      continue;

    //gate on charge changes in ZeroDegree
    if(rec->ChargeChange(beam->GetDelta(2),beam->GetDelta(3)))
      continue;

    //analysis

    Plastic *pl3 = fp[fpNr(3)]->GetPlastic();
    PL3PPAC3->Fill(pl3->GetTimeL() - pl3->GetTimeR(), fp[fpNr(3)]->GetTrack()->GetX());
    PL3PL3->Fill(log(pl3->GetChargeL()/pl3->GetChargeR()), pl3->GetTimeL() - pl3->GetTimeR());
    Plastic *pl7 = fp[fpNr(7)]->GetPlastic();
    PL7PPAC7->Fill(pl7->GetTimeL() - pl7->GetTimeR(), fp[fpNr(7)]->GetTrack()->GetX());
    PL7PL7->Fill(log(pl7->GetChargeL()/pl7->GetChargeR()), pl7->GetTimeL() - pl7->GetTimeR());
    Plastic *pl9 = fp[fpNr(9)]->GetPlastic();
    PL9PPAC9->Fill(pl9->GetTimeL() - pl9->GetTimeR(), fp[fpNr(9)]->GetTrack()->GetX());
    PL9PL9->Fill(log(pl9->GetChargeL()/pl9->GetChargeR()), pl9->GetTimeL() - pl9->GetTimeR());
    Plastic *pl11 = fp[fpNr(11)]->GetPlastic();
    PL11PPAC11->Fill(pl11->GetTimeL() - pl11->GetTimeR(), fp[fpNr(11)]->GetTrack()->GetX());
    PL11PL11->Fill(log(pl11->GetChargeL()/pl11->GetChargeR()), pl11->GetTimeL() - pl11->GetTimeR());
    
    
    //histos
    trigger->Fill(trigbit);
    bigrips->Fill(beam->GetAQ(BR_AoQ),beam->GetZ(BR_AoQ));
    zerodeg->Fill(beam->GetAQ(ZD_AoQ),beam->GetZ(ZD_AoQ));
    bigripsC->Fill(beam->GetCorrAQ(BR_AoQ),beam->GetZ(BR_AoQ));
    zerodegC->Fill(beam->GetCorrAQ(ZD_AoQ),beam->GetZ(ZD_AoQ));
    //beam
    for(unsigned short b=0;b<4;b++)
      delta[b]->Fill(beam->GetDelta(b));
    deltadiff[0]->Fill(beam->GetDelta(1) - beam->GetDelta(0));
    deltadiff[1]->Fill(beam->GetDelta(3) - beam->GetDelta(2));
   
    double brz = beam->GetZ(BR_AoQ);
    double braoq = beam->GetAQ(BR_AoQ);

    double zdz = beam->GetZ(ZD_AoQ);
    double zdaoq = beam->GetAQ(ZD_AoQ);
    //cout << brz << "\t" << zdz << endl;

    double f3a = fp[0]->GetTrack()->GetA();
    double f3x = fp[0]->GetTrack()->GetX();
    double f3q = sqrt(fp[0]->GetPlastic()->GetChargeL() * fp[0]->GetPlastic()->GetChargeR());

    double f5a = fp[1]->GetTrack()->GetA();
    double f5x = fp[1]->GetTrack()->GetX();

    double f7a = fp[2]->GetTrack()->GetA();
    double f7x = fp[2]->GetTrack()->GetX();
    double f7q = sqrt(fp[2]->GetPlastic()->GetChargeL() * fp[2]->GetPlastic()->GetChargeR());

    double f8a = fp[3]->GetTrack()->GetA();
    double f8x = fp[3]->GetTrack()->GetX();
    double f8q = sqrt(fp[3]->GetPlastic()->GetChargeL() * fp[3]->GetPlastic()->GetChargeR());

    double f9a = fp[4]->GetTrack()->GetA();
    double f9x = fp[4]->GetTrack()->GetX();

    double f11a = fp[5]->GetTrack()->GetA();
    double f11x = fp[5]->GetTrack()->GetX();
    double f11q = sqrt(fp[5]->GetPlastic()->GetChargeL() * fp[5]->GetPlastic()->GetChargeR());

    double raoq = 0;
    for(int in=0;in<incuts;in++){
      if(InCut[in]->IsInside(beam->GetAQ(2),beam->GetZ(2)))
	raoq = InrealAoQ[in];
    }
    if(raoq>0){
      brout << brz << "\t" << braoq << "\t";
      brout << f3a << "\t" << f3x << "\t" << f3q << "\t";
      brout << f5a << "\t" << f5x << "\t";
      brout << f7a << "\t" << f7x << "\t" << f7q << "\t";
      brout << raoq << endl;
    }
    raoq = 0;
    for(int ou=0;ou<oucuts;ou++){
      if(OuCut[ou]->IsInside(beam->GetAQ(5),beam->GetZ(5)))
	raoq = OurealAoQ[ou];
    }
    if(raoq>0){
      zdout << zdz << "\t" << zdaoq << "\t";
      zdout << f8a << "\t" << f8x << "\t" << f8q << "\t";
      zdout << f9a << "\t" << f9x << "\t";
      zdout << f11a << "\t" << f11x << "\t" << f11q << "\t";
      zdout << raoq << endl;
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
