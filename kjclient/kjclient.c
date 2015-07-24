#define AJ_MODULE KJOIN_CLIENT

#include <stdio.h>
#include <aj_debug.h>
#include <alljoyn.h>
#include <keapi/keapi.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

static const char ServiceName[] = "ru.rtsoft.dev.kjoin";
static const char ServicePath[] = "/keapi";
static const uint16_t ServicePort = 25;

uint8_t dbgKJOIN_CLIENT = 1;

/*
 * Buffer to hold the full service name. This buffer must be big enough to hold
 * a possible 255 characters plus a null terminator (256 bytes)
 */
static char fullServiceName[AJ_MAX_SERVICE_NAME_SIZE];

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
    "?sensorInfo inStr<i outStr>i outStr1>i outStr2>i outSt3r>i outStr4>i outStr5>i outSt6r>i outStr7>s",
    NULL
};

/**
 * A NULL terminated collection of all interfaces.
 */
static const AJ_InterfaceDescription sampleInterfaces[] = {
    sampleInterface,
    NULL
};

/**`
 * Objects implemented by the application. The first member in the AJ_Object structure is the path.
 * The second is the collection of all interfaces at that path.
 */
static const AJ_Object AppObjects[] = {
    { ServicePath, sampleInterfaces },
    { NULL }
};

#define KJOIN_CLIENT_BOARDINFO AJ_PRX_MESSAGE_ID(0, 0, 0)
#define KJOIN_CLIENT_COUNTSENSOR AJ_PRX_MESSAGE_ID(0, 0, 1)
#define KJOIN_CLIENT_SENSORVALUE AJ_PRX_MESSAGE_ID(0, 0, 2)
#define KJOIN_CLIENT_SENSORINFO AJ_PRX_MESSAGE_ID(0, 0, 3)

#define CONNECT_TIMEOUT    (1000 * 60)
#define UNMARSHAL_TIMEOUT  (1000 * 5)
#define METHOD_TIMEOUT     (100 * 10)

#define NUM_MESSAGES (10000)

typedef struct {
    AJ_BusAttachment *pBus;
    uint32_t sessionId;
    int overall;
    int sent;
    int processed;
    time_t start_time;
} GEN_DATA;

typedef enum {
    KJ_CLIENT_CONNECT,
    KJ_CLIENT_MSG_PROCESSING,
    KJ_CLIENT_START_BOARD_INFO,
    KJ_CLIENT_START_SENSOR_COUNT,
    KJ_CLIENT_START_SENSOR_VALUE,
    KJ_CLIENT_START_SENSOR_INFO,
    KJ_CLIENT_STOP
} KJ_CLIENT_STATE;

static void SendMsg(GEN_DATA *pData, uint32_t msgId) {
    if (pData->sent < pData->overall) {
        AJ_Status status;
        AJ_Message msg;

        status = AJ_MarshalMethodCall(pData->pBus, &msg, msgId, fullServiceName, pData->sessionId, 0, METHOD_TIMEOUT);

        switch (msgId) {
            case KJOIN_CLIENT_BOARDINFO:
                break;
            case KJOIN_CLIENT_COUNTSENSOR:
                break;
            case KJOIN_CLIENT_SENSORVALUE:
                if (status == AJ_OK) {
                    status = AJ_MarshalArgs(&msg, "i", 0);
                }
                break;
            case KJOIN_CLIENT_SENSORINFO:
                if (status == AJ_OK) {
                    status = AJ_MarshalArgs(&msg, "i", 0);
                }
                break;
        }

        if (status == AJ_OK) {
            status = AJ_DeliverMsg(&msg);
        }
        if (status != AJ_OK) {
            AJ_AlwaysPrintf(("AJ_MarshalMethodCall() for msg %X resulted in a status of 0x%04x.\n", msgId, status));
        } else {
            pData->sent++;
        }
    }
}

#define initMsgData(data) do { \
                memset(&data, 0, sizeof(data));\
                data.pBus = &bus;\
                data.sessionId = sessionId;\
                data.overall = NUM_MESSAGES;\
                data.start_time = time(NULL);\
} while (0)

int AJ_Main(void)
{
    KJ_CLIENT_STATE state, next_state;
    GEN_DATA data;
    AJ_BusAttachment bus;
    uint32_t sessionId;
    AJ_Status status;


    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();
    AJ_RegisterObjects(NULL, AppObjects);
    
    state = KJ_CLIENT_CONNECT;
    next_state = KJ_CLIENT_START_BOARD_INFO;

    while (state != KJ_CLIENT_STOP) {
        AJ_Message msg;

        if (state == KJ_CLIENT_CONNECT) {
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
                AJ_AlwaysPrintf(("StartClient returned AJ_OK, sessionId=%u.\n", sessionId));
                state = next_state;
                initMsgData(data);
            } else {
                AJ_AlwaysPrintf(("StartClient failed with status 0x%04x. Still trying...\n", status));
                break;
            }
        }


        if (state >= KJ_CLIENT_START_BOARD_INFO && state <= KJ_CLIENT_START_SENSOR_INFO) {

            switch (state) {
                case KJ_CLIENT_START_BOARD_INFO:
                    SendMsg(&data, KJOIN_CLIENT_BOARDINFO);
                    break;
                case KJ_CLIENT_START_SENSOR_COUNT:
                    SendMsg(&data, KJOIN_CLIENT_COUNTSENSOR);
                    break;
                case KJ_CLIENT_START_SENSOR_VALUE:
                    SendMsg(&data, KJOIN_CLIENT_SENSORVALUE);
                    break;
                case KJ_CLIENT_START_SENSOR_INFO:
                    SendMsg(&data, KJOIN_CLIENT_SENSORINFO);
                    break;
                default:
                    AJ_AlwaysPrintf(("Panic: unknown state: %d\n", state));
                    break;
            }

            state = KJ_CLIENT_MSG_PROCESSING;
        }
	
        if (state == KJ_CLIENT_MSG_PROCESSING) {
            status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);

            if (AJ_ERR_TIMEOUT == status) {
                AJ_AlwaysPrintf(("No message in %d seconds, still trying...\n", UNMARSHAL_TIMEOUT / 1000));
                continue;
            } else if (AJ_OK == status) {
                switch (msg.msgId) {
                case AJ_REPLY_ID(KJOIN_CLIENT_BOARDINFO):
                case AJ_REPLY_ID(KJOIN_CLIENT_COUNTSENSOR):
                case AJ_REPLY_ID(KJOIN_CLIENT_SENSORVALUE):
                case AJ_REPLY_ID(KJOIN_CLIENT_SENSORINFO):
                    {
                        data.processed++;

                        if (msg.msgId == AJ_REPLY_ID(KJOIN_CLIENT_BOARDINFO)) {
                            AJ_Arg arg;
                            status = AJ_UnmarshalArg(&msg, &arg);
                        } else if (msg.msgId == AJ_REPLY_ID(KJOIN_CLIENT_COUNTSENSOR)) {
                            int countSensor;
                            status = AJ_UnmarshalArgs(&msg, "i", &countSensor);
                        }  else if (msg.msgId == AJ_REPLY_ID(KJOIN_CLIENT_SENSORVALUE)) {
                            int value;
                            int sensorStatus;
                            status = AJ_UnmarshalArgs(&msg, "ii", &value, &sensorStatus);
                        }  else if (msg.msgId == AJ_REPLY_ID(KJOIN_CLIENT_SENSORINFO)) {
                            int sensorType;
                            int min;
                            int max;
                            int alarmHi;
                            int hystHi;
                            int alarmLo;
                            int hystLo;
                            char *name;
                            status = AJ_UnmarshalArgs(&msg, "iiiiiiis", &sensorType, &min, &max, &alarmHi, &hystHi, &alarmLo, &hystLo, &name);
                        }

                        if (AJ_OK != status) {
                            AJ_AlwaysPrintf(("%X : AJ_UnmarshalArg() failed with status %d.\n", msg.msgId, status));
                        }

                        if (data.processed == data.overall) {
                            int elapsed = time(NULL) - data.start_time;
                            AJ_AlwaysPrintf(("%X : %d messages received in %d seconds (%.2f ms per message).\n", 
                                msg.msgId, data.overall, 
                                elapsed, elapsed * 1000. / data.overall));
                            // received all messages, go to next message type
                            state = ++next_state;
                            initMsgData(data);
                  } else {
                            state = next_state;
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
                    AJ_AlwaysPrintf(("Unknown message: %X. Pass to the built-in handlers.", msg.msgId));
                    status = AJ_BusHandleBusMessage(&msg);
                    break;
                }
            } else {
                AJ_AlwaysPrintf(("AJ_UnmarshalMsg failed with status %d.\n", status));
            }

            /* Messages MUST be discarded to free resources. */
            
            AJ_CloseMsg(&msg);
          
            if (status == AJ_ERR_SESSION_LOST) {
                AJ_AlwaysPrintf(("AllJoyn disconnect.\n"));
                AJ_Disconnect(&bus);
                exit(0);
            }
        }
    }

    AJ_AlwaysPrintf(("KJoin client finished with status %d.\n", status));

    return status;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
