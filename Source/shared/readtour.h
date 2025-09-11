#ifndef READTOUR_H_DEFINED
#define READTOUR_H_DEFINED
#include "dmalloc.h"
#include "options_common.h"
#include "shared_structures.h"

// clang-format off
#define HERMVAL(p0,p1,m0,m1)   ((2.0*t3-3.0*t2+1.0)*(p0) +      (t3-2.0*t2+t)*(m0) +        (t3-t2)*(m1) + (-2.0*t3+3.0*t2)*(p1))
#define HERMDERIV(p0,p1,m0,m1)      ((6.0*t2-6.0*t)*(p0) + (3.0*t2-4.0*t+1.0)*(m0) + (3.0*t2-2.0*t)*(m1) +  (-6.0*t2+6.0*t)*(p1))
// clang-format on

/**
 * @brief Initialize a tour.
 *
 * @param[in] tourcoll The tour collection to base this tour on.
 * @param[out] touri A pointer to the tour to initialize.
 */
EXTERNCPP void InitTour(tour_collection *tourcoll, tourdata *touri);
/**
 * @brief Free a tour initialized with InitTour.
 *
 * @param touri A pointer to the tour to free
 */
EXTERNCPP void FreeTour(tourdata *touri);

EXTERNCPP void ReallocTourMemory(tour_collection *tourcoll);
EXTERNCPP void FreeTours(tour_collection *tourcoll);


EXTERNCPP keyframe *CopyFrame(const keyframe *framei);

/**
 * @brief Given a tour and a time, return the keyframe found at this time or
 * just before this time.
 *
 * @param[in] touri The tour in which to find the keyframe
 * @param[in] time The time to search for
 * @return The found keyframe
 */
EXTERNCPP keyframe *GetKeyFrame(const tourdata *touri, float time);
EXTERNCPP void GetKeyXYZ(float t, keyframe *this_key, float *xyz);
/**
 * @brief Free all of the frames in a tour.
 *
 * @param[inout] thistour The tour in which to free all frames
 */
EXTERNCPP void DeleteTourFrames(tourdata *thistour);
EXTERNCPP void SetTourXYZView(float t, tourdata *touri);
EXTERNCPP void GetKeyView(float t, keyframe *this_key, float *view);
EXTERNCPP void GetTourView(float t, tourdata *this_tour, float *view);
#endif
