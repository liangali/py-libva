#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <va/va.h>
#include <va/va_drm.h>

static VADisplay va_dpy = NULL;
static int drm_fd = -1;

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

    va_dpy = getVADisplay();

    va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);

    const char *driver = vaQueryVendorString(va_dpy);
    printf("####INFO: driver version: %s\n", driver);

    int num_entrypoint = vaMaxNumEntrypoints (va_dpy);
    printf("####INFO: entrypoint num: %d\n", num_entrypoint);

    int max_num_profiles = vaMaxNumProfiles(va_dpy);
    printf("####INFO: profile num: %d\n", max_num_profiles);

    closeVADisplay();

    printf("done\n");
    return 0;
}