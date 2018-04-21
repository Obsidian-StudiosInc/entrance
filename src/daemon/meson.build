inc = include_directories('.')

# entrance
entrance_srcs = [
	'../../src/event/entrance_event.h',	'../../src/event/entrance_event.c',
	'entrance_config.h',	'entrance_config.c',
	'entrance_session.h',	'entrance_session.c',
	'entrance_xserver.h',	'entrance_xserver.c',
	'entrance_server.h',	'entrance_server.c',
	'entrance_history.h',	'entrance_history.c',
	'entrance_action.h',	'entrance_action.c',
	'entrance_image.h',	'entrance_image.c',
	'entrance_theme.h',	'entrance_theme.c',
	'entrance.h',		'entrance.c'
]
entrance_pam_srcs = ['entrance_pam.h', 'entrance_pam.c']

if has_pam
	executable('entrance',
		sources			: [entrance_srcs, entrance_pam_srcs],
		include_directories	: [inc, pam_inc, top_inc],
		dependencies		: [crypt, deps, ecore_x, pam_dep, xcb],
		gui_app			: true,
		install			: true,
		install_dir             : dir_sbin
	)
else
	executable('entrance', 
		sources			: [entrance_srcs],
		include_directories	: [inc, top_inc],
		dependencies		: [crypt, deps, ecore_x, xcb],
		gui_app			: true,
		install			: true,
		install_dir             : dir_sbin
	)
endif
