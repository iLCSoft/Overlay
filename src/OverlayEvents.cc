
#include "OverlayEvents.h"
#include <iostream>

#ifdef MARLIN_USE_AIDA
#include <marlin/AIDAProcessor.h>
#include <AIDA/IHistogramFactory.h>
#include <AIDA/ICloud1D.h>
//#include <AIDA/IHistogram1D.h>
#endif

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/LCEventImpl.h>
#include "UTIL/LCTOOLS.h"
#include "Merger.h"

// #include <time.h>

using namespace lcio ;
using namespace marlin ;


OverlayEvents aOverlayEvents ;


OverlayEvents::OverlayEvents() : Processor("OverlayEvents") {
  
  // modify processor description
  _description = "Merge a number of events in a LCIO file into 1 event" ;
  
  
  // register steering parameters: name, description, class-variable, default value

  StringVec collections ;
  collections.push_back( "VXDCollection");
  collections.push_back( "SITCollection");
  collections.push_back( "SETCollection");
  collections.push_back( "FTDCollection");
  registerProcessorParameter( "MergedCollectionNames" ,
			      "Name of collections will be merged" ,
			      _mergedCollectionNames ,
			      collections );
  
  std::string outfileName("overlayEvent.slcio");
  registerProcessorParameter( "OutputFileName" , 
			      "Name of the lcio output file" ,
			      _outfileName ,
 			      outfileName ) ;  
   
}



void OverlayEvents::init() { 
  
  
  if( ! LCIO_VERSION_GE( 1 , 4 ) ) {
    throw Exception("  OverlayEvents requires LCIO v1.4 or higher \n"
 		    "  - please upgrade your LCIO version or disable the OverlayEvents processor ! ") ;
  }
  
  // usually a good idea to
  printParameters() ;
    
  
  // define output event
  _lcWriter = LCFactory::getInstance()->createLCWriter();
  _lcWriter->open(_outfileName , LCIO::WRITE_NEW);
  
  outEvt = new LCEventImpl();
  outEvt->setRunNumber(0);
  outEvt->setEventNumber(0);
        
  _nRun = 0 ;
  _nEvt = 0 ;
  
}



void OverlayEvents::processRunHeader( LCRunHeader* run) { 
  
  _lcWriter->writeRunHeader( run );
  
  _nRun++ ;
} 


void OverlayEvents::modifyEvent( LCEvent * evt ) {
  
  _activeRunNumber = evt->getRunNumber();
  const vector<string>* overlayEventColNames = evt->getCollectionNames();
  unsigned int nOverlayCollection = overlayEventColNames->size();
  unsigned int nMergedCollection = _mergedCollectionNames.size();  

  for(unsigned int i=0 ; i<nOverlayCollection ; i++){

    std::map< std::string , std::string > CollectionMap;    	    
    for(unsigned int j=0 ; j<nMergedCollection ; j++){

      if( (*overlayEventColNames)[i] == _mergedCollectionNames[j]){

	CollectionMap[(*overlayEventColNames)[i]] = (*overlayEventColNames)[i];
      }
      Merger::merge(evt, outEvt, &CollectionMap);
    }
    CollectionMap.clear();
  }
  
#ifdef MARLIN_USE_AIDA
#endif
  _nEvt ++ ;
}



void OverlayEvents::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void OverlayEvents::end(){

  // write LCIO file
  try{
    _lcWriter->writeEvent( outEvt );
  }
  catch(IOException& e){
    std::cerr << e.what() << std::endl;
  }
  _lcWriter->close();
  
}
