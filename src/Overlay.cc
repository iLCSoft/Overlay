#include "Overlay.h"
#include <iostream>

#ifdef MARLIN_USE_AIDA
#include <marlin/AIDAProcessor.h>
#include <AIDA/IHistogramFactory.h>
#include <AIDA/ICloud1D.h>
//#include <AIDA/IHistogram1D.h>
#endif

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>
#include <EVENT/MCParticle.h>
#include "IO/LCReader.h"
#include "UTIL/LCTOOLS.h"


using namespace lcio ;
using namespace marlin ;


Overlay aOverlay ;


Overlay::Overlay() : Processor("Overlay") {
  
  // modify processor description
  _description = "Opens a second (chain of) lcio file(s) and overlays events..." ;
  
  
  // register steering parameters: name, description, class-variable, default value
  
  StringVec files ;
  files.push_back( "overlay.slcio" )  ;

  registerProcessorParameter( "InputFileNames" , 
			      "Name of the lcio input file(s)"  ,
			      _fileNames ,
 			      files ) ;

}


void Overlay::init() { 
  

  if( ! LCIO_VERSION_GE( 1 , 4 ) ) {
    throw Exception("  Overlay requires LCIO v1.4 or higher \n"
 		    "  - please upgrade your LCIO version or disable the Overlay processor ! ") ;
  }

  // usually a good idea to
  printParameters() ;
  
  
  _lcReader = LCFactory::getInstance()->createLCReader() ;
  _lcReader->open( _fileNames  ) ; 
  

  _nRun = 0 ;
  _nEvt = 0 ;
  
}

void Overlay::processRunHeader( LCRunHeader* run) { 
  
  _nRun++ ;
} 

void Overlay::processEvent( LCEvent * evt ) { 
  
  static bool firstEvent = true ;
  static bool lastEvent = false ;
  
  // this gets called for every event 
  // usually the working horse ...
  
  LCEvent* overlayEvent ;

  if( ! lastEvent ) {
  
    overlayEvent = _lcReader->readNextEvent() ;
    
    if( !overlayEvent ) {
      lastEvent = true ;
      std::cout << "Warning: last event read from overlay stream at event " 
		<< evt->getEventNumber() 
		<< " in run " << evt->getRunNumber() 
		<< std::endl ;
    } else {


      // do the overlay here .....


    }
  }

#ifdef MARLIN_USE_AIDA
#endif
  
  firstEvent = false ;
  _nEvt ++ ;
}



void Overlay::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void Overlay::end(){ 
}

