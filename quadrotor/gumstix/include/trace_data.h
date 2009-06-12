/*
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at
 * Copyright (c) Rainer Trummer rtrummer@cs.uni-salzburg.at
 *
 * University Salzburg, www.uni-salzburg.at
 * Department of Computer Science, cs.uni-salzburg.at
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef TRACE_DATA_H
#define TRACE_DATA_H

#include <stdint.h>


/* Structure for representing trace data */
typedef struct
{
    int16_t z;              /* z from sonar sensor */
    int16_t filtered_z;     /* median-filtered z */
    int16_t estimated_z;    /* kalman-estimated z */
    int16_t estimated_dz;   /* kalman-estimated dz */
    int16_t ddz;            /* ddz from IMU sensor */
    int16_t filtered_ddz;   /* low-pass-filtered ddz */
    int16_t p_term;         /* computed p-term */
    int16_t i_term;         /* computed i-term */
    int16_t d_term;         /* computed d-term */
    int16_t dd_term;        /* computed dd-term */
    int16_t uz;             /* output from z-controller */
    int16_t cmd_z;          /* z-command from terminal */
    int16_t id;             /* packet id */

} trace_data_t;

#define TRACE_DATA_SIZE     26  /* byte size of trace_data_t */


static inline
int trace_data_to_stream( const trace_data_t *data, char *buf, int len )
{
    if( len == TRACE_DATA_SIZE )
    {
        buf[0]  = (char)( data->z >> 8 );
        buf[1]  = (char)( data->z );
        buf[2]  = (char)( data->filtered_z >> 8 );
        buf[3]  = (char)( data->filtered_z );
        buf[4]  = (char)( data->estimated_z >> 8 );
        buf[5]  = (char)( data->estimated_z );
        buf[6]  = (char)( data->estimated_dz >> 8 );
        buf[7]  = (char)( data->estimated_dz );
        buf[8]  = (char)( data->ddz >> 8 );
        buf[9]  = (char)( data->ddz );
        buf[10] = (char)( data->filtered_ddz >> 8 );
        buf[11] = (char)( data->filtered_ddz );
        buf[12] = (char)( data->p_term >> 8 );
        buf[13] = (char)( data->p_term );
        buf[14] = (char)( data->i_term >> 8 );
        buf[15] = (char)( data->i_term );
        buf[16] = (char)( data->d_term >> 8 );
        buf[17] = (char)( data->d_term );
        buf[18] = (char)( data->dd_term >> 8 );
        buf[19] = (char)( data->dd_term );
        buf[20] = (char)( data->uz >> 8 );
        buf[21] = (char)( data->uz );
        buf[22] = (char)( data->cmd_z >> 8 );
        buf[23] = (char)( data->cmd_z );
        buf[24] = (char)( data->id >> 8 );
        buf[25] = (char)( data->id );
        return( 0 );
    }

    return( -1 );
}

static inline
int trace_data_from_stream( trace_data_t *data, const char *buf, int len )
{
    if( len == TRACE_DATA_SIZE )
    {
        data->z            = (int16_t)( (buf[0]  << 8) | (buf[1]  & 0xFF) );
        data->filtered_z   = (int16_t)( (buf[2]  << 8) | (buf[3]  & 0xFF) );
        data->estimated_z  = (int16_t)( (buf[4]  << 8) | (buf[5]  & 0xFF) );
        data->estimated_dz = (int16_t)( (buf[6]  << 8) | (buf[7]  & 0xFF) );
        data->ddz          = (int16_t)( (buf[8]  << 8) | (buf[9]  & 0xFF) );
        data->filtered_ddz = (int16_t)( (buf[10] << 8) | (buf[11] & 0xFF) );
        data->p_term       = (int16_t)( (buf[12] << 8) | (buf[13] & 0xFF) );
        data->i_term       = (int16_t)( (buf[14] << 8) | (buf[15] & 0xFF) );
        data->d_term       = (int16_t)( (buf[16] << 8) | (buf[17] & 0xFF) );
        data->dd_term      = (int16_t)( (buf[18] << 8) | (buf[19] & 0xFF) );
        data->uz           = (int16_t)( (buf[20] << 8) | (buf[21] & 0xFF) );
        data->cmd_z        = (int16_t)( (buf[22] << 8) | (buf[23] & 0xFF) );
        data->id           = (int16_t)( (buf[24] << 8) | (buf[25] & 0xFF) );
        return( 0 );
    }

    return( -1 );
}


#endif /* !TRACE_DATA_H */

/* End of file */
