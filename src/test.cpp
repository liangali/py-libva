#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

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
    if (drm_fd < 0)
        return;

    close(drm_fd);
    drm_fd = -1;
}

int main() 
{
    int major_ver, minor_ver;
    VAStatus va_status;
    unsigned int i;
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
    
    closeVADisplay();

    printf("done\n");
    return 0;
}