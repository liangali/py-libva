#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <string>
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

static std::map<VAConfigAttribType, const char*> configattrib_map = 
{
    VA_ENUM_STR_MAP(VAConfigAttribRTFormat),
    VA_ENUM_STR_MAP(VAConfigAttribSpatialResidual),
    VA_ENUM_STR_MAP(VAConfigAttribSpatialClipping),
    VA_ENUM_STR_MAP(VAConfigAttribIntraResidual),
    VA_ENUM_STR_MAP(VAConfigAttribEncryption),
    VA_ENUM_STR_MAP(VAConfigAttribRateControl),
    VA_ENUM_STR_MAP(VAConfigAttribDecSliceMode),
    VA_ENUM_STR_MAP(VAConfigAttribDecJPEG),
    VA_ENUM_STR_MAP(VAConfigAttribDecProcessing),
    VA_ENUM_STR_MAP(VAConfigAttribEncPackedHeaders),
    VA_ENUM_STR_MAP(VAConfigAttribEncInterlaced),
    VA_ENUM_STR_MAP(VAConfigAttribEncMaxRefFrames),
    VA_ENUM_STR_MAP(VAConfigAttribEncMaxSlices),
    VA_ENUM_STR_MAP(VAConfigAttribEncSliceStructure),
    VA_ENUM_STR_MAP(VAConfigAttribEncMacroblockInfo),
    VA_ENUM_STR_MAP(VAConfigAttribMaxPictureWidth),
    VA_ENUM_STR_MAP(VAConfigAttribMaxPictureHeight),
    VA_ENUM_STR_MAP(VAConfigAttribEncJPEG),
    VA_ENUM_STR_MAP(VAConfigAttribEncQualityRange),
    VA_ENUM_STR_MAP(VAConfigAttribEncQuantization),
    VA_ENUM_STR_MAP(VAConfigAttribEncIntraRefresh),
    VA_ENUM_STR_MAP(VAConfigAttribEncSkipFrame),
    VA_ENUM_STR_MAP(VAConfigAttribEncROI),
    VA_ENUM_STR_MAP(VAConfigAttribEncRateControlExt),
    VA_ENUM_STR_MAP(VAConfigAttribProcessingRate),
    VA_ENUM_STR_MAP(VAConfigAttribEncDirtyRect),
    VA_ENUM_STR_MAP(VAConfigAttribEncParallelRateControl),
    VA_ENUM_STR_MAP(VAConfigAttribEncDynamicScaling),
    VA_ENUM_STR_MAP(VAConfigAttribFrameSizeToleranceSupport),
    VA_ENUM_STR_MAP(VAConfigAttribFEIFunctionType),
    VA_ENUM_STR_MAP(VAConfigAttribFEIMVPredictors),
    VA_ENUM_STR_MAP(VAConfigAttribStats),
    VA_ENUM_STR_MAP(VAConfigAttribEncTileSupport),
    VA_ENUM_STR_MAP(VAConfigAttribCustomRoundingControl),
    VA_ENUM_STR_MAP(VAConfigAttribQPBlockSize),
    VA_ENUM_STR_MAP(VAConfigAttribMaxFrameSize),
    VA_ENUM_STR_MAP(VAConfigAttribPredictionDirection),
    VA_ENUM_STR_MAP(VAConfigAttribMultipleFrame),
    VA_ENUM_STR_MAP(VAConfigAttribContextPriority),
    VA_ENUM_STR_MAP(VAConfigAttribTypeMax)
};

static std::map<VASurfaceAttribType, const char*> surfaceattrib_map = 
{
    VA_ENUM_STR_MAP(VASurfaceAttribNone), 
    VA_ENUM_STR_MAP(VASurfaceAttribPixelFormat), 
    VA_ENUM_STR_MAP(VASurfaceAttribMinWidth), 
    VA_ENUM_STR_MAP(VASurfaceAttribMaxWidth), 
    VA_ENUM_STR_MAP(VASurfaceAttribMinHeight), 
    VA_ENUM_STR_MAP(VASurfaceAttribMaxHeight), 
    VA_ENUM_STR_MAP(VASurfaceAttribMemoryType), 
    VA_ENUM_STR_MAP(VASurfaceAttribExternalBufferDescriptor), 
    VA_ENUM_STR_MAP(VASurfaceAttribUsageHint), 
    VA_ENUM_STR_MAP(VASurfaceAttribCount)
};

static std::map<const char*, uint32_t> rtformat_map = 
{
    {"VA_RT_FORMAT_YUV420", 0x00000001}, 
    {"VA_RT_FORMAT_YUV422", 0x00000002}, 
    {"VA_RT_FORMAT_YUV444", 0x00000004}, 
    {"VA_RT_FORMAT_YUV411", 0x00000008}, 
    {"VA_RT_FORMAT_YUV400", 0x00000010}, 
    {"VA_RT_FORMAT_YUV420_10", 0x00000100}, 
    {"VA_RT_FORMAT_YUV422_10", 0x00000200}, 
    {"VA_RT_FORMAT_YUV444_10", 0x00000400}, 
    {"VA_RT_FORMAT_YUV420_12", 0x00001000}, 
    {"VA_RT_FORMAT_YUV422_12", 0x00002000}, 
    {"VA_RT_FORMAT_YUV444_12", 0x00004000}, 
    {"VA_RT_FORMAT_RGB16", 0x00010000}, 
    {"VA_RT_FORMAT_RGB32", 0x00020000}, 
    {"VA_RT_FORMAT_RGBP", 0x00100000}, 
    {"VA_RT_FORMAT_RGB32_10", 0x00200000}, 
    {"VA_RT_FORMAT_PROTECTED", 0x80000000}, 
    {"VA_RT_FORMAT_RGB32_10BPP", 0x00200000}, 
    {"VA_RT_FORMAT_YUV420_10BPP", 0x00000100}
};

#define ADD_ATTRIB_STR(type) if (value & type) result.push_back(#type)
#define ADD_ATTRIB_VALUE()

std::vector<const char*> parseConfig(VAConfigAttribType type, int value)
{
    char *str = new char[256];
    sprintf(str, "0x%08x", value);
    std::vector<const char*> result;
    VAConfigAttribValEncROI* roi = (VAConfigAttribValEncROI*)&value;
    VAConfigAttribValEncRateControlExt* rce = (VAConfigAttribValEncRateControlExt*)&value;
    VAConfigAttribValMaxFrameSize* mfs = (VAConfigAttribValMaxFrameSize*)&value;

    switch (type)
    {
        case VAConfigAttribRTFormat:
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV420);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV422);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV444);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV411);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV400);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV420_10);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV422_10);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV444_10);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV420_12);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV422_12);
            ADD_ATTRIB_STR(VA_RT_FORMAT_YUV444_12);
            ADD_ATTRIB_STR(VA_RT_FORMAT_RGB16);
            ADD_ATTRIB_STR(VA_RT_FORMAT_RGB32);
            ADD_ATTRIB_STR(VA_RT_FORMAT_RGBP);
            ADD_ATTRIB_STR(VA_RT_FORMAT_RGB32_10);
            ADD_ATTRIB_STR(VA_RT_FORMAT_PROTECTED);
            break;
        case VAConfigAttribSpatialResidual:
            result.push_back(str);
            break;
        case VAConfigAttribSpatialClipping:
            result.push_back(str);
            break;
        case VAConfigAttribIntraResidual:
            result.push_back(str);
            break;
        case VAConfigAttribEncryption:
            result.push_back(str);
            break;
        case VAConfigAttribRateControl:
            ADD_ATTRIB_STR(VA_RC_NONE);
            ADD_ATTRIB_STR(VA_RC_CBR);
            ADD_ATTRIB_STR(VA_RC_VBR);
            ADD_ATTRIB_STR(VA_RC_VCM);
            ADD_ATTRIB_STR(VA_RC_CQP);
            ADD_ATTRIB_STR(VA_RC_VBR_CONSTRAINED);
            ADD_ATTRIB_STR(VA_RC_ICQ);
            ADD_ATTRIB_STR(VA_RC_MB);
            ADD_ATTRIB_STR(VA_RC_CFS);
            ADD_ATTRIB_STR(VA_RC_PARALLEL);
            ADD_ATTRIB_STR(VA_RC_QVBR);
            ADD_ATTRIB_STR(VA_RC_AVBR);
            ADD_ATTRIB_STR(VA_RC_TCBRC);
            break;
        case VAConfigAttribDecSliceMode:
            ADD_ATTRIB_STR(VA_DEC_SLICE_MODE_NORMAL);
            ADD_ATTRIB_STR(VA_DEC_SLICE_MODE_BASE);
            break;
        case VAConfigAttribDecJPEG:
            ADD_ATTRIB_STR(VA_ROTATION_NONE);
            ADD_ATTRIB_STR(VA_ROTATION_90);
            ADD_ATTRIB_STR(VA_ROTATION_180);
            ADD_ATTRIB_STR(VA_ROTATION_270);
            break;
        case VAConfigAttribDecProcessing:
            ADD_ATTRIB_STR(VA_DEC_PROCESSING_NONE);
            ADD_ATTRIB_STR(VA_DEC_PROCESSING);
            break;
        case VAConfigAttribEncPackedHeaders:
            ADD_ATTRIB_STR(VA_ENC_PACKED_HEADER_NONE);
            ADD_ATTRIB_STR(VA_ENC_PACKED_HEADER_SEQUENCE);
            ADD_ATTRIB_STR(VA_ENC_PACKED_HEADER_PICTURE);
            ADD_ATTRIB_STR(VA_ENC_PACKED_HEADER_SLICE);
            ADD_ATTRIB_STR(VA_ENC_PACKED_HEADER_MISC);
            ADD_ATTRIB_STR(VA_ENC_PACKED_HEADER_RAW_DATA);
            break;
        case VAConfigAttribEncInterlaced:
            ADD_ATTRIB_STR(VA_ENC_INTERLACED_NONE);
            ADD_ATTRIB_STR(VA_ENC_INTERLACED_FRAME);
            ADD_ATTRIB_STR(VA_ENC_INTERLACED_FIELD);
            ADD_ATTRIB_STR(VA_ENC_INTERLACED_MBAFF);
            ADD_ATTRIB_STR(VA_ENC_INTERLACED_PAFF);
            break;
        case VAConfigAttribEncMaxRefFrames:
            sprintf(str, "ref_num_list0 = %d, ref_num_list1 = %d", value&0xffff, (value>>16)&0xffff);
            result.push_back(str);
            break;
        case VAConfigAttribEncMaxSlices:
            sprintf(str, "%d", value);
            result.push_back(str);
            break;
        case VAConfigAttribEncSliceStructure:
            ADD_ATTRIB_STR(VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS);
            ADD_ATTRIB_STR(VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS);
            ADD_ATTRIB_STR(VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS);
            ADD_ATTRIB_STR(VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE);
            ADD_ATTRIB_STR(VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS);
            ADD_ATTRIB_STR(VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS);
            break;
        case VAConfigAttribEncMacroblockInfo:
            result.push_back(str);
            break;
        case VAConfigAttribMaxPictureWidth:
            sprintf(str, "%d", value);
            result.push_back(str);
            break;
        case VAConfigAttribMaxPictureHeight:
            sprintf(str, "%d", value);
            result.push_back(str);
            break;
        case VAConfigAttribEncJPEG:
            result.push_back(str);
            break;
        case VAConfigAttribEncQualityRange:
            sprintf(str, "%d", value);
            result.push_back(str);
            break;
        case VAConfigAttribEncQuantization:
            ADD_ATTRIB_STR(VA_ENC_QUANTIZATION_NONE);
            ADD_ATTRIB_STR(VA_ENC_QUANTIZATION_TRELLIS_SUPPORTED);
            break;
        case VAConfigAttribEncIntraRefresh:
            ADD_ATTRIB_STR(VA_ENC_INTRA_REFRESH_NONE);
            ADD_ATTRIB_STR(VA_ENC_INTRA_REFRESH_ROLLING_COLUMN);
            ADD_ATTRIB_STR(VA_ENC_INTRA_REFRESH_ROLLING_ROW);
            ADD_ATTRIB_STR(VA_ENC_INTRA_REFRESH_ADAPTIVE);
            ADD_ATTRIB_STR(VA_ENC_INTRA_REFRESH_CYCLIC);
            ADD_ATTRIB_STR(VA_ENC_INTRA_REFRESH_P_FRAME);
            ADD_ATTRIB_STR(VA_ENC_INTRA_REFRESH_B_FRAME);
            ADD_ATTRIB_STR(VA_ENC_INTRA_REFRESH_MULTI_REF);
            break;
        case VAConfigAttribEncSkipFrame:
            result.push_back(str);
            break;
        case VAConfigAttribEncROI:
            sprintf(str, "num_roi_regions = %d, priority_support = %d, delta_support = %d", 
                roi->bits.num_roi_regions,
                roi->bits.roi_rc_priority_support,
                roi->bits.roi_rc_qp_delta_support);
            result.push_back(str);
            break;
        case VAConfigAttribEncRateControlExt:
            sprintf(str, "max_temporal_layers = %d, temporal_layer_rate_control_flag = %d", 
                rce->bits.max_num_temporal_layers_minus1 + 1, 
                rce->bits.temporal_layer_bitrate_control_flag);
            result.push_back(str);
            break;
        case VAConfigAttribProcessingRate:
            ADD_ATTRIB_STR(VA_PROCESSING_RATE_NONE);
            ADD_ATTRIB_STR(VA_PROCESSING_RATE_ENCODE);
            ADD_ATTRIB_STR(VA_PROCESSING_RATE_DECODE);
            ADD_ATTRIB_STR(VA_ATTRIB_NOT_SUPPORTED);
            break;
        case VAConfigAttribEncDirtyRect:
            result.push_back(str);
            break;
        case VAConfigAttribEncParallelRateControl:
            sprintf(str, "maximum_supported_layer = %d", value);
            result.push_back(str);
            break;
        case VAConfigAttribEncDynamicScaling:
            result.push_back(str);
            break;
        case VAConfigAttribFrameSizeToleranceSupport:
            result.push_back(str);
            break;
        case VAConfigAttribFEIFunctionType:
            result.push_back(str);
            break;
        case VAConfigAttribFEIMVPredictors:
            result.push_back(str);
            break;
        case VAConfigAttribStats:
            result.push_back(str);
            break;
        case VAConfigAttribEncTileSupport:
            result.push_back(str);
            break;
        case VAConfigAttribCustomRoundingControl:
            result.push_back(value? "True" : "False");
            break;
        case VAConfigAttribQPBlockSize:
            result.push_back(str);
            break;
        case VAConfigAttribMaxFrameSize:
            sprintf(str, "max_frame_size = %d, multiple_pass = %d", mfs->bits.max_frame_size, mfs->bits.multiple_pass);
            result.push_back(str);
            break;
        case VAConfigAttribPredictionDirection:
            ADD_ATTRIB_STR(VA_PREDICTION_DIRECTION_PREVIOUS);
            ADD_ATTRIB_STR(VA_PREDICTION_DIRECTION_FUTURE);
            ADD_ATTRIB_STR(VA_PREDICTION_DIRECTION_BI_NOT_EMPTY);
            break;
        case VAConfigAttribMultipleFrame:
            result.push_back(str);
            break;
        case VAConfigAttribContextPriority:
            result.push_back(str);
            break;
        case VAConfigAttribTypeMax:
            result.push_back(str);
            break;
        default:
            break;
        }
        return result;
}

void parseSurfaceAttrib(VASurfaceAttrib sa, std::vector<const char*> & result)
{
    char *str = new char[256];
    int value = sa.value.value.i;
    char* p = (char*)&sa.value.value.i;
    sprintf(str, "0x%08x", sa.value.value.i);

    switch (sa.type)
    {
    case VASurfaceAttribPixelFormat:
        sprintf(str, "VA_FOURCC_%c%c%c%c", p[0], p[1], p[2], p[3]);
        result.push_back(str);
        break;
    case VASurfaceAttribMinWidth:
        sprintf(str, "%d", sa.value.value.i);
        result.push_back(str);
        break;
    case VASurfaceAttribMaxWidth:
        sprintf(str, "%d", sa.value.value.i);
        result.push_back(str);
        break;
    case VASurfaceAttribMinHeight:
        sprintf(str, "%d", sa.value.value.i);
        result.push_back(str);
        break;
    case VASurfaceAttribMaxHeight:
        sprintf(str, "%d", sa.value.value.i);
        result.push_back(str);
        break;
    case VASurfaceAttribMemoryType:
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_MEM_TYPE_VA);
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_MEM_TYPE_V4L2);
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR);
        break;
    case VASurfaceAttribExternalBufferDescriptor:
        result.push_back(str);
        break;
    case VASurfaceAttribUsageHint:
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_USAGE_HINT_GENERIC);
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_USAGE_HINT_DECODER);
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER);
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_USAGE_HINT_VPP_READ);
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_USAGE_HINT_VPP_WRITE);
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_USAGE_HINT_DISPLAY);
        ADD_ATTRIB_STR(VA_SURFACE_ATTRIB_USAGE_HINT_EXPORT);
        break;
    default:
        break;
    }
}
VAProfile str2Profile(const char* str)
{
    for(auto m : profile_map) {
        if (strcmp(str, m.second) == 0) {
            return m.first;
        }
    }
    return VAProfileNone;
}

VAEntrypoint str2Entrypoint(const char* str)
{
    for(auto m : entrypoint_map) {
        if (strcmp(str, m.second) == 0) {
            return m.first;
        }
    }
    return VAEntrypointVLD;
}

uint32_t str2RTFormat(const char* str)
{
    uint32_t result = 0;
    for (auto m: rtformat_map) {
        if (strcmp(str, m.first) == 0) {
            result = m.second;
            break;
        }
    }
    return result;
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
    //printf("####INFO: va_dpy = 0x%x, %d\n", va_dpy, va_dpy);

    va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);

    return uint64_t(va_dpy);
}

std::vector<const char*> getProfiles() 
{
    std::vector<const char*> profile_list;

    int max_num_profiles = vaMaxNumProfiles(va_dpy);
    //printf("####INFO: max_num_profiles = %d\n", max_num_profiles);
    if (max_num_profiles <= 0) {
        return profile_list;
    }

    int num_profiles = 0;
    std::vector<VAProfile> profile_full(max_num_profiles, VAProfileNone);
    va_status = vaQueryConfigProfiles(va_dpy, (VAProfile*)profile_full.data(), &num_profiles);
    //printf("####INFO: num_profiles = %d\n", num_profiles);
    if (va_status != VA_STATUS_SUCCESS) {
        return profile_list;
    }
    
    int profile_index = 0;
    for (auto p : profile_full) {
        if (profile_index < num_profiles) {
            profile_list.push_back(profile_map[p]);
        }
        profile_index++;
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

std::map<const char*, std::vector<const char*>> getConfigs(const char* profile_str, const char* entrypoint_str) 
{
    std::map<const char*, std::vector<const char*>> config_list;
    VAProfile profile = str2Profile(profile_str);
    VAEntrypoint entrypoint = str2Entrypoint(entrypoint_str);

    if (profile == VAProfileNone && entrypoint == VAEntrypointVideoProc) {
        VAConfigAttrib attrib = {};
        attrib.type = VAConfigAttribRTFormat;
        va_status = vaGetConfigAttributes(va_dpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1);
        if (va_status != VA_STATUS_SUCCESS) {
            return config_list; 
        }

        VAConfigID config_id = 0;
        va_status = vaCreateConfig(va_dpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1, &config_id);
        if (va_status != VA_STATUS_SUCCESS) {
            return config_list; 
        }

        uint32_t num_surf_attribs = 0;
        va_status = vaQuerySurfaceAttributes(va_dpy, config_id, nullptr, &num_surf_attribs);
        if (va_status != VA_STATUS_SUCCESS) {
            return config_list; 
        }

        std::vector<VASurfaceAttrib> surf_attribs(num_surf_attribs);
        va_status = vaQuerySurfaceAttributes(va_dpy, config_id, surf_attribs.data(), &num_surf_attribs);
        if (va_status != VA_STATUS_SUCCESS) {
            return config_list; 
        }

        for (auto a: surf_attribs) {
            auto ret = config_list.insert(
                std::pair<const char*, std::vector<const char*>>(
                    surfaceattrib_map[a.type], std::vector<const char*>()
                )
            );
            parseSurfaceAttrib(a, ret.first->second);
            //printf("####INFO: type = %d, flags = 0x%08x, value = 0x%0x8\n", a.type, a.flags, a.value.value.i);
        }
    } else {
        int max_num_attributes = vaMaxNumConfigAttributes(va_dpy);
        std::vector<VAConfigAttrib> attrib_list(max_num_attributes);
        for (size_t i = 0; i < max_num_attributes; i++) {
            attrib_list[i].type = (VAConfigAttribType)i;
        }
    
        va_status = vaGetConfigAttributes(va_dpy, profile, entrypoint, attrib_list.data(), max_num_attributes);
        if (VA_STATUS_ERROR_UNSUPPORTED_PROFILE == va_status || 
            VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT == va_status ) {
            return config_list;
        }
    
        for (auto a: attrib_list) {
            if (a.value != VA_ATTRIB_NOT_SUPPORTED) {
                config_list[configattrib_map[a.type]] = parseConfig(a.type, a.value);
                //printf("####INFO: type = %-40s, value = 0x%08x\n", configattrib_map[a.type], a.value);
            }
        }
    }
    
    return config_list;
}

std::vector<const char*> getRTFormat()
{
    std::vector<const char*> rtformat_list;
    for (auto m: rtformat_map) {
        rtformat_list.push_back(m.first);
    }
    return rtformat_list;
}

uint32_t createSurface(uint32_t width, uint32_t height, const char* format, uint32_t numSurf=1)
{
    VASurfaceID surfID = VA_INVALID_ID;
    uint32_t formatRT  = str2RTFormat(format); // VA_RT_FORMAT_YUV420;
    if (formatRT == 0) {
        printf("ERROR: Invalid RT format %s \n", format);
        return VA_INVALID_ID;
    }
    
    // uint32_t fourcc  = VA_FOURCC('N','V','1','2');
    // VASurfaceAttrib surf_attrib = {};
    // surf_attrib.type =  VASurfaceAttribPixelFormat;
    // surf_attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;
    // surf_attrib.value.type = VAGenericValueTypeInteger;
    // surf_attrib.value.value.i = fourcc;
    va_status = vaCreateSurfaces(va_dpy, formatRT, width, height, &surfID, numSurf, nullptr, 0);
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaCreateSurfaces failed\n");
        return VA_INVALID_ID; 
    }
    return surfID;
}

void destorySurface(VASurfaceID surfID, uint32_t numSurf=1)
{
    vaDestroySurfaces(va_dpy, &surfID, numSurf);
}

std::map<const char*, uint64_t> querySurfaceInfo(VASurfaceID surfID)
{
    std::map<const char*, uint64_t> surfInfo;
    VAImage img = {};
    va_status = vaDeriveImage(va_dpy, surfID, &img);
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaCreateSurfaces failed\n");
        return surfInfo; 
    }

    surfInfo["image_id"] = img.image_id;
    surfInfo["buf_id"] = img.buf;
    surfInfo["fourcc"] = img.format.fourcc;
    surfInfo["byte_order"] = img.format.byte_order;
    surfInfo["bits_per_pixel"] = img.format.bits_per_pixel;
    surfInfo["depth"] = img.format.depth;
    surfInfo["red_mask"] = img.format.red_mask;
    surfInfo["green_mask"] = img.format.green_mask;
    surfInfo["blue_mask"] = img.format.blue_mask;
    surfInfo["alpha_mask"] = img.format.alpha_mask;
    surfInfo["width"] = img.width;
    surfInfo["height"] = img.height;
    surfInfo["data_size"] = img.data_size;
    surfInfo["num_planes"] = img.num_planes;
    surfInfo["pitch[0]"] = img.pitches[0];
    surfInfo["pitch[1]"] = img.pitches[1];
    surfInfo["pitch[2]"] = img.pitches[2];
    surfInfo["offset[0]"] = img.offsets[0];
    surfInfo["offset[1]"] = img.offsets[1];
    surfInfo["offset[2]"] = img.offsets[2];
    surfInfo["num_palette_entries"] = img.num_palette_entries;
    surfInfo["entry_bytes"] = img.entry_bytes;
    surfInfo["order[0]"] = img.component_order[0];
    surfInfo["order[1]"] = img.component_order[1];
    surfInfo["order[2]"] = img.component_order[2];
    surfInfo["order[3]"] = img.component_order[3];
    vaDestroyImage(va_dpy, img.image_id);
    return surfInfo;
}

uint32_t createContext(int dst_width, int dst_height, VASurfaceID dst_surf)
{
    VAConfigAttrib attrib = {};
    attrib.type = VAConfigAttribRTFormat;
    va_status = vaGetConfigAttributes(va_dpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1);
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaGetConfigAttributes failed\n");
        return VA_INVALID_ID; 
    }

    VAConfigID config_id = 0;
    va_status = vaCreateConfig(va_dpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1, &config_id);
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaCreateConfig failed\n");
        return VA_INVALID_ID; 
    }

    VAContextID ctx_id = 0;
    va_status = vaCreateContext(va_dpy, config_id, dst_width, dst_height, VA_PROGRESSIVE, &dst_surf, 1, &ctx_id);
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaCreateContext failed\n");
        return VA_INVALID_ID; 
    }

    return ctx_id;
}

void destroyContext(VAContextID ctx_id)
{
    vaDestroyContext(va_dpy, ctx_id);
}

int vppExecute(VAContextID ctx_id, VASurfaceID src_surf, VASurfaceID dst_surf)
{
    std::map<const char*, uint64_t> srcmap = querySurfaceInfo(src_surf);
    std::map<const char*, uint64_t> dstmap = querySurfaceInfo(dst_surf);
    uint16_t srcw = (uint16_t)srcmap["width"];
    uint16_t srch = (uint16_t)srcmap["height"];
    uint16_t dstw = (uint16_t)dstmap["width"];
    uint16_t dsth = (uint16_t)dstmap["height"];

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
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaCreateBuffer failed\n");
        return VA_INVALID_ID; 
    }

    va_status = vaBeginPicture(va_dpy, ctx_id, dst_surf);
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaBeginPicture failed\n");
        return VA_INVALID_ID; 
    }

    va_status = vaRenderPicture(va_dpy, ctx_id, &pipeline_buf_id, 1);
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaRenderPicture failed\n");
        return VA_INVALID_ID; 
    }

    va_status = vaEndPicture(va_dpy, ctx_id);
    if (va_status != VA_STATUS_SUCCESS) {
        printf("ERROR: vaEndPicture failed\n");
        return VA_INVALID_ID; 
    }

    vaDestroyBuffer(va_dpy, pipeline_buf_id);

    return va_status;
}

PYBIND11_MODULE(pylibva, m) {
    m.doc() = "libva python bindings"; // optional module docstring
    m.def("init", &vaInit, "Initialize VADisplay");
    m.def("close", &vaClose, "Close VADisplay and drm fd");

    m.def("profiles", &getProfiles, "Query supported profiles");
    m.def("entrypoints", &getEntrypoints, "Query supported entrypoints for a given profile");
    m.def("configs", &getConfigs, "Get attributes for a given profile/entrypoint pair");

    m.def("get_rtformat", &getRTFormat, "Get RT format list");
    m.def("query_info", &querySurfaceInfo, "Destroy VASurface");

    m.def("create_surface", &createSurface, "Create VASurface");
    m.def("destroy_surface", &destorySurface, "Destroy VASurface");

    m.def("create_context", &createContext, "Create VAContextID");
    m.def("destroy_context", &destroyContext, "Destroy VAContextID");

    m.def("vpp_execute", &vppExecute, "Execute VPP");
}

