#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "entrance.h"
#include "entrance_pam.h"

 /*
  *   ...
  *   pam_start(...);                Initializes the PAM library
  *   ...
  *   if ( ! pam_authenticate(...) ) Autenticates using modules
  *      error_exit();
  *   ...
  *   if ( ! pam-acct_mgmt(...) )    Checks for a valid, unexpired account and
  *                                   verifies access restrictions with "account" modules
  *       error_exit();
  *   ...
  *   pam_setcred(...)               Sets extra credentials, e.g. a Kerberos ticket
  *   ...
  *   pam_open_session(...);         Sets up the session with "session" modules
  *   do_stuff();
  *
  *   pam_close_session(...);        Tear-down session using the "session" modules
  *   pam_end(...);
  *   */
static int _entrance_pam_conv(int num_msg,
                              const struct pam_message **msg,
                              struct pam_response **resp,
                              void *appdata_ptr);

static struct pam_conv _pam_conversation;
static pam_handle_t* _pam_handle = NULL;
static int last_result;
static char *_login = NULL;
static char *_passwd = NULL;

static int
_entrance_pam_conv(int num_msg,
                   const struct pam_message **msg,
                   struct pam_response **resp,
                   void *appdata_ptr EINA_UNUSED)
{
   int i;
   *resp = (struct pam_response *) calloc(num_msg, sizeof(struct pam_response));
   for (i = 0; i < num_msg; i++)
     {
        resp[i]->resp_retcode=0;
        switch(msg[i]->msg_style)
          {
           case PAM_PROMPT_ECHO_ON:
              PT("echo on");
              resp[i]->resp = _login;
              break;
           case PAM_PROMPT_ECHO_OFF:
              PT("echo off");
              resp[i]->resp = _passwd;
              break;
           case PAM_ERROR_MSG:
              PT("error msg %s", msg[i]->msg);
              break;
           case PAM_TEXT_INFO:
              PT("info %s", msg[i]->msg);
              break;
           default:
              break;
          }
     }
   return PAM_SUCCESS;
}

int
entrance_pam_open_session(void)
{
   last_result = pam_setcred(_pam_handle, PAM_ESTABLISH_CRED);
   switch (last_result)
     {
      case PAM_CRED_ERR:
      case PAM_USER_UNKNOWN:
         PT("PAM user unknow");
         return 1;
      case PAM_AUTH_ERR:
      case PAM_PERM_DENIED:
         PT("PAM error on login password");
         return 1;
      default:
         PT("PAM open warning unknow error");
         return 1;
      case PAM_SUCCESS:
         break;
     }
   last_result = pam_open_session(_pam_handle, 0);
   switch(last_result)
     {
      default:
         //case PAM_SESSION_ERROR: ???
         pam_setcred(_pam_handle, PAM_DELETE_CRED);
         entrance_pam_end();
      case PAM_SUCCESS:
         break;
     }
   return 0;
}

void
entrance_pam_close_session(const Eina_Bool opened)
{
   PT("PAM close session");
   last_result = pam_close_session(_pam_handle, PAM_SILENT);
   switch (last_result)
     {
      default:
         //case PAM_SESSION_ERROR:
         PT("error on close session");
         pam_setcred(_pam_handle, PAM_DELETE_CRED);
         entrance_pam_end();
      case PAM_SUCCESS:
         break;
     }
   if (opened)
     {
        last_result = pam_setcred(_pam_handle, PAM_DELETE_CRED);
        switch(last_result)
          {
           default:
           case PAM_CRED_ERR:
           case PAM_CRED_UNAVAIL:
           case PAM_CRED_EXPIRED:
           case PAM_USER_UNKNOWN:
              entrance_pam_end();
           case PAM_SUCCESS:
              break;
          }
     }
}

int
entrance_pam_end(void)
{
   int result;
   result = pam_end(_pam_handle, last_result);
   _pam_handle = NULL;
   return result;
}

int
entrance_pam_authenticate(void)
{
   last_result = pam_authenticate(_pam_handle, 0);
   switch (last_result)
     {
      case PAM_ABORT:
      case PAM_AUTHINFO_UNAVAIL:
         PT("PAM error !");
         entrance_pam_end();
         return 1;
      case PAM_USER_UNKNOWN:
         PT("PAM user unknow error !");
         return 1;
      case PAM_MAXTRIES:
         PT("PAM max tries error !");
         entrance_server_client_wait();
         return 1;
      case PAM_CRED_INSUFFICIENT:
         PT("PAM don't have sufficient credential to authenticate !");
         return 1;
      case PAM_AUTH_ERR:
         PT("PAM authenticate error !");
         return 1;
      case PAM_PERM_DENIED:
         PT("PAM permission denied !");
         return 1;
      default:
         PT("PAM auth warning unknown error");
         return 1;
      case PAM_SUCCESS:
         break;
     }
   last_result=pam_acct_mgmt(_pam_handle, PAM_SILENT);
   switch(last_result)
     {
      default:
         //case PAM_NEW_AUTHTOKEN_REQD:
      case PAM_ACCT_EXPIRED:
         PT("PAM user acct expired error");
         entrance_pam_end();
         return 1;
      case PAM_USER_UNKNOWN:
         PT("PAM user unknow error");
         entrance_pam_end();
         return 1;
      case PAM_AUTH_ERR:
         PT("PAM auth error");
         return 1;
      case PAM_PERM_DENIED:
         PT("PAM perm_denied error");
         return 1;
      case PAM_SUCCESS:
         break;
     }

   return 0;
}

int
entrance_pam_init(const char *service, const char *display, const char *user)
{
   int status;

   if (!service || !*service) goto pam_error;
   if (!display || !*display) goto pam_error;

   _pam_conversation.conv = _entrance_pam_conv;
   _pam_conversation.appdata_ptr = NULL;

   if (_pam_handle) entrance_pam_end();
   status = pam_start(service, user, &_pam_conversation, &_pam_handle);
   if (status != 0) goto pam_error;
   status = entrance_pam_item_set(ENTRANCE_PAM_ITEM_TTY, display);
   if (status != 0) goto pam_error;
   status = entrance_pam_item_set(ENTRANCE_PAM_ITEM_RUSER, user);
   if (status != 0) goto pam_error;
   return 0;

pam_error:
   PT("PAM error !!!");
   return 1;
}

int
entrance_pam_item_set(ENTRANCE_PAM_ITEM_TYPE type, const void *value)
{
   last_result = pam_set_item(_pam_handle, type, value);
   if (last_result == PAM_SUCCESS) {
      return 0;
   }

   PT("PAM error: %d on %d", last_result, type);
   return 1;
}

const void *
entrance_pam_item_get(ENTRANCE_PAM_ITEM_TYPE type)
{
   const void *data;
   last_result = pam_get_item(_pam_handle, type, &data);
   switch (last_result) {
      default:
      case PAM_SYSTEM_ERR:
         entrance_pam_end();
         PT("error on pam item get");
      case PAM_PERM_DENIED: /* Here data was NULL */
      case PAM_SUCCESS:
         break;
   }
   return data;
}

int
entrance_pam_env_set(const char *env, const char *value)
{
   char buf[1024];
   if (!env || !value) return 1;
   snprintf(buf, sizeof(buf), "%s=%s", env, value);
   last_result = pam_putenv(_pam_handle, buf);
   switch (last_result)
     {
      default:
      case PAM_PERM_DENIED:
      case PAM_ABORT:
      case PAM_BUF_ERR:
         entrance_pam_end();
         return 1;
      case PAM_SUCCESS:
         break;
     }
   return 0;

}

char **
entrance_pam_env_list_get(void)
{
   return pam_getenvlist(_pam_handle);
}

void
entrance_pam_shutdown(void)
{
  if(_login)
    free(_login);
  if(_passwd)
    free(_passwd);
}

int
entrance_pam_auth_set(const char *login, const char *passwd)
{
   if (!login)
     return 1;
   _login = strdup(login);
   if (passwd)
     _passwd = strdup(passwd);
   return 0;
}

int
entrance_pam_passwd_set(const char *passwd)
{
   _passwd = strdup(passwd);
   if (!_passwd)
     return 1;
   return 0;
}

