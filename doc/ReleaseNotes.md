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
