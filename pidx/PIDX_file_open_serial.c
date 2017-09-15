#include "PIDX_file_handler.h"

/// Function to get file descriptor when opening an existing IDX file
PIDX_return_code PIDX_file_open(const char* filename, PIDX_flags flags, PIDX_access access_type, PIDX_point dims, PIDX_file* file)
{
  int i;
  char file_name_skeleton[1024];

  if (strncmp(".idx", &filename[strlen(filename) - 4], 4) != 0 && !filename)
    return PIDX_err_name;

  *file = malloc(sizeof (*(*file)) );
  memset(*file, 0, sizeof (*(*file)) );

  (*file)->flags = flags;

  (*file)->idx = (idx_dataset)malloc(sizeof (*((*file)->idx)));
  memset((*file)->idx, 0, sizeof (*((*file)->idx)));

  (*file)->idx_c = malloc(sizeof (*((*file)->idx_c)));
  memset((*file)->idx_c, 0, sizeof (*((*file)->idx_c)));

  (*file)->idx_d = (idx_dataset_derived_metadata)malloc(sizeof (*((*file)->idx_d)));
  memset((*file)->idx_d, 0, sizeof (*((*file)->idx_d)));

  (*file)->idx_dbg = malloc(sizeof (*((*file)->idx_dbg)));
  memset((*file)->idx_dbg, 0, sizeof (*((*file)->idx_dbg)));

  (*file)->idx_d->time = malloc(sizeof (*((*file)->idx_d->time)));
  memset((*file)->idx_d->time, 0, sizeof (*((*file)->idx_d->time)));
  (*file)->idx_d->time->sim_start = PIDX_get_time();

  (*file)->idx_c->global_comm = access_type->comm;
  (*file)->idx_c->local_comm = access_type->comm;
  // MPI_Comm_rank((*file)->idx_c->global_comm, &((*file)->idx_c->grank));
  // MPI_Comm_size((*file)->idx_c->global_comm, &((*file)->idx_c->gnprocs));
  // MPI_Comm_rank((*file)->idx_c->local_comm, &((*file)->idx_c->lrank));
  // MPI_Comm_size((*file)->idx_c->local_comm, &((*file)->idx_c->lnprocs));

  for (i = 0; i < PIDX_MAX_DIMENSIONS; i++)
  {
    (*file)->idx_d->partition_count[i] = 1;
    (*file)->idx_d->partition_offset[i] = 0;
  }


  (*file)->idx_dbg->debug_do_rst = 1;
  (*file)->idx_dbg->debug_do_chunk = 1;
  (*file)->idx_dbg->debug_do_compress = 1;
  (*file)->idx_dbg->debug_do_hz = 1;
  (*file)->idx_dbg->debug_do_agg = 1;
  (*file)->idx_dbg->debug_do_io = 1;
  (*file)->idx_dbg->debug_rst = 0;
  (*file)->idx_dbg->debug_hz = 0;

  (*file)->local_group_index = 0;
  (*file)->local_group_count = 0;
  (*file)->flush_used = 0;
  (*file)->write_on_close = 0;
  //(*file)->one_time_initializations = 0;

  (*file)->idx->zfp_precisison = 0;

  (*file)->idx_d->reduced_res_from = 0;
  (*file)->idx_d->reduced_res_to = 0;

  (*file)->idx_d->raw_io_pipe_length = 0;

  (*file)->idx_d->color = 0;
  (*file)->idx->agg_counter = 0;

  (*file)->idx->io_type = PIDX_IDX_IO;
  //(*file)->enable_raw_dump = 0;

  (*file)->idx_d->parallel_mode = access_type->parallel;
  (*file)->idx->shared_face = 0;

  (*file)->idx->current_time_step = 0;
  (*file)->idx->variable_count = -1;
  (*file)->idx->group_index_tracker = 0;
  (*file)->local_group_count = 1;
  (*file)->idx->enable_rst = 1;
  (*file)->idx->enable_agg = 1;
  (*file)->idx->compression_type = PIDX_NO_COMPRESSION;

  strncpy(file_name_skeleton, filename, strlen(filename) - 4);
  file_name_skeleton[strlen(filename) - 4] = '\0';

  sprintf((*file)->idx->filename, "%s.idx", file_name_skeleton);
  sprintf((*file)->idx->filename_global, "%s.idx", file_name_skeleton);
  sprintf((*file)->idx->filename_partition, "%s.idx", file_name_skeleton);
  sprintf((*file)->idx->filename_file_zero, "%s.idx", file_name_skeleton);

#if 0
  if ((*file)->idx_d->partition_count[0] == 1 && (*file)->idx_d->partition_count[1] == 1 && (*file)->idx_d->partition_count[2] == 1)
    sprintf((*file)->idx->filename, "%s.idx", file_name_skeleton);
  else
    sprintf((*file)->idx->filename, "%s_%d.idx", file_name_skeleton, (*file)->idx_d->color);
#endif


  (*file)->idx->bits_per_block = PIDX_default_bits_per_block;
  (*file)->idx->blocks_per_file = PIDX_default_blocks_per_file;

  (*file)->idx_d->wavelet_levels = 0;
  (*file)->idx_d->wavelet_imeplementation_type = -1;//WAVELET_STENCIL;

  //initialize logic_to_physic transform to identity
  (*file)->idx->transform[0]  = 1.0;
  (*file)->idx->transform[5]  = 1.0;
  (*file)->idx->transform[10] = 1.0;
  (*file)->idx->transform[15] = 1.0;

  memset((*file)->idx->bitPattern, 0, 512);
  memset((*file)->idx->bitSequence, 0, 512);
  memset((*file)->idx->reg_patch_size, 0, sizeof(unsigned long long) * PIDX_MAX_DIMENSIONS);

  (*file)->idx->compression_bit_rate = 64;
  (*file)->idx->compression_factor = 1;
  for (i=0;i<PIDX_MAX_DIMENSIONS;i++)
    (*file)->idx->chunk_size[i] = 1;

  (*file)->idx->variable_group_count = 1;
  (*file)->idx_d->dimension = 0;
  (*file)->idx_d->samples_per_block = (int)pow(2, PIDX_default_bits_per_block);
  (*file)->idx_d->maxh = 0;
  (*file)->idx_d->max_file_count = 0;
  (*file)->idx_d->fs_block_size = 0;
  (*file)->idx_d->start_fs_block = 0;
  //(*file)->idx_d->agg_buffer->agg_f = 1;
  (*file)->idx_d->data_core_count = -1;

  (*file)->idx_d->pidx_version = 1;

  (*file)->idx_dbg->state_dump = PIDX_NO_META_DATA_DUMP;

  (*file)->idx->endian = 1;

  for (i = 0; i < 16; i++)
  {
    (*file)->idx->variable_grp[i] = malloc(sizeof(*((*file)->idx->variable_grp[i])));
    memset((*file)->idx->variable_grp[i], 0, sizeof(*((*file)->idx->variable_grp[i])));
  }

  int var = 0, variable_counter = 0, count = 0, len = 0;
  char *pch, *pch1;
  char line [ 512 ];

  if ((*file)->idx_c->grank == 0)
  {
    FILE *fp = fopen((*file)->idx->filename, "r");
    if (fp == NULL)
    {
      fprintf(stderr, "Error Opening %s\n", (*file)->idx->filename);
      return PIDX_err_file;
    }

    while (fgets(line, sizeof (line), fp) != NULL)
    {
      line[strcspn(line, "\r\n")] = 0;

      if (strcmp(line, "(box)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;

        pch = strtok(line, " ");
        count = 0;
        while (pch != NULL)
        {
          if (count % 2 == 1 && count / 2 < PIDX_MAX_DIMENSIONS)
          {
            (*file)->idx->bounds[count / 2] = atoi(pch) + 1;
            (*file)->idx->box_bounds[count / 2] = atoi(pch) + 1;
          }
          count++;
          pch = strtok(NULL, " ");
        }
      }

      if (strcmp(line, "(partition size)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;

        pch = strtok(line, " ");
        count = 0;
        while (pch != NULL)
        {
          (*file)->idx_d->partition_size[count] = atoi(pch);
          count++;
          pch = strtok(NULL, " ");
        }
      }

      if (strcmp(line, "(partition count)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;

        pch = strtok(line, " ");
        count = 0;
        while (pch != NULL)
        {
          (*file)->idx_d->partition_count[count] = atoi(pch);
          count++;
          pch = strtok(NULL, " ");
        }
      }

      if (strcmp(line, "(restructure box size)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;

        pch = strtok(line, " ");
        count = 0;
        while (pch != NULL)
        {
          (*file)->idx->reg_patch_size[count] = atoi(pch);
          count++;
          pch = strtok(NULL, " ");
        }
      }

      if (strcmp(line, "(raw_dump)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;

        pch = strtok(line, " ");
        count = 0;
        while (pch != NULL)
        {
          (*file)->idx->reg_patch_size[count] = atoi(pch);
          count++;
          pch = strtok(NULL, " ");
        }

        (*file)->idx->io_type = PIDX_RAW_IO;
        (*file)->idx_d->pidx_version = 0;
        //(*file)->idx->endian = 0;
      }

      if (strcmp(line, "(cores)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        (*file)->idx_d->data_core_count = atoi(line);
      }

      if (strcmp(line, "(io mode)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        int mode = atoi(line);

        if (mode == 1)
          (*file)->idx->io_type = PIDX_IDX_IO;
        else if (mode == 2)
          (*file)->idx->io_type = PIDX_GLOBAL_PARTITION_IDX_IO;
        else if (mode == 3)
          (*file)->idx->io_type = PIDX_LOCAL_PARTITION_IDX_IO;
        else if (mode == 4)
          (*file)->idx->io_type = PIDX_RAW_IO;
      }

      if (strcmp(line, "(endian)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        (*file)->idx->endian = atoi(line);
      }

      if (strcmp(line, "(fields)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        count = 0;
        variable_counter = 0;

        while (line[0] != '(')
        {
          (*file)->idx->variable_grp[0]->variable[variable_counter] = malloc(sizeof (*((*file)->idx->variable_grp[0]->variable[variable_counter])));
          if ((*file)->idx->variable_grp[0]->variable[variable_counter] == NULL)
            return PIDX_err_file;

          memset((*file)->idx->variable_grp[0]->variable[variable_counter], 0, sizeof (*((*file)->idx->variable_grp[0]->variable[variable_counter])));

          pch1 = strtok(line, " +");
          while (pch1 != NULL)
          {
            if (count == 0)
            {
              char* temp_name = strdup(pch1);
              strcpy((*file)->idx->variable_grp[0]->variable[variable_counter]->var_name, /*strdup(pch1)*/temp_name);
              free(temp_name);
            }

            if (count == 1)
            {
              len = strlen(pch1) - 1;
              if (pch1[len] == '\n')
                pch1[len] = 0;

              strcpy((*file)->idx->variable_grp[0]->variable[variable_counter]->type_name, pch1);
              int ret;
              int bits_per_sample = 0;
              ret = PIDX_default_bits_per_datatype((*file)->idx->variable_grp[0]->variable[variable_counter]->type_name, &bits_per_sample);
              if (ret != PIDX_success)  return PIDX_err_file;

              (*file)->idx->variable_grp[0]->variable[variable_counter]->bpv = bits_per_sample;
              (*file)->idx->variable_grp[0]->variable[variable_counter]->vps = 1;
            }
            count++;
            pch1 = strtok(NULL, " +");
          }
          count = 0;

          if( fgets(line, sizeof line, fp) == NULL)
            return PIDX_err_file;
          line[strcspn(line, "\r\n")] = 0;
          variable_counter++;
        }
        (*file)->idx->variable_grp[0]->variable_count = variable_counter;
      }

      if (strcmp(line, "(bits)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        strcat((*file)->idx->bitSequence, line);
      }

      if (strcmp(line, "(bitsperblock)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        (*file)->idx->bits_per_block = atoi(line);
        (*file)->idx_d->samples_per_block = (int)pow(2, (*file)->idx->bits_per_block);
      }

      if (strcmp(line, "(compression type)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        (*file)->idx->compression_type = atoi(line);
        if ((*file)->idx->compression_type != PIDX_NO_COMPRESSION)
        {
          int i1 = 0;
          for (i1 = 0; i1 < PIDX_MAX_DIMENSIONS; i1++)
            (*file)->idx->chunk_size[i1] = 4;
        }
      }

      if (strcmp(line, "(compressed box)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;

        pch = strtok(line, " ");
        count = 0;
        while (pch != NULL)
        {
          (*file)->idx->chunk_size[count] = atoi(pch);
          count++;
          pch = strtok(NULL, " ");
        }

        //if((*file)->idx->chunk_size[0] < 0 || (*file)->idx->chunk_size[1] < 0 || (*file)->idx->chunk_size[2] < 0)
        //  return PIDX_err_box;
      }

      if (strcmp(line, "(compression bit rate)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        (*file)->idx->compression_bit_rate = atoi(line);
      }

      if (strcmp(line, "(blocksperfile)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        (*file)->idx->blocks_per_file= atoi(line);
      }

      if (strcmp(line, "(file system block size)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
        (*file)->idx_d->fs_block_size = atoi(line);
      }

      if (strcmp(line, "(filename_template)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;
        line[strcspn(line, "\r\n")] = 0;
      }

      if (strcmp(line, "(time)") == 0)
      {
        if( fgets(line, sizeof line, fp) == NULL)
          return PIDX_err_file;

        line[strcspn(line, "\r\n")] = 0;

        pch = strtok(line, " ");

        if(pch != NULL)
          (*file)->idx->first_tstep = atoi(pch);
        else
          return PIDX_err_file;

        pch = strtok(NULL, " ");

        if(pch != NULL)
          (*file)->idx->last_tstep = atoi(pch);
        else
          return PIDX_err_file;
      }
    }
    fclose(fp);
  }

  (*file)->idx->variable_count = (*file)->idx->variable_grp[0]->variable_count;

#if PIDX_HAVE_MPI
  if ((*file)->idx_d->parallel_mode == 1)
  {
    // MPI_Bcast((*file)->idx->bounds, PIDX_MAX_DIMENSIONS, MPI_UNSIGNED_LONG_LONG, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast((*file)->idx->box_bounds, PIDX_MAX_DIMENSIONS, MPI_UNSIGNED_LONG_LONG, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast((*file)->idx->reg_patch_size, PIDX_MAX_DIMENSIONS, MPI_UNSIGNED_LONG_LONG, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast((*file)->idx->chunk_size, PIDX_MAX_DIMENSIONS, MPI_UNSIGNED_LONG_LONG, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx->endian), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx_d->pidx_version), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx->blocks_per_file), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx->bits_per_block), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx->variable_count), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx->variable_grp[0]->variable_count), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast((*file)->idx->bitSequence, 512, MPI_CHAR, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx_d->data_core_count), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast((*file)->idx_d->partition_count, PIDX_MAX_DIMENSIONS, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast((*file)->idx_d->partition_size, PIDX_MAX_DIMENSIONS, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast((*file)->idx_d->partition_offset, PIDX_MAX_DIMENSIONS, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx->compression_bit_rate), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx->compression_type), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx->io_type), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
    // MPI_Bcast(&((*file)->idx_d->fs_block_size), 1, MPI_INT, 0, (*file)->idx_c->global_comm);

    if ((*file)->idx->compression_type == PIDX_CHUNKING_ZFP)
    {
      if ((*file)->idx->compression_bit_rate == 32)
        (*file)->idx->compression_factor = 2;
      if ((*file)->idx->compression_bit_rate == 16)
        (*file)->idx->compression_factor = 4;
      if ((*file)->idx->compression_bit_rate == 8)
        (*file)->idx->compression_factor = 8;
      if ((*file)->idx->compression_bit_rate == 4)
        (*file)->idx->compression_factor = 16;
      if ((*file)->idx->compression_bit_rate == 2)
        (*file)->idx->compression_factor = 32;
      if ((*file)->idx->compression_bit_rate == 1)
        (*file)->idx->compression_factor = 64;
    }
  }

  if ((*file)->idx->io_type != PIDX_RAW_IO)
  {
    int total_header_size = (10 + (10 * (*file)->idx->blocks_per_file)) * sizeof (uint32_t) * (*file)->idx->variable_count;
    (*file)->idx_d->start_fs_block = total_header_size / (*file)->idx_d->fs_block_size;
    if (total_header_size % (*file)->idx_d->fs_block_size)
      (*file)->idx_d->start_fs_block++;

    (*file)->idx_d->samples_per_block = (int)pow(2, (*file)->idx->bits_per_block);
  }

  if((*file)->idx_c->grank != 0)
  {
    for (var = 0; var < (*file)->idx->variable_count; var++)
    {
      (*file)->idx->variable_grp[0]->variable[var] = malloc(sizeof (*((*file)->idx->variable_grp[0]->variable[var])));
      memset((*file)->idx->variable_grp[0]->variable[var], 0, sizeof (*((*file)->idx->variable_grp[0]->variable[var])));
    }
  }
#endif

  for (var = 0; var < (*file)->idx->variable_count; var++)
  {
#if PIDX_HAVE_MPI
    if ((*file)->idx_d->parallel_mode == 1)
    {
      // MPI_Bcast(&((*file)->idx->variable_grp[0]->variable[var]->bpv), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
      // MPI_Bcast(&((*file)->idx->variable_grp[0]->variable[var]->vps), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
      // MPI_Bcast((*file)->idx->variable_grp[0]->variable[var]->var_name, 512, MPI_CHAR, 0, (*file)->idx_c->global_comm);
      // MPI_Bcast((*file)->idx->variable_grp[0]->variable[var]->type_name, 512, MPI_CHAR, 0, (*file)->idx_c->global_comm);
    }
#endif
    (*file)->idx->variable_grp[0]->variable[var]->sim_patch_count = 0;
  }

  if ((*file)->idx_c->grank == 0)
  {
    int ret;
    struct stat stat_buf;
    ret = stat((*file)->idx->filename, &stat_buf);
    if (ret != 0)
    {
      fprintf(stderr, "[%s] [%d] Unable to identify File-System block size\n", __FILE__, __LINE__);
      return PIDX_err_file;
    }
    (*file)->idx_d->fs_block_size = stat_buf.st_blksize;
  }

#if PIDX_HAVE_MPI
  // if ((*file)->idx_d->parallel_mode == 1)
  //   MPI_Bcast(&((*file)->idx_d->fs_block_size), 1, MPI_INT, 0, (*file)->idx_c->global_comm);
#endif

  (*file)->idx->flip_endian = 0;

  if ((*file)->idx->io_type == PIDX_IDX_IO)
    (*file)->idx->reg_box_set = PIDX_BOX_FROM_BITSTRING;
  else
    (*file)->idx->reg_box_set = PIDX_USER_RST_BOX;
  unsigned int endian = 1;
  int current_endian = 0;
  char *c = (char*)&endian;
  if (*c)
    current_endian = 1;
  else
    current_endian = 0;

  if (current_endian == (*file)->idx->endian)
    (*file)->idx->flip_endian = 0;
  else
    (*file)->idx->flip_endian = 1;

  memcpy(dims, (*file)->idx->bounds, (sizeof(unsigned long long) * PIDX_MAX_DIMENSIONS));

  return PIDX_success;
}


PIDX_return_code PIDX_query_box(PIDX_file file, PIDX_point box_dims)
{
  if(!file)
    return PIDX_err_file;

  memcpy(file->idx->box_bounds, box_dims, PIDX_MAX_DIMENSIONS * sizeof(unsigned long long));

  return PIDX_success;
}
