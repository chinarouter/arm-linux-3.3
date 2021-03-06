#ifndef _FMJPEG_ENC_VG2_H_
#define _FMJPEG_ENC_VG2_H_

#include "video_entity.h"


#define MJE_MAX_RECORD 100
struct mje_property_record_t {
    struct video_property_t property[MJE_MAX_RECORD];
    struct video_entity_t   *entity;
    int    job_id;
};

#if 0 //allocation from videograph
#include "ms/ms_core.h"
#define driver_alloc(x) ms_small_fast_alloc(x)
#define driver_free(x) ms_small_fast_free(x)
#else
//#define driver_alloc(x) kmalloc(x,GFP_ATOMIC)
//#define driver_free(x) kfree(x)
#endif

#endif

