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
#include <stdlib.h>
#include <aj_debug.h>
#include <alljoyn.h>
#include <keapi/keapi.h>
#include <time.h>

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
    "?boardName Name>s Date>i SCount>i", /* Method at index 1. */
    "?sensorStatus id<i idBack>i SName>s SType>i STemp>i SStatus>i",
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
#define BASIC_CLIENT_CAT AJ_PRX_MESSAGE_ID(0, 0, 0)
#define BASIC_CLIENT_SENSOR_INFO AJ_APP_MESSAGE_ID(0, 0, 1)

#define CONNECT_TIMEOUT    (1000 * 60)
#define UNMARSHAL_TIMEOUT  (1000 * 5)
#define METHOD_TIMEOUT     (100 * 10)

void MakeMethodCall(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, BASIC_CLIENT_CAT, fullServiceName, sessionId, 0, METHOD_TIMEOUT);

    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }

    AJ_InfoPrintf(("MakeMethodCall() resulted in a status of 0x%04x.\n", status));
}

void MakeMethodCall1(AJ_BusAttachment* bus, uint32_t sessionId, int id)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, BASIC_CLIENT_SENSOR_INFO, fullServiceName, sessionId, 0, METHOD_TIMEOUT);
    printf("KEK\n");
    
    if (status == AJ_OK) {
        status = AJ_MarshalArgs(&msg, "i", id);
    }

    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }

    AJ_InfoPrintf(("MakeMethodCall() resulted in a status of 0x%04x.\n", status));
}

char *TypeDecode(int code)
{
	if      (code == 1) return "KEAPI_TEMP_CPU"; 
	else if (code == 2) return "KEAPI_TEMP_BOX"; 
	else if (code == 3) return "KEAPI_TEMP_ENV"; 
	else if (code == 4) return "KEAPI_TEMP_BOARD"; 
	else if (code == 5) return "KEAPI_TEMP_BACKPLANE"; 
	else if (code == 6) return "KEAPI_TEMP_CHIPSET"; 
	else if (code == 7) return "KEAPI_TEMP_VIDEO"; 
	else if (code == 8) return "KEAPI_TEMP_OTHER"; 
	return "error";
}

char *StatusDecode(int code)
{
	if      (code == 1) return "KEAPI_SENSOR_STATUS_ACTIVE"; 
	else if (code == 2) return "KEAPI_SENSOR_STATUS_ALARM"; 
	else if (code == 3) return "KEAPI_SENSOR_STATUS_BROKEN"; 
	else if (code == 4) return "KEAPI_SENSOR_STATUS_SHORTCIRCUIT"; 
	return "error";
}

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint8_t done = FALSE;
    uint32_t sessionId = 0;
    int SenCount,id;
    KEAPI_BOARD_INFO BoardInfo;
	KEAPI_SENSOR_VALUE SVal[20];
	int i,SType[20];
	char *BName[20];

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();
    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(NULL, AppObjects);

    while (!done) {
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

                MakeMethodCall(&bus, sessionId);
                printf("%d\n", SenCount);
            } else {
                AJ_InfoPrintf(("StartClient returned 0x%04x.\n", status));
                break;
            }
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);

        if (AJ_ERR_TIMEOUT == status) {
            continue;
        }

        if (AJ_OK == status) {
            switch (msg.msgId) {
            case AJ_REPLY_ID(BASIC_CLIENT_CAT):
                {
					char *Name;
					const int Mtime;

                    status = AJ_UnmarshalArgs(&msg, "sii", &Name, &Mtime, &SenCount);

                    if (AJ_OK == status) {						
						system("clear");
						printf("Название платы: %s\n", Name);
						printf("Дата производства: %s", ctime(&Mtime));
						printf("Колличество сенсоров: %d\n\n", SenCount);

						if (SenCount!=0) MakeMethodCall1(&bus, sessionId, 0);
						else done = TRUE;

                        //done = TRUE;
                    } else {
                        AJ_InfoPrintf(("AJ_UnmarshalArg() returned status %d.\n", status));
                        /* Try again because of the failure. */
                        MakeMethodCall(&bus, sessionId);
                    }
                }
                break;

            case AJ_REPLY_ID(BASIC_CLIENT_SENSOR_INFO):
                {
                	printf("KEK\n");
                	int LocalId;
                	AJ_UnmarshalArgs(&msg, "i", &LocalId);
				    AJ_UnmarshalArgs(&msg, "siii", &BName[LocalId], &SType[LocalId], &SVal[LocalId].value, &SVal[LocalId].status);

                    if (AJ_OK == status) {				
						printf("Сенсор №%d:\n",LocalId+1);
						printf("    Имя         - %s\n",BName[LocalId]);
						printf("    Тип         - %s\n",TypeDecode(SType[LocalId]));
						printf("    Температура - %dºС\n",SVal[LocalId].value/1000);
						printf("    Статус      - %s\n",StatusDecode(SVal[LocalId].status));
						printf("\n");

                        done = TRUE;
                    } else {
                        AJ_InfoPrintf(("AJ_UnmarshalArg() returned status %d.\n", status));
                        /* Try again because of the failure. */
                        MakeMethodCall(&bus, sessionId);
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
