/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2007,2008,2009 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef __COGL_CONTEXT_H
#define __COGL_CONTEXT_H

#include "cogl-internal.h"

#if HAVE_COGL_GL
#include "cogl-context-driver-gl.h"
#endif

#if HAVE_COGL_GLES || HAVE_COGL_GLES2
#include "cogl-context-driver-gles.h"
#endif

#include "cogl-context-winsys.h"
#include "cogl-primitives.h"
#include "cogl-clip-stack.h"
#include "cogl-matrix-stack.h"
#include "cogl-pipeline-private.h"
#include "cogl-buffer-private.h"
#include "cogl-bitmask.h"
#include "cogl-atlas.h"

typedef struct
{
  GLfloat v[3];
  GLfloat t[2];
  GLubyte c[4];
} CoglTextureGLVertex;

typedef struct
{
  /* Features cache */
  CoglFeatureFlags        feature_flags;
  CoglFeatureFlagsPrivate feature_flags_private;

  CoglHandle        default_pipeline;
  CoglHandle        default_layer_0;
  CoglHandle        default_layer_n;
  CoglHandle        dummy_layer_dependant;

  /* Enable cache */
  unsigned long     enable_flags;

  gboolean          enable_backface_culling;
  CoglFrontWinding  flushed_front_winding;

  gboolean          indirect;

  /* A few handy matrix constants */
  CoglMatrix        identity_matrix;
  CoglMatrix        y_flip_matrix;

  /* Client-side matrix stack or NULL if none */
  CoglMatrixMode    flushed_matrix_mode;

  GArray           *texture_units;
  int               active_texture_unit;

  CoglPipelineFogState legacy_fog_state;

  /* Pipelines */
  CoglPipeline     *opaque_color_pipeline; /* used for set_source_color */
  CoglPipeline     *blended_color_pipeline; /* used for set_source_color */
  CoglPipeline     *texture_pipeline; /* used for set_source_texture */
  GString          *codegen_header_buffer;
  GString          *codegen_source_buffer;
  GList            *source_stack;

  int               legacy_state_set;

#ifdef HAVE_COGL_GL
  GHashTable       *arbfp_cache;
#endif

  /* Textures */
  CoglHandle        default_gl_texture_2d_tex;
  CoglHandle        default_gl_texture_rect_tex;

  /* Batching geometry... */
  /* We journal the texture rectangles we want to submit to OpenGL so
   * we have an oppertunity to optimise the final order so that we
   * can batch things together. */
  GArray           *journal;
  GArray           *logged_vertices;
  GArray           *journal_flush_attributes_array;
  size_t            journal_needed_vbo_len;
  GArray           *journal_clip_bounds;

  GArray           *polygon_vertices;

  /* Some simple caching, to minimize state changes... */
  CoglPipeline     *current_pipeline;
  unsigned long     current_pipeline_changes_since_flush;
  gboolean          current_pipeline_skip_gl_color;
  unsigned long     current_pipeline_age;

  GArray           *pipeline0_nodes;
  GArray           *pipeline1_nodes;

  /* Bitmask of attributes enabled. On GLES2 these are the vertex
     attribute numbers and on regular GL these are only used for the
     texture coordinate arrays */
  CoglBitmask       arrays_enabled;
  /* These are temporary bitmasks that are used when disabling
     texcoord arrays. They are here just to avoid allocating new ones
     each time */
  CoglBitmask       arrays_to_change;
  CoglBitmask       temp_bitmask;

  gboolean          gl_blend_enable_cache;

  gboolean              depth_test_enabled_cache;
  CoglDepthTestFunction depth_test_function_cache;
  gboolean              depth_writing_enabled_cache;
  float                 depth_range_near_cache;
  float                 depth_range_far_cache;

  gboolean              legacy_depth_test_enabled;

  float             point_size_cache;

  CoglBuffer       *current_buffer[COGL_BUFFER_BIND_TARGET_COUNT];

  /* Framebuffers */
  GSList           *framebuffer_stack;
  CoglHandle        window_buffer;
  gboolean          dirty_bound_framebuffer;
  gboolean          dirty_gl_viewport;

  /* Primitives */
  CoglPath         *current_path;
  CoglPipeline     *stencil_pipeline;

  /* Pre-generated VBOs containing indices to generate GL_TRIANGLES
     out of a vertex array of quads */
  CoglHandle        quad_buffer_indices_byte;
  unsigned int      quad_buffer_indices_len;
  CoglHandle        quad_buffer_indices;

  CoglIndices      *rectangle_byte_indices;
  CoglIndices      *rectangle_short_indices;
  int               rectangle_short_indices_len;

  gboolean          in_begin_gl_block;

  CoglPipeline     *texture_download_pipeline;

  CoglAtlas        *atlas;

  /* This debugging variable is used to pick a colour for visually
     displaying the quad batches. It needs to be global so that it can
     be reset by cogl_clear. It needs to be reset to increase the
     chances of getting the same colour during an animation */
  guint8            journal_rectangles_color;

  /* Cached values for GL_MAX_TEXTURE_[IMAGE_]UNITS to avoid calling
     glGetInteger too often */
  GLint             max_texture_units;
  GLint             max_texture_image_units;
  GLint             max_activateable_texture_units;

  /* Fragment processing programs */
  CoglHandle              current_program;

  CoglPipelineProgramType current_fragment_program_type;
  CoglPipelineProgramType current_vertex_program_type;
  GLuint                  current_gl_program;

  /* List of types that will be considered a subclass of CoglTexture in
     cogl_is_texture */
  GSList           *texture_types;

  /* List of types that will be considered a subclass of CoglBuffer in
     cogl_is_buffer */
  GSList           *buffer_types;

  /* Clipping */
  /* TRUE if we have a valid clipping stack flushed. In that case
     current_clip_stack will describe what the current state is. If
     this is FALSE then the current clip stack is completely unknown
     so it will need to be reflushed. In that case current_clip_stack
     doesn't need to be a valid pointer. We can't just use NULL in
     current_clip_stack to mark a dirty state because NULL is a valid
     stack (meaning no clipping) */
  gboolean          current_clip_stack_valid;
  /* The clip state that was flushed. This isn't intended to be used
     as a stack to push and pop new entries. Instead the current stack
     that the user wants is part of the framebuffer state. This is
     just used to record the flush state so we can avoid flushing the
     same state multiple times. When the clip state is flushed this
     will hold a reference */
  CoglClipStack    *current_clip_stack;
  /* Whether the stencil buffer was used as part of the current clip
     state. If TRUE then any further use of the stencil buffer (such
     as for drawing paths) would need to be merged with the existing
     stencil buffer */
  gboolean          current_clip_stack_uses_stencil;

  CoglContextDriver drv;
  CoglContextWinsys winsys;
} CoglContext;

CoglContext *
_cogl_context_get_default ();

/* Obtains the context and returns retval if NULL */
#define _COGL_GET_CONTEXT(ctxvar, retval) \
CoglContext *ctxvar = _cogl_context_get_default (); \
if (ctxvar == NULL) return retval;

#define NO_RETVAL

#endif /* __COGL_CONTEXT_H */
