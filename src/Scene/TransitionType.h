#ifndef TRANSITION_TYPE_H
#define TRANSITION_TYPE_H


#ifdef None
#undef None
#endif
#ifdef Status
#undef Status
#endif

enum class TransitionType { None, FadeBlack, CameraJump };

#endif
