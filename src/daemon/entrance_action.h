#ifndef Entrance_Action_
#define Entrance_Action_

void entrance_action_init(void);
void entrance_action_shutdown(void);
Eina_List *entrance_action_get(void);
void entrance_action_run(int action);

#endif
