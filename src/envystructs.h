/***************************************************************************
 *   Copyright (C) 2007 by Valentin Rusu   *
 *   kenvy24@rusu.info   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef _ENVY_STRUCTS_H_
#define _ENVY_STRUCTS_H_


/**
 * Router sources: these are routed to either digital L/R out or analog L/R out
 */
#define R_SRC_PCM "PCM Out"
#define R_SRC_ANALOG "H/W In 0"
#define R_SRC_DIGITAL_L "IEC958 In L"
#define R_SRC_DIGITAL_R "IEC958 In R"
#define R_SRC_MIXER "Digital Mixer"


/**
 * HW settings items
 */

#define HW_ENUM_INTERNAL_CLOCK "Multi Track Internal Clock"
#define HW_ENUM_CLOCK_DEFAULT "Multi Track Internal Clock Default"
#define HW_ENUM_DEEMPHASIS "Deemphasis"
#define HW_BOOL_RATE_LOCKING "Multi Track Rate Locking"
#define HW_BOOL_RATE_RESET "Multi Track Rate Reset"

/**
 * Number of stereo outputs
 * NOTE : be warned that, for some reasons, alsa tool envy24control counts each
 * pcm output independently
 */

#define MAX_PCM_OUTPUTS   5

/**
 * NOTE: a channel has two, left and right, streams
 */
#define MAX_SPDIF_CHANNELS  1

/**
 * number of *stereo* input channels
 */
#define MAX_INPUT_CHANNELS  4

/**
 * number of *stereo* output channels
 */
#define MAX_OUTPUT_CHANNELS 4

/**
 * number of *stereo* output SPDIF channels
 */
#define MAX_SPDIF_CHANNELS	1

/**
 * Structure used by level monitors
 */
struct StereoLevels {
  int left;
  int right;
};

enum LeftRight { 
    LEFT = 0, 
    RIGHT = 1 
};

/**
 * Structure used to adjust mixer levels
 */
struct MixerAdjustement {
    int volume;
    int stereo;
};

/**
 * Envy cards handle two kinds of routes
 */
enum RouteKind  { 
    ANALOG, 
    SPDIF 
};

#endif // _ENVY_STRUCTS_H_
