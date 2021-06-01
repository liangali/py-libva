#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include <vector>
#include <set>
#include <map>

#include <va/va.h>
#include <va/va_drm.h>

using namespace std;

VAStatus va_status;
int major_ver, minor_ver;
int ret_val = 0;
static VADisplay va_dpy = NULL;
static int drm_fd = -1;

#define CHECK_VASTATUS(va_status,func, ret)                             \
if (va_status != VA_STATUS_SUCCESS) {                                   \
    fprintf(stderr,"%s failed with error code %d (%s),exit\n",func, va_status, vaErrorStr(va_status)); \
    ret_val = ret;                                                      \
    exit(1);                                                         \
}

#define VA_PROFILE_MAP(P)  {P, #P}

static std::map<VAProfile, const char*> profile_map = {
    VA_PROFILE_MAP(VAProfileNone),
};

VADisplay getVADisplay(void)
{
    const char *drm_device_paths[] = {
        "/dev/dri/renderD128",
        "/dev/dri/card0",
        NULL
    };

    for (int i = 0; drm_device_paths[i]; i++) {
        drm_fd = open(drm_device_paths[i], O_RDWR);
        if (drm_fd < 0)
            continue;

        va_dpy = vaGetDisplayDRM(drm_fd);
        if (va_dpy)
            return va_dpy;

        close(drm_fd);
        drm_fd = -1;
    }

    return NULL;
}

void closeVADisplay()
{
    vaTerminate(va_dpy);
    
    if (drm_fd < 0)
        return;

    close(drm_fd);
    drm_fd = -1;
}

int test_caps() 
{
    va_dpy = getVADisplay();
    printf("####INFO: va_dpy = %x\n", va_dpy);

    va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);

    const char *driver = vaQueryVendorString(va_dpy);
    printf("####INFO: driver version: %s\n", driver);

    int num_entrypoint = vaMaxNumEntrypoints (va_dpy);
    printf("####INFO: entrypoint num: %d\n", num_entrypoint);

    int max_num_profiles = vaMaxNumProfiles(va_dpy);
    printf("####INFO: profile num: %d\n", max_num_profiles);

    int num_profiles = 0;
    vector<VAProfile> profile_list(max_num_profiles, VAProfileNone);
    va_status = vaQueryConfigProfiles(va_dpy, (VAProfile*)profile_list.data(), &num_profiles);
    CHECK_VASTATUS(va_status, "vaQueryConfigProfiles", 6);

    set<VAProfile> profile_set;
    for (auto p: profile_list) {
        if (p != VAProfileNone) {
            profile_set.insert(p);
        }
    }

    VAConfigAttrib attrib = {};
    attrib.type = VAConfigAttribRTFormat;
    va_status = vaGetConfigAttributes(va_dpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1);
    CHECK_VASTATUS(va_status, "vaGetConfigAttributes", 6);
    printf("####LOG: VideoProc Attrib type = %d, value = 0x%08x\n", attrib.type, attrib.value);

    VAConfigID config_id = 0;
    va_status = vaCreateConfig(va_dpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1, &config_id);
    CHECK_VASTATUS(va_status, "vaCreateConfig", 6);

    uint32_t num_surf_attribs = 0;
    va_status = vaQuerySurfaceAttributes(va_dpy, config_id, nullptr, &num_surf_attribs);
    vector<VASurfaceAttrib> surf_attribs(num_surf_attribs);
    va_status = vaQuerySurfaceAttributes(va_dpy, config_id, surf_attribs.data(), &num_surf_attribs);
    CHECK_VASTATUS(va_status, "vaQuerySurfaceAttributes", 6);
    for (auto a: surf_attribs) {
        printf("####LOG: type = %d, flags = 0x%08x, value = 0x%0x8\n", a.type, a.flags, a.value.value.i);
    }

    VASurfaceID surf_id;
    uint32_t w = 1280, h = 720;
    vaCreateSurfaces(va_dpy, VA_RT_FORMAT_YUV420, w, h, &surf_id, 1, nullptr, 0);
    CHECK_VASTATUS(va_status, "vaCreateSurfaces", 6);

    VAContextID ctx_id;
    va_status = vaCreateContext(va_dpy, config_id, w, h, VA_PROGRESSIVE, &surf_id, 1, &ctx_id);
    CHECK_VASTATUS(va_status, "vaCreateContext", 6);

    uint32_t filter_num = VAProcFilterCount;
    vector<VAProcFilterType> filter_list(filter_num);
    va_status = vaQueryVideoProcFilters(va_dpy, ctx_id, filter_list.data(), &filter_num);
    CHECK_VASTATUS(va_status, "vaQueryVideoProcFilters", 6);

    for (auto f: filter_list) {
        uint32_t caps_num = 0;
        uint8_t caps_data[1024]; 
        vaQueryVideoProcFilterCaps(va_dpy, ctx_id, f, caps_data, &caps_num);
        printf("####LOG: filter_type = %d, caps_num = %d\n", f, caps_num);
    }

    VAProcPipelineCaps pipeline_caps = {};
    va_status = vaQueryVideoProcPipelineCaps(va_dpy, ctx_id, nullptr, 0, &pipeline_caps);
    CHECK_VASTATUS(va_status, "vaQueryVideoProcPipelineCaps", 6);
    printf("####LOG: size of VAProcPipelineCaps = %d\n", sizeof(pipeline_caps));

    closeVADisplay();

    printf("done\n");
    return 0;
}

void printVAImage(VAImage &img)
{
    printf("\nVAImage: ================\n");
    printf("VAImage: image_id = %d\n", img.image_id);
    printf("VAImage: buf_id   = %d\n", img.buf);
    printf("VAImage: format: \n");
    printf("            fourcc         = 0x%08x\n", img.format.fourcc);
    printf("            byte_order     = %d\n", img.format.byte_order);
    printf("            bits_per_pixel = %d\n", img.format.bits_per_pixel);
    printf("            depth          = %d\n", img.format.depth);
    printf("            red_mask       = %d\n", img.format.red_mask);
    printf("            green_mask     = %d\n", img.format.green_mask);
    printf("            blue_mask      = %d\n", img.format.blue_mask);
    printf("            alpha_mask     = %d\n", img.format.alpha_mask);
    printf("VAImage: width = %d\n", img.width);
    printf("VAImage: height = %d\n", img.height);
    printf("VAImage: data_size = %d\n", img.data_size);
    printf("VAImage: num_planes = %d\n", img.num_planes);
    printf("VAImage: pitches = [%d, %d, %d]\n", img.pitches[0], img.pitches[1], img.pitches[2]);
    printf("VAImage: offsets = [%d, %d, %d]\n", img.offsets[0], img.offsets[1], img.offsets[2]);
    printf("VAImage: num_palette_entries = %d\n", img.num_palette_entries);
    printf("VAImage: entry_bytes = %d\n", img.entry_bytes);
    printf("VAImage: component_order = [%d, %d, %d, %d]\n", img.component_order[0], img.component_order[1], img.component_order[2], img.component_order[3]);
    printf("VAImage: ================\n");
}

int upload_surface(VASurfaceID surf_id)
{
    VAImage va_img = {};
    void *surf_ptr = nullptr;

    va_status = vaDeriveImage(va_dpy, surf_id, &va_img);
    CHECK_VASTATUS(va_status, "vaDeriveImage", 1);
    printVAImage(va_img);
    uint16_t w = va_img.width;
    uint16_t h = va_img.height;
    uint32_t pitch = va_img.pitches[0];
    uint32_t uv_offset = va_img.offsets[1];

    va_status = vaMapBuffer(va_dpy, va_img.buf, &surf_ptr);
    CHECK_VASTATUS(va_status, "vaMapBuffer", 1);

    vector<char> src(w*h*3/2, 0);
    char* dst = (char*)surf_ptr;
    FILE* fp = fopen("../../test.nv12", "rb");
    fread(src.data(), w*h*3/2, 1, fp);
    fclose(fp);

    memset(dst, 0, va_img.data_size);
    // Y plane
    for (size_t i = 0; i < h; i++)
        memcpy(dst+i*pitch, src.data()+i*w, w);
    // UV plane
    for (size_t i = 0; i < h/2; i++)
        memcpy(dst+ uv_offset + i*pitch, src.data()+(h+i)*w, w);
    
    vaUnmapBuffer(va_dpy, va_img.buf);
    vaDestroyImage(va_dpy, va_img.image_id);

    return 0;
}

int save_surface(VASurfaceID surf_id)
{
    VAImage va_img = {};
    void *surf_ptr = nullptr;

    va_status = vaDeriveImage(va_dpy, surf_id, &va_img);
    CHECK_VASTATUS(va_status, "vaDeriveImage", 1);
    uint16_t w = va_img.width;
    uint16_t h = va_img.height;
    uint32_t pitch = va_img.pitches[0];
    uint32_t uv_offset = va_img.offsets[1];
    printVAImage(va_img);
    
    va_status = vaMapBuffer(va_dpy, va_img.buf, &surf_ptr);
    CHECK_VASTATUS(va_status, "vaMapBuffer", 1);

    char* src = (char*)surf_ptr;
    vector<char> dst(w*h*3/2, 0);

    // Y plane
    for (size_t i = 0; i < h; i++)
        memcpy(dst.data()+i*w, src+i*pitch, w);
    // UV plane
    for (size_t i = 0; i < h/2; i++)
        memcpy(dst.data()+(h+i)*w, src+uv_offset+i*pitch, w);

    FILE* fp = fopen("../../test.out.nv12", "wb");
    fwrite(dst.data(), w*h*3/2, 1, fp);
    fclose(fp);

    vaUnmapBuffer(va_dpy, va_img.buf);
    vaDestroyImage(va_dpy, va_img.image_id);

    return 0;
}

int test_vpp()
{
    uint32_t srcw = 320;
    uint32_t srch = 240;
    uint32_t dstw = 640;
    uint32_t dsth = 480;
    uint32_t src_fourcc  = VA_FOURCC('N','V','1','2');
    uint32_t dst_fourcc  = VA_FOURCC('N','V','1','2'); //VA_FOURCC('I','4','2','0');
    uint32_t src_format  = VA_RT_FORMAT_YUV420;
    uint32_t dst_format  = VA_RT_FORMAT_YUV420;
    VASurfaceID src_surf = VA_INVALID_ID;
    VASurfaceID dst_surf = VA_INVALID_ID;
    
    va_dpy = getVADisplay();
    va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);

    VAConfigAttrib attrib = {};
    attrib.type = VAConfigAttribRTFormat;
    va_status = vaGetConfigAttributes(va_dpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1);
    CHECK_VASTATUS(va_status, "vaGetConfigAttributes", 1);

    VAConfigID config_id = 0;
    va_status = vaCreateConfig(va_dpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1, &config_id);
    CHECK_VASTATUS(va_status, "vaCreateConfig", 1);
    
    VASurfaceAttrib surf_attrib = {};
    surf_attrib.type =  VASurfaceAttribPixelFormat;
    surf_attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;
    surf_attrib.value.type = VAGenericValueTypeInteger;
    surf_attrib.value.value.i = src_fourcc;
    va_status = vaCreateSurfaces(va_dpy, src_format, srcw, srch, &src_surf, 1, &surf_attrib, 1);
    CHECK_VASTATUS(va_status, "vaCreateSurfaces", 1);
    printf("####LOG: src_surf = %d\n", src_surf);

    surf_attrib.type =  VASurfaceAttribPixelFormat;
    surf_attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;
    surf_attrib.value.type = VAGenericValueTypeInteger;
    surf_attrib.value.value.i = dst_fourcc;
    va_status = vaCreateSurfaces(va_dpy, dst_format, dstw, dsth, &dst_surf, 1, &surf_attrib, 1);
    CHECK_VASTATUS(va_status, "vaCreateSurfaces", 1);
    printf("####LOG: dst_surf = %d\n", dst_surf);

    VAContextID ctx_id = 0;
    va_status = vaCreateContext(va_dpy, config_id, dstw, dsth, VA_PROGRESSIVE, &dst_surf, 1, &ctx_id);
    CHECK_VASTATUS(va_status, "vaCreateContext", 1);
    printf("####LOG: ctx_id = 0x%08x\n", ctx_id);

    upload_surface(src_surf);

    VAProcPipelineParameterBuffer pipeline_param = {};
    VARectangle src_rect = {0, 0, srcw, srch};
    VARectangle dst_rect = {0, 0, dstw, dsth};
    VABufferID pipeline_buf_id = VA_INVALID_ID;
    uint32_t filter_count = 0;
    VABufferID filter_buf_id = VA_INVALID_ID;
    pipeline_param.surface = src_surf;
    pipeline_param.surface_region = &src_rect;
    pipeline_param.output_region = &dst_rect;
    pipeline_param.filter_flags = 0;
    pipeline_param.filters      = &filter_buf_id;
    pipeline_param.num_filters  = filter_count;
    va_status = vaCreateBuffer(va_dpy, ctx_id, VAProcPipelineParameterBufferType, sizeof(pipeline_param), 1, &pipeline_param, &pipeline_buf_id);
    CHECK_VASTATUS(va_status, "vaCreateBuffer", 1);

    va_status = vaBeginPicture(va_dpy, ctx_id, dst_surf);
    CHECK_VASTATUS(va_status, "vaBeginPicture", 1);

    va_status = vaRenderPicture(va_dpy, ctx_id, &pipeline_buf_id, 1);
    CHECK_VASTATUS(va_status, "vaRenderPicture", 1);

    va_status = vaEndPicture(va_dpy, ctx_id);
    CHECK_VASTATUS(va_status, "vaEndPicture", 1);

    save_surface(dst_surf);

    vaDestroyBuffer(va_dpy, pipeline_buf_id);
    vaDestroySurfaces(va_dpy, &src_surf, 1);
    vaDestroySurfaces(va_dpy, &dst_surf, 1);
    vaDestroyContext(va_dpy, ctx_id);
    vaDestroyConfig(va_dpy, config_id);
    closeVADisplay();

    printf("done\n");
    return 0;
}

int test_attrib()
{
    va_dpy = getVADisplay();
    va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);
    printf("####Test: major_ver = %d, minor_ver = %d\n", major_ver, minor_ver);

    printf("####Test: %d\n", vaMaxNumDisplayAttributes(va_dpy));

    VADisplayAttribute attr_list[16] = {};
    int num_attributes = 0;
    va_status = vaQueryDisplayAttributes(va_dpy, attr_list, &num_attributes);
    CHECK_VASTATUS(va_status, "vaQueryDisplayAttributes", 1);

    VADisplayAttribValMemoryRegion mem_region = {};
    mem_region.value = attr_list[0].value;

    VADisplayAttribute mem_attribute = {};
    mem_attribute.type = VADisplayAttribMemoryRegion;
    va_status = vaGetDisplayAttributes(va_dpy, &mem_attribute, 1);
    CHECK_VASTATUS(va_status, "vaGetDisplayAttributes", 1);
    VADisplayAttribValMemoryRegion* p = (VADisplayAttribValMemoryRegion*)&mem_attribute.value;
    printf("####Test: current_tile = %d, tile_num = %d, tile_mask = 0x%08x\n", p->bits.current_memory_region, p->bits.local_memory_regions, p->bits.memory_regions_masks);

    VADisplayAttribute mem_attribute2 = {};
    mem_attribute2.type = VADisplayAttribMemoryRegion;
    mem_attribute2.value = mem_region.value;
    VADisplayAttribValMemoryRegion* p_mem_region = (VADisplayAttribValMemoryRegion*)&mem_attribute2.value;
    p_mem_region->bits.current_memory_region = 1;
    va_status = vaSetDisplayAttributes(va_dpy, &mem_attribute2, 1);
    CHECK_VASTATUS(va_status, "vaSetDisplayAttributes", 1);

    // creat RT surface
    VASurfaceID test_surf = VA_INVALID_ID;
    VASurfaceAttrib surf_attrib = {};
    uint32_t test_fourcc  = VA_FOURCC('N','V','1','2');
    uint32_t test_format  = VA_RT_FORMAT_YUV420;
    surf_attrib.type =  VASurfaceAttribPixelFormat;
    surf_attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;
    surf_attrib.value.type = VAGenericValueTypeInteger;
    surf_attrib.value.value.i = test_fourcc;
    va_status = vaCreateSurfaces(va_dpy, test_format, 640, 480, &test_surf, 1, &surf_attrib, 1);
    CHECK_VASTATUS(va_status, "vaCreateSurfaces", 1);
    printf("####Test: test_surf = %d\n", test_surf);
    vaDestroySurfaces(va_dpy, &test_surf, 1);

    closeVADisplay();
    return 0;
}

int main()
{
    //test_caps();
    //test_vpp();

    test_attrib();

    return 0;
}