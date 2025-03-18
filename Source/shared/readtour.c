#include "options.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmalloc.h"
#include "scontour2d.h"
#include "datadefs.h"
#include "histogram.h"
#include "isobox.h"
#include "smokeviewdefs.h"
#include "string_util.h"
#include "structures.h"

#include "readobject.h"
#include "readtour.h"

#include "shared_structures.h"

/* ------------------ ReallocTourMemory ------------------------ */

void ReallocTourMemory(tour_collection *tourcoll) {
  tourdata *touri;

  if(tourcoll->tour_ntimes > 0) {
    for(int i = 0; i < tourcoll->ntourinfo; i++) {
      touri = tourcoll->tourinfo + i;
      FREEMEMORY(touri->path_times);
      NewMemory((void **)&touri->path_times,
                tourcoll->tour_ntimes * sizeof(float));
      touri->ntimes = tourcoll->tour_ntimes;
    }
    FREEMEMORY(tourcoll->tour_t);
    FREEMEMORY(tourcoll->tour_t2);
    FREEMEMORY(tourcoll->tour_dist);
    FREEMEMORY(tourcoll->tour_dist2);
    FREEMEMORY(tourcoll->tour_dist3);
    NewMemory((void **)&tourcoll->tour_t,
              tourcoll->tour_ntimes * sizeof(float));
    NewMemory((void **)&tourcoll->tour_t2,
              tourcoll->tour_ntimes * sizeof(float));
    NewMemory((void **)&tourcoll->tour_dist,
              tourcoll->tour_ntimes * sizeof(float));
    NewMemory((void **)&tourcoll->tour_dist2,
              tourcoll->tour_ntimes * sizeof(float));
    NewMemory((void **)&tourcoll->tour_dist3,
              (tourcoll->tour_ntimes + 10) * sizeof(float));
  }
}

/* ------------------ FreeTour ------------------------ */

void FreeTour(tourdata *touri) {
  int i;
  keyframe *framei;

  for(i = 0; i < touri->nkeyframes; i++) {
    framei = touri->keyframe_list[i];
    FREEMEMORY(framei);
  }
  FREEMEMORY(touri->keyframe_list);
  FREEMEMORY(touri->keyframe_times);
  FREEMEMORY(touri->timeslist);
  FREEMEMORY(touri->path_times);
}

/* ------------------ FreeTours ------------------------ */

void FreeTours(tour_collection *tourcoll) {
  int i;

  if(tourcoll->ntourinfo > 0) {
    for(i = 0; i < tourcoll->ntourinfo; i++) {
      tourdata *touri;

      touri = tourcoll->tourinfo + i;
      FreeTour(touri);
    }
    FREEMEMORY(tourcoll->tourinfo);
  }
  tourcoll->ntourinfo = 0;
}

/* ------------------ InitTour ------------------------ */

void InitTour(tour_collection *tourcoll, tourdata *touri) {
  touri->glui_avatar_index = 0;
  touri->display2 = 0;
  touri->display = 0;
  touri->periodic = 0;
  touri->first_frame.prev = NULL;
  touri->first_frame.next = &touri->last_frame;
  touri->first_frame.time = -1000000000.0;
  touri->first_frame.set_tour_time = 0;

  touri->last_frame.prev = &touri->first_frame;
  touri->last_frame.next = NULL;
  touri->last_frame.time = 1000000000.0;
  touri->last_frame.set_tour_time = 0;

  touri->nkeyframes = 0;
  touri->ntimes = tourcoll->tour_ntimes;
  touri->keyframe_times = NULL;
  touri->keyframe_list = NULL;
  touri->timeslist = NULL;
  touri->path_times = NULL;
  touri->global_dist = 0.0;
  touri->startup = 0;
  touri->isDefault = 0;
}

/* ------------------ HermiteView ------------------------ */

void HermiteView(float t, keyframe *kf1, keyframe *kf2, float *view) {
  float *p0, *p1, *m0, *m1;
  float t3, t2;

  t2 = t * t;
  t3 = t2 * t;
  p0 = kf1->view_smv;
  p1 = kf2->view_smv;
  m0 = kf1->view_tangent_right;
  m1 = kf2->view_tangent_left;
  view[0] = HERMVAL(p0[0], p1[0], m0[0], m1[0]);
  view[1] = HERMVAL(p0[1], p1[1], m0[1], m1[1]);
  view[2] = HERMVAL(p0[2], p1[2], m0[2], m1[2]);
}

/* ------------------ GetKeyFrame ------------------------ */

keyframe *GetKeyFrame(const tourdata *touri, float time) {
  keyframe *first_key, *last_key, *this_key;

  first_key = touri->first_frame.next;
  if(time < first_key->time) return first_key;

  last_key = touri->last_frame.prev;
  if(time >= last_key->time) return last_key->prev;

  for(this_key = first_key; this_key != last_key; this_key = this_key->next) {
    keyframe *next_key;

    next_key = this_key->next;
    if(this_key->time <= time && time <= next_key->time) return this_key;
  }
  return last_key->prev;
}

/* ------------------ GetKeyView ------------------------ */

void GetKeyView(float t, keyframe *this_key, float *view) {
  keyframe *next_key;
  float dt, t_scaled;

  t_scaled = 0.0;
  next_key = this_key->next;
  dt = next_key->time - this_key->time;
  if(dt > 0.0) t_scaled = CLAMP((t - this_key->time) / dt, 0.0, 1.0);
  HermiteView(t_scaled, this_key, next_key, view);
}
/* ------------------ GetTourView ------------------------ */

void GetTourView(float t, tourdata *this_tour, float *view) {
  keyframe *this_key;

  this_key = GetKeyFrame(this_tour, t);
  GetKeyView(t, this_key, view);
}

/* ------------------ GetTourVal ------------------------ */

void GetTourXYZView(float time, float *times, float *vals, int n, float *val3) {
  int left;
  float *v1, *v2, factor;

  if(time <= times[0]) {
    memcpy(val3, vals, 3 * sizeof(float));
    return;
  }
  if(time >= times[n - 1]) {
    memcpy(val3, vals + 3 * (n - 1), 3 * sizeof(float));
    return;
  }

  left = GetInterval(time, times, n);
  if(left == n - 1) left = n - 2;
  v1 = vals + 3 * left;
  v2 = vals + 3 * (left + 1);
  factor = (time - times[left]) / (times[left + 1] - times[left]);
  val3[0] = (1.0 - factor) * v1[0] + factor * v2[0];
  val3[1] = (1.0 - factor) * v1[1] + factor * v2[1];
  val3[2] = (1.0 - factor) * v1[2] + factor * v2[2];
}

/* ------------------ CopyFrame ------------------------ */

keyframe *CopyFrame(const keyframe *framei) {
  keyframe *frame;

  NewMemory((void **)&frame, sizeof(keyframe));
  memcpy(frame, framei, sizeof(keyframe));

  CheckMemory;
  return frame;
}

/* ------------------ DeleteTourFrames ------------------------ */

void DeleteTourFrames(tourdata *thistour) {
  keyframe *frame;

  for(frame = thistour->first_frame.next; frame->next != NULL;) {
    keyframe *next;

    next = frame->next;
    FREEMEMORY(frame);
    frame = next;
  }
  thistour->first_frame.next = &(thistour->last_frame);
  thistour->last_frame.prev = &(thistour->first_frame);
}
