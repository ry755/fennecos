/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include <kernel/ide.h>
#include <kernel/ramdisk.h>

/* Definitions of physical drive number for each drive */
#define DEV_RAM 0
#define DEV_IDE 1

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv) {
    case DEV_RAM:
        return RES_OK;

    case DEV_IDE:
        return RES_OK;
    }

    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv) {
    case DEV_RAM:
        return RES_OK;

    case DEV_IDE:
        return RES_OK;
    }

    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    LBA_t sector,	/* Start sector in LBA */
    UINT count		/* Number of sectors to read */
)
{
    switch (pdrv) {
    case DEV_RAM:
        if (!buff)
            return RES_PARERR;

        for (; count > 0; count--) {
            read_ramdisk_sector(buff, sector++);
            buff += 512;
        }

        return RES_OK;

    case DEV_IDE:
        if (!buff)
            return RES_PARERR;

        for (; count > 0; count--) {
            read_sector(buff, sector++);
            buff += 512;
        }

        return RES_OK;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    LBA_t sector,		/* Start sector in LBA */
    UINT count			/* Number of sectors to write */
)
{
    switch (pdrv) {
    case DEV_RAM:
        if (!buff)
            return RES_PARERR;

        for (; count > 0; count--) {
            write_ramdisk_sector(buff, sector++);
            buff += 512;
        }

        return RES_OK;

    case DEV_IDE:
        if (!buff)
            return RES_PARERR;

        for (; count > 0; count--) {
            write_sector(buff, sector++);
            buff += 512;
        }

        return RES_OK;
    }

    return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    switch (pdrv) {
    case DEV_RAM:
        return ramdisk_ioctl(cmd, buff);

    case DEV_IDE:
        return ide_ioctl(cmd, buff);
    }

    return RES_PARERR;
}
