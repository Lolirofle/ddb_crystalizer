#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <deadbeef/deadbeef.h>

enum{
    CRYSTALIZER_PARAM_INTENSITY,
    CRYSTALIZER_PARAM_COUNT
};

static DB_functions_t *deadbeef;
static DB_dsp_t plugin;

typedef struct{
    ddb_dsp_context_t ctx;
    float intensity;
    int channels;
    float *prev_samples;
} ddb_crystalizer_t;

ddb_dsp_context_t *crystalizer_open(void){
    ddb_crystalizer_t *crystalizer = malloc(sizeof(ddb_crystalizer_t));
    DDB_INIT_DSP_CONTEXT(crystalizer,ddb_crystalizer_t,&plugin);

    //Initialise.
    crystalizer->intensity = 0.1;
    crystalizer->channels = 0;
    crystalizer->prev_samples = NULL;

    return(ddb_dsp_context_t *)crystalizer;
}

void crystalizer_close(ddb_dsp_context_t *ctx){
    ddb_crystalizer_t *crystalizer = (ddb_crystalizer_t *)ctx;
    if(crystalizer->prev_samples){
        free(crystalizer->prev_samples);
        crystalizer->prev_samples = NULL;
    }
    free(crystalizer);
}

void crystalizer_reset(ddb_dsp_context_t *ctx){
    ddb_crystalizer_t *crystalizer = (ddb_crystalizer_t *)ctx;
    if(crystalizer->prev_samples){
        memset(crystalizer->prev_samples,0.0,sizeof(float)*crystalizer->channels);
    }
}

/* Based on the Audacious plugin of the same name:
 * https://github.com/audacious-media-player/audacious-plugins/blob/master/src/crystalizer/crystalizer.cc
 * Due to the algorithm in this function being almost identical to the original, their copyright notice will be presented below:
 *
 * Copyright (c) 2008 William Pitcock <nenolod@nenolod.net>
 * Copyright (c) 2010-2012 John Lindgren <john.lindgren@tds.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice is present in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
int crystalizer_process(ddb_dsp_context_t *ctx, float *samples, int nframes, int maxframes, ddb_waveformat_t *fmt, float *r){
    ddb_crystalizer_t *crystalizer = (ddb_crystalizer_t *)ctx;

    if(crystalizer->channels != fmt->channels){
        if(fmt->channels != 0 && (crystalizer->prev_samples = realloc(crystalizer->prev_samples,sizeof(float)*fmt->channels))){
            crystalizer->channels = fmt->channels;
        }else{
            crystalizer->channels = 0;
        }
    }

    for(int i=0; i<nframes; i++){
        for(int c=0; c < crystalizer->channels; c++){
            float current = *samples;
            *samples++ = current + (current - crystalizer->prev_samples[c]) * crystalizer->intensity;
            crystalizer->prev_samples[c] = current;
        }
    }
    return nframes;
}

const char *crystalizer_get_param_name(int p){
    switch(p){
    case CRYSTALIZER_PARAM_INTENSITY:
        return "Intensity";
    default:
        fprintf(stderr, "crystalizer_param_name: invalid param index (%d)\n", p);
    }
    return NULL;
}

int crystalizer_num_params(void){
    return CRYSTALIZER_PARAM_COUNT;
}

void crystalizer_set_param(ddb_dsp_context_t *ctx, int p, const char *val){
    ddb_crystalizer_t *crystalizer = (ddb_crystalizer_t *)ctx;
    switch(p){
    case CRYSTALIZER_PARAM_INTENSITY:
        crystalizer->intensity = atof(val);
        break;
    default:
        fprintf(stderr, "crystalizer_param: invalid param index (%d)\n", p);
    }
}

void crystalizer_get_param(ddb_dsp_context_t *ctx, int p, char *val, int sz){
    ddb_crystalizer_t *crystalizer = (ddb_crystalizer_t *)ctx;
    switch(p){
    case CRYSTALIZER_PARAM_INTENSITY:
        snprintf(val, sz, "%f", crystalizer->intensity);
        break;
    default:
        fprintf(stderr, "crystalizer_get_param: invalid param index (%d)\n", p);
    }
}

static const char settings_dlg[] =
    "property \"Intensity\" spinbtn[0,10,0.1] 0 0.5;\n"
;

static DB_dsp_t plugin = {
    .plugin.api_vmajor = DB_API_VERSION_MAJOR,
    .plugin.api_vminor = DB_API_VERSION_MINOR,
    .open = crystalizer_open,
    .close = crystalizer_close,
    .process = crystalizer_process,
    .plugin.version_major = 0,
    .plugin.version_minor = 1,
    .plugin.type = DB_PLUGIN_DSP,
    .plugin.id = "crystalizer",
    .plugin.name = "Crystalizer",
    .plugin.descr = "Crystalizer DSP Plugin",
    .plugin.copyright = "copyright message - author(s), license, etc",
    .plugin.website = "http://example.org",
    .num_params = crystalizer_num_params,
    .get_param_name = crystalizer_get_param_name,
    .set_param = crystalizer_set_param,
    .get_param = crystalizer_get_param,
    .reset = crystalizer_reset,
    .configdialog = settings_dlg,
};

DB_plugin_t * ddb_crystalizer_load(DB_functions_t *ddb){
    deadbeef = ddb;
    return &plugin.plugin;
}
