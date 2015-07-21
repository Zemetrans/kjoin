/**
 * @file
 */
/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#define AJ_MODULE BASIC_SERVICE

#include <stdio.h>
#include <aj_debug.h>
#include <alljoyn.h>
#include <keapi/keapi.h>
#include <time.h>
#include <stdlib.h>


int main()
{
   int i;
   char buf[256], buf1[256];
   KEApiLibInitialize();
   int32_t TempSensorCount;
   KEAPI_SENSOR_VALUE SensorValue;
   KEAPI_SENSOR_INFO SensorInfo;
   
   KEApiGetTempSensorCount(&TempSensorCount);
   printf("Sensor Count: %d\n", TempSensorCount);
   //sprintf(buf,"%d",TempSensorCount);
   //printf("Buf: %s\n", buf);
   for (i = 0; i < TempSensorCount; ++i) {
   	KEApiGetTempSensorValue(i, &SensorValue);
   	KEApiGetTempSensorInfo(i, &SensorInfo);
   	sprintf(buf,"Sensor %d, temp = %d,  status: %d\n", i, SensorValue.value, SensorValue.status);
   	sprintf(buf1,"Sensor %d, type: %d, min temp = %d, max temp = %d, alarmHi = %d,\n hystHi = %d, alarmLo = %d, hystLo = %d\n",
   		i, SensorInfo.type, SensorInfo.min, SensorInfo.max, SensorInfo.alarmHi, 
   		SensorInfo.hystHi, SensorInfo.alarmLo, SensorInfo.hystLo);
   	printf("%s", buf);
   	printf("%s\n",buf1);
   }
   char s[5] ="5";
   int d = atoi(s);
   printf("After atoi: %d\n", d);
   return 0;
}

