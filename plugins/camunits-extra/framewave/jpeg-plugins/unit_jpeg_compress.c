#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "jpeglib.h"
#include "jerror.h"

#include <camunits/plugin.h>

#define err(args...) fprintf(stderr, args)

typedef struct {
    CamUnit parent;
    
    /*< private >*/
    CamUnitControl * quality_control;
    CamFrameBuffer * outbuf;
} CamfwJpegCompress;

typedef struct {
    CamUnitClass parent_class;
} CamfwJpegCompressClass;

CamfwJpegCompress * camfw_jpeg_compress_new (void);

static int _jpegfw_compress_8u_gray (const uint8_t * src, int width, 
        int height, int stride, uint8_t * dest, int * destsize, int quality);
static int _jpegfw_compress_8u_rgb (const uint8_t * src, int width, 
        int height, int stride, uint8_t * dest, int * destsize, int quality);
static int _jpegfw_compress_8u_bgra (const uint8_t * src, int width, 
        int height, int stride, uint8_t * dest, int * destsize, int quality);

// ============== CamfwJpegCompress ===============
static void on_input_frame_ready (CamUnit * super, const CamFrameBuffer *inbuf,
        const CamUnitFormat *infmt);
static void on_input_format_changed (CamUnit *super, 
        const CamUnitFormat *infmt);
static int _stream_init (CamUnit * super, const CamUnitFormat * format);
static int _stream_shutdown (CamUnit * super);

GType camfw_jpeg_compress_get_type (void);
CAM_PLUGIN_TYPE (CamfwJpegCompress, camfw_jpeg_compress, CAM_TYPE_UNIT);

void cam_plugin_initialize (GTypeModule * module);
CamUnitDriver * cam_plugin_create (GTypeModule * module);

/* These next two functions are required as entry points for the
 * plug-in API. */
void
cam_plugin_initialize (GTypeModule * module)
{
    camfw_jpeg_compress_register_type (module);
}

CamUnitDriver *
cam_plugin_create (GTypeModule * module)
{
    return cam_unit_driver_new_stock_full ("framewave", "jpeg_compress",
            "JPEG Compress", 0, 
            (CamUnitConstructor)camfw_jpeg_compress_new, module);
}

static void
camfw_jpeg_compress_init (CamfwJpegCompress *self)
{
    // constructor.  Initialize the unit with some reasonable defaults here.
    CamUnit *super = CAM_UNIT (self);

    self->quality_control = cam_unit_add_control_int (super, "quality", 
            "Quality", 1, 100, 1, 94, 1);
    g_signal_connect (G_OBJECT(self), "input-format-changed",
            G_CALLBACK(on_input_format_changed), NULL);
}

static void
camfw_jpeg_compress_class_init (CamfwJpegCompressClass *klass)
{
    klass->parent_class.on_input_frame_ready = on_input_frame_ready;
    klass->parent_class.stream_init = _stream_init;
    klass->parent_class.stream_shutdown = _stream_shutdown;
}

CamfwJpegCompress * 
camfw_jpeg_compress_new()
{
    return (CamfwJpegCompress*)(
            g_object_new(camfw_jpeg_compress_get_type(), NULL));
}

static int 
_stream_init (CamUnit * super, const CamUnitFormat * fmt)
{
    CamfwJpegCompress *self = (CamfwJpegCompress*) (super);
    self->outbuf = cam_framebuffer_new_alloc(fmt->width * fmt->height * 4);
    return 0;
}

static int 
_stream_shutdown (CamUnit * super)
{
    CamfwJpegCompress * self = (CamfwJpegCompress*) super;
    g_object_unref(self->outbuf);
    self->outbuf = NULL;
    return 0;
}

static void 
on_input_frame_ready (CamUnit *super, const CamFrameBuffer *inbuf,
        const CamUnitFormat *infmt)
{
    CamfwJpegCompress * self = (CamfwJpegCompress*) (super);
    const CamUnitFormat *outfmt = cam_unit_get_output_format(super);

    int width = infmt->width;
    int height = infmt->height;
    int outsize = self->outbuf->length;
    int quality = cam_unit_control_get_int (self->quality_control);

    if (infmt->pixelformat == CAM_PIXEL_FORMAT_RGB)
        _jpegfw_compress_8u_rgb (inbuf->data, width, height, 
                infmt->row_stride, self->outbuf->data, &outsize, quality);
    else if (infmt->pixelformat == CAM_PIXEL_FORMAT_BGRA)
        _jpegfw_compress_8u_bgra (inbuf->data, width, height, 
                infmt->row_stride, self->outbuf->data, &outsize, quality);
    else if (infmt->pixelformat == CAM_PIXEL_FORMAT_GRAY)
        _jpegfw_compress_8u_gray (inbuf->data, width, height, 
                infmt->row_stride, self->outbuf->data, &outsize, quality);

    cam_framebuffer_copy_metadata(self->outbuf, inbuf);
    self->outbuf->bytesused = outsize;

    cam_unit_produce_frame (super, self->outbuf, outfmt);
}

static void
on_input_format_changed (CamUnit *super, const CamUnitFormat *infmt)
{
    cam_unit_remove_all_output_formats (super);

    if (!infmt) return;

    if (! (infmt->pixelformat == CAM_PIXEL_FORMAT_GRAY ||
           infmt->pixelformat == CAM_PIXEL_FORMAT_RGB ||
           infmt->pixelformat == CAM_PIXEL_FORMAT_BGRA)) return;

    cam_unit_add_output_format (super, CAM_PIXEL_FORMAT_MJPEG,
            NULL, infmt->width, infmt->height, 0);
}

static void
init_destination (j_compress_ptr cinfo)
{
    /* do nothing */
}

static boolean
empty_output_buffer (j_compress_ptr cinfo)
{
    fprintf (stderr, "Error: JPEG compressor ran out of buffer space\n");
    return TRUE;
}

static void
term_destination (j_compress_ptr cinfo)
{
    /* do nothing */
}

static int 
_jpegfw_compress_8u_gray (const uint8_t * src, int width, int height, int stride,
        uint8_t * dest, int * destsize, int quality)
{
    struct jpegfw_compress_struct cinfo;
    struct jpegfw_error_mgr jerr;
    struct jpegfw_destination_mgr jdest;
    int out_size = *destsize;

    cinfo.err = jpegfw_std_error (&jerr);
    jpegfw_create_compress (&cinfo);
    jdest.next_output_byte = dest;
    jdest.free_in_buffer = out_size;
    jdest.init_destination = init_destination;
    jdest.empty_output_buffer = empty_output_buffer;
    jdest.term_destination = term_destination;
    cinfo.dest = &jdest;

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
    jpegfw_set_defaults (&cinfo);
    jpegfw_set_quality (&cinfo, quality, TRUE);

    jpegfw_start_compress (&cinfo, TRUE);
    while (cinfo.next_scanline < height) {
        JSAMPROW row = (JSAMPROW)(src + cinfo.next_scanline * stride);
        jpegfw_write_scanlines (&cinfo, &row, 1);
    }
    jpegfw_finish_compress (&cinfo);
    *destsize = out_size - jdest.free_in_buffer;
    jpegfw_destroy_compress (&cinfo);
    return 0;
}

static int 
_jpegfw_compress_8u_rgb (const uint8_t * src, int width, int height, int stride,
        uint8_t * dest, int * destsize, int quality)
{
    struct jpegfw_compress_struct cinfo;
    struct jpegfw_error_mgr jerr;
    struct jpegfw_destination_mgr jdest;
    int out_size = *destsize;

    cinfo.err = jpegfw_std_error (&jerr);
    jpegfw_create_compress (&cinfo);
    jdest.next_output_byte = dest;
    jdest.free_in_buffer = out_size;
    jdest.init_destination = init_destination;
    jdest.empty_output_buffer = empty_output_buffer;
    jdest.term_destination = term_destination;
    cinfo.dest = &jdest;

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpegfw_set_defaults (&cinfo);
    jpegfw_set_quality (&cinfo, quality, TRUE);

    jpegfw_start_compress (&cinfo, TRUE);
    while (cinfo.next_scanline < height) {
        JSAMPROW row = (JSAMPROW)(src + cinfo.next_scanline * stride);
        jpegfw_write_scanlines (&cinfo, &row, 1);
    }
    jpegfw_finish_compress (&cinfo);
    *destsize = out_size - jdest.free_in_buffer;
    jpegfw_destroy_compress (&cinfo);
    return 0;
}

static int 
_jpegfw_compress_8u_bgra (const uint8_t * src, int width, int height, int stride,
        uint8_t * dest, int * destsize, int quality)
{
    struct jpegfw_compress_struct cinfo;
    struct jpegfw_error_mgr jerr;
    struct jpegfw_destination_mgr jdest;
    int out_size = *destsize;

    cinfo.err = jpegfw_std_error (&jerr);
    jpegfw_create_compress (&cinfo);
    jdest.next_output_byte = dest;
    jdest.free_in_buffer = out_size;
    jdest.init_destination = init_destination;
    jdest.empty_output_buffer = empty_output_buffer;
    jdest.term_destination = term_destination;
    cinfo.dest = &jdest;

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpegfw_set_defaults (&cinfo);
    jpegfw_set_quality (&cinfo, quality, TRUE);

    jpegfw_start_compress (&cinfo, TRUE);
    while (cinfo.next_scanline < height) {
        uint8_t buf[width*3];
        cam_pixel_convert_8u_bgra_to_8u_rgb (buf, 0, width, 1,
                src + cinfo.next_scanline * stride, 0);
        JSAMPROW row = (JSAMPROW) buf;
        jpegfw_write_scanlines (&cinfo, &row, 1);
    }
    jpegfw_finish_compress (&cinfo);
    *destsize = out_size - jdest.free_in_buffer;
    jpegfw_destroy_compress (&cinfo);
    return 0;
}
