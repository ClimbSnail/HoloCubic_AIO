#include "ff.h"

FRESULT f_open(FIL *fp, const char *path, BYTE mode)
{
    (void)fp; (void)path; (void)mode;
    return FR_OK;
}

FRESULT f_close(FIL *fp)
{
    (void)fp;
    return FR_OK;
}

FRESULT f_read(FIL *fp, void *buff, UINT btr, UINT *br)
{
    (void)fp; (void)buff;
    if (br) *br = 0;
    return FR_OK;
}

FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw)
{
    (void)fp; (void)buff;
    if (bw) *bw = btw;
    return FR_OK;
}

FRESULT f_lseek(FIL *fp, FSIZE_t ofs)
{
    (void)fp; (void)ofs;
    return FR_OK;
}

FSIZE_t f_tell(FIL *fp)
{
    (void)fp;
    return 0;
}

FSIZE_t f_size(FIL *fp)
{
    (void)fp;
    return 0;
}

FRESULT f_opendir(DIR *dp, const char *path)
{
    (void)dp; (void)path;
    return FR_OK;
}

FRESULT f_readdir(DIR *dp, FILINFO *fno)
{
    (void)dp; (void)fno;
    return FR_OK;
}

FRESULT f_closedir(DIR *dp)
{
    (void)dp;
    return FR_OK;
}