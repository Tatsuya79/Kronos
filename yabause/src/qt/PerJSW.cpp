/*  Copyright 2005 Guillaume Duhamel
    Copyright 2005-2006 Theo Berkau
	Copyright 2008 Filipe Azevedo <pasnox@gmail.com>

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "PerJSW.h"
#include "JSWHelper.h"

#include <QDebug>

extern "C"
{
	static JSWHelperAttributes* mJoysticks = 0;
	static JSWHelperJoystick* mJoystickP1 = 0;
	u32 mJoystickLastAxisP1[] = { 0, 0, 0, 0, 0, 0 }; // increase this if your joystick can handle more than 6 axis

	int PERJSWInit(void);
	void PERJSWDeInit(void);
	int PERJSWHandleEvents(void);
	void PERJSWNothing(void);
#ifdef USENEWPERINTERFACE
	PortData_struct *PERJSWGetPerDataP1(void);
	PortData_struct *PERJSWGetPerDataP2(void);

	static PortData_struct port1;
	static PortData_struct port2;
	u32 PERJSWScan(const char * name);
	void PERJSWFlush(void);
#endif

	PerInterface_struct PERJSW = {
	PERCORE_JSW,
	"JSW Joystick Interface",
	PERJSWInit,
	PERJSWDeInit,
#ifndef USENEWPERINTERFACE
	PERJSWHandleEvents
#else
	PERJSWHandleEvents,
	PERJSWGetPerDataP1,
	PERJSWGetPerDataP2,
	PERJSWNothing,
	PERJSWScan,
	1,
	PERJSWFlush
#endif
	};

	int PERJSWInit(void)
	{
		if ( !mJoysticks )
			mJoysticks = JSWHelper::joysticks();
		if ( !mJoystickP1 )
			mJoystickP1 = mJoysticks->joystick( 0 );
		if ( !mJoystickP1 )
			return 0;
		if ( mJoystickP1->status() != JSSuccess )
			PERJSWDeInit();
		return mJoystickP1 ? ( mJoystickP1->status() == JSSuccess ? 0 : -1 ) : -1;
	}

	void PERJSWDeInit(void)
	{
		delete mJoystickP1;
		mJoystickP1 = 0;
		delete mJoysticks;
		mJoysticks = 0;
	}

	void PERJSWNothing(void)
	{}

	u32 hashAxis( int a, double v )
	{
		u32 r = u32( v < 0 ? -v : v +1 );
		a % 2 ? r-- : r++;
		return r;
	}

	int PERJSWHandleEvents(void)
	{
		if ( mJoystickP1 )
		{
			// handle joystick events
			js_data_struct jsd = *mJoystickP1->datas();
			if ( JSUpdate( &jsd ) == JSGotEvent )
			{
				// check axis
				for ( int i = 0; i < jsd.total_axises; i++ )
				{
					// double cur = JSGetAxisCoeffNZ( &jsd, i ); // should normally use this is joy is calibrate using jscalibrator
					double cur = jsd.axis[i]->cur;
					double cen = jsd.axis[i]->cen;
					if ( cur == cen )
						PerKeyUp( mJoystickLastAxisP1[i] );
					else
					{
						u32 k = hashAxis( i, cur ) +100; // +100 to avoid conflict with buttons as cur range is -1.0 - +1.0
						PerKeyDown( k );
						mJoystickLastAxisP1[i] = k;
					}
				}
				// check buttons
				for ( int i = 0; i < jsd.total_buttons; i++ )
				{
					int bs = JSGetButtonState( &jsd, i );
					if ( bs == 0 )
						PerKeyUp( i +1 );
					else
						PerKeyDown( i +1 );
				}
			}
		}
	
		// need fix
	   if (YabauseExec() != 0)
		  return -1;
	   if (YabauseExec() != 0)
		  return -1;
	   if (YabauseExec() != 0)
		  return -1;
	   if (YabauseExec() != 0)
		  return -1;
	   if (YabauseExec() != 0)
		  return -1;
	   if (YabauseExec() != 0)
		  return -1;
	   if (YabauseExec() != 0)
		  return -1;
	   if (YabauseExec() != 0)
		  return -1;
	   return 0;
	}

#ifdef USENEWPERINTERFACE
	PortData_struct* PERJSWGetPerDataP1(void)
	{
	   // fix me, but this is the basic idea. Basically make sure the structure
	   // is completely ready before you return
	   port1.data[0] = 0xF1;
	   port1.data[1] = 0x02;
	   port1.data[2] = buttonbits >> 8;
	   port1.data[3] = buttonbits & 0xFF;
	   port1.size = 4;
	   return &port1;
	}

	PortData_struct* PERJSWGetPerDataP2(void)
	{
	   port2.data[0] = 0xF0;
	   port2.size = 1;
	   return &port2;
	}

	PerInfo_struct* PERJSWGetList(void)
	{
	   // Returns a list of peripherals available along with information on each
	   // peripheral
		return 0;
	}

	u32 PERJSWScan( const char* n )
	{
		if ( !mJoystickP1 )
			return 0;
		u32 k = 0;
		js_data_struct jsd = *mJoystickP1->datas();
		if ( JSUpdate( &jsd ) == JSGotEvent )
		{
			// check axis
			for ( int i = 4; i < jsd.total_axises; i++ )
			{
				// double cur = JSGetAxisCoeffNZ( &jsd, i ); // should normally use this is joy is calibrate using jscalibrator
				double cur = jsd.axis[i]->cur;
				double cen = jsd.axis[i]->cen;
				if ( cur != cen )
				{
					k = hashAxis( i, cur ) +100; // +100 to avoid conflict with buttons as cur range is -1.0 - +1.0
					break;
				}
			}
			// check buttons
			if ( k == 0 )
			{
				for ( int i = 0; i < jsd.total_buttons; i++ )
				{
					if ( JSGetButtonState( &jsd, i ) )
					{
						k = i +1;
						break;
					}
				}
			}
		}
		if ( k != 0 )
			PerSetKey( k, n );
		return k;
	}

	void PERJSWFlush(void)
	{}
#endif
}