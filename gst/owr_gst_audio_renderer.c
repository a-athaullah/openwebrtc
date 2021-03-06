/*
 * Copyright (c) 2015, Igalia S.L
 *     Author: Philippe Normand <philn@igalia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

/*/
\*\ OwrGstAudioRenderer
/*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "owr_gst_audio_renderer.h"

#include "owr_media_renderer_private.h"
#include "owr_private.h"

#define DEFAULT_AUDIO_SINK "pulsesink"

#define OWR_GST_AUDIO_RENDERER_GET_PRIVATE(obj)    (G_TYPE_INSTANCE_GET_PRIVATE((obj), OWR_GST_TYPE_AUDIO_RENDERER, OwrGstAudioRendererPrivate))

G_DEFINE_TYPE(OwrGstAudioRenderer, owr_gst_audio_renderer, OWR_TYPE_AUDIO_RENDERER)

enum {
    PROP_0,
    PROP_SINK,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void owr_gst_audio_renderer_set_property(GObject *object, guint property_id,
    const GValue *value, GParamSpec *pspec);
static void owr_gst_audio_renderer_get_property(GObject *object, guint property_id,
    GValue *value, GParamSpec *pspec);

static GstElement *owr_gst_audio_renderer_get_sink(OwrMediaRenderer *renderer);

struct _OwrGstAudioRendererPrivate {
    GstElement *sink;
};

static void owr_gst_audio_renderer_dispose(GObject *object)
{
    OwrGstAudioRenderer *renderer = OWR_GST_AUDIO_RENDERER(object);
    OwrGstAudioRendererPrivate *priv = renderer->priv;

    if (priv->sink) {
        gst_object_unref(priv->sink);
        priv->sink = NULL;
    }

    G_OBJECT_CLASS(owr_gst_audio_renderer_parent_class)->dispose(object);
}

static void owr_gst_audio_renderer_class_init(OwrGstAudioRendererClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    OwrMediaRendererClass *media_renderer_class = OWR_MEDIA_RENDERER_CLASS(klass);

    g_type_class_add_private(klass, sizeof(OwrGstAudioRendererPrivate));

    obj_properties[PROP_SINK] = g_param_spec_object("sink", "sink",
        "Audio sink to use for rendering (default: pulsesink)", G_TYPE_OBJECT,
        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    gobject_class->set_property = owr_gst_audio_renderer_set_property;
    gobject_class->get_property = owr_gst_audio_renderer_get_property;

    gobject_class->dispose = owr_gst_audio_renderer_dispose;

    media_renderer_class->get_sink = (void *(*)(OwrMediaRenderer *))owr_gst_audio_renderer_get_sink;

    g_object_class_install_properties(gobject_class, N_PROPERTIES, obj_properties);
}

static void owr_gst_audio_renderer_init(OwrGstAudioRenderer *renderer)
{
    OwrGstAudioRendererPrivate *priv;
    renderer->priv = priv = OWR_GST_AUDIO_RENDERER_GET_PRIVATE(renderer);

    priv->sink = NULL;
}

static void owr_gst_audio_renderer_set_property(GObject *object, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
    OwrGstAudioRendererPrivate *priv;
    GstElement *sink;

    g_return_if_fail(object);
    priv = OWR_GST_AUDIO_RENDERER_GET_PRIVATE(object);

    switch (property_id) {
    case PROP_SINK:
        sink = g_value_get_object(value);
        if (!GST_IS_ELEMENT(sink))
            break;
        if (priv->sink)
            gst_object_unref(priv->sink);
        priv->sink = sink;
        gst_object_ref_sink(sink);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void owr_gst_audio_renderer_get_property(GObject *object, guint property_id,
    GValue *value, GParamSpec *pspec)
{
    OwrGstAudioRendererPrivate *priv;

    g_return_if_fail(object);
    priv = OWR_GST_AUDIO_RENDERER_GET_PRIVATE(object);

    switch (property_id) {
    case PROP_SINK:
        g_value_set_object(value, priv->sink);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

/**
 * owr_gst_audio_renderer_new: (constructor)
 * @sink:
 *
 * Returns: The new #OwrGstAudioRenderer
 */
OwrGstAudioRenderer *owr_gst_audio_renderer_new(GstElement *sink)
{
    return g_object_new(OWR_GST_TYPE_AUDIO_RENDERER,
        "media-type", OWR_MEDIA_TYPE_AUDIO,
        "sink", sink,
        NULL);
}

static GstElement *owr_gst_audio_renderer_get_sink(OwrMediaRenderer *renderer)
{
    OwrGstAudioRenderer *audio_renderer;
    OwrGstAudioRendererPrivate *priv;
    GstElement *sink;

    g_assert(renderer);
    audio_renderer = OWR_GST_AUDIO_RENDERER(renderer);
    priv = audio_renderer->priv;

    sink = priv->sink ? priv->sink : gst_element_factory_make(DEFAULT_AUDIO_SINK, "audio-renderer-sink");
    return sink;
}
