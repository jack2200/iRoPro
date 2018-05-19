// Processing code for landmarks.

#ifndef _RAPID_PBD_LANDMARKS_H_
#define _RAPID_PBD_LANDMARKS_H_

#include "rapid_pbd_msgs/Landmark.h"

namespace msgs = rapid_pbd_msgs;
namespace rapid {
namespace pbd {
// Sets the box's orientation to the identity if its x and y dimensions are
// close. This is because the box likely represents something symmetric about
// the z axis.
void ProcessSurfaceBox(const msgs::Landmark& in, msgs::Landmark* out);
}  // namespace pbd
}  // namespace rapid

#endif  // _RAPID_PBD_LANDMARKS_H_
