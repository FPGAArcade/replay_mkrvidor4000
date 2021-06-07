/*--------------------------------------------------------------------
 *                       Replay Firmware
 *                      www.fpgaarcade.com
 *                     All rights reserved.
 *
 *                     admin@fpgaarcade.com
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *--------------------------------------------------------------------
 *
 * Copyright (c) 2020, The FPGAArcade community (see AUTHORS.txt)
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if defined(ARDUINO)   // sketches already have a main()
int replay_main(void);
#else
int main(void);
#endif

#ifdef __cplusplus
} // extern "C"
#endif
