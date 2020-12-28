#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <vector>
#include <map>

#include <va/va.h>
#include <va/va_drm.h>

static VAStatus va_status;
static VADisplay va_dpy = NULL;
static int drm_fd = -1;

#define VA_PROFILE_MAP(P)  {P, #P}

static std::map<VAProfile, const char*> profile_map = {
    VA_PROFILE_MAP(VAProfileNone),
    VA_PROFILE_MAP(VAProfileMPEG2Simple),
    VA_PROFILE_MAP(VAProfileMPEG2Main),
    VA_PROFILE_MAP(VAProfileMPEG4Simple),
    VA_PROFILE_MAP(VAProfileMPEG4AdvancedSimple),
    VA_PROFILE_MAP(VAProfileMPEG4Main),
    VA_PROFILE_MAP(VAProfileH264Baseline),
    VA_PROFILE_MAP(VAProfileH264Main),
    VA_PROFILE_MAP(VAProfileH264High),
    VA_PROFILE_MAP(VAProfileVC1Simple),
    VA_PROFILE_MAP(VAProfileVC1Main),
    VA_PROFILE_MAP(VAProfileVC1Advanced),
    VA_PROFILE_MAP(VAProfileH263Baseline),
    VA_PROFILE_MAP(VAProfileJPEGBaseline),
    VA_PROFILE_MAP(VAProfileH264ConstrainedBaseline),
    VA_PROFILE_MAP(VAProfileVP8Version0_3),
    VA_PROFILE_MAP(VAProfileH264MultiviewHigh),
    VA_PROFILE_MAP(VAProfileH264StereoHigh),
    VA_PROFILE_MAP(VAProfileHEVCMain),
    VA_PROFILE_MAP(VAProfileHEVCMain10),
    VA_PROFILE_MAP(VAProfileVP9Profile0),
    VA_PROFILE_MAP(VAProfileVP9Profile1),
    VA_PROFILE_MAP(VAProfileVP9Profile2),
    VA_PROFILE_MAP(VAProfileVP9Profile3),
    VA_PROFILE_MAP(VAProfileHEVCMain12),
    VA_PROFILE_MAP(VAProfileHEVCMain422_10),
    VA_PROFILE_MAP(VAProfileHEVCMain422_12),
    VA_PROFILE_MAP(VAProfileHEVCMain444),
    VA_PROFILE_MAP(VAProfileHEVCMain444_10),
    VA_PROFILE_MAP(VAProfileHEVCMain444_12),
    VA_PROFILE_MAP(VAProfileHEVCSccMain),
    VA_PROFILE_MAP(VAProfileHEVCSccMain10),
    VA_PROFILE_MAP(VAProfileHEVCSccMain444),
    VA_PROFILE_MAP(VAProfileAV1Profile0),
    VA_PROFILE_MAP(VAProfileAV1Profile1),
    VA_PROFILE_MAP(VAProfileHEVCSccMain444_10)
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

int add(int i, int j) 
{
    return i + j;
}

uint64_t init()
{
    int major_ver, minor_ver;

    va_dpy = getVADisplay();
    printf("####INFO: va_dpy = 0x%x, %d\n", va_dpy, va_dpy);

    va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);

    return uint64_t(va_dpy);
}

std::vector<const char*> getProfiles() 
{
    std::vector<const char*> profile_list;

    int max_num_profiles = vaMaxNumProfiles(va_dpy);
    printf("####INFO: max_num_profiles = %d\n", max_num_profiles);
    if (max_num_profiles <= 0) {
        return profile_list;
    }

    int num_profiles = 0;
    std::vector<VAProfile> profile_full(max_num_profiles, VAProfileNone);
    va_status = vaQueryConfigProfiles(va_dpy, (VAProfile*)profile_full.data(), &num_profiles);
    printf("####INFO: num_profiles = %d\n", num_profiles);
    if (va_status != VA_STATUS_SUCCESS) {
        return profile_list;
    }

    for (auto p : profile_full) {
        if (p != VAProfileNone) {
            profile_list.push_back(profile_map[p]);
        }
    }

    return profile_list;
}

PYBIND11_MODULE(pylibva, m) {
    m.doc() = "libva python bindings"; // optional module docstring
    m.def("add", &add, "A function which adds two numbers");
    m.def("init", &init, "function to init va");
    m.def("profiles", &getProfiles, "Get all supported VA profiles");
}

