/* Copyright (c) 2010 James Grenning and Contributed to Hunt Project
 * ==========================================
 *  Hunt Project - A Test Framework for C
 *  Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
 *  [Released under MIT License. Please refer to license.txt for details]
 * ========================================== */

#ifndef D_hunt_output_Spy_H
#define D_hunt_output_Spy_H

void HuntOutputCharSpy_Create(int s);
void HuntOutputCharSpy_Destroy(void);
void HuntOutputCharSpy_OutputChar(int c);
const char * HuntOutputCharSpy_Get(void);
void HuntOutputCharSpy_Enable(int enable);

#endif
