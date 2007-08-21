#include "Merger.h"
#include "Exceptions.h"
#include "marlin/Processor.h"

#include "lcio.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/SimCalorimeterHitImpl.h"
#include "IMPL/CalorimeterHitImpl.h"
#include "IMPL/TrackerHitImpl.h" 

#include "IMPL/SimTrackerHitImpl.h" 

using namespace EVENT ;
using namespace IMPL ;
using namespace marlin ;

#include <iostream>
using namespace std ;

  
  inline long long cellID2long(int id0, int id1) { return ((long long) id0 << 32) | id1; }
    

  void Merger::mergeMC(LCEvent* srcEvent, LCEvent* destEvent, string mcDestString) {
    int numMCcollections = 0;
    string mcSrcString;
    
    const vector<string>* srcColNames = srcEvent->getCollectionNames();
    vector<string>::const_iterator srcEndIt = srcColNames->end();
    for (vector<string>::const_iterator it = srcColNames->begin() ; it < srcEndIt ; it++ ) {
      if( (srcEvent->getCollection(*it))->getTypeName() == LCIO::MCPARTICLE ){
        numMCcollections++;
        mcSrcString = *it;
      }
    }
    if (numMCcollections == 1) {
//TRY?
      Merger::mergeMC(srcEvent, mcSrcString, destEvent, mcDestString);
    } else { 
      streamlog_out( WARNING ) << "Source collection number of MC Collections != 1" << endl;
    }
    return;
  }
  
  
  void Merger::mergeMC(LCEvent* srcEvent, string mcSrcString, LCEvent* destEvent, string mcDestString) {
    
    try {
      destEvent->getCollection(mcDestString);
    }
    catch (DataNotAvailableException& e) {   // if (does_not_exist(mcDestCollection)) {
      LCCollectionVec* mcVec = new LCCollectionVec( LCIO::MCPARTICLE )  ;
      destEvent->addCollection( mcVec , mcDestString ) ;
    }
     
    try {
      Merger::merge(srcEvent->getCollection(mcSrcString), destEvent->getCollection(mcDestString));
    }
    catch (exception& e) {
      streamlog_out( WARNING ) << "Failed to merge montecarlo collections" << endl;
      return;
    }
    srcEvent->removeCollection(mcSrcString);
    Merger::merge(srcEvent, destEvent);
    return;
  }
  
  
  void Merger::merge(LCEvent* srcEvent, LCEvent* destEvent) {
    const vector<string>* srcColNames = srcEvent->getCollectionNames();
    const vector<string>* destColNames = destEvent->getCollectionNames();
    
    vector<string>::const_iterator destBeginIt = destColNames->begin();
    vector<string>::const_iterator destEndIt   = destColNames->end();
    vector<string>::const_iterator srcEndIt    = srcColNames->end();
    for (vector<string>::const_iterator it = srcColNames->begin() ; it < srcEndIt ; it++ ) {
      if (destEndIt != find(destBeginIt, destEndIt, *it)) {

        Merger::merge(srcEvent->getCollection(*it), destEvent->getCollection(*it));
        
      }
    }
    return;
  }
  
  
  void Merger::merge(LCEvent* srcEvent, LCEvent* destEvent, map<string, string> *mergeMap) {
    
    map<string, string>::iterator it;
    map<string, string>::iterator endIt = mergeMap->end();
    for ( it=mergeMap->begin() ; it != endIt ; it++ ) {
      LCCollection *srcCol, *destCol;
      
      try {
        srcCol = srcEvent->getCollection((*it).first);
      } catch (DataNotAvailableException& e) {
        streamlog_out( WARNING ) << "Source collection " << (*it).first << " does not exist." << endl;
        continue;
      }
      
      try {
        destCol = destEvent->getCollection((*it).second);
      } catch (DataNotAvailableException& e) {
        streamlog_out( WARNING ) << "Destination collection " << (*it).second  << " was created." << endl;
        destCol = new LCCollectionVec( srcCol->getTypeName() ) ;
        destEvent->addCollection( destCol , (*it).second ) ;
      }
      
      Merger::merge(srcCol, destCol);
      
    }
    return;
  }

  
  void Merger::merge(LCEvent* srcEvent, string srcString, LCEvent* destEvent, string destString) {
    try {
      Merger::merge(srcEvent->getCollection(srcString), destEvent->getCollection(destString));
    }
    catch (exception& e) {
      streamlog_out( WARNING ) << e.what() << endl;
    }
    return;
  }
  
  
  void Merger::merge(LCCollection* src, LCCollection* dest) {
    int nElementsSrc, nElementsDest;
    const string destType = dest->getTypeName();
    
    // check if collections have the same type
    if (destType != src->getTypeName()) {
      streamlog_out( WARNING ) << "merge not possible, collections of different type" << endl;
      return;
    }
    
    streamlog_out( MESSAGE ) << "merging collection of type: " << destType << " --- ";
        
    // ** SIMTRACKERHIT **
    if ((destType == LCIO::SIMTRACKERHIT) || (destType == LCIO::MCPARTICLE))  {
      streamlog_out( MESSAGE ) << "merging" << endl;
      
      // running trough all the elements in the collection.
      nElementsSrc = src->getNumberOfElements();
      for (int i=nElementsSrc-1; i>=0; i--) {
        dest->addElement( src->getElementAt(i) );
        src->removeElementAt(i);
      }
      return;
    }
    
    // ** SIMCALORIMETERHIT **
    if (destType == LCIO::SIMCALORIMETERHIT ) {
      SimCalorimeterHitImpl *srcHit, *destHit;
    
      streamlog_out( MESSAGE ) << "merging" << endl;
      nElementsSrc = src->getNumberOfElements();
      nElementsDest = dest->getNumberOfElements();
      
      // create a map of dest Collection
      map<long long, SimCalorimeterHitImpl*> destMap;
      map<long long, SimCalorimeterHitImpl*>::iterator destMapIt;
      pair<map<long long, SimCalorimeterHitImpl*>::iterator,bool> res;
      for (int i=0; i<nElementsDest; i++) {
        destHit = dynamic_cast<SimCalorimeterHitImpl*> ( dest->getElementAt(i) );
        res = destMap.insert( pair<long long, SimCalorimeterHitImpl*>(cellID2long(destHit->getCellID0(), destHit->getCellID1()), destHit) );
      }

      // process the src collection and merge with dest
      for (int i=nElementsSrc-1; i>=0 ; i--) {
        srcHit = dynamic_cast<SimCalorimeterHitImpl*> ( src->getElementAt(i) );
        if ((destMapIt = destMap.find(cellID2long(srcHit->getCellID0(), srcHit->getCellID1()))) == destMap.end()) {
          dest->addElement( srcHit );
        } else {
          int numMC = srcHit->getNMCContributions();
          
          for( int j=0 ; j<numMC ; j++){
            destMapIt->second->addMCParticleContribution( srcHit->getParticleCont(j), srcHit->getEnergyCont(j), srcHit->getTimeCont(j), srcHit->getPDGCont(j));
          }
          
          delete srcHit;
        }
        src->removeElementAt(i);
      }
      return;
    }
    
    // ** DEFAULT **
    streamlog_out( MESSAGE ) << "merge not possible for this type" << endl;
    return;
  }
