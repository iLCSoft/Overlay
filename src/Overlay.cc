#define BGNAME "BG-Expectation"

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
#include "Merger.h"

#include "CLHEP/Random/RandPoisson.h"
// #include <time.h>

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
  
  int num;
  num = 1;
  registerProcessorParameter( "NumberOverlayEvents" , 
                              "Overlay each event with this number of background events. (weak)" ,
                              _numOverlay ,
                              num ) ;
 
  double exp;
  exp = 1.;
  registerProcessorParameter( BGNAME , 
                              "Overlay each event by a number of BG events according to a poisson distribution with this expectation value. (strong)" ,
                              _bgExpectation ,
                              exp ) ;

  StringVec map;
  registerProcessorParameter( "CollectionMap" , 
                              "Map containing pairs of collection to be merged (srcName, destName)"  ,
                              _colVec ,
                              map ) ;
  
}


void Overlay::init() { 
  

  if( ! LCIO_VERSION_GE( 1 , 4 ) ) {
    throw Exception("  Overlay requires LCIO v1.4 or higher \n"
 		    "  - please upgrade your LCIO version or disable the Overlay processor ! ") ;
  }

  // usually a good idea to
  printParameters() ;
  
  
  _lcReader = LCFactory::getInstance()->createLCReader() ;


  streamlog_out( DEBUG ) << " opening first file for overlay : " << _fileNames[0]  << std::endl ;

  _lcReader->open( _fileNames  ) ; 
  
  CLHEP::HepRandom::setTheSeed(time(NULL));
  
  StringVec::iterator it;
  StringVec::iterator endIt = _colVec.end();

  int oddNumOfCols = _colVec.size() & 0x1;
  if (oddNumOfCols) { 
    streamlog_out( WARNING ) << "Odd number of collection names, last collection ignored" << std::endl;
    --endIt;
  }

  for (it=_colVec.begin(); it < endIt; ++it) {
    std::string  key = (*it);
    it++;
    _colMap[key] = (*it);
  }

  _nRun = 0 ;
  _nEvt = 0 ;
  
}

void Overlay::processRunHeader( LCRunHeader* run) { 
  
  _nRun++ ;
} 

void Overlay::modifyEvent( LCEvent * evt ) {
  LCEvent* overlayEvent ;
  long num;
  
  if (parameterSet(BGNAME)) {
    num = CLHEP::RandPoisson::shoot(_bgExpectation);
  } else { 
    num = _numOverlay;
  }
  streamlog_out( DEBUG ) << "** Processing event nr " << evt->getEventNumber() << "\n   overlaying " << num << " background events." << std::endl;
  
  
  for(long i=0; i < num  ; i++ ) {
  
    overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;
  //     overlayEvent = _lcReader->readNextEvent() ;
      
    if( !overlayEvent ) {
      _lcReader->close() ; 
      _lcReader->open( _fileNames  ) ; 
      
      streamlog_out( WARNING4 ) << " ---- Overlay stream has been reset to first element ---- "
                                << std::endl ;
      
      overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;
    }
      
  //   LCTOOLS::dumpEvent( evt ); 
    if (_colMap.size() == 0) {
      Merger::merge(overlayEvent, evt);
    } else {
      Merger::merge(overlayEvent, evt, &_colMap);
    }
    
  }

  
#ifdef MARLIN_USE_AIDA
#endif
  _nEvt ++ ;
}



void Overlay::check( LCEvent * evt ) { 
  // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void Overlay::end(){ 
}

