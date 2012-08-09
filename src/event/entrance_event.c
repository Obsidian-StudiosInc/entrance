#include <Eina.h>
#include <Eet.h>
#include "entrance_event.h"

#define ENTRANCE_EVENT_AUTH_NAME "EntranceEventAuth"
#define ENTRANCE_EVENT_MAXTRIES_NAME "EntranceEventMaxtries"
#define ENTRANCE_EVENT_XSESSIONS_NAME "EntranceEventSession"
#define ENTRANCE_EVENT_STATUS_NAME "EntranceEventStatus"
#define ENTRANCE_EVENT_USERS_NAME "EntranceEventUsers"
#define ENTRANCE_EVENT_ACTIONS_NAME "EntranceEventActions"
#define ENTRANCE_EVENT_ACTION_NAME "EntranceEventAction"

static Eina_Bool _entrance_event_type_set(const char *type, void *data, Eina_Bool unknow);
static const char *_entrance_event_type_get(const void *data, Eina_Bool *unknow);

static Eet_Data_Descriptor *_entrance_event_auth_dd();
static Eet_Data_Descriptor *_entrance_event_status_dd();
static Eet_Data_Descriptor *_entrance_event_xsessions_dd();

static Eina_Bool
_entrance_event_type_set(const char *type, void *data, Eina_Bool unknow)
{
   Entrance_Event_Type *ev = data;
   if (unknow)
     return EINA_FALSE;
   if (!strcmp(type, ENTRANCE_EVENT_AUTH_NAME))
     *ev = ENTRANCE_EVENT_AUTH;
   else if (!strcmp(type, ENTRANCE_EVENT_MAXTRIES_NAME))
     *ev = ENTRANCE_EVENT_MAXTRIES;
   else if (!strcmp(type, ENTRANCE_EVENT_STATUS_NAME))
     *ev = ENTRANCE_EVENT_STATUS;
   else if (!strcmp(type, ENTRANCE_EVENT_XSESSIONS_NAME))
     *ev = ENTRANCE_EVENT_XSESSIONS;
   else if (!strcmp(type, ENTRANCE_EVENT_USERS_NAME))
     *ev = ENTRANCE_EVENT_USERS;
   else if (!strcmp(type, ENTRANCE_EVENT_ACTIONS_NAME))
     *ev = ENTRANCE_EVENT_ACTIONS;
   else if (!strcmp(type, ENTRANCE_EVENT_ACTION_NAME))
     *ev = ENTRANCE_EVENT_ACTION;
   else
     {
        printf("error on type set\n");
        *ev = ENTRANCE_EVENT_UNKNOWN;
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static const char *
_entrance_event_type_get(const void *data, Eina_Bool *unknow)
{
   const Entrance_Event_Type *ev = data;
   if (*ev == ENTRANCE_EVENT_AUTH)
     return ENTRANCE_EVENT_AUTH_NAME;
   else if (*ev == ENTRANCE_EVENT_MAXTRIES)
     return ENTRANCE_EVENT_MAXTRIES_NAME;
   else if (*ev == ENTRANCE_EVENT_STATUS)
     return ENTRANCE_EVENT_STATUS_NAME;
   else if (*ev == ENTRANCE_EVENT_XSESSIONS)
     return ENTRANCE_EVENT_XSESSIONS_NAME;
   else if (*ev == ENTRANCE_EVENT_USERS)
     return ENTRANCE_EVENT_USERS_NAME;
   else if (*ev == ENTRANCE_EVENT_ACTIONS)
     return ENTRANCE_EVENT_ACTIONS_NAME;
   else if (*ev == ENTRANCE_EVENT_ACTION)
     return ENTRANCE_EVENT_ACTION_NAME;
   if (*unknow)
     {
        printf("error on type get\n");
        *unknow = EINA_TRUE;
     }
   return NULL;
}

static Eet_Data_Descriptor *
_entrance_event_xsessions_dd()
{
   Eet_Data_Descriptor_Class eddc, eddcl;
   Eet_Data_Descriptor *edd, *eddl;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Xsession);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Xsession, "name",
                                 name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Xsession, "icon",
                                 icon, EET_T_STRING);

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddcl, Entrance_Xsessions_Event);
   eddl = eet_data_descriptor_stream_new(&eddcl);
   EET_DATA_DESCRIPTOR_ADD_LIST(eddl, Entrance_Xsessions_Event, "xsessions", xsessions, edd);
   return eddl;
}

static Eet_Data_Descriptor *
_entrance_event_auth_dd()
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Auth_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Auth_Event, "login",
                                 login, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Auth_Event, "password",
                                 password, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Auth_Event, "session",
                                 session, EET_T_STRING);
   return edd;

}

static Eet_Data_Descriptor *
_entrance_event_maxtries_dd()
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Maxtries_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Maxtries_Event, "maxtries",
                                 maxtries, EET_T_UCHAR);
   return edd;
}

static Eet_Data_Descriptor *
_entrance_event_status_dd()
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Status_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Status_Event, "granted",
                                 granted, EET_T_UCHAR);
   return edd;

}

static Eet_Data_Descriptor *
_entrance_event_users_dd()
{
   Eet_Data_Descriptor *edd, *eddl;
   Eet_Data_Descriptor_Class eddc, eddcl;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_User);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_User, "login",
                                 login, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_User, "image",
                                 image, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_User, "lsess",
                                 lsess, EET_T_STRING);
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddcl, Entrance_Users_Event);
   eddl = eet_data_descriptor_stream_new(&eddcl);
   EET_DATA_DESCRIPTOR_ADD_LIST(eddl, Entrance_Users_Event, "users", users, edd);
   return eddl;
}

static Eet_Data_Descriptor *
_entrance_event_actions_dd()
{
   Eet_Data_Descriptor *edd, *eddl;
   Eet_Data_Descriptor_Class eddc, eddcl;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Action);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Action, "label",
                                 label, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Action, "id",
                                 id, EET_T_INT);
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddcl, Entrance_Actions_Event);
   eddl = eet_data_descriptor_stream_new(&eddcl);
   EET_DATA_DESCRIPTOR_ADD_LIST(eddl, Entrance_Actions_Event, "actions", actions, edd);
   return eddl;
}

static Eet_Data_Descriptor *
_entrance_event_action_dd()
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Status_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Action_Event, "action",
                                 action, EET_T_INT);
   return edd;
}

static Eet_Data_Descriptor *
_entrance_event_new()
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *unified;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Event);
   edd = eet_data_descriptor_stream_new(&eddc);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _entrance_event_type_get;
   eddc.func.type_set = _entrance_event_type_set;
   unified = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_XSESSIONS_NAME, _entrance_event_xsessions_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_AUTH_NAME, _entrance_event_auth_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_MAXTRIES_NAME, _entrance_event_maxtries_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_STATUS_NAME, _entrance_event_status_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_USERS_NAME, _entrance_event_users_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_ACTIONS_NAME, _entrance_event_actions_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_ACTION_NAME, _entrance_event_action_dd());

   EET_DATA_DESCRIPTOR_ADD_UNION(edd, Entrance_Event, "event", event, type, unified);
   return edd;
}

void *
entrance_event_encode(Entrance_Event *eev, int *size)
{
   Eet_Data_Descriptor *edd;

   edd = _entrance_event_new();

   return eet_data_descriptor_encode(edd, eev, size);
}

Entrance_Event *
entrance_event_decode(void *data, int size)
{
   Eet_Data_Descriptor *edd;

   edd = _entrance_event_new();

   return eet_data_descriptor_decode(edd, data, size);
}

