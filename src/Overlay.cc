#define BGNAME "expBG"

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
  num = 0;
  registerProcessorParameter( "NumberOverlayEvents" , 
                              "Overlay each event with this number of background events. (default 0)" ,
                              _numOverlay ,
                              num ) ;
  
  bool runOverlay;
  runOverlay = false;
  registerProcessorParameter( "runOverlay" , 
                              "Overlay each event with the content of one run." ,
                              _runOverlay ,
                              runOverlay ) ;
 
 
  double exp;
  exp = 1.;
  registerProcessorParameter( BGNAME , 
                              "Add additional background events according to a poisson distribution with this expectation value. (non, if parameter not set)" ,
                              _expBG ,
                              exp ) ;

  StringVec map;
  map.push_back( "mcParticles mcParticlesBG" );
//   map.push_back( "" );
  registerProcessorParameter( "CollectionMap" , 
                              "Pairs of collection to be merged"  ,
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
  
  // opening background input
  _lcReader = LCFactory::getInstance()->createLCReader() ;
  streamlog_out( DEBUG ) << " opening first file for overlay : " << _fileNames[0]  << std::endl ;
  _lcReader->open( _fileNames  ) ; 


  // initalisation of random number generator
  CLHEP::HepRandom::setTheSeed(time(NULL));


  // preparing colleciton map for merge
  StringVec::iterator it;
  StringVec::iterator endIt = _colVec.end();
  
  int oddNumOfCols = _colVec.size() & 0x1;
  if (oddNumOfCols) { 
    streamlog_out( WARNING ) << "Odd number of collection names, last collection ignored." << std::endl;
    --endIt;
  }
  
  for (it=_colVec.begin(); it < endIt; ++it) {  // treating pairs of collection names
    std::string  key = (*it);  //src
    ++it;
    _colMap[key] = (*it);// src -> dest
  }


  // handle runOverlay parameter
  if (_runOverlay) {
    _numOverlay = 1;
    if (parameterSet(BGNAME) || parameterSet("NumberOverlayEvents")) {
      streamlog_out( WARNING4 ) << "Using runOverlay, NumberOverlayEvents and expBG get ignored." << std::endl;
    }
  }

  
  
  _nRun = 0 ;
  _nEvt = 0 ;
  
}



void Overlay::processRunHeader( LCRunHeader* run) { 
  
  _nRun++ ;
} 




void Overlay::modifyEvent( LCEvent * evt ) {
  static LCEvent* overlayEvent ;
  long num = _numOverlay;
  
  // initialise static event storage overlay Event
  if( isFirstEvent() ) {
    overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;
    _activeRunNumber = overlayEvent->getRunNumber();
  } 
  

  if (parameterSet(BGNAME) && !_runOverlay) {
    num += CLHEP::RandPoisson::shoot(_expBG);
  }
  
  streamlog_out( DEBUG ) << "** Processing event nr " << evt->getEventNumber() << "\n   overlaying " << num << " background events." << std::endl;
  
  
  // core - add correct number of bg events to EVT
  // if runs are added, the loop counter i will be 
  // reset to zero in every pass
  for(long i=0; i < num  ; i++ ) {

    // merge event from storage with EVT
    streamlog_out( DEBUG ) << "loop: " << i << std::endl;
    if (_colMap.size() == 0) {
      Merger::merge(overlayEvent, evt);
    } else {
      Merger::merge(overlayEvent, evt, &_colMap);
    }

    // load new event into storage, restart input "stream" if necessary
    overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;  
    if( !overlayEvent ) {
      _lcReader->close() ; 
      _lcReader->open( _fileNames  ) ; 

      streamlog_out( WARNING4 ) << "Overlay stream has been reset to first element."
          << std::endl ;

      overlayEvent = _lcReader->readNextEvent( LCIO::UPDATE ) ;
      _activeRunNumber = overlayEvent->getRunNumber();
      i = num;
    }

    // if processing runs: 
    // check exit criteria and reset loop counter if necessary
    if (_runOverlay) {
      i=0;
      int runNumber = overlayEvent->getRunNumber();
      if (runNumber != _activeRunNumber) {
        _activeRunNumber = runNumber;
        i=num;
      }
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

