inc = include_directories('.')
executable('entrance_client', [
		'../../src/event/entrance_event.h',	'../../src/event/entrance_event.c',
		'entrance_client.h',	'entrance_client.c',
		'entrance_conf.h',	'entrance_conf.c',
		'entrance_conf_main.h',	'entrance_conf_main.c',
		'entrance_conf_theme.h','entrance_conf_theme.c',
		'entrance_conf_user.h',	'entrance_conf_user.c',
		'entrance_connect.h',	'entrance_connect.c',
		'entrance_fill.h',	'entrance_fill.c',
		'entrance_login.h',	'entrance_login.c',
		'entrance_gui.h',	'entrance_gui.c'
	],
	include_directories	: [inc, top_inc],
	dependencies		: [deps, ecore_x],
	gui_app			: true,
	install			: true,
	install_dir             : dir_sbin
)
