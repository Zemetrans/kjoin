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
#include <string.h>

#define CONNECT_ATTEMPTS   10
static const char ServiceName[] = "ru.rtsoft.dev.kjoin";
static const char ServicePath[] = "/keapi";
static const uint16_t ServicePort = 25;

uint8_t dbgBASIC_SERVICE = 0;
/**
 * The interface name followed by the method signatures.
 *
 * See also .\inc\aj_introspect.h
 */
static const char* const sampleInterface[] = {
    "ru.rtsoft.dev.kjoin",   /* The first entry is the interface name. */
    "?boardName outStr>s", /* Method at index 0. */
    "?countSensor outStr>i",
    "?sensorValue inStr<i outStr>i outStr1>i",
    "?sensorInfo inStr<i outStr>i outStr1>i outStr2>i outSt3r>i outStr4>i outStr5>i outSt6r>i outStr6>s",
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
 * The value of the arguments are the indices of the
 * object path in AppObjects (above), interface in sampleInterfaces (above), and
 * member indices in the interface.
 * The 'cat' index is 1 because the first entry in sampleInterface is the interface name.
 * This makes the first index (index 0 of the methods) the second string in
 * sampleInterface[] which, for illustration purposes is a dummy entry.
 * The index of the method we implement for basic_service, 'cat', is 1 which is the third string
 * in the array of strings sampleInterface[].
 *
 * See also .\inc\aj_introspect.h
 */
#define BASIC_SERVICE_BOARDINFO AJ_APP_MESSAGE_ID(0, 0, 0)
#define BASIC_SERVICE_COUNTSENSOR AJ_APP_MESSAGE_ID(0, 0, 1)
#define BASIC_SERVICE_SENSORVALUE AJ_APP_MESSAGE_ID(0, 0, 2)
#define BASIC_SERVICE_SENSORINFO AJ_APP_MESSAGE_ID(0, 0, 3)

static AJ_Status AppHandleInfo(AJ_Message* msg)
{
#define BUFFER_SIZE 256
    char buffer[BUFFER_SIZE];
    KEApiLibInitialize();
    KEAPI_BOARD_INFO BoardInfo;
    KEApiGetBoardInfo(&BoardInfo);
    AJ_Message reply;
    AJ_Arg replyArg;

    AJ_MarshalReplyMsg(msg, &reply);

    /* We have the arguments. Now do the concatenation. */
    strncpy(buffer, BoardInfo.boardName, BUFFER_SIZE);
    buffer[BUFFER_SIZE - 1] = '\0';
    KEApiLibUnInitialize();
    AJ_InitArg(&replyArg, AJ_ARG_STRING, 0, buffer, 0);
    AJ_MarshalArg(&reply, &replyArg);
    return AJ_DeliverMsg(&reply);

#undef BUFFER_SIZE
}

static AJ_Status AppHandleCount(AJ_Message* msg)
{
#define BUFFER_SIZE 20
    char buf[BUFFER_SIZE];
    KEApiLibInitialize();
    int32_t TempSensorCount;
    AJ_Message reply;
    AJ_Arg replyArg;
      
    AJ_MarshalReplyMsg(msg, &reply);
    
    KEApiGetTempSensorCount(&TempSensorCount);
    AJ_InitArg(&replyArg, AJ_ARG_STRING, 0, buf, 0);
    AJ_MarshalArgs(&reply, "i", TempSensorCount);
    KEApiLibUnInitialize();
    return AJ_DeliverMsg(&reply);
    
#undef BUFFER_SIZE
}

static AJ_Status AppHandleValue(AJ_Message* msg)
{
#define BUFFER_SIZE 256
    int num;
    char buf[BUFFER_SIZE];
    KEApiLibInitialize();
    KEAPI_SENSOR_VALUE SensorValue;
    AJ_Message reply;
    AJ_Arg replyArg;
    
    AJ_UnmarshalArgs(msg, "i", &num);
    AJ_MarshalReplyMsg(msg, &reply);
    KEApiGetTempSensorValue(num, &SensorValue);
    
    AJ_InitArg(&replyArg, AJ_ARG_STRING, 0, buf, 0);
    AJ_MarshalArgs(&reply, "ii", SensorValue.value, SensorValue.status);
    KEApiLibUnInitialize();
    return AJ_DeliverMsg(&reply);
    
#undef BUFFER_SIZE
}
static AJ_Status AppHandleSensorInfo(AJ_Message* msg)
{
#define BUFFER_SIZE 256
    int num;
    char type[BUFFER_SIZE];
    KEApiLibInitialize();
    KEAPI_SENSOR_INFO SensorInfo;
    AJ_Message reply;
    AJ_Arg replyArg;
    
    AJ_UnmarshalArgs(msg, "i", &num);
    AJ_MarshalReplyMsg(msg, &reply);
    
    KEApiGetTempSensorInfo(num, &SensorInfo);
    AJ_InitArg(&replyArg, AJ_ARG_STRING, 0, type, 0);
    AJ_MarshalArgs(&reply, "iiiiiiis", SensorInfo.type, SensorInfo.min, SensorInfo.max, SensorInfo.alarmHi, SensorInfo.hystHi,SensorInfo.alarmLo, SensorInfo.hystLo, SensorInfo.name);
    KEApiLibUnInitialize();
    return AJ_DeliverMsg(&reply);
    
#undef BUFFER_SIZE
}
/* All times are expressed in milliseconds. */
#define CONNECT_TIMEOUT     (1000 * 60)
#define UNMARSHAL_TIMEOUT   (1000 * 5)
#define SLEEP_TIME          (1000 * 2)

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint32_t sessionId = 0;

    /* One time initialization before calling any other AllJoyn APIs. */
    AJ_Initialize();

    /* This is for debug purposes and is optional. */
    AJ_PrintXML(AppObjects);

    AJ_RegisterObjects(AppObjects, NULL);

    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartService(&bus,
                                     NULL,
                                     CONNECT_TIMEOUT,
                                     FALSE,
                                     ServicePort,
                                     ServiceName,
                                     AJ_NAME_REQ_DO_NOT_QUEUE,
                                     NULL);

            if (status != AJ_OK) {
                continue;
            }

            AJ_InfoPrintf(("StartService returned %d, session_id=%u\n", status, sessionId));
            connected = TRUE;
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);

        if (AJ_ERR_TIMEOUT == status) {
            continue;
        }

        if (AJ_OK == status) {
            switch (msg.msgId) {
            case AJ_METHOD_ACCEPT_SESSION:
                {
                    uint16_t port;
                    char* joiner;
                    AJ_UnmarshalArgs(&msg, "qus", &port, &sessionId, &joiner);
                    status = AJ_BusReplyAcceptSession(&msg, TRUE);
                    AJ_InfoPrintf(("Accepted session session_id=%u joiner=%s\n", sessionId, joiner));
                }
                break;

            case BASIC_SERVICE_BOARDINFO:
                status = AppHandleInfo(&msg);
                break;

            case BASIC_SERVICE_COUNTSENSOR:
            	status = AppHandleCount(&msg);
            	break;
            
            case BASIC_SERVICE_SENSORVALUE:
            	status = AppHandleValue(&msg);
            	break;
            	
            case BASIC_SERVICE_SENSORINFO:
            	status = AppHandleSensorInfo(&msg);
            	break;
            	
            case AJ_SIGNAL_SESSION_LOST_WITH_REASON:
                {
                    uint32_t id, reason;
                    AJ_UnmarshalArgs(&msg, "uu", &id, &reason);
                    AJ_AlwaysPrintf(("Session lost. ID = %u, reason = %u", id, reason));
                }
                break;

            default:
                /* Pass to the built-in handlers. */
                status = AJ_BusHandleBusMessage(&msg);
                break;
            }
        }

        /* Messages MUST be discarded to free resources. */
        AJ_CloseMsg(&msg);

        if (status == AJ_ERR_READ) {
            AJ_AlwaysPrintf(("AllJoyn disconnect.\n"));
            AJ_Disconnect(&bus);
            connected = FALSE;

            /* Sleep a little while before trying to reconnect. */
            AJ_Sleep(SLEEP_TIME);
        }
    }

    AJ_AlwaysPrintf(("Basic service exiting with status %d.\n", status));

    return status;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
