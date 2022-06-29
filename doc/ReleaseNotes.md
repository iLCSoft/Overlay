# v00-22-04

* 2022-06-28 Thomas Madlener ([PR#25](https://github.com/iLCSoft/Overlay/pull/25))
  - Make doxygen cmake config work with newer versions of cmake (>= 3.17)

# v00-22-03

* 2021-08-27 Andre Sailer ([PR#24](https://github.com/iLCSoft/Overlay/pull/24))
  - CI: build against LCG_99pyhon2 gcc8 and LCG_100 gcc10, clang11

* 2021-08-02 Thomas Madlener ([PR#23](https://github.com/iLCSoft/Overlay/pull/23))
  - Add coverity scan workflow that runs daily

# v00-22-02

* 2021-03-01 tmadlener ([PR#22](https://github.com/iLCSoft/Overlay/pull/22))
  - Migrate CI to github actions

* 2021-03-01 hegarcia ([PR#21](https://github.com/iLCSoft/Overlay/pull/21))
  - fix for merging SimCalorimeterHits in Merger.cc
        - Adding length and position to the MC Contributions for use in the SDHCALDigi.

# v00-22-01

* 2020-07-09 Placido Fernandez Declara ([PR#20](https://github.com/iLCSoft/overlay/pull/20))
  - OverlayTiming/OverlayTimingGeneric: Fixed accessing empty file name for background file

# v00-22

* 2019-10-07 Remi Ete ([PR#19](https://github.com/iLCSoft/Overlay/pull/19))
  - Fixed streamlog usage using pre-processor macro from streamlog
  - Fixed marlin includes inside `MARLIN_USE_AIDA` condition

* 2019-08-13 Frank Gaede ([PR#18](https://github.com/iLCSoft/Overlay/pull/18))
  - make compatible w/ c++17
      - replace std::random_shuffle w/ std::shuffle in OverlayTiming
      - use std::mt19937 rather than CLHEP::RandFlat

* 2019-05-27 Jenny List ([PR#17](https://github.com/iLCSoft/Overlay/pull/17))
  - modified `Merger::merge(LCCollection* , LCCollection* )` to be able to merge any type of collection. 
  - by default a collection is treated as tracker hits previously, namely a simple copy.

# v00-21

* 2018-01-31 Ete Remi ([PR#14](https://github.com/iLCSoft/Overlay/pull/14))
  - Overlay processor
     - Added number of overlaid events in event parameters
     - Added total number of overlaid events in event parameters
  - Fixed compiler warnings

* 2018-02-12 Ete Remi ([PR#15](https://github.com/iLCSoft/Overlay/pull/15))
  - Complete re-write of Overlay processor
     - Removed processor parameters (RunOverlay and NSkipEventsRandom)
     - Overlay of multiple files work as for a single file
     - If no collection specified in config, overlay all present collections 
     - Write in the event parameters: 
        - Overlaid run and event numbers
        - Number of overlaid events per processor and from all processors
     - Code documented and c++11 styled
     - Methods and class members cleaned-up

* 2018-03-21 Ete Remi ([PR#16](https://github.com/iLCSoft/Overlay/pull/16))
  - Overlay processor: 
     - Added new processor parameter: list of collections to avoid to overlay

# v00-20

* 2017-11-09 Ete Remi ([PR#11](https://github.com/iLCSoft/Overlay/pull/11))
  - Overlay processor : 
    - changed default collection names for processor parameter CollectionMap
      - default names are MCParticle MCParticle
    - changed condition for overlaying all collections
      - if the parameter CollectionMap is not set then process all collections

* 2017-10-20 Andre Sailer ([PR#9](https://github.com/iLCSoft/Overlay/pull/9))
  - OverlayTiming[Generic]: exhaust all events in a file instead of opening a file for each event. Should be reproducible for when not skipping events.
  - OverlayTiming[Generic]: add parameters that allow setting the initial state also after skipping events.

* 2017-11-03 Emilia Leogrande ([PR#10](https://github.com/iLCSoft/Overlay/pull/10))
  - OverlayTiming[Generic]: optionally prevent re-use of background files
     * keep track of used files and only use files that haven't been used before
     * add parameter AllowReusingBackgroundFiles (default true for backward compatibility)

* 2017-11-10 Ete Remi ([PR#12](https://github.com/iLCSoft/Overlay/pull/12))
  - Overlay processor : Open LCIO files not in init() but in modifyEvent() only if isFirstEvent() is true
    - Allows to do not open a lcio file if the processor condition at runtime is false
  - Overlay processor : Missing delete call for lcReader at end of processing (memory leak)

# v00-19

* 2017-07-07 Andre Sailer ([PR#7](https://github.com/iLCSoft/Overlay/pull/7))
  - OverlayTiming[Generic]: clean the LCReader instance at the end

* 2017-07-24 Andre Sailer ([PR#8](https://github.com/iLCSoft/Overlay/pull/8))
  - OverlayTiming[Generic]: get random seed from eventSeeder

# v00-18

* 2017-05-29 Andre Sailer ([PR#6](https://github.com/iLCSoft/Overlay/pull/6))
  - New processor OverlayTimingGeneric: Same as OverlayTiming, but easier configuration of the collections to be merged. No more hard coding of parameters for each parameter name. 
  Shares most of the code and parameters with OverlayTiming
  To configure collections and integration times:
    ```
     <parameter name="Collection_IntegrationTimes" type="StringVec" >
        VertexBarrelCollection        10
        VertexEndcapCollection        10
  
        InnerTrackerBarrelCollection  10
        InnerTrackerEndcapCollection  10
  
        OuterTrackerBarrelCollection  10
        OuterTrackerEndcapCollection  10
  
        ECalBarrelCollection          10
        ECalEndcapCollection          10
        ECalPlugCollection            10
  
        HCalBarrelCollection          10
        HCalEndcapCollection          10
        HCalRingCollection            10
  
        YokeBarrelCollection          10
        YokeEndcapCollection          10
  
        LumiCalCollection             10
        BeamCalCollection             10
      </parameter>
    ```
  - Fix bug in OverlayTiming::cellID2long which returned the same `long long` for different cellID pairs

# v00-17

# v00-16

 Y.Voutsinas
   - add parameter 'KeepPairsMCPinfo' tp OverlayBX 
     ( already partly implemented in v00-15 )
 

# v00-15


F. Gaede
   - protect against calling LCReader::skipNEvents() with 0 ( now also fixed in LCIO trunk )

R. Simoniello
   - new collections added for CLIC

A. Sailer
   - Remove Ansi and other definitions, allow compiling with different standard


# v00-14


   - An option to remove VTXPixelHits Collection is added. (A.Miyamoto)


# v00-13

   - changes in Overlay processor:

    - updated to use MCParticle::setOverlay() 
       
    - added method Overlay::readNextEvent() to properly
         read the next event - either with direct access (only one
         input file given ) or with skipNEvents
       
     - fixed logic to ensure same event gets same overlay
         events - regardless of SkipNEvents parameter

   - changes to OverlayBX processor:

    - switched to use  Global::EVENTSEEDER mechanism for
         reproducible random numbers 

   - introduced namespace overlay in all classes

   - removed using namespace from header files

   - added propset Id


# v00-12

   - changes to OverlayBX processor:
    
    -  fixed seg fault (removed unneeded code in init_geometry()
       and _vxdLadders )
    - added namespace (name clash w/ helper classes VXDLayer in MarlinReco)
   -  fixed extraction of layer from cellID
        using ILDCellID0::encoder_string
   
   - changes to Overlay processor:

    - switched to use the  Global::EVENTSEEDER mechanism for
      reproducible random numbers 

    - changed overlayEvent from static to member variable


# v00-11-02

   - bug fixed in Merger.cc - undo the unnecessary comment out



# v00-11-01


   - bug fixes:

    - OverlayTiming.cc: fixed correct particle time for background MCParticles based on their BX (M. Killenberg)

    - FPCCDOverlayBX.cc: fixed gcc 4.5 issue (D. Kamai)


# v00-11

   - New processor (OverlayEvents) to merge a number of events in a LCIO file into one event.

   - First version of background overlay processor for FPCCD (FPCCDOverlayBX)

   - Added dependency to Marlinutil (needed by FPCCOverlay ) 



# v00-10-00

   - Several code improvements and corrections to OverlayTiming processor (J. Marshall, A. Sailer)

     - Delete Calorimeter hits that are not added to the collection

     - Timing window is extended so that it runs from -0.25ns to numeric_limits<float>::max() ns if an unrecognized collection name is specified.

     - Time of flight function is now inline, its use of pow() has been removed and any unnecessary calls to this function have also gone.

      - Made MCParticle collection name configurable

      - Setting "lcioDetailedShowerMode false" is now default (see http://indico.cern.ch/conferenceDisplay.py?confId=107327),
            so calls to "CalorimeterHit->getPDGCont(j)" do not make sense. Remove these calls to avoid logic based on uninitialized variables.
        
      - Clear DestMap at the end of ModifyEvent, do not only run this in Check() as this can be suppressed.

      - Use one destMap per collection to avoid adding calorimeterHits from one subDetector to a different.

      - Copy MCParticles for physics event into a new collection.

      - Tidied-up code with an emphasis on removing risk posed by uninitialized variables e.g.
            create stack pointers within for loops, rather than re-using single pointer declared outside loop, etc.
            Removed unused member variables, tidied header file includes, improved const-correctness and removed tab characters.
            Use of parentheses to clarify logic in if statements



# v00-09-01

   - Fixed address memory management issues associated with removing elements from an LCIO collection. When an element is removed, it is the user's
     responsibility to take care of the memory management, e.g. by deleting the object or by adding it to another collection in the LCEvent.
      Cosmetic change: added a typedef for "std::map<long long, SimCalorimeterHit*>" to improve readability. (J. Marshall)

   - Make OverlayTiming runable without specifying files. Do not run overlay loop if nothing is supposed to be overlaid. (A. Sailer)

    - Modified file handling, minor improvements (P. Shade)

# v00-09

   - minor modifications
      - timing window start time set to small negative value
      - simplified cmake files



# v00-08

   - added new processor OverlayTimimg (P.Schade)
     for CLIC related bg studies



# v00-07-04

   - same as v00-07-03 with fix for build on MacOS (streamlog)

# v00-07-03

   - minor fixes and improvements 

     - CMakeLists.txt: 
       - BUILD_32BIT_COMPATIBLE set per default to OFF
       - added exclude pattern for .svn dirs
        - added ctest

     - fixed doxygen documentation


# v00-07-01

   - bug fix: incorrect library version numbers

# v00-07

   - added overlay of many bunch crossings for TPC hits, 
     properly shifted in z according to time structure.
   - introduced random rotation of TPC hits in azimuthal angle    
   - added LCIO::THBIT_MOMENTUM for newly created TPC collections
   - MCParticle pointer set to 0 for non-VXD SimTrackerhits
   - bug fix: nVXDBX for nVXDHits


# v00-06-01

   - bug fix for overlaying exactly 1 BX for the VXD 



# v00-06

   - experimental version of overlaying background for many bunch crossings

# v00-04

   - use optional direct access in LCIO - requires v01-11 or higher

# v00-03

   - made gcc 4.3 compliant
   - made cmake 2.6 compliant
   - added 32 bit compatibility build option


# v00-02

   - added JoinEvents processor      
         (requires LCIO v01-08-05 or higher )

       recommended Marlin v00-09-10, LCIO v01-09

# v00-01   initial version
