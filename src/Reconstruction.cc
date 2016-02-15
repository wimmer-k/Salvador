#include "Reconstruction.hh"
using namespace std;

/*!
  Constructor, reads in the settings for the reconstruction
  \param settings the settings file
*/
Reconstruction::Reconstruction(char *settings){
  TEnv *set = new TEnv(settings);
  fverbose = set->GetValue("VerboseLevel",0);
  fbeta = set->GetValue("AverageBeta",0.5);
  string posfile = set->GetValue("InteractionPoints",(char*)"settings/iponts.dat");
  if(fverbose>0){
    cout << "fverbose " << fverbose << endl;
    cout << "fbeta " << fbeta << endl;
    cout << "positions " << posfile << endl;
  }
  ReadPositions((const char*)posfile.c_str());
  if(fverbose>1){
    for(unsigned short i=0;i<fpositions.size();i++){
      cout << i << "\t" << fpositions[i][0]<< "\t" << fpositions[i][1]<< "\t" << fpositions[i][2]<<endl;
    }
  }
}
/*!
  Reads in the average positions of first interaction points from the simulation
  \param infile the file with the interaction point averages
*/
void Reconstruction::ReadPositions(const char *infile){
  TEnv *pos = new TEnv(infile);
  for(int i=0;i<MAXNCRYSTAL;i++){
    vector<double> r;
    r.resize(3);
    r.at(0) = pos->GetValue(Form("Average.Position.X.%d",i),0.0);
    r.at(1) = pos->GetValue(Form("Average.Position.Y.%d",i),0.0);
    r.at(2) = pos->GetValue(Form("Average.Position.Z.%d",i),0.0);
    fpositions.push_back(r);
  }
}
/*!
  Sort the hits by their energy, first the one with the highest energy
  \param hits a vector with the unsorted hits
  \returns a vector with the sorted hits
*/
vector<DALIHit*> Reconstruction::Sort(vector<DALIHit*> hits){
  if(fverbose>2){
    cout << "before sorting " << endl;
    for(vector<DALIHit*>::iterator hit0=hits.begin(); hit0!=hits.end(); hit0++){
      cout << (*hit0)->GetID() <<"\t"<< (*hit0)->GetEnergy() << endl;
    }
  }
  sort(hits.begin(), hits.end(), HitComparer());
  if(fverbose>2){
    cout << "after sorting " << endl;
    for(vector<DALIHit*>::iterator hit0=hits.begin(); hit0!=hits.end(); hit0++){
      cout << (*hit0)->GetID() <<"\t"<<(*hit0)->GetEnergy() << endl;
    }
  }
  return hits;
}
/*!
  Sets the positions of the DALIHits to the average positions determined from the simulation
  \param dali the input DALI object
*/
void Reconstruction::SetPositions(DALI* dali){
  for(unsigned short g=0;g<dali->GetMult();g++){
    DALIHit * hit = dali->GetHit(g);
    short id = hit->GetID();
    if(id<0||id>MAXNCRYSTAL){
      cout << "invalid ID in DALI" <<endl;
      hit->Print();
      return;
    }      
    //cout << id << "\t" << fpositions[id][0] << endl;
    hit->SetPos(fpositions[id][0],fpositions[id][1],fpositions[id][2]);
    hit->DopplerCorrect(fbeta);
    if(fverbose>2)
      cout << id << "\t" << hit->GetEnergy()<< "\t" << hit->GetDCEnergy()<<endl;
  }

}

// /*!

//  */
// void Reconstruction::ReadAddBackTable(){
  
// }
