
#ifndef MAKE_TRITONMP
#define MAKE_TRITONMP

namespace MacroPlace {
class tritonmp_external;
}

namespace ord {

class OpenRoad;

MacroPlace::tritonmp_external *
makeTritonmp();

void
initTritonmp(OpenRoad *openroad);

void
deleteTritonmp(MacroPlace::tritonmp_external *tritonmp);

}

#endif
