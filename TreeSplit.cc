#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sys/time.h>
#include <signal.h>
#include "TArtStoreManager.hh"
#include "TArtEventStore.hh"
#include "TArtBigRIPSParameters.hh"
#include "TArtDALIParameters.hh"
#include "TArtCalibPID.hh"
#include "TArtCalibDALI.hh"
#include "TArtCalibPPAC.hh"
#include "TArtCalibPlastic.hh"
#include "TArtCalibIC.hh"
#include "TArtCalibFocalPlane.hh"
#include "TArtEventInfo.hh"
#include "TArtPlastic.hh"
#include "TArtIC.hh"
#include "TArtPPAC.hh"
#include "TArtRecoPID.hh"
#include "TArtRecoRIPS.hh"
#include "TArtRecoTOF.hh"
#include "TArtRecoBeam.hh"
#include "TArtFocalPlane.hh"
#include "TArtTOF.hh"
#include "TArtRIPS.hh"
#include "TArtBeam.hh"

#include "TFile.h"
#include "TChain.h"
#include "TCutG.h"
#include "TKey.h"
#include "TStopwatch.h"
#include "CommandLineInterface.hh"
using namespace TMath;
using namespace std;
const Int_t kMaxDALINaI = 147;

bool signal_received = false;
void signalhandler(int sig);
double get_time();
int main(int argc, char* argv[]){
  double time_start = get_time();  
  TStopwatch timer;
  timer.Start();
  signal(SIGINT,signalhandler);
  vector<char*> InputFiles;
  char* OutputFile = NULL;
  char* CutFile = NULL;
  int nmax =0;
  int vl =0;

  CommandLineInterface* interface = new CommandLineInterface();
  interface->Add("-i", "inputfiles", &InputFiles);
  interface->Add("-o", "outputfile", &OutputFile);
  interface->Add("-n", "nmax", &nmax);
  interface->Add("-v", "verbose", &vl);
  interface->CheckFlags(argc, argv);

  if(InputFiles.size() == 0 || OutputFile == NULL){
    cerr<<"You have to provide at least one input file and the output file!"<<endl;
    exit(1);
  }
  cout<<"input file(s):"<<endl;
  for(unsigned int i=0; i<InputFiles.size(); i++){
    cout<<InputFiles[i]<<endl;
  }
  cout<<"output file: "<<OutputFile<< endl;

  TChain* tr;
  tr = new TChain("tree");
  cout << "creating outputfile " << endl;
  TFile* outfile = new TFile(OutputFile,"recreate");
    
  if(outfile->IsZombie()){
    return 4;
  }
  for(unsigned int i=0; i<InputFiles.size(); i++){
    tr->Add(InputFiles[i]);
  }

  if(tr == NULL){
    cout << "could not find tree in file " << endl;
    for(unsigned int i=0; i<InputFiles.size(); i++){
      cout<<InputFiles[i]<<endl;
    }
    return 3;
  }
  Int_t trigbit =0;
  Double_t tof[3] ={0,0,0};
  Double_t beta[3] ={0,0,0};
  Double_t delta[4] ={0,0,0,0};
  Double_t zet[6] ={0,0,0,0,0,0};
  Double_t zetc[6] ={0,0,0,0,0,0};
  Double_t aoq[6] ={0,0,0,0,0,0};
  Double_t aoqc[6] ={0,0,0,0,0,0};
  tr->SetBranchAddress("fbit",&trigbit);
  tr->SetBranchAddress("tof",&tof);
  tr->SetBranchAddress("beta",&beta);
  tr->SetBranchAddress("delta",&delta);
  tr->SetBranchAddress("zet",&zet);
  tr->SetBranchAddress("zetc",&zetc);
  tr->SetBranchAddress("aoq",&aoq);
  tr->SetBranchAddress("aoqc",&aoqc);

  Int_t           DALINaI;
  UInt_t          DALINaI_fUniqueID[kMaxDALINaI];   //[DALINaI_]
  // UInt_t          DALINaI_fBits[kMaxDALINaI];   //[DALINaI_]
  Int_t           DALINaI_id[kMaxDALINaI];   //[DALINaI_]
  // Int_t           DALINaI_fpl[kMaxDALINaI];   //[DALINaI_]
  // TString         DALINaI_name[kMaxDALINaI];
  // Int_t           DALINaI_fDataState[kMaxDALINaI];   //[DALINaI_]
  // Int_t           DALINaI_fADC[kMaxDALINaI];   //[DALINaI_]
  // Int_t           DALINaI_fTDC[kMaxDALINaI];   //[DALINaI_]
  // Int_t           DALINaI_layer[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_theta[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fXPos[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fYPos[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fZPos[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_costheta[kMaxDALINaI];   //[DALINaI_]
  Double_t        DALINaI_fEnergy[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fDoppCorEnergy[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fEnergyWithoutT[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fTime[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fTimeOffseted[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fTimeTrueEnergy[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fTimeTrueDoppCorEnergy[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fTimeTrueTime[kMaxDALINaI];   //[DALINaI_]
  // Double_t        DALINaI_fTimeTrueTimeOffseted[kMaxDALINaI];   //[DALINaI_]

  //tr->SetBranchAddress("DALINaI",&DALINaI);
  tr->SetBranchAddress("DALINaI.fUniqueID",&DALINaI_fUniqueID);
  tr->SetBranchAddress("DALINaI.id",&DALINaI_id);
  tr->SetBranchAddress("DALINaI.fEnergy",&DALINaI_fEnergy);
  
  //objects//doesn't work
  //TClonesArray* ionchamber = new TClonesArray;
  //tr->SetBranchAddress("BigRIPSIC",&ionchamber);
  //TClonesArray *dalicalib = new TClonesArray();
  //tr->SetBranchAddress("DALINaI",&dalicalib);

  Double_t nentries = tr->GetEntries();
  Int_t nbytes = 0;
  Int_t status;
  cout << nentries << " entries in tree " << endl;
  if(nmax>0){
    nentries = nmax;
    cout << "reading until event " << nentries << endl;
  }
  for(int i=0; i<nentries;i++){
    if(signal_received){
      break;
    }
    if(vl>2)
      cout << "getting entry " << i << endl;
    status = tr->GetEvent(i);
    if(vl>2)
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

    //operations
    cout << "trigbit\t" << trigbit << endl;
    cout << "tof\t" << tof[0] << "\t" << tof[1] << "\t" << tof[2] << endl;
    cout << "DALINaI\t" << DALINaI_fUniqueID << endl;
    // for(int j=0;j<kMaxDALINaI;j++){
    //   cout << j << "\t"<< DALINaI_id[j] << "\t" << DALINaI_fEnergy[j] << endl;
    // }


    if(i%10000 == 0){
      double time_end = get_time();
      cout << setw(5) << setiosflags(ios::fixed) << setprecision(1) << (100.*i)/nentries <<
	" % done\t" << (Float_t)i/(time_end - time_start) << " events/s " << 
	(nentries-i)*(time_end - time_start)/(Float_t)i << "s to go \r" << flush;
    }
  }
  cout << endl; 

  outfile->Close();

  delete tr;
  double time_end = get_time();
  cout << "Run time " << time_end - time_start << " s." << endl;
 
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
