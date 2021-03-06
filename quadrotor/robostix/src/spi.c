/*****************************************************************************/
/*   This code is part of the JAviator project: javiator.cs.uni-salzburg.at  */
/*                                                                           */
/*   spi.c    SPI interface used for communication with the Gumstix.         */
/*                                                                           */
/*   Copyright (c) 2006-2013 Rainer Trummer <rainer.trummer@gmail.com>       */
/*                                                                           */
/*   This program is free software; you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by    */
/*   the Free Software Foundation; either version 2 of the License, or       */
/*   (at your option) any later version.                                     */
/*                                                                           */
/*   This program is distributed in the hope that it will be useful,         */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/*   GNU General Public License for more details.                            */
/*                                                                           */
/*   You should have received a copy of the GNU General Public License       */
/*   along with this program; if not, write to the Free Software Foundation, */
/*   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.      */
/*                                                                           */
/*****************************************************************************/

#include <avr/interrupt.h>
#include <string.h>

#include "protocol.h"
#include "spi.h"


/*****************************************************************************/
/*                                                                           */
/*   Private Definitions                                                     */
/*                                                                           */
/*****************************************************************************/

/* Global variables */
static          uint8_t     rx_buf[ COMM_BUF_SIZE ];
static          uint8_t     tx_buf[ COMM_BUF_SIZE ];
static volatile uint8_t     rx_items;
static volatile uint8_t     tx_items;
static volatile uint8_t     rx_index;
static volatile uint8_t     tx_index;
static volatile uint8_t     new_data;
static volatile uint8_t     spi_rx;
static volatile uint8_t     spi_tx;
static volatile uint8_t     spi_if;

/* Forward declarations */
static uint8_t  is_valid_data( const uint8_t *, uint8_t );


/*****************************************************************************/
/*                                                                           */
/*   Public Functions                                                        */
/*                                                                           */
/*****************************************************************************/

/* Initializes the SPI interface for communication
*/
void spi_init( void )
{
    /* make MISO an output, MOSI, SCK, and SS inputs */
    DDRB |= (1 << DDB3);

    /* enable SPI interface and interrupt */
    SPCR = (1 << SPE) | (1 << SPIE);

    /* initialize global variables */
    rx_items = 0;
    tx_items = 0;
    rx_index = 0;
    tx_index = 0;
    new_data = 0;
    spi_rx   = 0;
    spi_tx   = 0;
    spi_if   = 0;

}

/* Returns 1 if new data available, 0 otherwise
*/
uint8_t spi_is_new_data( void )
{
    /* check for SPI interrupt */
	if( spi_if )
    {
        spi_if = 0;

		/* check RX data stream for packet marks and payload size */
		if( rx_index == 0 )
		{
			/* check for first packet mark */
			if( spi_rx == COMM_PACKET_MARK )
			{
				rx_items = COMM_OVERHEAD;
			}
		}
        else
        if( rx_index == 1 )
		{
			/* check for second packet mark */
			if( spi_rx != COMM_PACKET_MARK )
			{
				rx_items = 0;
				rx_index = 0;
			}
		}
        else
        {
            /* indicate that the receive buffer is being updated
               and thus data are no longer secure to be copied */
            new_data = 0;

            /* read data from SPI port */
			rx_buf[ rx_index ] = spi_rx;

			if( rx_index == 3 )
			{
				rx_buf[0] = COMM_PACKET_MARK;
				rx_buf[1] = COMM_PACKET_MARK;

				/* second header byte contains payload size */
				rx_items += rx_buf[3];

				/* check for valid packet size */
				if( rx_items > COMM_BUF_SIZE )
				{
					rx_items = 0;
					rx_index = 0;
				}
			}
		}

		/* check for end of RX data stream */
		if( rx_items && ++rx_index == rx_items )
		{
			/* check for valid packet content */
			if( is_valid_data( rx_buf + 2, rx_items - 2 ) )
			{
				new_data = 1;
			}

			rx_items = 0;
		}

		/* check for TX items to transmit */
		if( tx_items )
		{
            /* write data to SPI port */
			spi_tx = tx_buf[ tx_index ];

			/* check for end of TX data stream */
			if( ++tx_index == tx_items )
			{
				tx_items = 0;
			}
		}
		else
		{
            /* transmit null-byte */
			spi_tx = 0;
		}

		/* check for end of RX transmission */
		if( !rx_items )
		{
			rx_index = 0;
		}

		/* check for end of TX transmission */
		if( !tx_items ) 
		{
			tx_index = 0;
		}
	}

    return( new_data );
}

/* Copies the received data to the given buffer.
   Returns 0 if successful, -1 otherwise.
*/
int8_t spi_get_data( uint8_t *buf )
{
    /* check that we're not receiving data currently */
    if( !new_data )
    {
        return( -1 );
    }

    cli( ); /* disable interrupts */

    /* copy received data packet to given buffer */
    memcpy( buf, rx_buf + 2, rx_buf[3] + COMM_OVERHEAD - 2 );

    /* clear new-data indicator */
    new_data = 0;

    sei( ); /* enable interrupts */

    return( 0 );
}

/* Sends a data via the SPI interface.
   Note: the interface operates in Slave Mode, hence
   the packet will be prepared and sent with the next
   transmission started by the SPI Master.
   Returns 0 if successful, -1 otherwise.
*/
int8_t spi_send_data( uint8_t id, const uint8_t *data, uint8_t size )
{
    uint16_t checksum = id + size;

    /* check that we're not transmitting data currently */
    if( tx_items )
    {
        return( -1 );
    }

    /* write packet header */
    tx_buf[ tx_items++ ] = COMM_PACKET_MARK;
    tx_buf[ tx_items++ ] = COMM_PACKET_MARK;
    tx_buf[ tx_items++ ] = id;
    tx_buf[ tx_items++ ] = size;

    /* write packet payload */
    while( size-- )
    {
        checksum += *data;
        tx_buf[ tx_items++ ] = *data++;
    }

    /* write packet checksum */
    tx_buf[ tx_items++ ] = (uint8_t)( checksum >> 8 );
    tx_buf[ tx_items++ ] = (uint8_t)( checksum );

    return( 0 );
}

/* Resets the SPI interface
*/
void spi_reset( void )
{
    rx_items = 0;
    tx_items = 0;
    rx_index = 0;
    tx_index = 0;
    new_data = 0;
    spi_rx   = 0;
    spi_tx   = 0;
    spi_if   = 0;
}


/*****************************************************************************/
/*                                                                           */
/*   Private Functions                                                       */
/*                                                                           */
/*****************************************************************************/

/* Returns 1 if the packet contains valid data, 0 otherwise
*/
static uint8_t is_valid_data( const uint8_t *data, uint8_t size )
{
    uint16_t checksum = (data[ size-2 ] << 8) | data[ size-1 ];

    size -= 2;

    /* iterate over payload */
    while( size-- )
    {
        checksum -= data[ size ];
    }

    return( checksum == 0 );
}

/* SPI Transmission Complete callback function
*/
SIGNAL( SIG_SPI )
{
	SPDR   = spi_tx;
	spi_rx = SPDR;
	spi_if = 1;
}

/* End of file */
