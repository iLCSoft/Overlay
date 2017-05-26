 #ifndef OverlayTimingGeneric_h
#define OverlayTimingGeneric_h 1

#include "OverlayTiming.h"

#include "marlin/Processor.h"
#include "marlin/EventModifier.h"

#include "lcio.h"

#include <cmath>
#include <limits>

namespace EVENT{
  class SimCalorimeterHit;
  class LCRunHeader;
  class LCEvent;
  class LCCollection;
}

class OverlayTimingGeneric : public overlay::OverlayTiming {
public:
  virtual marlin::Processor* newProcessor();
  virtual std::string const& name() const;

  OverlayTimingGeneric();
  OverlayTimingGeneric( OverlayTimingGeneric const&) = delete;
  OverlayTimingGeneric& operator=(OverlayTimingGeneric const&) = delete;

  virtual void init();

protected:

  virtual void define_time_windows(const std::string &collectionName);
  std::vector<std::string> _collectionTimesVec{"BeamCalCollection", "10"};
  std::map< std::string, float > _collectionIntegrationTimes{};

};

inline marlin::Processor *OverlayTimingGeneric::newProcessor()
{
  return new OverlayTimingGeneric();
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline const std::string &OverlayTimingGeneric::name() const
{
  return marlin::Processor::name();
}

#endif
