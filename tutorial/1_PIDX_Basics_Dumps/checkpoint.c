/*****************************************************
 **  PIDX Parallel I/O Library                      **
 **  Copyright (c) 2010-2014 University of Utah     **
 **  Scientific Computing and Imaging Institute     **
 **  72 S Central Campus Drive, Room 3750           **
 **  Salt Lake City, UT 84112                       **
 **                                                 **
 **  PIDX is licensed under the Creative Commons    **
 **  Attribution-NonCommercial-NoDerivatives 4.0    **
 **  International License. See LICENSE.md.         **
 **                                                 **
 **  For information about this project see:        **
 **  http://www.cedmav.com/pidx                     **
 **  or contact: pascucci@sci.utah.edu              **
 **  For support: PIDX-support@visus.net            **
 **                                                 **
 *****************************************************/

/*

             *---------*--------*
           /         /         /| P7
          *---------*---------* |
         /         /         /| |
        *---------*---------* | *
        |         |         | |/|           --------->        IDX Format
        |         |         | * |
        | P4      | P5      |/| | P3
        *---------*---------* | *
        |         |         | |/
        |         |         | *
        | P0      | P1      |/
        *---------*---------*

*/

#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <PIDX.h>

enum { X, Y, Z, NUM_DIMS };
static int process_count = 1, rank = 0;
static unsigned long long global_box_size[3] = {162, 162, 42};
static unsigned long long local_box_offset[3];
static unsigned long long local_box_size[3] = {20, 20, 20};
static int time_step_count = 1;
static int variable_count = 1;
static char output_file_template[512] = "test";
static double **data;
static char output_file_name[512] = "test.idx";
static int values_per_sample = 1;
static char *usage = "Serial Usage: ./checkpoint -g 32x32x32 -l 32x32x32 -v 3 -t 16 -f output_idx_file_name\n"
                     "Parallel Usage: mpirun -n 8 ./checkpoint -g 32x32x32 -l 16x16x16 -f output_idx_file_name -v 3 -t 16\n"
                     "  -g: global dimensions\n"
                     "  -l: local (per-process) dimensions\n"
                     "  -f: IDX filename\n"
                     "  -t: number of timesteps\n"
                     "  -v: number of variables\n";


//----------------------------------------------------------------
static void terminate()
{
#if PIDX_HAVE_MPI
  MPI_Abort(MPI_COMM_WORLD, -1);
#else
  exit(-1);
#endif
}

//----------------------------------------------------------------
static void terminate_with_error_msg(const char *format, ...)
{
  va_list arg_ptr;
  va_start(arg_ptr, format);
  vfprintf(stderr, format, arg_ptr);
  va_end(arg_ptr);
  terminate();
}

//----------------------------------------------------------------
/*
static void rank_0_print(const char *format, ...)
{
  if (rank != 0) return;
  va_list arg_ptr;
  va_start(arg_ptr, format);
  vfprintf(stderr, format, arg_ptr);
  va_end(arg_ptr);
}
*/

//----------------------------------------------------------------
static void init_mpi(int argc, char **argv)
{
#if PIDX_HAVE_MPI
  if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    terminate_with_error_msg("ERROR: MPI_Init error\n");
  if (MPI_Comm_size(MPI_COMM_WORLD, &process_count) != MPI_SUCCESS)
    terminate_with_error_msg("ERROR: MPI_Comm_size error\n");
  if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS)
    terminate_with_error_msg("ERROR: MPI_Comm_rank error\n");
#endif
}

//----------------------------------------------------------------
static void shutdown_mpi()
{
#if PIDX_HAVE_MPI
  MPI_Finalize();
#endif
}

static void calculate_per_process_offsets()
{
/*
    if (rank == 0) { local_box_offset[0] = 0; local_box_offset[1] = 0; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 4) { local_box_offset[0] = 21; local_box_offset[1] = 0; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 15) { local_box_offset[0] = 40; local_box_offset[1] = 61; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 12) { local_box_offset[0] = 40; local_box_offset[1] = 41; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 20) { local_box_offset[0] = 61; local_box_offset[1] = 41; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 5) { local_box_offset[0] = 21; local_box_offset[1] = 0; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 36) { local_box_offset[0] = 101; local_box_offset[1] = 0; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 40) { local_box_offset[0] = 101; local_box_offset[1] = 41; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 13) { local_box_offset[0] = 40; local_box_offset[1] = 41; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 44) { local_box_offset[0] = 101; local_box_offset[1] = 81; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 7) { local_box_offset[0] = 21; local_box_offset[1] = 21; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 55) { local_box_offset[0] = 121; local_box_offset[1] = 61; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 52) { local_box_offset[0] = 121; local_box_offset[1] = 41; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 27) { local_box_offset[0] = 81; local_box_offset[1] = 21; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 60) { local_box_offset[0] = 120; local_box_offset[1] = 121; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 30) { local_box_offset[0] = 81; local_box_offset[1] = 61; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 45) { local_box_offset[0] = 101; local_box_offset[1] = 81; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 34) { local_box_offset[0] = 80; local_box_offset[1] = 101; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 76) { local_box_offset[0] = 141; local_box_offset[1] = 121; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 50) { local_box_offset[0] = 121; local_box_offset[1] = 21; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 24) { local_box_offset[0] = 81; local_box_offset[1] = 0; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 2) { local_box_offset[0] = 0; local_box_offset[1] = 21; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 53) { local_box_offset[0] = 121; local_box_offset[1] = 41; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 47) { local_box_offset[0] = 101; local_box_offset[1] = 101; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 11) { local_box_offset[0] = 41; local_box_offset[1] = 21; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 9) { local_box_offset[0] = 41; local_box_offset[1] = 0; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 8) { local_box_offset[0] = 41; local_box_offset[1] = 0; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 14) { local_box_offset[0] = 40; local_box_offset[1] = 61; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 31) { local_box_offset[0] = 81; local_box_offset[1] = 61; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 17) { local_box_offset[0] = 61; local_box_offset[1] = 0; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 67) { local_box_offset[0] = 141; local_box_offset[1] = 21; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 6) { local_box_offset[0] = 21; local_box_offset[1] = 21; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 74) { local_box_offset[0] = 141; local_box_offset[1] = 101; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 16) { local_box_offset[0] = 61; local_box_offset[1] = 0; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 10) { local_box_offset[0] = 41; local_box_offset[1] = 21; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 37) { local_box_offset[0] = 101; local_box_offset[1] = 0; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 18) { local_box_offset[0] = 61; local_box_offset[1] = 21; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 33) { local_box_offset[0] = 80; local_box_offset[1] = 81; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 32) { local_box_offset[0] = 80; local_box_offset[1] = 81; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 26) { local_box_offset[0] = 81; local_box_offset[1] = 21; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 64) { local_box_offset[0] = 141; local_box_offset[1] = 0; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 25) { local_box_offset[0] = 81; local_box_offset[1] = 0; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 28) { local_box_offset[0] = 81; local_box_offset[1] = 41; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 38) { local_box_offset[0] = 101; local_box_offset[1] = 21; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 21) { local_box_offset[0] = 61; local_box_offset[1] = 41; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 42) { local_box_offset[0] = 101; local_box_offset[1] = 61; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 22) { local_box_offset[0] = 61; local_box_offset[1] = 61; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 51) { local_box_offset[0] = 121; local_box_offset[1] = 21; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 49) { local_box_offset[0] = 121; local_box_offset[1] = 0; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 39) { local_box_offset[0] = 101; local_box_offset[1] = 21; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 48) { local_box_offset[0] = 121; local_box_offset[1] = 0; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 54) { local_box_offset[0] = 121; local_box_offset[1] = 61; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 19) { local_box_offset[0] = 61; local_box_offset[1] = 21; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 71) { local_box_offset[0] = 141; local_box_offset[1] = 61; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 57) { local_box_offset[0] = 121; local_box_offset[1] = 81; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 46) { local_box_offset[0] = 101; local_box_offset[1] = 101; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 70) { local_box_offset[0] = 141; local_box_offset[1] = 61; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 56) { local_box_offset[0] = 121; local_box_offset[1] = 81; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 1) { local_box_offset[0] = 0; local_box_offset[1] = 0; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 77) { local_box_offset[0] = 141; local_box_offset[1] = 121; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 58) { local_box_offset[0] = 121; local_box_offset[1] = 101; local_box_offset[2] = 0; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 73) { local_box_offset[0] = 141; local_box_offset[1] = 81; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 72) { local_box_offset[0] = 141; local_box_offset[1] = 81; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 66) { local_box_offset[0] = 141; local_box_offset[1] = 21; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 3) { local_box_offset[0] = 0; local_box_offset[1] = 21; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 65) { local_box_offset[0] = 141; local_box_offset[1] = 0; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 68) { local_box_offset[0] = 141; local_box_offset[1] = 41; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 29) { local_box_offset[0] = 81; local_box_offset[1] = 41; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 35) { local_box_offset[0] = 80; local_box_offset[1] = 101; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 23) { local_box_offset[0] = 61; local_box_offset[1] = 61; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 78) { local_box_offset[0] = 141; local_box_offset[1] = 141; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 61) { local_box_offset[0] = 120; local_box_offset[1] = 121; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 62) { local_box_offset[0] = 120; local_box_offset[1] = 141; local_box_offset[2] = 0; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 79) { local_box_offset[0] = 141; local_box_offset[1] = 141; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
    if (rank == 59) { local_box_offset[0] = 121; local_box_offset[1] = 101; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 41) { local_box_offset[0] = 101; local_box_offset[1] = 41; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 43) { local_box_offset[0] = 101; local_box_offset[1] = 61; local_box_offset[2] = 21; local_box_size[0] = 20; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 69) { local_box_offset[0] = 141; local_box_offset[1] = 41; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 75) { local_box_offset[0] = 141; local_box_offset[1] = 101; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 20; local_box_size[2] = 21; }
    if (rank == 63) { local_box_offset[0] = 120; local_box_offset[1] = 141; local_box_offset[2] = 21; local_box_size[0] = 21; local_box_size[1] = 21; local_box_size[2] = 21; }
*/

  int sub_div[NUM_DIMS];
  sub_div[X] = (global_box_size[X] / local_box_size[X]);
  sub_div[Y] = (global_box_size[Y] / local_box_size[Y]);
  sub_div[Z] = (global_box_size[Z] / local_box_size[Z]);
  local_box_offset[Z] = (rank / (sub_div[X] * sub_div[Y])) * local_box_size[Z];
  int slice = rank % (sub_div[X] * sub_div[Y]);
  local_box_offset[Y] = (slice / sub_div[X]) * local_box_size[Y];
  local_box_offset[X] = (slice % sub_div[X]) * local_box_size[X];

  /*
  if (rank % 2 == 0)
  {
    local_box_size[X] = 0;
    local_box_size[Y] = 0;
    local_box_size[Z] = 0;
  }
  */
  //if (local_box_offset[1] == 750 && local_box_offset[2] == 150)
  //  printf("rank === %d\n", rank);

  /*
  if (rank % 4 == 0)
  {
    local_box_size[0] = local_box_size[0];
    local_box_size[1] = local_box_size[1];
    local_box_size[2] = local_box_size[2];
  }
  else if (rank % 4 == 1)
  {
    local_box_offset[1] = (3 * local_box_offset[1]) / 4;
    local_box_size[1] = (3 * local_box_size[1]) / 4;
  }
  else if (rank % 4 == 2)
  {
    local_box_offset[1] = (2 * local_box_offset[1]) / 4;
    local_box_size[1] = (2 * local_box_size[1]) / 4;
  }
  else if (rank % 4 == 3)
  {
    local_box_offset[1] = (1 * local_box_offset[1]) / 4;
    local_box_size[1] = (1 * local_box_size[1]) / 4;
  }
  */

  /*
  if (rank >= 0 && rank < 16)
  {
    local_box_size[0] = local_box_size[0];
    local_box_size[1] = local_box_size[1];
    local_box_size[2] = local_box_size[2];
  }
  else if (rank >= 16 && rank < 32)
  {
    local_box_offset[1] = local_box_offset[1] / 2;
    local_box_size[1] = local_box_size[1] / 2;
  }
  else if (rank >= 32 && rank < 48)
  {
    local_box_offset[1] = local_box_offset[1] / 4;
    local_box_size[1] = local_box_size[1] / 4;
  }
  else if (rank >= 48 && rank < 64)
  {
    local_box_offset[1] = local_box_offset[1] / 8;
    local_box_size[1] = local_box_size[1] / 8;
  }
  */
}

static void create_synthetic_simulation_data()
{
  int var = 0;
  unsigned long long i, j, k, vps = 0;

  data = (double**)malloc(sizeof(*data) * variable_count);
  memset(data, 0, sizeof(*data) * variable_count);

  // Synthetic simulation data

  for(var = 0; var < variable_count; var++)
  {
    data[var] = (double*)malloc(sizeof (double) * local_box_size[0] * local_box_size[1] * local_box_size[2] * values_per_sample);
    for (k = 0; k < local_box_size[2]; k++)
      for (j = 0; j < local_box_size[1]; j++)
        for (i = 0; i < local_box_size[0]; i++)
        {
          unsigned long long index = (unsigned long long) (local_box_size[0] * local_box_size[1] * k) + (local_box_size[0] * j) + i;
          for (vps = 0; vps < values_per_sample; vps++)
            data[var][index * values_per_sample + vps] = var + ((global_box_size[0] * global_box_size[1]*(local_box_offset[2] + k))+(global_box_size[0]*(local_box_offset[1] + j)) + (local_box_offset[0] + i));
        }
  }
}

static void destroy_synthetic_simulation_data()
{
  int var = 0;
  for(var = 0; var < variable_count; var++)
  {
    free(data[var]);
    data[var] = 0;
  }
  free(data);
  data = 0;
}

///< Parse the input arguments
static void parse_args(int argc, char **argv)
{
  char flags[] = "g:l:f:t:v:";
  int one_opt = 0;

  while ((one_opt = getopt(argc, argv, flags)) != EOF)
  {
    /* postpone error checking for after while loop */
    switch (one_opt)
    {
    case('g'): // global dimension
      if ((sscanf(optarg, "%lldx%lldx%lld", &global_box_size[0], &global_box_size[1], &global_box_size[2]) == EOF) || (global_box_size[0] < 1 || global_box_size[1] < 1 || global_box_size[2] < 1))
        terminate_with_error_msg("Invalid global dimensions\n%s", usage);
      break;

    case('l'): // local dimension
      if ((sscanf(optarg, "%lldx%lldx%lld", &local_box_size[0], &local_box_size[1], &local_box_size[2]) == EOF) ||(local_box_size[0] < 1 || local_box_size[1] < 1 || local_box_size[2] < 1))
        terminate_with_error_msg("Invalid local dimension\n%s", usage);
      break;

    case('f'): // output file name
      if (sprintf(output_file_template, "%s", optarg) < 0)
        terminate_with_error_msg("Invalid output file name template\n%s", usage);
      sprintf(output_file_name, "%s%s", output_file_template, ".idx");
      break;

    case('t'): // number of timesteps
      if (sscanf(optarg, "%d", &time_step_count) < 0)
        terminate_with_error_msg("Invalid variable file\n%s", usage);
      break;

    case('v'): // number of variables
      if (sscanf(optarg, "%d", &variable_count) < 0)
        terminate_with_error_msg("Invalid variable file\n%s", usage);
      break;

    default:
      terminate_with_error_msg("Wrong arguments\n%s", usage);
    }
  }
}

static void check_args()
{
  if (global_box_size[X] < local_box_size[X] || global_box_size[Y] < local_box_size[Y] || global_box_size[Z] < local_box_size[Z])
    terminate_with_error_msg("ERROR: Global box is smaller than local box in one of the dimensions\n");

  // check if the number of processes given by the user is consistent with the actual number of processes needed
  int brick_count = (int)((global_box_size[X] + local_box_size[X] - 1) / local_box_size[X]) *
                    (int)((global_box_size[Y] + local_box_size[Y] - 1) / local_box_size[Y]) *
                    (int)((global_box_size[Z] + local_box_size[Z] - 1) / local_box_size[Z]);
  if(brick_count != process_count)
    terminate_with_error_msg("ERROR: Number of sub-blocks (%d) doesn't match number of processes (%d)\n", brick_count, process_count);

}

int main(int argc, char **argv)
{
  init_mpi(argc, argv);
  parse_args(argc, argv);
  //check_args();
  calculate_per_process_offsets();
  create_synthetic_simulation_data();

  //rank_0_print("Simulation Data Created\n");

  int ret;
  int var;
  int ts;
  PIDX_file file;            // IDX file descriptor
  PIDX_variable* variable;   // variable descriptor

  variable = (PIDX_variable*)malloc(sizeof(*variable) * variable_count);
  memset(variable, 0, sizeof(*variable) * variable_count);

  PIDX_point global_size, local_offset, local_size;
  PIDX_set_point_5D(global_size, global_box_size[0], global_box_size[1], global_box_size[2], 1, 1);
  PIDX_set_point_5D(local_offset, local_box_offset[0], local_box_offset[1], local_box_offset[2], 0, 0);
  PIDX_set_point_5D(local_size, local_box_size[0], local_box_size[1], local_box_size[2], 1, 1);

  //  Creating access
  PIDX_access access;
  PIDX_create_access(&access);
#if PIDX_HAVE_MPI
  PIDX_set_mpi_access(access, MPI_COMM_WORLD);
#endif

  for (ts = 0; ts < time_step_count; ts++)
  {
    //  PIDX mandatory calls
    ret = PIDX_file_create(output_file_name, PIDX_MODE_CREATE, access, &file);
    if (ret != PIDX_success)  terminate_with_error_msg("PIDX_file_create");

    ret = PIDX_set_dims(file, global_size);
    if (ret != PIDX_success)  terminate_with_error_msg("PIDX_set_dims");
    ret = PIDX_set_current_time_step(file, 0);
    if (ret != PIDX_success)  terminate_with_error_msg("PIDX_set_current_time_step");
    ret = PIDX_set_variable_count(file, variable_count);
    if (ret != PIDX_success)  terminate_with_error_msg("PIDX_set_variable_count");

    //PIDX_debug_rst(file, 1);
    //PIDX_debug_hz(file, 1);
    //PIDX_debug_disable_hz(file);
    //PIDX_debug_disable_io(file);
    //PIDX_dump_agg_info(file, 1);
    PIDX_set_block_count(file, 256);
    PIDX_set_block_size(file, 15);
    //PIDX_set_aggregation_factor(file, 2);



    //PIDX_debug_disable_restructuring(file);
    //PIDX_debug_disable_chunking(file);
    //PIDX_debug_disable_hz(file);
    //PIDX_debug_disable_compression(file);
    //PIDX_debug_disable_agg(file);
    //PIDX_debug_disable_io(file);

    //PIDX_activate_local_aggregation(file);

    //PIDX_set_variable_pile_length(file, 0);
    //int64_t restructured_box_size[5] = {64, 64, 64, 1, 1};
    //ret = PIDX_set_restructuring_box(file, restructured_box_size);
    //if (ret != PIDX_success)  terminate_with_error_msg("PIDX_set_restructuring_box");

    //PIDX_set_compression_type(file, PIDX_CHUNKING_ONLY);

    //PIDX_set_compression_type(file, PIDX_CHUNKING_ZFP);
    //PIDX_set_lossy_compression_bit_rate(file, 8);

    char var_name[512];
    for (var = 0; var < variable_count; var++)
    {
      sprintf(var_name, "variable_%d", var);

      ret = PIDX_variable_create(var_name,  values_per_sample * sizeof(double) * 8, FLOAT64 , &variable[var]);
      if (ret != PIDX_success)  terminate_with_error_msg("PIDX_variable_create");

      ret = PIDX_variable_write_data_layout(variable[var], local_offset, local_size, data[var], PIDX_row_major);
      if (ret != PIDX_success)  terminate_with_error_msg("PIDX_variable_data_layout");

      ret = PIDX_append_and_write_variable(file, variable[var]);
      if (ret != PIDX_success)  terminate_with_error_msg("PIDX_append_and_write_variable");
    }

    ret = PIDX_close(file);
    if (ret != PIDX_success)  terminate_with_error_msg("PIDX_close");
  }

  ret = PIDX_close_access(access);
  if (ret != PIDX_success)  terminate_with_error_msg("PIDX_close_access");

  free(variable);
  variable = 0;

  destroy_synthetic_simulation_data();
  shutdown_mpi();

  return 0;
}