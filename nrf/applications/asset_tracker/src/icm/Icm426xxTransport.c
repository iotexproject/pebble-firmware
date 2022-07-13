/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2015-2015 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively “Software”) is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

#include "Icm426xxTransport.h"

#include "InvError.h"


int inv_icm426xx_read_reg(struct inv_icm426xx * s, uint8_t reg, uint32_t len, uint8_t * buf)
{
	// First field of struct inv_icm426xx is assumed to be a struct inv_icm426xx_serif object.
	// So let's cast s to struct inv_icm426xx_serif and ignore the rest of struct inv_icm426xx.
	struct inv_icm426xx_serif *serif = (struct inv_icm426xx_serif *)s;
	
	if(len > serif->max_read)
		return INV_ERROR_SIZE;

	if(serif->read_reg(serif, reg, buf, len) != 0)
		return INV_ERROR_TRANSPORT;

	return 0;
}

int inv_icm426xx_write_reg(struct inv_icm426xx * s, uint8_t reg, uint32_t len, const uint8_t * buf)
{
	// First field of struct inv_icm426xx is assumed to be a struct inv_icm426xx_serif object.
	// So let's cast s to struct inv_icm426xx_serif and ignore the rest of struct inv_icm426xx.
	struct inv_icm426xx_serif *serif = (struct inv_icm426xx_serif *)s;

	if(len > serif->max_write)
		return INV_ERROR_SIZE;

	if(serif->write_reg(serif, reg, buf, len) != 0)
		return INV_ERROR_TRANSPORT;

	return 0;
}
