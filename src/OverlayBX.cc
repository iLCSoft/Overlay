#define BGNAME "expBG"

#include "OverlayBX.h"
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
//#include <EVENT/SimTrackerHit.h>
#include <IMPL/SimTrackerHitImpl.h>
#include <IMPL/SimCalorimeterHitImpl.h>
#include <IMPL/LCFlagImpl.h>
#include "UTIL/LCTOOLS.h"
#include "UTIL/CellIDDecoder.h"
#include "UTIL/ILDConf.h"
#include "Merger.h"

#include <marlin/Global.h>
#include <gear/GEAR.h>
#include <gear/VXDParameters.h>
#include <gear/VXDLayerLayout.h>
#include <gear/TPCParameters.h>

#include <gearimpl/Vector3D.h>



#include "CLHEP/Random/RandFlat.h"
// #include "CLHEP/Vector/TwoVector.h"
// #include <time.h>

using namespace lcio ;
using namespace marlin ;


OverlayBX aOverlayBX ;


OverlayBX::OverlayBX() : Processor("OverlayBX") {

  // modify processor description
  _description = "Overlays background lcio files for many bunch crossings " ;


  // register steering parameters: name, description, class-variable, default value
  //   StringVec   _inputFileNames ;
  //   int         _eventsPerBX;

  //   float       _bxTime_s ;
  //   int         _maxBXsTPC ;
  //   float       _tpcVdrift_mm_s ;
  //   FloatVec    _vxdLayerReadOutTimes ;
  //   std::string _vxdCollection ;
  //   StringVec   _mergeCollections ;
  //   int         _ranSeed  ;
  //   ;
  //   std::vector< LCReader* > _lcReaders ;
  //   int _nRun ;
  //   int _nEvt ;

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

  registerProcessorParameter( "BunchCrossingTime" , 
			      "time between bunch crossings [s] - default 3.0e-7 (300 ns)" ,
			      _bxTime_s ,
			      float(3.0e-7) ) ;

  registerProcessorParameter( "TPCDriftvelocity" , 
			      "[mm/s] (float) - default 5.0e+7 (5cm/us)" ,
			      _tpcVdrift_mm_s ,
			      float(5.0e+7) ) ;

  registerProcessorParameter( "MaxBXsTPC" , 
			      "maximum of BXs to be overlayed for the TPC; -1: compute from length"
			      " and BXtime; default 10" ,
			      _maxBXsTPC ,
			      int(10) ) ;

  registerProcessorParameter( "PhiRotateTPCHits" , 
			      "if true, rotate the bg events by a random azimuthal angle - default false" ,
			      _phiRotateTPCHits ,
			      false) ;

  FloatVec vxdTimes ;
  vxdTimes.push_back( 50. ) ;
  vxdTimes.push_back( 50. ) ;
  vxdTimes.push_back( 200. ) ;
  vxdTimes.push_back( 200. ) ;
  vxdTimes.push_back( 200. ) ;
  vxdTimes.push_back( 200. ) ;
  
  registerProcessorParameter( "VXDLayerReadOutTimes" , 
			      "readout time per layer in us - default 50. 50. 200. 200. 200. 200"  ,
			      _vxdLayerReadOutTimes ,
			      vxdTimes ) ;
  
  
  StringVec tpcVec;
  tpcVec.push_back( "TPCCollection TPCCollection" );
  

  registerProcessorParameter( "TPCCollections" , 
			      "Pairs of TPC collection names to be overlayed (BG name first)"  ,
			      _tpcCollections ,
			      tpcVec ) ;
  
  registerProcessorParameter( "VXDCollection" , 
			      "collection of VXD SimTrackerHits" ,
			      _vxdCollection,
			      std::string("VXDCollection") ) ;
  
  registerProcessorParameter( "RandomSeed" , 
			      "random seed - default 42" ,
			      _ranSeed ,
			      int(42) ) ;
  
  StringVec exMap;
  exMap.push_back( "mcParticlesBG mcParticles" );


  registerOptionalParameter( "MergeCollections" , 
			     "Pairs of collection names with one bx to be overlayed (BG name first)"  ,
			     _mergeCollections ,
			     exMap ) ;

}



void OverlayBX::init() { 


  //FIXME: have new LCIO version with readRandomEvent()
  if( ! LCIO_VERSION_GE( 1 , 4 ) ) {
    throw Exception("  OverlayBX requires LCIO v1.4 or higher \n"
		    "  - please upgrade your LCIO version or disable the OverlayBX processor ! ") ;
  }

  // usually a good idea to
  printParameters() ;


  // initalisation of random number generator
  CLHEP::HepRandom::setTheSeed( _ranSeed ) ;


  // create readers for background input files
  _lcReaders.resize( _inputFileNames.size() ) ;
  
  for( unsigned i=0 ; i < _inputFileNames.size() ; ++i ) {
    
    _lcReaders[i] = LCFactory::getInstance()->createLCReader() ;
    
    //     streamlog_out( DEBUG4 ) << " opening file for overlay : " << _inputFileNames[i]  << std::endl ;
    //     _lcReaders[i]->open( _inputFileNames[i]  ) ; 
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
  
  
  // --- now preparing tpc collecitons map for merge -----------------
  endIt = _tpcCollections.end();
  
  if (_tpcCollections.size() & 0x1) { 
    streamlog_out( WARNING ) << "Odd number of TPC collection names, last collection ignored." << std::endl;
    --endIt;
  }
  
  streamlog_out( DEBUG ) << " merging following TPC collections from background to physics collections: " 
			 << std::endl  ;
  
  for (it=_tpcCollections.begin(); it < endIt; ++it) {  // treating pairs of collection names
    
    streamlog_out( DEBUG ) << "    " << *it << "  -> " ;
    
    std::string  key = (*it);  //src
    _tpcMap[key] = *(++it) ;// src -> dest
    
    streamlog_out( DEBUG ) << "    " << *it << std::endl ;
  }

  //---------------------------------------------------------------------
  
  init_geometry() ; 
  
  
  streamlog_out( MESSAGE ) << " --- pair background in VXD detector : " << std::endl ;
  
  for( unsigned i=0 ; i < _vxdLayers.size() ; ++i){
    
    streamlog_out( MESSAGE ) << " for layer " << i << " overlay " <<   _vxdLayers[i].nBX  << " BXs of pair bg " 
			     << std::endl;
  }


  // FIXME: do the TPC intialization also in the init() method ...


  _nRun = 0 ;
  _nEvt = 0 ;


  _lastBXNum = -1 ; 
  _lastEvent = -1 ; 
  _currentRdr = -1 ;
}



void OverlayBX::processRunHeader( LCRunHeader* run) { 

  _nRun++ ;
} 



// LCEvent*  OverlayBX::readNextEvent( int bxNum ){
  
//   static int _lastBXNum = -1 ; // fixme: make this a class variable....
//   static int _lastEvent = -1 ; 
//   static int _currentRdr = -1 ;
  


//   streamlog_out( DEBUG2 ) << " >>>> readNextEvent( " <<  bxNum << ") called; "
// 			  << " _lastBXNum  " << _lastBXNum  
// 			  << " _currentRdr  " << _currentRdr  
// 			  << std::endl ;
  
//   if( bxNum != _lastBXNum ) {
    
//     // open a new reader .....
    
//     int nRdr = _lcReaders.size() ;
//     int iRdr = _currentRdr + 1 ; //(int) ( CLHEP::RandFlat::shoot() * nRdr ) ; 
//     if( iRdr >= nRdr )
//       iRdr = 0 ;

//     if( _currentRdr != -1 ) {
      
//       _lcReaders[_currentRdr]->close() ; 
      
//       streamlog_out( DEBUG ) << " >>>> closing reader " << _currentRdr 
// 			     << " of " << nRdr  << std::endl ;
//     }

//     streamlog_out( DEBUG4 ) << " >>>> reading next BX  from reader " << iRdr 
// 			    << " of " << nRdr  
// 			    << " for BX : " << bxNum 
// 			    << " [read  " << _lastEvent << " evts for last BX]" 
// 			    << std::endl ;
    

//     _currentRdr = iRdr ;
//     _lcReaders[_currentRdr]->open( _inputFileNames[_currentRdr]  ) ; 
//     _lastEvent = -1 ;
    
//     _lastBXNum = bxNum ;
//   }

//   LCEvent* evt =  _lcReaders[_currentRdr]->readNextEvent( LCIO::UPDATE ) ;
    
//   if( evt == 0 ) {
//     _lastBXNum = -1 ;
//   }else{
//     ++_lastEvent ;
//   }

//   return evt ;
// }

LCEvent*  OverlayBX::readNextEvent( int bxNum ){
  


  streamlog_out( DEBUG ) << " >>>> readNextEvent( " <<  bxNum << ") called; "
			 << " _lastBXNum  " << _lastBXNum  
			 << " _currentRdr  " << _currentRdr  
			 << std::endl ;
  
  if( bxNum != _lastBXNum ) {
    
    // open a new reader .....
    
    int nRdr = _lcReaders.size() ;
    int iRdr = (int) ( CLHEP::RandFlat::shoot() * nRdr ) ; 
    
    if( _currentRdr != -1 ) {
      
      _lcReaders[_currentRdr]->close() ; 
      
      streamlog_out( DEBUG ) << " >>>> closing reader " << _currentRdr 
			     << " of " << nRdr  << std::endl ;
    }
    
    streamlog_out( DEBUG4 ) << " >>>> reading next BX  from reader " << iRdr 
 			    << " of " << nRdr  
 			    << " for BX : " << bxNum 
 			    << " [read  " << _lastEvent << " evts for last BX]" 
 			    << std::endl ;

    _currentRdr = iRdr ;

    _lcReaders[_currentRdr]->open( _inputFileNames[_currentRdr]  ) ; 

    _lastEvent = -1 ;

    _lastBXNum = bxNum ;
  }

  LCEvent* evt =  _lcReaders[_currentRdr]->readNextEvent( LCIO::UPDATE ) ;
    
  if( evt == 0 ) {
    _lastBXNum = -1 ;
  }else{
    ++_lastEvent ;
  }

  return evt ;
}


// LCEvent*  OverlayBX::readNextEvent(){
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

void OverlayBX::modifyEvent( LCEvent * evt ) {
  
  if( streamlog::out.write< streamlog::DEBUG3 >() ) 
    LCTOOLS::dumpEvent( evt ) ;
  
  //  long num = _eventsPerBX;


  //-----  get number of BX for VXD layers:
  int nVXDBX = 1 ;

  for( unsigned i=0 ; i < _vxdLayers.size() ; ++i){
    if( _vxdLayers[i].nBX > nVXDBX ) 
      nVXDBX = _vxdLayers[i].nBX ;
  }
  
  //---- get number of BXs to be overlaid in TPC 
  const gear::TPCParameters& gearTPC = Global::GEAR->getTPCParameters() ;
  double tpcHalfLength = gearTPC.getMaxDriftLength()  ;

  // drift length per bunch crossing
  double drLenBX = _bxTime_s * _tpcVdrift_mm_s ; 

  int nTPCBX = _maxBXsTPC ;

  double zShiftStart = 0. ;
  
  if( nTPCBX < 0 ){ // compute #BXs from the length of the TPC and the drift velocity
    
    nTPCBX = 2 * int( tpcHalfLength / drLenBX ) ;
    zShiftStart = - tpcHalfLength ;
    
  } else {  // use fixed number of BXs to be overlaid
    
    zShiftStart = - ( nTPCBX / 2. ) * drLenBX ;

  }

  //-----------------------------------
  

  int numBX = ( nTPCBX > nVXDBX ?  nTPCBX : nVXDBX ) ; 


  if( _eventsPerBX >= 0 ){ 
    
    streamlog_out( DEBUG1 ) << "** Processing event nr " << evt->getEventNumber() 
			    << "\n overlaying " << nVXDBX << " bunchcrossings with " 
			    << _eventsPerBX << " background events each." << std::endl;
  } else {
    
    _eventsPerBX =  ( 0x1 << 30 )  ;  //  essentially infinity ... 

    streamlog_out( DEBUG1 ) << "** Processing event nr " << evt->getEventNumber() 
			    << "\n overlaying " << numBX << " bunchcrossings from complete files ! " 
			    << "  [vxd: " << nVXDBX << " , tpc: " << nTPCBX << "] " 
			    << " (_eventsPerBX = " << _eventsPerBX << " ) "
			    << std::endl;

  }
  
  //------------------------------------------------------------
  LCCollection* vxdCol = 0 ; 
  
  try { 
    
    vxdCol = evt->getCollection( _vxdCollection ) ;
    
  } catch( DataNotAvailableException& e) {
    
    // make sure there is a VXD collection in the event
    streamlog_out( DEBUG1 ) << " created new vxd hit collection " <<  _vxdCollection 
			    << std::endl ;
    
    vxdCol = new LCCollectionVec( LCIO::SIMTRACKERHIT )  ;
    evt->addCollection(  vxdCol , _vxdCollection  ) ;
  }

  //------------------------------------------------------------

  // loop over all TPC collection names and create collection if needed...

  for(StrMap::iterator it=_tpcMap.begin() ; it != _tpcMap.end() ; ++it ) {
    const std::string& tpcName = it->second ; // map is (bgName->physicsName)
    
    LCCollection* tpcCol = 0 ; 
    
    try { 
      
      tpcCol = evt->getCollection( tpcName ) ;
      
    } catch( DataNotAvailableException& e) {
      
      // make sure there is a TPC collection in the event
      streamlog_out( DEBUG1 ) << " created new tpc hit collection " <<  tpcName 
			      << std::endl ;
      
      tpcCol = new LCCollectionVec( LCIO::SIMTRACKERHIT )  ;
      
      LCFlagImpl thFlag(0) ;
      thFlag.setBit( LCIO::THBIT_MOMENTUM ) ;
      tpcCol->setFlag( thFlag.getFlag()  ) ;

      evt->addCollection(  tpcCol , tpcName  ) ;
    }
  }
  //------------------------------------------------------------
  
  int nVXDHits = 0 ;
  int nTPCHits = 0 ;

  for(int i = 0  ; i < numBX  ; i++ ) {

    // loop over events in one BX ......
    for(long j=0; j < _eventsPerBX  ; j++ ) {

      LCEvent* olEvt  = readNextEvent(i) ;
      
      if( olEvt == 0 ) 
	break ;


      streamlog_out( DEBUG ) << " merge bg event for BX  " << i << " :" 
			     << olEvt->getRunNumber()  << "  - "
			     << olEvt->getEventNumber()  
			     << std::endl;
      
      try { 
	
	LCCollection* vxdBGCol = olEvt->getCollection( _vxdCollection ) ;
	
	if( i < nVXDBX )
	  nVXDHits += mergeVXDColsFromBX( vxdCol , vxdBGCol , i )  ;
 
      } catch( DataNotAvailableException& e) {}
      

      // overlay bg for all TPC collections
      for(StrMap::iterator it=_tpcMap.begin() ; it != _tpcMap.end() ; ++it ) {
	const std::string& tpcBGName = it->first  ;  // map is (bgName->physicsName)
	const std::string& tpcName   = it->second ; 
	
	try { 
	  
	  LCCollection* tpcCol   = evt->getCollection( tpcName ) ;
	  LCCollection* tpcBGCol = olEvt->getCollection( tpcBGName ) ;
	  
	  //overlay TPC hits shifted by nBX * drLenBX
	  if( i < nTPCBX )
	      nTPCHits += mergeTPCColsFromBX( tpcCol , tpcBGCol ,  zShiftStart + i * drLenBX   )  ;
	    //nTPCHits += mergeTPCColsFromBX( tpcCol , tpcBGCol ,  0  )  ; // no z shift for testing
	  
	} catch( DataNotAvailableException& e) {}
	
      }

      if( i==0 ) { // -----    merge hits of detectors w/ high time resolution for one  BX -------
	
	//fg: as we drop the MCParticle, we have to set the Pointers to 0 

	for(StrMap::iterator it=_colMap.begin() ; it!= _colMap.end() ; ++it ) {

	  LCCollection *srcCol = 0 ;
	  
	  try {
	    
	    srcCol = olEvt->getCollection((*it).first);
	    
	    std::string type = srcCol->getTypeName() ;
	    
	    if( type == LCIO::SIMTRACKERHIT ) {
	      int nHits = srcCol->getNumberOfElements();
	      
	      streamlog_out( DEBUG1 ) << " --- setting MCParticle to 0 for : " << nHits  
				      << " hits from collection " << (*it).first << endl;
	      
	      for (int i=0; i<nHits; i++){
		SimTrackerHitImpl* bgHit = dynamic_cast<SimTrackerHitImpl*>( srcCol->getElementAt(i) ) ;
		bgHit->setMCParticle( 0 ) ;
	      }
	    }
	    // FIXME .....	  
	    // 	  if( type == LCIO::SIMCALORIMETERHIT ) {
	    // 	    SimCalorimeterHitImpl* bgHit = dynamic_cast<SimCalorimeterHitImpl*>( srcCol->getElementAt(i) ) ;
	    // 	    //fg: this is more complicated...	    bgHit->setMCParticle( 0 ) ;
	    // 	  }
	    
	  } catch (DataNotAvailableException& e) {
	    // streamlog_out( DEBUG ) << "The source collection " << (*it).first 
	    //                        << " does not exist." << endl;
	  }

	}


	Merger::merge( olEvt, evt, &_colMap ) ;
      }
      
    }
  }
  
  streamlog_out( DEBUG3 ) << " total number of VXD bg hits: " << nVXDHits 
			    << std::endl ;
    

  if( streamlog::out.write< streamlog::DEBUG3 >() ) 
    LCTOOLS::dumpEvent( evt ) ;


  _nEvt ++ ;
}


int OverlayBX::mergeVXDColsFromBX( LCCollection* vxdCol , LCCollection* vxdBGCol , int bxNum ) {
  
  static CellIDDecoder<SimTrackerHit> idDec( ILDCellID0::encoder_string ) ;

  // the hits are simply overlaid - no shift in r-phi along the ladder 
  // is applied; this should be ok if the ladders are not read out along z
  // - in reality the innermost ladders will have faster readout than outermost ladders
  //   and thus shifting the hits might help reducing ghost tracks...
  
  const string destType = vxdCol->getTypeName();
  int nHits = 0 ;
  
  // check if collections have the same type
  if (destType != vxdBGCol->getTypeName()) {
    streamlog_out( WARNING ) << "merge not possible, collections of different type" << endl;
    return nHits ;
  }
  

  if ( destType == LCIO::SIMTRACKERHIT  )  {
    
    // running trough all the elements in the collection.
    nHits = vxdBGCol->getNumberOfElements();
    
    streamlog_out( DEBUG1 ) << " merging VXD hits from bg : " << nHits << endl;
    
    for (int i=nHits-1; i>=0; i--){ 
      // loop from back in order to remove vector elements from end ...
      
      //      SimTrackerHit* sth = dynamic_cast<SimTrackerHit*>(  vxdBGCol->getElementAt(i) ) ;
      //LCObject* bgHit = vxdBGCol->getElementAt(i) ;

      SimTrackerHitImpl* bgHit = dynamic_cast<SimTrackerHitImpl*>( vxdBGCol->getElementAt(i) ) ;

      vxdBGCol->removeElementAt(i);


      //      int layer = ( bgHit->getCellID() & 0xff )  ;
      int layer =  idDec( bgHit )[ ILDCellID0::layer ] ;


      if( bxNum < _vxdLayers[ layer-1 ].nBX  ) {
	
	// explicitly set a null pointer as MCParticle collection is not merged 
	bgHit->setMCParticle( 0 ) ;
	vxdCol->addElement( bgHit );

      } else {

	nHits -- ;
	// if hit not added we need to delete it as we removed from the collection (vector) 
	delete bgHit ;
      }
    }

  } else {
    
    streamlog_out( ERROR ) << " mergeVXDColsFromBX : wrong collection type  : " << destType  << endl;

  }

  return nHits ;
}


//------------------------------------------------------------------------------------------------


int OverlayBX::mergeTPCColsFromBX( LCCollection* tpcCol , LCCollection* tpcBGCol , float zPosShift ) {
  
  // hits are overlayed shifted in z according to the drift distance per bunch crossing  
  // ...
  
  //  const gear::TPCParameters& gearTPC = Global::GEAR->getTPCParameters() ;
  // TPC half length
  double tpcHLen = Global::GEAR->getTPCParameters().getMaxDriftLength()  ;


  const string destType = tpcCol->getTypeName();
  int nHits = 0 ;
  
  // check if collections have the same type
  if (destType != tpcBGCol->getTypeName()) {
    streamlog_out( WARNING ) << "merge not possible, collections of different type" << endl;
    return nHits ;
  }

  double phiRot = 0.0 ;

  if( _phiRotateTPCHits ){

    phiRot  =  CLHEP::RandFlat::shoot() * 2.* M_PI  ; 
  }

  if ( destType == LCIO::SIMTRACKERHIT  )  {
    
    // running trough all the elements in the collection.
    nHits = tpcBGCol->getNumberOfElements();
    
    streamlog_out( DEBUG1 ) << " merging TPC hits from bg : " << nHits << endl;
    
    for (int i=nHits-1; i>=0; i--){ 
      // loop from back in order to remove vector elements from end ...
      
      SimTrackerHitImpl* bgHit = dynamic_cast<SimTrackerHitImpl*>( tpcBGCol->getElementAt(i) ) ;

      tpcBGCol->removeElementAt(i);



      //******************************************************
      
      const double* pos =  bgHit->getPosition() ;

      gear::Vector3D posV( pos[0] , pos[1], pos[2] ) ;


      if( _phiRotateTPCHits ){

	gear::Vector3D rotV( posV.rho() , posV.phi() + phiRot , posV.z() , gear::Vector3D::cylindrical ) ; 

	posV = rotV ;

      }

      double newPos[3] ;
      newPos[0] = posV[0] ;
      newPos[1] = posV[1] ;
      newPos[2] = posV[2] ;


      if( newPos[2] >  0 ) { // positive z
	
	newPos[2] += zPosShift ;   
	
	if( newPos[2] >= 0 && newPos[2] <= tpcHLen ){
	  
	  bgHit->setPosition(  newPos ) ;
	  
	  bgHit->setMCParticle( 0 ) ;
	  
	  tpcCol->addElement( bgHit );

	} else {


	  nHits -- ;
	  // if hit not added we need to delete it as we removed from the collection (vector) 
	  delete bgHit ;
	}
 
      } else {  // negative z
	
	newPos[2] -= zPosShift ;
	
	if( newPos[2] <= 0 && newPos[2] >= -tpcHLen ){
	  
	  bgHit->setPosition(  newPos ) ;
	  
	  bgHit->setMCParticle( 0 ) ;
	  
	  tpcCol->addElement( bgHit );
	  
	} else {

	  nHits -- ;
	  // if hit not added we need to delete it as we removed from the collection (vector) 
	  delete bgHit ;
	}
      }
      //******************************************************

    }

  } else {
    
    streamlog_out( ERROR ) << " mergeTPCColsFromBX : wrong collection type  : " << destType  << endl;

  }

  return nHits ;
}


void OverlayBX::check( LCEvent * evt ) { 

  static CellIDDecoder<SimTrackerHit> idDec( ILDCellID0::encoder_string ) ;


#ifdef MARLIN_USE_AIDA
  struct H1D{
    enum { 
      hitsLayer1,
      hitsLayer2,
      hitsLayer3,
      hitsLayer4,
      hitsLayer5,
      hitsLayer6,
      size 
    }  ;
  };

  if( isFirstEvent() ) {
    
    _hist1DVec.resize( H1D::size )   ;
    
    float  hitMax =  100000. ;
    _hist1DVec[ H1D::hitsLayer1 ] = AIDAProcessor::histogramFactory(this)->createHistogram1D( "hitsLayer1", 
											      "hits Layer 1", 
											      100, 0. ,hitMax ) ; 
    _hist1DVec[ H1D::hitsLayer2 ] = AIDAProcessor::histogramFactory(this)->createHistogram1D( "hitsLayer2", 
											      "hits Layer 2", 
											      100, 0. , hitMax ) ; 
    _hist1DVec[ H1D::hitsLayer3 ] = AIDAProcessor::histogramFactory(this)->createHistogram1D( "hitsLayer3", 
											      "hits Layer 3", 
											      100, 0. , hitMax ) ; 
    _hist1DVec[ H1D::hitsLayer4 ] = AIDAProcessor::histogramFactory(this)->createHistogram1D( "hitsLayer4", 
											      "hits Layer 4", 
											      100, 0. ,hitMax ) ; 
    _hist1DVec[ H1D::hitsLayer5 ] = AIDAProcessor::histogramFactory(this)->createHistogram1D( "hitsLayer5", 
											      "hits Layer 5", 
											      100, 0. , hitMax ) ; 
    _hist1DVec[ H1D::hitsLayer6 ] = AIDAProcessor::histogramFactory(this)->createHistogram1D( "hitsLayer6", 
											      "hits Layer 6", 
											      100, 0. , hitMax ) ; 
  }
#endif



  LCCollection* vxdCol = 0 ; 

  int nhit = 0 ;
  int nHitL1 = 0 ;
  int nHitL2 = 0 ;
  int nHitL3 = 0 ;
  int nHitL4 = 0 ;
  int nHitL5 = 0 ;
  int nHitL6 = 0 ;

  
  try { 
    vxdCol = evt->getCollection( _vxdCollection ) ;

    int nH = vxdCol->getNumberOfElements() ;
    

    streamlog_out( MESSAGE4 ) <<  "  ++++ " << evt->getEventNumber() << "  " <<  nH << std::endl ; 


    for(int i=0; i<nH ; ++i){
      
      SimTrackerHit* sth = dynamic_cast<SimTrackerHit*>(  vxdCol->getElementAt(i) ) ;


      int layer =  idDec( sth )[ ILDCellID0::layer ] ;
      
      if( layer == 1 ) nHitL1++ ; 
      else if( layer == 2 ) nHitL2++ ; 
      else if( layer == 3 ) nHitL3++ ; 
      else if( layer == 4 ) nHitL4++ ; 
      else if( layer == 5 ) nHitL5++ ; 
      else if( layer == 6 ) nHitL6++ ; 
      

      MCParticle* mcp = sth->getMCParticle() ;
      if( mcp != 0  ){
	nhit++ ;
      }

    }
    if( nhit != nH ) {
      streamlog_out( ERROR ) << " found " << nhit << " MCParticles for " << nH 
			     << " SimTrackerHits "  << std::endl ; 
    } else {
      streamlog_out( DEBUG ) << " OK ! - found " << nhit << " MCParticle links for " << nH 
			     << " SimTrackerHits "  << std::endl ; 
    }
    

  } catch( DataNotAvailableException& e) {}
  
#ifdef MARLIN_USE_AIDA
  _hist1DVec[ H1D::hitsLayer1 ]->fill( nHitL1 ) ;
  _hist1DVec[ H1D::hitsLayer2 ]->fill( nHitL2 ) ;
  _hist1DVec[ H1D::hitsLayer3 ]->fill( nHitL3 ) ;
  _hist1DVec[ H1D::hitsLayer4 ]->fill( nHitL4 ) ;
  _hist1DVec[ H1D::hitsLayer5 ]->fill( nHitL5 ) ;
  _hist1DVec[ H1D::hitsLayer6 ]->fill( nHitL6 ) ;
#endif
  
}


void OverlayBX::end(){ 
  
//   // close all open input files
//   for( unsigned i=0 ; i < _lcReaders.size() ; ++i ) {
//     _lcReaders[i]->close() ;
//   }

  streamlog_out( MESSAGE ) << " overlayed pair background in VXD detector : " << std::endl ;

  for( unsigned i=0 ; i < _vxdLayers.size() ; ++i){

    streamlog_out( MESSAGE ) << " layer " << i << " overlayed " <<   _vxdLayers[i].nBX  << " BXs of pair bg " 
			     << std::endl;

    double area =  _vxdLayers[i].ladderArea * _vxdLayers[i].nLadders ;


#ifdef MARLIN_USE_AIDA

    streamlog_out( MESSAGE ) << " -> average number of hits:  " 
			     <<  _hist1DVec[ i ]->mean() 
			     << " -    hits/ mm " <<  _hist1DVec[ i ]->mean() / area  
			     << " -    occupancy (25mu) " << _hist1DVec[ i ]->mean() / area / 160. 
			     << std::endl ;

#endif

  }
  
}





void OverlayBX::init_geometry(){

  //get VXD geometry info
  const gear::VXDParameters& gearVXD = Global::GEAR->getVXDParameters() ;
  const gear::VXDLayerLayout& layerVXD = gearVXD.getVXDLayerLayout(); 
  
  if( (unsigned) layerVXD.getNLayers() !=   _vxdLayerReadOutTimes.size()  ){
    
    
    streamlog_out( ERROR  ) << " *************************************************** " << std::endl 
                            << " wrong number of readout times: " <<  _vxdLayerReadOutTimes.size() 
                            << " for " << layerVXD.getNLayers() << " VXD layers "
      //                            << "  - do nothing ! " << std::endl 
                            << " *************************************************** " << std::endl  ;

  }

  //  _vxdLadders.resize( layerVXD.getNLayers() ) ; 
  _vxdLayers.resize(  layerVXD.getNLayers() ) ; 


  
  streamlog_out( DEBUG ) << " initializing VXD ladder geometry ... for " << layerVXD.getNLayers()  << " layers " << std::endl ;

  streamlog_out( DEBUG ) << " sizeof(VXDLadder) : " <<  sizeof(VXDLadder)   << " sizeof(CLHEP::Hep2Vector) " <<  sizeof(CLHEP::Hep2Vector)  <<   std::endl ;
  

  for( int i=0 ; i <  layerVXD.getNLayers() ; i++ ) {
    
    double 	phi0 = layerVXD.getPhi0 (i) ;    
    //    double 	dist = layerVXD.getSensitiveDistance (i) ;
    
    //    double 	thick = layerVXD.getSensitiveThickness (i) ;
    double 	offs =  layerVXD.getSensitiveOffset (i) ;
    double 	width = layerVXD.getSensitiveWidth (i) ;
    
    // -----fg:  gear length is half length really !!!!!
    double 	len =   2 * layerVXD.getSensitiveLength (i) ; 
    
  
    int nLad  = layerVXD.getNLadders(i) ;


    _vxdLayers[i].nBX = (int) ( _vxdLayerReadOutTimes[i]*1.e-6 / _bxTime_s ) ;
    _vxdLayers[i].width = width ;
    _vxdLayers[i].ladderArea = width * len ;
    _vxdLayers[i].nLadders = nLad ;
    
    streamlog_out( DEBUG ) << " layer: " << i 
			   << " phi0 : " << phi0
			   << " offs : " << offs
			   << " width : " << width
			   << " nBX : " << _vxdLayers[i].nBX
			   << std::endl ;

    streamlog_out( DEBUG ) << "      .... layer " << i << " has  "  << nLad  << " ladders " << std::endl ;
 
    // _vxdLadders[i].resize( nLad ) ;

    // for( int j=0 ; j < nLad ; j++ ) {

    //   double phi = phi0 + j *  ( 2 * M_PI ) /  nLad  ; 

    //   // point in middle of sensitive ladder  (w/o offset)
    //   CLHEP::Hep2Vector pM ;
    //   pM.setPolar(  dist + thick / 2. , phi ) ;
      
    //   // direction vector along ladder in rphi (negative) 
    //   CLHEP::Hep2Vector v0 ;
    //   v0.setPolar( width /2. + offs  ,  phi +  M_PI / 2.) ;

    //   // v0.setPolar(   - (width / 2.  - offs ) ,  phi +  M_PI / 2.) ;
    //   // point p1:   'lower left corner of sensitive surface' (seen from outside)
    //   // gear::Vector3D p1 = p0 + v0 ;    
      
      
    //   CLHEP::Hep2Vector  p0 = pM + v0 ;  

    //   // v1: direction along  rphi
    //   CLHEP::Hep2Vector v1 ;
    //   v1.setPolar( width ,  phi - M_PI/2.) ;

    //   // other end of ladder
    //   CLHEP::Hep2Vector  p1 = p0 + v1 ;  


    //   VXDLadder& l = _vxdLadders.at(i).at(j) ; //_vxdLadders[i][j] ;
    //   l.phi = phi ;
    //   l.p0 = p0 ;
    //   l.p1 = p1 ;
    //   l.u  = v1.unit()  ;

    //   streamlog_out( DEBUG ) << " layer: " << i << " - ladder: " << j 
    //   			     << " phi : " << l.phi
    //   			     << " p0 : "  << l.p0 
    //   			     << " p1 : "  << l.p1 
    //   			     << " u  : "  << l.u 
    //   			     << " width: " << v1.mag() 
    //   			     << " dist: " << pM.r() 
    //   			     << std::endl ;

    // }

  }
  
  return ;
}
