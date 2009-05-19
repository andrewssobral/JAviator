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
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "../shared/protocol.h"
#include "../shared/transfer.h"
#include "controller.h"
#include "communication.h"
#include "javiator_port.h"
#include "inertial_port.h"
#include "terminal_port.h"

static command_data_t   command_data;
static ctrl_params_t    r_p_params;
static ctrl_params_t    yaw_params;
static ctrl_params_t    alt_params;
static ctrl_params_t    x_y_params;
static comm_channel_t * comm_channel;
static comm_packet_t    comm_packet;
static char             comm_packet_buf[ COMM_BUF_SIZE ];
static volatile int     shut_down;
static volatile int     test_mode;
static volatile int     mode_switch;
static volatile int     base_motor_speed;
static volatile int     multiplier = 1;
static volatile int     new_r_p_params;
static volatile int     new_yaw_params;
static volatile int     new_alt_params;
static volatile int     new_x_y_params;
static volatile int     new_command_data;

static pthread_mutex_t terminal_lock = PTHREAD_MUTEX_INITIALIZER;

static inline void lock(void)
{
	pthread_mutex_lock(&terminal_lock);
}

static inline void unlock(void)
{
	pthread_mutex_unlock(&terminal_lock);
}

static inline int set_idle_limit( const comm_packet_t *packet )
{
    char *p = (char *) packet->payload;
    base_motor_speed = (int)( (short)( p[0] << 8 ) | (short)( p[1] & 0xFF ) );
    fprintf( stdout, "parameter update: Idle Limit\n--> %d\n", base_motor_speed );

    return( 0 );
}

static inline int set_test_mode( const comm_packet_t *packet )
{
    char *p = (char *) packet->payload;
    test_mode = (int) p[0];
    return( 0 );
}

static inline int set_mode_switch( void )
{
    mode_switch = 1;
    return( 0 );
}

static inline int set_shut_down( void )
{
    shut_down = 1;
    return( 0 );
}

static inline int parse_command_data_packet( const comm_packet_t *packet )
{
    new_command_data = 1;
    return command_data_from_stream( &command_data, packet->payload, packet->size );
}

static inline int parse_r_p_params_packet( const comm_packet_t *packet )
{
    new_r_p_params = 1;
    return ctrl_params_from_stream( &r_p_params, packet->payload, packet->size );
}

static inline int parse_yaw_params_packet( const comm_packet_t *packet )
{
    new_yaw_params = 1;
    return ctrl_params_from_stream( &yaw_params, packet->payload, packet->size );
}

static inline int parse_alt_params_packet( const comm_packet_t *packet )
{
    new_alt_params = 1;
    return ctrl_params_from_stream( &alt_params, packet->payload, packet->size );
}

static inline int parse_x_y_params_packet( const comm_packet_t *packet )
{
    new_x_y_params = 1;
    return ctrl_params_from_stream( &x_y_params, packet->payload, packet->size );
}

static int process_data_packet( const comm_packet_t *packet )
{
	int retval;
    if( !packet )
    {
        return( -1 );
    }

	lock();
    switch( packet->type )
    {
        case COMM_COMMAND_DATA:
            retval = parse_command_data_packet( packet );
			break;

        case COMM_R_P_PARAMS:
            retval = parse_r_p_params_packet( packet );
			break;

        case COMM_YAW_PARAMS:
            retval = parse_yaw_params_packet( packet );
			break;

        case COMM_ALT_PARAMS:
            retval = parse_alt_params_packet( packet );
			break;

        case COMM_X_Y_PARAMS:
            retval = parse_x_y_params_packet( packet );
			break;

        case COMM_IDLE_LIMIT:
            retval = set_idle_limit( packet );
			break;

        case COMM_TEST_MODE:
            retval = set_test_mode( packet );
			break;

        case COMM_SWITCH_MODE:
            retval = set_mode_switch( );
			break;

        case COMM_SHUT_DOWN:
            retval = set_shut_down( );
			break;

        default:
			unlock();
            return javiator_port_forward( packet );
    }
	unlock();
    return( retval );
}

int terminal_port_tick( void )
{
    int res = comm_recv_packet( comm_channel, &comm_packet );

    if( res == 0 )
    {
        process_data_packet( &comm_packet );
    }
    else
    if( res == EAGAIN )
    {
        res = 0;
    }
    else
    if( res == -1 )
    {
        fprintf( stderr, "ERROR: invalid data from terminal channel\n" );
    }
    else
    {
        fprintf( stderr, "ERROR: cannot receive from terminal channel\n" );
    }

    return( res );
}

static int running;
static pthread_t thread;
static void *terminal_thread(void *arg)
{
	int ret;
	while (running) {
		ret = comm_channel->poll(comm_channel, 0);
		if (ret > 0) {
			terminal_port_tick();
		} else {
			fprintf(stderr, "WARNING %s %d: poll returned %d\n", __FILE__,__LINE__, ret);
		}
	}

	return NULL;
}

static void start_terminal_thread(void)
{
	struct sched_param param;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	
	sched_getparam(0, &param);
	if (param.sched_priority > 0) {
		param.sched_priority--;
		pthread_attr_setschedparam(&attr, &param);		
		pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	}

	pthread_create(&thread, &attr, terminal_thread, NULL);

}

int terminal_port_init( comm_channel_t *channel )
{
    static int already_initialized = 0;

    if( !already_initialized )
    {
        memset( &command_data, 0, sizeof( command_data ) );
        memset( &r_p_params,   0, sizeof( r_p_params ) );
        memset( &yaw_params,   0, sizeof( yaw_params ) );
        memset( &alt_params,   0, sizeof( alt_params ) );
        memset( &x_y_params,   0, sizeof( x_y_params ) );
        memset( &comm_packet,  0, sizeof( comm_packet ) );

        comm_channel         = channel;
        comm_packet.buf_size = COMM_BUF_SIZE;
        comm_packet.payload  = comm_packet_buf;
        already_initialized  = 1;

		running = 1;
		start_terminal_thread();
        return( 0 );
    }

    fprintf( stderr, "WARNING: terminal port already initialized\n" );
    return( -1 );
}

int terminal_port_set_multiplier( int m )
{
    return( multiplier = m );
}

int terminal_port_reset_shut_down( void )
{
    shut_down = 0;
    return( 0 );
}

int terminal_port_is_new_command_data( void )
{
    return( new_command_data );
}

int terminal_port_is_new_r_p_params( void )
{
    return( new_r_p_params );
}

int terminal_port_is_new_yaw_params( void )
{
    return( new_yaw_params );
}

int terminal_port_is_new_alt_params( void )
{
    return( new_alt_params );
}

int terminal_port_is_new_x_y_params( void )
{
    return( new_x_y_params );
}

int terminal_port_is_test_mode( void )
{
    return( test_mode );
}

int terminal_port_is_mode_switch( void )
{
    int __mode_switch = mode_switch;
    mode_switch = 0;
    return( __mode_switch );
}

int terminal_port_is_shut_down( void )
{
    return( shut_down );
}

int terminal_port_get_command_data( command_data_t *data )
{
	lock();
    memcpy( data, &command_data, sizeof( *data ) );
    new_command_data = 0;
	unlock();
    return( 0 );
}

int terminal_port_get_r_p_params( ctrl_params_t *params )
{
	lock();
    memcpy( params, &r_p_params, sizeof( *params ) );
    new_r_p_params = 0;
	unlock();
    return( 0 );
}

int terminal_port_get_yaw_params( ctrl_params_t *params )
{
	lock();
    memcpy( params, &yaw_params, sizeof( *params ) );
    new_yaw_params = 0;
	unlock();
    return( 0 );
}

int terminal_port_get_alt_params( ctrl_params_t *params )
{
	lock();
    memcpy( params, &alt_params, sizeof( *params ) );
    new_alt_params = 0;
	unlock();
    return( 0 );
}

int terminal_port_get_x_y_params( ctrl_params_t *params )
{
	lock();
    memcpy( params, &x_y_params, sizeof( *params ) );
    new_x_y_params = 0;
	unlock();
    return( 0 );
}

int terminal_port_get_base_motor_speed( void )
{
    return( base_motor_speed );
}

int terminal_port_send_sensor_data( const sensor_data_t *data )
{
    char buf[ SENSOR_DATA_SIZE ];
    comm_packet_t packet;

    sensor_data_to_stream( data, buf, SENSOR_DATA_SIZE );

    packet.type     = COMM_SENSOR_DATA;
    packet.size     = SENSOR_DATA_SIZE;
    packet.buf_size = SENSOR_DATA_SIZE;
    packet.payload  = buf;

    return comm_send_packet( comm_channel, &packet );
}

int terminal_port_send_motor_signals( const motor_signals_t *signals )
{
    char buf[ MOTOR_SIGNALS_SIZE ];
    comm_packet_t packet;

    motor_signals_to_stream( signals, buf, MOTOR_SIGNALS_SIZE );

    packet.type     = COMM_MOTOR_SIGNALS;
    packet.size     = MOTOR_SIGNALS_SIZE;
    packet.buf_size = MOTOR_SIGNALS_SIZE;
    packet.payload  = buf;

    return comm_send_packet( comm_channel, &packet );
}

int terminal_port_send_motor_offsets( const command_data_t *offsets )
{
    char buf[ COMMAND_DATA_SIZE ];
    comm_packet_t packet;

    command_data_to_stream( offsets, buf, COMMAND_DATA_SIZE );

    packet.type     = COMM_MOTOR_OFFSETS;
    packet.size     = COMMAND_DATA_SIZE;
    packet.buf_size = COMMAND_DATA_SIZE;
    packet.payload  = buf;

    return comm_send_packet( comm_channel, &packet );
}

int terminal_port_send_state_and_mode( const int state, const int mode )
{
    char buf[2] = { (char) state, (char) mode };
    comm_packet_t packet;

    packet.type     = COMM_STATE_MODE;
    packet.size     = 2;
    packet.buf_size = 2;
    packet.payload  = buf;

    return comm_send_packet( comm_channel, &packet );
}

int terminal_port_send_report(
        const sensor_data_t   *sensors,
        const motor_signals_t *signals,
        const command_data_t  *offsets,
        const int state, const int mode )
{
    static int counter = 1;
    char buf[ SENSOR_DATA_SIZE
            + MOTOR_SIGNALS_SIZE
            + COMMAND_DATA_SIZE
            + 2 ]; /* state and mode */
    comm_packet_t packet;
    int i = 0;

    if( --counter == 0 )
    {
        sensor_data_to_stream( sensors, &buf[i], SENSOR_DATA_SIZE );
        i += SENSOR_DATA_SIZE;

        motor_signals_to_stream( signals, &buf[i], MOTOR_SIGNALS_SIZE );
        i += MOTOR_SIGNALS_SIZE;

        command_data_to_stream( offsets, &buf[i], COMMAND_DATA_SIZE );
        i += COMMAND_DATA_SIZE;

        buf[i++] = (char) state;
        buf[i++] = (char) mode;

        packet.type     = COMM_GROUND_REPORT;
        packet.size     = i;
        packet.buf_size = i;
        packet.payload  = buf;
        counter         = multiplier;

        return comm_send_packet( comm_channel, &packet );
    }

    return( 0 );
}

int terminal_port_send_trace_data( const trace_data_t *data )
{
    char buf[ TRACE_DATA_SIZE ];
    comm_packet_t packet;

    trace_data_to_stream( data, buf, TRACE_DATA_SIZE );

    packet.type     = COMM_TRACE_DATA;
    packet.size     = TRACE_DATA_SIZE;
    packet.buf_size = TRACE_DATA_SIZE;
    packet.payload  = buf;

    return comm_send_packet( comm_channel, &packet );
}

int terminal_port_forward( const comm_packet_t *packet )
{
    return comm_send_packet( comm_channel, packet );
}

/* End of file */
