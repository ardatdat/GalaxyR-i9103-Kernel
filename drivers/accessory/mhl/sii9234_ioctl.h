/**
 * (C) COPYRIGHT 2010 SAMSUNG ELECTRONICS
 * Silicon Image MHL(Mobile HD Link) Transmitter device driver
 *
 * File Name : sii9234_ioctl.h
 *
 * Author    : Woojong Yoo
 *		woojong.yoo@samsung.com
 *
 * Description: Source file for MHL sii9234 Transciever
 */


#define MHL_MAGIC_NUMBER	'M'

#define MHL_SWITCH_ON		_IO(MHL_MAGIC_NUMBER, 0)
#define MHL_SWITCH_OFF		_IO(MHL_MAGIC_NUMBER, 1)
#define MHL_HW_RESET		_IO(MHL_MAGIC_NUMBER, 2)
#define MHL_INIT		_IO(MHL_MAGIC_NUMBER, 3)
