#include "lvgl.h"
#include "sim_data_path.h"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

static char g_sdBasePath[512] = "";

static void computeSdBasePath()
{
    static bool computed = false;
    if (computed) return;
    computed = true;
    snprintf(g_sdBasePath, sizeof(g_sdBasePath), "%s", sim_data_get_sd_path());
}

static void toNativePath(const char *lvPath, char *nativePath, size_t size)
{
    computeSdBasePath();
    if (lvPath[0] == 'S' && lvPath[1] == ':')
        lvPath += 2;
    if (lvPath[0] == '/' || lvPath[0] == '\\')
        lvPath += 1;
    snprintf(nativePath, size, "%s\\%s", g_sdBasePath, lvPath);
    for (char *p = nativePath; *p; ++p)
        if (*p == '/') *p = '\\';
}

struct NativeFile {
    FILE *fp;
};

struct NativeDir {
    intptr_t handle;
    bool first;
    struct _finddata_t data;
};

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    LV_UNUSED(drv);
    char nativePath[512];
    toNativePath(path, nativePath, sizeof(nativePath));

    const char *modeStr;
    if (mode == LV_FS_MODE_WR) {
        modeStr = "wb";
    } else if (mode == LV_FS_MODE_RD) {
        modeStr = "rb";
    } else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) {
        modeStr = "rb+";
    } else {
        return NULL;
    }

    FILE *fp = fopen(nativePath, modeStr);
    if (!fp) {
        return NULL;
    }

    NativeFile *nf = (NativeFile *)lv_mem_alloc(sizeof(NativeFile));
    if (!nf) {
        fclose(fp);
        return NULL;
    }
    nf->fp = fp;
    return nf;
}

static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p)
{
    LV_UNUSED(drv);
    NativeFile *nf = (NativeFile *)file_p;
    if (nf && nf->fp) {
        fclose(nf->fp);
    }
    lv_mem_free(file_p);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    LV_UNUSED(drv);
    NativeFile *nf = (NativeFile *)file_p;
    size_t n = fread(buf, 1, btr, nf->fp);
    if (br) *br = (uint32_t)n;
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    LV_UNUSED(drv);
    NativeFile *nf = (NativeFile *)file_p;
    size_t n = fwrite(buf, 1, btw, nf->fp);
    if (bw) *bw = (uint32_t)n;
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    LV_UNUSED(drv);
    NativeFile *nf = (NativeFile *)file_p;
    int origin;
    switch (whence) {
        case LV_FS_SEEK_SET: origin = SEEK_SET; break;
        case LV_FS_SEEK_CUR: origin = SEEK_CUR; break;
        case LV_FS_SEEK_END: origin = SEEK_END; break;
        default: return LV_FS_RES_UNKNOWN;
    }
    fseek(nf->fp, (long)pos, origin);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    LV_UNUSED(drv);
    NativeFile *nf = (NativeFile *)file_p;
    if (pos_p) *pos_p = (uint32_t)ftell(nf->fp);
    return LV_FS_RES_OK;
}

static void *fs_dir_open(lv_fs_drv_t *drv, const char *path)
{
    LV_UNUSED(drv);
    char nativePath[512];
    toNativePath(path, nativePath, sizeof(nativePath));

    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*", nativePath);

    NativeDir *nd = (NativeDir *)lv_mem_alloc(sizeof(NativeDir));
    if (!nd) return NULL;

    nd->handle = _findfirst(pattern, &nd->data);
    if (nd->handle == -1) {
        lv_mem_free(nd);
        return NULL;
    }
    nd->first = true;
    return nd;
}

static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *dir_p, char *fn)
{
    LV_UNUSED(drv);
    NativeDir *nd = (NativeDir *)dir_p;
    if (!nd) return LV_FS_RES_UNKNOWN;

    if (nd->first) {
        nd->first = false;
    } else {
        if (_findnext(nd->handle, &nd->data) != 0) {
            fn[0] = '\0';
            return LV_FS_RES_OK;
        }
    }

    strncpy(fn, nd->data.name, 255);
    fn[255] = '\0';

    if (nd->data.attrib & _A_SUBDIR) {
        size_t len = strlen(fn);
        if (len + 1 < 256) {
            fn[len] = '/';
            fn[len + 1] = '\0';
        }
    }

    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *dir_p)
{
    LV_UNUSED(drv);
    NativeDir *nd = (NativeDir *)dir_p;
    if (nd) {
        _findclose(nd->handle);
        lv_mem_free(nd);
    }
    return LV_FS_RES_OK;
}

extern "C" void lv_fs_fatfs_init(void)
{
    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    fs_drv.letter = 'S';
    fs_drv.cache_size = 0;

    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;
    fs_drv.dir_open_cb = fs_dir_open;
    fs_drv.dir_read_cb = fs_dir_read;
    fs_drv.dir_close_cb = fs_dir_close;

    lv_fs_drv_register(&fs_drv);
}