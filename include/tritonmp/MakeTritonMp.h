
#ifndef MAKE_TRITONMP
#define MAKE_TRITONMP

namespace MacroPlace {
class tritonmp_external;
}

namespace ord {

class OpenRoad;

MacroPlace::tritonmp_external *
makeTritonMp();

void
initTritonMp(OpenRoad *openroad);

void
deleteTritonMp(MacroPlace::tritonmp_external *tritonmp);

}

#endif
