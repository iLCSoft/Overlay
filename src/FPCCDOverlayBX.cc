#define BGNAME "expBG"

#include "FPCCDOverlayBX.h"
#include <iostream>

#ifdef MARLIN_USE_AIDA
#include <marlin/AIDAProcessor.h>
#include <AIDA/IHistogramFactory.h>
#include <AIDA/ICloud1D.h>
//#include <AIDA/IHistogram1D.h>
#endif

#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>
#include "IO/LCReader.h"
//#include <EVENT/SimTrackerHit.h>
#include <IMPL/SimTrackerHitImpl.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <IMPL/LCFlagImpl.h>
#include "UTIL/LCTOOLS.h"
#include "Merger.h"
#include "FPCCDData.h"
#include "FPCCDPixelHit.h"

#include <marlin/Global.h>
#include <gear/GEAR.h>
#include <gear/VXDParameters.h>
#include <gear/VXDLayerLayout.h>

#include <gearimpl/Vector3D.h>

using namespace lcio ;
using namespace marlin ;


namespace overlay {

  FPCCDOverlayBX aFPCCDOverlayBX ;


  FPCCDOverlayBX::FPCCDOverlayBX() : Processor("FPCCDOverlayBX") {

    // modify processor description
    _description = "Overlays LCGeneVTXPixelHits of background lcio files for many bunch crossings " ;


    StringVec files ;
    files.push_back( "overlay.slcio" )  ;

    registerProcessorParameter( "BackgroundFileNames" , 
				"Name of the lcio input file(s) with background - assume one file per bunch crossing."  ,
				_inputFileNames ,
				files ) ;

    registerProcessorParameter( "MaxNumberOfEventsPerFile" , 
				"Max number of events to read from one file - default: -1, i.e. one file per BX.",
				_eventsPerBX ,
				int(-1) ) ;

    registerProcessorParameter( "NumberOfBunchCrossings" ,
				"Number of  bunch crossings [s] - default 100" ,
				_numBX ,
				int(100) ) ;
    
    registerProcessorParameter( "VXDCollection" , 
				"collection of VXD SimTrackerHits" ,
				_vxdCollection,
				std::string("VXDCollection") ) ;
  
    registerProcessorParameter( "VTXPixelHits" , 
				"collection of VTX PixelHits" ,
				_vtxPixelHitsCollection,
				std::string("VTXPixelHits") ) ;
  
    StringVec exMap;
    exMap.push_back( "VTXPixelHits VTXPixelHits" );


    registerOptionalParameter( "MergeCollections" , 
			       "Pairs of collection names with one bx to be overlayed (BG name first)"  ,
			       _mergeCollections ,
			       exMap ) ;

  }



  void FPCCDOverlayBX::init() { 


    //FIXME: have new LCIO version with readRandomEvent()
    if( ! LCIO_VERSION_GE( 1 , 4 ) ) {
      throw Exception("  FPCCDOverlayBX requires LCIO v1.4 or higher \n"
		      "  - please upgrade your LCIO version or disable the FPCCDOverlayBX processor ! ") ;
    }

    // usually a good idea to
    printParameters() ;

    // create readers for background input files
    _lcReaders.resize( _inputFileNames.size() ) ;
  
    for( unsigned i=0 ; i < _inputFileNames.size() ; ++i ) {
    
      _lcReaders[i] = LCFactory::getInstance()->createLCReader() ;
    }
    //-----  preparing collection map for detectors where on BX is overlayed  ----------
    StringVec::iterator it;
    StringVec::iterator endIt = _mergeCollections.end();

    if ( _mergeCollections.size() & 0x1 ) { 
      streamlog_out( WARNING ) << "Odd number of collection names, last collection ignored." << std::endl;
      --endIt;
    }
  
    streamlog_out( MESSAGE ) << " will overlay 1 BX for following collections: " 
			     << std::endl  ;
  
    for (it=_mergeCollections.begin(); it < endIt; ++it) {  // treating pairs of collection names
    
      streamlog_out( DEBUG ) << "    " << *it << "  -> " ;
    
      std::string  key = (*it);  //src
      _colMap[key] = *(++it) ;// src -> dest
    
      streamlog_out( DEBUG ) << "    " << *it << std::endl ;
    
    }
  

    _nRun = 0 ;
    _nEvt = 0 ;
  }



  void FPCCDOverlayBX::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
  } 

  LCEvent* FPCCDOverlayBX::readNextEvent( int bxNum ){

    static int lastBXNum = -1 ; // fixme: make this a class variable....
    static int lastEvent = -1 ; 
    static int currentRdr = -1 ;


    streamlog_out( DEBUG4 ) << " >>>> readNextEvent( " <<  bxNum << ") called; "
			    << " lastBXNum  " << lastBXNum  
			    << " currentRdr  " << currentRdr  
			    << std::endl ;
  
    if( bxNum != lastBXNum ) {
    
      // open a new reader .....
    
      int nRdr = _tmpInputFileNames.size(); // not read same file twice or more.
    
      if( currentRdr != -1 ) {
      
	_lcReaders[currentRdr]->close() ; 
      
	streamlog_out( DEBUG4 ) << " >>>> closing reader " << currentRdr 
				<< " of " << nRdr  << std::endl ;
      }

      currentRdr = nRdr -1 ; // not read same file twice or more.
      _lcReaders[currentRdr]->open( _tmpInputFileNames[currentRdr]  ) ; 
      StringVec::iterator it = _tmpInputFileNames.begin() + currentRdr ;
      it = _tmpInputFileNames.erase(it); // erase the already read file name.
      lastEvent = -1 ;

      lastBXNum = bxNum ;
    }

    LCEvent* evt = _lcReaders[currentRdr]->readNextEvent( LCIO::UPDATE ) ;

    if( evt == 0 ) {
      lastBXNum = -1 ;
    }else{
      ++lastEvent ;
    }
    return evt;
  }


  // LCEvent*  FPCCDOverlayBX::readNextEvent(){
  //   int nRdr = _lcReaders.size() ;
  //   int iRdr = (int) ( CLHEP::RandFlat::shoot() * nRdr ) ; 
  //   streamlog_out( DEBUG ) << " reading next event from reader " << iRdr 
  // 			 << " of " << nRdr  << std::endl ;
  //   //FIXME: need to read random events from this file
  //   // -> makes no sense as guinea pig files are ordered !!!!
  //   LCEvent* evt = _lcReaders[iRdr]->readNextEvent( LCIO::UPDATE ) ;
  //   if( evt == 0 ) { // for now just close and reopen the file
  //     streamlog_out( MESSAGE ) << " ------ reopen reader " << iRdr 
  // 			     << " of " << nRdr  << std::endl ;
  //     _lcReaders[iRdr]->close() ; 
  //     _lcReaders[iRdr]->open( _inputFileNames[iRdr]  ) ; 
  //     evt = _lcReaders[iRdr]->readNextEvent( LCIO::UPDATE ) ;
  //   }
  //   return evt ;
  // }



  //-----------------------------------------------------------------------------------------------

  void FPCCDOverlayBX::modifyEvent( LCEvent * evt ) {

    _colMap.clear();
    _colMap.insert(  std::map<std::string, std::string>::value_type(_vtxPixelHitsCollection, _vtxPixelHitsCollection));

    if( streamlog::out.write< streamlog::DEBUG3 >() ) 
      LCTOOLS::dumpEvent( evt ) ;
  
    //-----------------------------------
    int numBX =  _numBX;

    init_geometry();

  
    //-----------------------------------

    if( _eventsPerBX >= 0 ){ 
    
      streamlog_out( DEBUG4 ) << "** Processing event nr " << evt->getEventNumber() 
			      << "\n overlaying " << numBX << " bunchcrossings with " 
			      << _eventsPerBX << " background events each." << std::endl;
    } else {
    
      _eventsPerBX =  ( 0x1 << 30 )  ;  //  essentially infinity ... 

    }
  
    //------------------------------------------------------------
    LCCollection* vxdCol = 0 ; 
    LCCollection* vxdBGCol = 0 ;
  
    try { 
    
      vxdCol = evt->getCollection( _vtxPixelHitsCollection ) ;
    
    }catch( DataNotAvailableException& e) {
      // make sure there is a VXD collection in the event
      streamlog_out( DEBUG1 ) << " created new vxd hit collection " <<  _vxdCollection 
			      << std::endl ;
      vxdCol = new LCCollectionVec( LCIO::LCGENERICOBJECT )  ;
      evt->addCollection(  vxdCol , _vtxPixelHitsCollection  ) ;
    
    }
  
    //------------------------------------------------------------
  
    _tmpInputFileNames = _inputFileNames;
  
    int nVXDHits = 0 ;
    FPCCDData theSignal(_nLayer,_maxLadder);
    FPCCDData theBkg(_nLayer,_maxLadder);
  
    int nSignalhit;
    int nBkghit;
    int nElementsSrc;
    int nElementsDest;

    nSignalhit = theSignal.unpackPixelHits( *vxdCol );
  
    LCEvent* olEvt = 0;
    for(int i = 0  ; i < numBX  ; i++ ) {
    
      // loop over events in one BX ......
      for(long j=0; j < _eventsPerBX  ; j++ ) {
      
	olEvt =  readNextEvent(i);

	if( olEvt == 0 ) 
	  break ;
      
	streamlog_out( DEBUG ) << " merge bg event for BX  " << i << " :" 
			       << olEvt->getRunNumber()  << "  - "
			       << olEvt->getEventNumber()  
			       << std::endl;
      
        try { 

	  vxdBGCol = olEvt->getCollection( _vtxPixelHitsCollection ) ;

	  nBkghit = theBkg.unpackPixelHits(*vxdBGCol);

	  theSignal.Add(theBkg);	
	
	  theBkg.clear();
	
	  nElementsSrc = vxdBGCol->getNumberOfElements();

	  for(int i=nElementsSrc-1 ; i>=0 ; i--){
	    vxdBGCol->removeElementAt(i);
	  }
	
	} catch( DataNotAvailableException& e) {}
      
      }
    }

    theBkg.clear();
  
    nElementsDest = vxdCol->getNumberOfElements();
    for(int i=nElementsDest-1 ; i>=0 ; i--){
      vxdCol->removeElementAt(i);
    }
  
    theSignal.packPixelHits(*vxdCol);
    theSignal.clear();
  
    streamlog_out( DEBUG3 ) << " total number of VXD bg hits: " << nVXDHits 
			    << std::endl ;
  
    if( streamlog::out.write< streamlog::DEBUG3 >() )
      LCTOOLS::dumpEvent( evt ) ;
  
    _nEvt ++ ;
  }


  //------------------------------------------------------------------------------------------------



  void FPCCDOverlayBX::check( LCEvent * evt ) { 

  }


  void FPCCDOverlayBX::end(){ 
     
    streamlog_out( MESSAGE ) << " overlayed pair background in VXD detector : " << std::endl ;
  
  }

  // =====================================================================
  void FPCCDOverlayBX::init_geometry() 
  { 
    // Save frequently used parameters.

    const gear::VXDParameters &gearVXD = Global::GEAR->getVXDParameters();
    const gear::VXDLayerLayout &layerVXD = gearVXD.getVXDLayerLayout();
  
    _nLayer = layerVXD.getNLayers() ;
    _geodata.resize(_nLayer);
    _maxLadder = 0;
  
    for(int ly=0;ly<_nLayer;ly++){
      _geodata[ly].nladder = layerVXD.getNLadders(ly);           // Number of ladders in this layer
      if( _maxLadder < _geodata[ly].nladder ) { _maxLadder = _geodata[ly].nladder; }
    }
  }

} // namespace
