/*
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
#define AJ_MODULE BASIC_CLIENT

#include <stdio.h>
#include <aj_debug.h>
#include <alljoyn.h>
#include <keapi/keapi.h>
#include <time.h>
#include <stdlib.h>

static const char ServiceName[] = "ru.rtsoft.dev.kjoin";
static const char ServicePath[] = "/keapi";
static const uint16_t ServicePort = 25;

/*
 * Buffer to hold the full service name. This buffer must be big enough to hold
 * a possible 255 characters plus a null terminator (256 bytes)
 */
static char fullServiceName[AJ_MAX_SERVICE_NAME_SIZE];

uint8_t dbgBASIC_CLIENT = 0;
/**
 * The interface name followed by the method signatures.
 *
 * See also .\inc\aj_introspect.h
 */
static const char* const sampleInterface[] = {
    "ru.rtsoft.dev.kjoin",   /* The first entry is the interface name. */
    "?boardName outStr>s", /* Method at index 0. */
    "?countSensor outStr>s",
    "?sensorValue inStr<i outStr>s",
    "?sensorInfo inStr<i outStr>s",
    NULL
};

/**
 * A NULL terminated collection of all interfaces.
 */
static const AJ_InterfaceDescription sampleInterfaces[] = {
    sampleInterface,
    NULL
};

/**
 * Objects implemented by the application. The first member in the AJ_Object structure is the path.
 * The second is the collection of all interfaces at that path.
 */
static const AJ_Object AppObjects[] = {
    { ServicePath, sampleInterfaces },
    { NULL }
};

/*
 * The value of the arguments are the indices of the object path in AppObjects (above),
 * interface in sampleInterfaces (above), and member indices in the interface.
 * The 'cat' index is 2. The reason for this is as follows: The first entry in sampleInterface
 * is the interface name. This makes the first index (index 0 of the methods) the second string in
 * sampleInterface[]. The two dummy entries are indices 0 and 1. The index of the method we
 * implement for basic_client, 'cat', is 2 which is the fourth string in the array of strings
 * sampleInterface[].
 *
 * See also .\inc\aj_introspect.h
 */
#define BASIC_CLIENT_BOARDINFO AJ_PRX_MESSAGE_ID(0, 0, 0)
#define BASIC_CLIENT_COUNTSENSOR AJ_PRX_MESSAGE_ID(0, 0, 1)
#define BASIC_CLIENT_SENSORVALUE AJ_PRX_MESSAGE_ID(0, 0, 2)
#define BASIC_CLIENT_SENSORINFO AJ_PRX_MESSAGE_ID(0, 0, 3)

#define CONNECT_TIMEOUT    (1000 * 60)
#define UNMARSHAL_TIMEOUT  (1000 * 5)
#define METHOD_TIMEOUT     (100 * 10)

void MakeMethodCall(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, BASIC_CLIENT_BOARDINFO, fullServiceName, sessionId, 0, METHOD_TIMEOUT);

    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }

    AJ_InfoPrintf(("MakeMethodCall() resulted in a status of 0x%04x.\n", status));
}

void MakeMethodCall1(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, BASIC_CLIENT_COUNTSENSOR, fullServiceName, sessionId, 0, METHOD_TIMEOUT);

    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }

    AJ_InfoPrintf(("MakeMethodCall1() resulted in a status of 0x%04x.\n", status));
}

void MakeMethodCall2(AJ_BusAttachment* bus, uint32_t sessionId, int numSen)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, BASIC_CLIENT_SENSORVALUE, fullServiceName, sessionId, 0, METHOD_TIMEOUT);

    if (status == AJ_OK) {
        status = AJ_MarshalArgs(&msg, "i", numSen);
    }

    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }

    AJ_InfoPrintf(("MakeMethodCall2() resulted in a status of 0x%04x.\n", status));
}

void MakeMethodCall3(AJ_BusAttachment* bus, uint32_t sessionId, int numSen)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, BASIC_CLIENT_SENSORINFO, fullServiceName, sessionId, 0, METHOD_TIMEOUT);

    if (status == AJ_OK) {
        status = AJ_MarshalArgs(&msg, "i", numSen);
    }

    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }

    AJ_InfoPrintf(("MakeMethodCall2() resulted in a status of 0x%04x.\n", status));
}
int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint8_t done = FALSE;
    uint32_t sessionId = 0;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();
    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(NULL, AppObjects);
    
    int flag = 1;
    int flag_val = 0;
    int flag_info = 0;
    int flag_val_iter = 0;
    //int flag_val_info = 0;
    int count = -1;
    //int iter;
   
    while (!done) {
    	printf("WHILE\n");
        AJ_Message msg;
        if (!connected) {
            status = AJ_StartClientByName(&bus,
                                          NULL,
                                          CONNECT_TIMEOUT,
                                          FALSE,
                                          ServiceName,
                                          ServicePort,
                                          &sessionId,
                                          NULL,
                                          fullServiceName);

            if (status == AJ_OK) {
                AJ_InfoPrintf(("StartClient returned %d, sessionId=%u.\n", status, sessionId));
                connected = TRUE;
		if (flag) {
                	MakeMethodCall(&bus, sessionId);
                	printf("MMC done\n");
                }
            } else {
                AJ_InfoPrintf(("StartClient returned 0x%04x.\n", status));
                break;
            }
        }
        if (flag_val_iter == count) {
		done = TRUE;
		continue;
	}
	
	if (!flag) {
		printf("In if\n");
		MakeMethodCall1(&bus, sessionId);
		//printf("MMC1 done\n");
		flag++;
	}
	
	if ((flag_val) && (!flag_info)) {
		printf("In if Val\n");
		MakeMethodCall2(&bus, sessionId, flag_val_iter);
		//MakeMethodCall3(&bus, sessionId, flag_val_iter);
		//++flag_val_iter;
		printf("Flag_val_iter temp: %d\n", flag_val_iter);
		//printf("MMC2 done\n");
		//flag_info++;
	}
	
	if (flag_info) {
		MakeMethodCall3(&bus, sessionId, flag_val_iter);
		flag_info--;
		printf("Flag_val_iter: %d\n", flag_val_iter);
		++flag_val_iter;
	}
	
        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);

        if (AJ_ERR_TIMEOUT == status) {
            continue;
        }

        if (AJ_OK == status) {
            switch (msg.msgId) {
            case AJ_REPLY_ID(BASIC_CLIENT_BOARDINFO):
                {
                    AJ_Arg arg;

                    status = AJ_UnmarshalArg(&msg, &arg);

                    if (AJ_OK == status) {
                        AJ_AlwaysPrintf(("'%s.%s' (path='%s') returned '%s'.\n", fullServiceName, "boardName",
                                         ServicePath, arg.val.v_string));
                        --flag;
                        printf("flag after --: %d\n", flag);
                        //done = TRUE;
                    } else {
                        AJ_InfoPrintf(("AJ_UnmarshalArg() returned status %d.\n", status));
                        /* Try again because of the failure. */
                        MakeMethodCall(&bus, sessionId);
                    }
                }
                break;

            case AJ_REPLY_ID(BASIC_CLIENT_COUNTSENSOR):
                {
                    AJ_Arg arg;
		    printf("In AJ_REPLY_ID\n");
                    status = AJ_UnmarshalArg(&msg, &arg);

                    if (AJ_OK == status) {
                        AJ_AlwaysPrintf(("'%s.%s' (path='%s') returned '%s'.\n", fullServiceName, "countSensor",
                                         ServicePath, arg.val.v_string));
                        count = atoi(arg.val.v_string);
                        printf("count: %d\n", count);
                        //done = TRUE;
                        flag_val++;
                    } else {
                        AJ_InfoPrintf(("AJ_UnmarshalArg() returned status %d.\n", status));
                        /* Try again because of the failure. */
                        MakeMethodCall1(&bus, sessionId);
                    }
                }
                break;
            
            case AJ_REPLY_ID(BASIC_CLIENT_SENSORVALUE):
                {
                    AJ_Arg arg;
		    printf("In AJ_REPLY_ID\n");
                    status = AJ_UnmarshalArg(&msg, &arg);

                    if (AJ_OK == status) {
                        AJ_AlwaysPrintf(("'%s.%s' (path='%s') returned: \n'%s'.\n", fullServiceName, "sensorValue",
                                         ServicePath, arg.val.v_string));
                        //if (flag_val_iter == count) {
                        	//done = TRUE;
                        //}
                        flag_info++;
                        
                    } else {
                        AJ_InfoPrintf(("AJ_UnmarshalArg() returned status %d.\n", status));
                        /* Try again because of the failure. */
                        MakeMethodCall1(&bus, sessionId);
                    }
                }
                break;
            case AJ_REPLY_ID(BASIC_CLIENT_SENSORINFO):
                {
                    AJ_Arg arg;
		    printf("In AJ_REPLY_ID\n");
                    status = AJ_UnmarshalArg(&msg, &arg);

                    if (AJ_OK == status) {
                        AJ_AlwaysPrintf(("'%s.%s' (path='%s') returned: \n'%s'.\n", fullServiceName, "sensorInfo",
                                         ServicePath, arg.val.v_string));
                        //if (flag_val_iter == count) {
                        	//printf("Flag_val_iter: %d\n", flag_val_iter);
                        	//done = TRUE;
                        //}
                        
                    } else {
                        AJ_InfoPrintf(("AJ_UnmarshalArg() returned status %d.\n", status));
                        /* Try again because of the failure. */
                        MakeMethodCall1(&bus, sessionId);
                    }
                }
                break;
                
            case AJ_SIGNAL_SESSION_LOST_WITH_REASON:
                /* A session was lost so return error to force a disconnect. */
                {
                    uint32_t id, reason;
                    AJ_UnmarshalArgs(&msg, "uu", &id, &reason);
                    AJ_AlwaysPrintf(("Session lost. ID = %u, reason = %u", id, reason));
                }
                status = AJ_ERR_SESSION_LOST;
                break;

            default:
                /* Pass to the built-in handlers. */
                status = AJ_BusHandleBusMessage(&msg);
                break;
            }
        }

        /* Messages MUST be discarded to free resources. */
        
        AJ_CloseMsg(&msg);
      

        if (status == AJ_ERR_SESSION_LOST) {
            AJ_AlwaysPrintf(("AllJoyn disconnect.\n"));
            AJ_Disconnect(&bus);
            exit(0);
        }
    }

    AJ_AlwaysPrintf(("Basic client exiting with status %d.\n", status));

    return status;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
