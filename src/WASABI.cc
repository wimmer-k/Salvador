#include "WASABI.hh"
/*!
  Sort the hits by their energy, first the one with the lowest energy
  \param hits a vector with the unsorted hits
  \return a vector with the sorted hits
*/
vector<WASABIHit*> DSSSD::Revert(vector<WASABIHit*> hits){
  sort(hits.begin(), hits.end(), WASABIHitComparer());
  reverse(hits.begin(), hits.end());
  return hits;
}

/*!
  Sort the hits by their energy, first the one with the highest energy
  \param hits a vector with the unsorted hits
  \return a vector with the sorted hits
*/
vector<WASABIHit*> DSSSD::Sort(vector<WASABIHit*> hits){
  sort(hits.begin(), hits.end(), WASABIHitComparer());
  return hits;
}
/*!
  Addback neighboring hits
*/
void DSSSD::Addback(){
  //cout << "addback" << endl;
  vector<WASABIHit*> unusedhits;
  unusedhits.clear();
  for(unsigned short i=0;i<fhitsX.size();i++){
    unusedhits.push_back((WASABIHit*)fhitsX.at(i)->Clone());	
  }
  unusedhits = Revert(unusedhits);
  while(unusedhits.size() > 0){
    //first one is automatically good
    vector<WASABIHit*> currenthits; //current hits contains all hits that have already been added to thishit, to still know their positions
    currenthits.push_back(unusedhits.back());
    WASABIHit* thishit = unusedhits.back();
    unusedhits.pop_back();

    // how many hits added
    int added = 0;
    do{
      added =0;
      int addme =-1;
      bool found = false;
      for(unsigned short k=0;k<currenthits.size();k++){
	for(unsigned short j=0;j<unusedhits.size();j++){
	  if(Addback(currenthits.at(k),unusedhits.at(j))){	    
	    addme = j;
	    found = true;
	    break;
	  }
	}//loop unused hits
	if(found)
	  break;
      }//loop add back hits
      if(found && addme >-1){
	added++;
	currenthits.push_back(unusedhits.at(addme));
	thishit->AddBackHit(unusedhits.at(addme));
	unusedhits.erase(unusedhits.begin() + addme);
      }
    }while(added>0);//do
    //done with addback for thishit
    //add the addbacked hit into the vector
    AddABHitX(thishit);
  }//while unused hits

  //addback Ystrips
  unusedhits.clear();
  for(unsigned short i=0;i<fhitsY.size();i++){
    unusedhits.push_back((WASABIHit*)fhitsY.at(i)->Clone());	
  }
  unusedhits = Revert(unusedhits);
  while(unusedhits.size() > 0){
    //first one is automatically good
    vector<WASABIHit*> currenthits; //current hits contains all hits that have already been added to thishit, to still know their positions
    currenthits.push_back(unusedhits.back());
    WASABIHit* thishit = unusedhits.back();
    unusedhits.pop_back();

    // how many hits added
    int added = 0;
    do{
      added =0;
      int addme =-1;
      bool found = false;
      for(unsigned short k=0;k<currenthits.size();k++){
	for(unsigned short j=0;j<unusedhits.size();j++){
	  if(Addback(currenthits.at(k),unusedhits.at(j))){	    
	    addme = j;
	    found = true;
	    break;
	  }
	}//loop unused hits
	if(found)
	  break;
      }//loop add back hits
      if(found && addme >-1){
	added++;
	currenthits.push_back(unusedhits.at(addme));
	thishit->AddBackHit(unusedhits.at(addme));
	unusedhits.erase(unusedhits.begin() + addme);
      }
    }while(added>0);//do
    //done with addback for thishit
    //add the addbacked hit into the vector
    AddABHitY(thishit);
  }//while unused hits



  
  //Print();

}

/*!
  Checks the distance or angle between two hits
  \param hit0 first hit
  \param hit1 second hit
  \return true if the hits are presumably from one electron
*/
bool DSSSD::Addback(WASABIHit* hit0, WASABIHit* hit1){
  if(fabs(hit0->GetStrip() - hit1->GetStrip()) == 1)
    return true;
  return false;
}
