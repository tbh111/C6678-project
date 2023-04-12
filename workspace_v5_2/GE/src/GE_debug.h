/***************************************************************************\
 *       Copyright (C) 2013 Texas Instruments Incorporated.      *
 *                           All Rights Reserved                            *
 *                                                                          *
 * GENERAL DISCLAIMER                                                       *
 * ------------------                                                       *
 * All software and related documentation is provided "AS IS" and without   *
 * warranty or support of any kind and Texas Instruments expressly disclaims*
 * all other warranties, express or implied, including, but not limited to, *
 * the implied warranties of merchantability and fitness for a particular   *
 * purpose.  Under no circumstances shall Texas Instruments be liable for   *
 * any incidental, special or consequential damages that result from the    *
 * use or inability to use the software or related documentation, even if   *
 * Texas Instruments has been advised of the liability.                     *
   **************************************************************************
This file Print GE status and error information
****************************************************************************
 * Created by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             * 
 *            June 11, 2013                                                 *
\***************************************************************************/
#ifndef _GE_DEBUG_H
#define _GE_DEBUG_H

extern void print_GE_link_ability(Uint32 uiPort);

extern void print_GE_status_error();

//print GE ethernet port error/status
extern void print_GE_ethernet_ports_status();

#endif
