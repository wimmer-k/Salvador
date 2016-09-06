#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sys/time.h>
#include <signal.h>
#include "TArtStoreManager.hh"
#include "TArtEventStore.hh"
#include "TArtDALIParameters.hh"
#include "TArtEventInfo.hh"
#include "TArtCalibDALI.hh"
#include "TArtDALINaI.hh"

#include "TFile.h"
#include "TTree.h"
#include "TStopwatch.h"


#include "CommandLineInterface.hh"
#include "Settings.hh"

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
  cout << "\"Soft Self-portrait with Fried Bacon\" (1941), Salvador Dali" << endl;
  cout << "Unpacker for calibration data DALI" << endl;
  char* InputFile;
  char* OutputFile = NULL;
  char* SetFile = NULL;
  char* CutFile = NULL;
  int nmax = 0;
  int vl = 0;

  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "input file", &InputFile);
  interface->Add("-o", "output file", &OutputFile);
  interface->Add("-s", "settings file", &SetFile);
  interface->Add("-n", "nmax", &nmax);
  interface->Add("-v", "verbose", &vl);
  interface->CheckFlags(argc, argv);

  if(InputFile == NULL || OutputFile == NULL){
    cerr<<"You have to provide at least one input file and the output file!"<<endl;
    exit(1);
  }
  cout<<"input file:"<<InputFile<<endl;
  cout<<"output file: "<<OutputFile<< endl;
  if(SetFile == NULL)
    cerr<<"No settings file! Using standard values"<<endl;
  else
    cout<<"settings file:"<<SetFile<<endl;

  cout << "creating outputfile " << endl;
  TFile* outfile = new TFile(OutputFile,"recreate");
    
  if(outfile->IsZombie()){
    return 4;
  }
  Settings *set = new Settings(SetFile);

  // Create StoreManager both for calibration "TArtCalib..." and treatment "TArtReco..."
  //------------------------------------------------------------------------------------
  TArtStoreManager * sman = TArtStoreManager::Instance();

  // Create EventStore to control the loop and get the EventInfo
  //------------------------------------------------------------
  TArtEventStore *estore = new TArtEventStore();
  estore->SetInterrupt(&signal_received); 
  estore->Open(InputFile);
  std::cout<<"estore ->"<< InputFile <<std::endl;


  // Create DALIParameters to get ".xml"
  TArtDALIParameters *dpara = TArtDALIParameters::Instance();
  dpara->LoadParameter(set->DALIFile());
  
  // Create CalibDALI to get and calibrate raw data
  TArtCalibDALI *dalicalib = new TArtCalibDALI();

  // output tree
  TTree *tr = new TTree("tr","Data Tree");
  //branch for trig bit
  int trigbit = 0;
  tr->Branch("trigbit",&trigbit,"trigbit/I");
  //branch for DALI
  DALI *dali = new DALI;
  tr->Branch("dali",&dali,320000);
  

  int ctr =0;
  while(estore->GetNextEvent() && !signal_received){
    //clearing
    trigbit = 0;
    dali->Clear();

    
    //dali
    dalicalib->ClearData();
    dalicalib->SetVertex(0.0);
    //Add above to remove F8plastic tof.
    dalicalib->ReconstructData();

    //cout << "dalicalib->GetNumNaI() " << dalicalib->GetNumNaI() << endl;
    for(unsigned short g=0; g<dalicalib->GetNumNaI()-2; g++){//last two are junk?
      TArtDALINaI* hit = (TArtDALINaI*)dalicalib->GetNaIArray()->At(g);
      DALIHit *dhit = new DALIHit();
      dhit->SetID(hit->GetID());
      dhit->SetADC(hit->GetRawADC());
      dhit->SetTDC(hit->GetRawTDC());
      dhit->SetEnergy(hit->GetEnergy());
      dhit->SetPos(hit->GetXPos(),hit->GetYPos(),hit->GetZPos());
      dhit->SetDCEnergy(hit->GetDoppCorEnergy());	
      dhit->SetTime(hit->GetTime());  
      dhit->SetTOffset(hit->GetTimeOffseted());
      //      if(dhit->GetEnergy()>0)
      dali->AddHit(dhit);
    }
    

    //fill the tree
    tr->Fill();

    //output
    if(ctr%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << ctr << " events done " << setiosflags(ios::fixed) << setprecision(1) << (Float_t)ctr/(time_end - time_start) << " events/s \r" << flush;
    }
    ctr++;

    if(nmax>0 && ctr>nmax-1)
      break;
  }

  double time_end = get_time();
  cout << "Program Run time: " << time_end - time_start << " s." << endl;
  cout << "Total of " << ctr << " events processed, " << ctr/(time_end - time_start) << " events/s." << endl;
  cout << BLUE << tr->GetEntries() << DEFCOLOR << " entries written to tree ("<<BLUE<<tr->GetZipBytes()/(1024*1024)<< DEFCOLOR<<" MB)"<< endl;
  tr->Write("",TObject::kOverwrite);
  outfile->Close();
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
