#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include <vector>
#include <set>
#include <map>

#include <va/va.h>
#include <va/va_drm.h>

using namespace std;

#define CHECK_VASTATUS(va_status,func, ret)                             \
if (va_status != VA_STATUS_SUCCESS) {                                   \
    fprintf(stderr,"%s failed with error code %d (%s),exit\n",func, va_status, vaErrorStr(va_status)); \
    ret_val = ret;                                                      \
    exit(1);                                                         \
}

static VADisplay va_dpy = NULL;
static int drm_fd = -1;

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
    int major_ver, minor_ver;
    VAStatus va_status;
    int ret_val = 0;
    
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

int test_vpp()
{
    int major_ver, minor_ver;
    VAStatus va_status;
    int ret_val = 0;
    
    va_dpy = getVADisplay();
    printf("####INFO: va_dpy = %x, va_dpy = %d\n", va_dpy, va_dpy);
    va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);


    closeVADisplay();
    printf("done\n");
    return 0;
}

int main()
{
    //test_caps();

    test_vpp();

    return 0;
}