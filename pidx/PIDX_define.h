#ifndef __PIDX_DEFINE_H
#define __PIDX_DEFINE_H


#ifdef __cplusplus
extern "C" {
#endif

#define PIDX_MAX_DIMENSIONS 3

#define PIDX_HAVE_MPI 1
#define PIDX_HAVE_ZFP 0
#define PIDX_HAVE_PNETCDF 0
#define PIDX_HAVE_NETCDF 0
#define PIDX_HAVE_HDF5 0
#define PIDX_HAVE_NVISUSIO 0

#define PIDX_MAX_TEMPLATE_DEPTH 6

#ifndef __cplusplus
#  define _XOPEN_SOURCE 600
#endif

#ifdef BGQ
  #define _XOPEN_SOURCE 600
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#endif


enum IO_READ_WRITE {PIDX_READ, PIDX_WRITE};

#define PIDX_NO_COMPRESSION 0
#define PIDX_CHUNKING_ONLY 1
#define PIDX_CHUNKING_ZFP 2
#define PIDX_CHUNKING_ZFP_ACCURACY 3

#define PIDX_row_major                           0
#define PIDX_column_major                        1

#define PIDX_RAW_IO                                   1
#define PIDX_IDX_IO                                   2
#define PIDX_LOCAL_PARTITION_IDX_IO                   3
#define PIDX_GLOBAL_PARTITION_IDX_IO                  4

#define PIDX_default_bits_per_block              15
#define PIDX_default_blocks_per_file             256


/// Create the file if it does not exist.
#define PIDX_MODE_CREATE              1

/// Error creating a file that already exists.
#define PIDX_MODE_EXCL               64

#define PIDX_MODE_RDONLY              2  /* ADIO_RDONLY */
#define PIDX_MODE_WRONLY              4  /* ADIO_WRONLY  */
#define PIDX_MODE_RDWR                8  /* ADIO_RDWR  */
#define PIDX_MODE_DELETE_ON_CLOSE    16  /* ADIO_DELETE_ON_CLOSE */
#define PIDX_MODE_UNIQUE_OPEN        32  /* ADIO_UNIQUE_OPEN */

#define PIDX_MODE_APPEND            128  /* ADIO_APPEND */
#define PIDX_MODE_SEQUENTIAL        256  /* ADIO_SEQUENTIAL */


/// IDX specifies generic types using simple strings consisting of an unambiguous data type and
/// C array syntax, e.g. "float32[3]".  In the PIDX library, we declare types using strings so
/// users can easily override the provided defaults.

typedef char PIDX_data_type[512];

// PLEASE NOTE: these are example types, not a complete list of possible IDX types

extern PIDX_data_type INT8;
extern PIDX_data_type INT8_GA;
extern PIDX_data_type INT8_RGB;
extern PIDX_data_type INT8_RGBA;

extern PIDX_data_type UINT8;
extern PIDX_data_type UINT8_GA;
extern PIDX_data_type UINT8_RGB;
extern PIDX_data_type UINT8_RGBA;

extern PIDX_data_type INT16;
extern PIDX_data_type INT16_GA;
extern PIDX_data_type INT16_RGB;
extern PIDX_data_type INT16_RGBA;

extern PIDX_data_type UINT16;
extern PIDX_data_type UINT16_GA;
extern PIDX_data_type UINT16_RGB;
extern PIDX_data_type UINT16_RGBA;

extern PIDX_data_type INT32;
extern PIDX_data_type INT32_GA;
extern PIDX_data_type INT32_RGB;
extern PIDX_data_type INT32_RGBA;

extern PIDX_data_type UINT32;
extern PIDX_data_type UINT32_GA;
extern PIDX_data_type UINT32_RGB;
extern PIDX_data_type UINT32_RGBA;

extern PIDX_data_type INT64;
extern PIDX_data_type INT64_GA;
extern PIDX_data_type INT64_RGB;
extern PIDX_data_type INT64_RGBA;

extern PIDX_data_type UINT64;
extern PIDX_data_type UINT64_GA;
extern PIDX_data_type UINT64_RGB;
extern PIDX_data_type UINT64_RGBA;

extern PIDX_data_type FLOAT32;
extern PIDX_data_type FLOAT32_GA;
extern PIDX_data_type FLOAT32_RGB;
extern PIDX_data_type FLOAT32_RGBA;

extern PIDX_data_type FLOAT64;
extern PIDX_data_type FLOAT64_GA;
extern PIDX_data_type FLOAT64_RGB;
extern PIDX_data_type FLOAT64_RGBA;

#ifdef __cplusplus
}
#endif

#endif