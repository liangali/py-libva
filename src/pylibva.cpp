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

#define VA_ENUM_STR_MAP(P)  {P, #P}

static std::map<VAProfile, const char*> profile_map = 
{
    VA_ENUM_STR_MAP(VAProfileNone),
    VA_ENUM_STR_MAP(VAProfileMPEG2Simple),
    VA_ENUM_STR_MAP(VAProfileMPEG2Main),
    VA_ENUM_STR_MAP(VAProfileMPEG4Simple),
    VA_ENUM_STR_MAP(VAProfileMPEG4AdvancedSimple),
    VA_ENUM_STR_MAP(VAProfileMPEG4Main),
    VA_ENUM_STR_MAP(VAProfileH264Baseline),
    VA_ENUM_STR_MAP(VAProfileH264Main),
    VA_ENUM_STR_MAP(VAProfileH264High),
    VA_ENUM_STR_MAP(VAProfileVC1Simple),
    VA_ENUM_STR_MAP(VAProfileVC1Main),
    VA_ENUM_STR_MAP(VAProfileVC1Advanced),
    VA_ENUM_STR_MAP(VAProfileH263Baseline),
    VA_ENUM_STR_MAP(VAProfileJPEGBaseline),
    VA_ENUM_STR_MAP(VAProfileH264ConstrainedBaseline),
    VA_ENUM_STR_MAP(VAProfileVP8Version0_3),
    VA_ENUM_STR_MAP(VAProfileH264MultiviewHigh),
    VA_ENUM_STR_MAP(VAProfileH264StereoHigh),
    VA_ENUM_STR_MAP(VAProfileHEVCMain),
    VA_ENUM_STR_MAP(VAProfileHEVCMain10),
    VA_ENUM_STR_MAP(VAProfileVP9Profile0),
    VA_ENUM_STR_MAP(VAProfileVP9Profile1),
    VA_ENUM_STR_MAP(VAProfileVP9Profile2),
    VA_ENUM_STR_MAP(VAProfileVP9Profile3),
    VA_ENUM_STR_MAP(VAProfileHEVCMain12),
    VA_ENUM_STR_MAP(VAProfileHEVCMain422_10),
    VA_ENUM_STR_MAP(VAProfileHEVCMain422_12),
    VA_ENUM_STR_MAP(VAProfileHEVCMain444),
    VA_ENUM_STR_MAP(VAProfileHEVCMain444_10),
    VA_ENUM_STR_MAP(VAProfileHEVCMain444_12),
    VA_ENUM_STR_MAP(VAProfileHEVCSccMain),
    VA_ENUM_STR_MAP(VAProfileHEVCSccMain10),
    VA_ENUM_STR_MAP(VAProfileHEVCSccMain444),
    VA_ENUM_STR_MAP(VAProfileAV1Profile0),
    VA_ENUM_STR_MAP(VAProfileAV1Profile1),
    VA_ENUM_STR_MAP(VAProfileHEVCSccMain444_10)
};

static std::map<VAEntrypoint, const char*> entrypoint_map = 
{
    VA_ENUM_STR_MAP(VAEntrypointVLD),
    VA_ENUM_STR_MAP(VAEntrypointIZZ),
    VA_ENUM_STR_MAP(VAEntrypointIDCT),
    VA_ENUM_STR_MAP(VAEntrypointMoComp),
    VA_ENUM_STR_MAP(VAEntrypointDeblocking),
    VA_ENUM_STR_MAP(VAEntrypointEncSlice),
    VA_ENUM_STR_MAP(VAEntrypointEncPicture),
    VA_ENUM_STR_MAP(VAEntrypointEncSliceLP),
    VA_ENUM_STR_MAP(VAEntrypointVideoProc),
    VA_ENUM_STR_MAP(VAEntrypointFEI),
    VA_ENUM_STR_MAP(VAEntrypointStats)
};

VAProfile str2Profile(const char* str)
{
    for(auto m : profile_map) {
        if (strcmp(str, m.second) == 0) {
            return m.first;
        }
    }
    return VAProfileNone;
}

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

int add(int i, int j) 
{
    return i + j;
}

void vaClose()
{
    vaTerminate(va_dpy);

    if (drm_fd < 0)
        return;

    close(drm_fd);
    drm_fd = -1;
}

uint64_t vaInit()
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

std::vector<const char*> getEntrypoints(const char* profile_str) 
{
    std::vector<const char*> entrypoint_list;

    int num_entrypoint = 0;
    int max = vaMaxNumEntrypoints (va_dpy);
    std::vector<VAEntrypoint> entrypoints(max);
    VAProfile profile = str2Profile(profile_str);

    va_status = vaQueryConfigEntrypoints(va_dpy, profile, entrypoints.data(), &num_entrypoint);
    if (va_status != VA_STATUS_SUCCESS) {
        return entrypoint_list;
    }

    for (auto e: entrypoints) {
        if (int(e) != 0) {
            entrypoint_list.push_back(entrypoint_map[e]);
        }
    }

    return entrypoint_list;
}

PYBIND11_MODULE(pylibva, m) {
    m.doc() = "libva python bindings"; // optional module docstring
    m.def("add", &add, "A function which adds two numbers");
    m.def("init", &vaInit, "Initialize VADisplay");
    m.def("close", &vaClose, "Close VADisplay and drm fd");
    m.def("profiles", &getProfiles, "Get all supported VA Profiles");
    m.def("entrypoints", &getEntrypoints, "Get Entrypoints list of a Profile");
}

