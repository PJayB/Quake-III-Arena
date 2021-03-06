/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "snd_local.h"
#ifndef WIN8
#   include "../win32/win_snd.h"
#endif
#include "../xaudio/xaudio_public.h"

// @pjb: sound API definition
typedef struct {
    void        (* Shutdown)( void );
    qboolean    (* Init)( void );
    int         (* GetDMAPos)( void );
    void        (* BeginPainting)( int reserve );
    void        (* Submit)( int offset, int length );
    void        (* Activate)( void );
} soundApi_t;

static soundApi_t s_soundDriver;

void SND_InitDriverAPI( void )
{
    cvar_t* snd_driver = Cvar_Get( "snd_driver", "dsound", CVAR_ARCHIVE | CVAR_LATCH );

    Com_Memset( &s_soundDriver, 0, sizeof( s_soundDriver ) );
       
#ifndef WIN8
    if ( Q_strncmp( snd_driver->string, "dsound", 6 ) == 0 ) {
        s_soundDriver.Shutdown = DirectSound_Shutdown;
        s_soundDriver.Init = DirectSound_Init;
        s_soundDriver.GetDMAPos = DirectSound_GetDMAPos;
        s_soundDriver.BeginPainting = DirectSound_BeginPainting;
        s_soundDriver.Submit = DirectSound_Submit;
        s_soundDriver.Activate = DirectSound_Activate;
    } else if ( Q_strncmp( snd_driver->string, "xaudio", 6 ) == 0 ) {
#endif
        s_soundDriver.Shutdown = XAudio_Shutdown;
        s_soundDriver.Init = XAudio_Init;
        s_soundDriver.GetDMAPos = XAudio_GetDMAPos;
        s_soundDriver.BeginPainting = XAudio_BeginPainting;
        s_soundDriver.Submit = XAudio_Submit;
        s_soundDriver.Activate = XAudio_Activate;
#ifndef WIN8
    } else {
        Com_Error( ERR_FATAL, "Unknown sound driver '%s'\n", snd_driver->string );
    }
#endif
}


/*
==================
SNDDMA_Shutdown
==================
*/
void SNDDMA_Shutdown( void ) {
	Com_DPrintf( "Shutting down sound system\n" );

    s_soundDriver.Shutdown();

    Com_Memset( &s_soundDriver, 0, sizeof( s_soundDriver ) );
}

/*
==================
SNDDMA_Init

Initialize direct sound
Returns false if failed
==================
*/
qboolean SNDDMA_Init(void) {

    SND_InitDriverAPI();

	memset ((void *)&dma, 0, sizeof (dma));

    return s_soundDriver.Init();
}

/*
==============
SNDDMA_GetDMAPos

return the current sample position (in mono samples read)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
int SNDDMA_GetDMAPos( void ) {

    return s_soundDriver.GetDMAPos();
}

/*
==============
SNDDMA_BeginPainting

Makes sure dma.buffer is valid
===============
*/
void SNDDMA_BeginPainting( int reserve ) {

    s_soundDriver.BeginPainting( reserve );
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
Also unlocks the dsound buffer
===============
*/
void SNDDMA_Submit( int offset, int length ) {

    s_soundDriver.Submit( offset, length );
}

/*
=================
SNDDMA_Activate

When we change windows we need to do this
=================
*/
void SNDDMA_Activate( void ) {

    // @pjb: this can happen before the sound driver has been initialized
    if ( s_soundDriver.Activate ) {
        s_soundDriver.Activate();
    }
}
