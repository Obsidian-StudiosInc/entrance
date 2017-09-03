# project
# REMOVE ME: temp line for version commit 0.1.0_alpha1 remove for alpha2
project('entrance', 'c',
        version        : '0.1.0_alpha1',
        license        : 'GPL-3.0',
        default_options: [ 'c_std=gnu99' ],
        meson_version  : '>= 0.40.0')
base_url = 'https://github.com/Obsidian-StudiosInc/'

# global arguments
add_global_arguments('-W', language : 'c')

# global variables
cc       = meson.get_compiler('c')
proj     = meson.project_name()
ver      = meson.project_version()
chmod    = join_paths(meson.source_root(), 'meson-chmod.sh')
top_inc  = include_directories('.')

# dependencies
efl_version = '>= 1.19.0'
crypt   = cc.find_library('crypt')
deps	= dependency('elementary', version: efl_version)
ecore_x	= dependency('ecore-x',	   version: efl_version)
edje	= dependency('edje',       version: efl_version)
xcb	= dependency('xcb')

has_pam = get_option('pam')
has_ck  = get_option('consolekit')

if has_pam
        pam_dep = cc.find_library('pam')
        pam_inc  = include_directories('/usr/include/security')
endif

##### edje_cc binary compiler tool
edje_cmd = get_option('edje-cc')
if edje_cmd == ''
  edje_cmd = join_paths(edje.get_pkgconfig_variable('prefix'),
                        'bin', 'edje_cc')
endif

##### dir locations
dir_prefix = get_option('prefix')
dir_lib    = join_paths(dir_prefix, get_option('libdir'), proj)
dir_data   = join_paths(dir_prefix, get_option('datadir'))
dir_sbin   = join_paths(dir_prefix, get_option('sbindir'))
dir_syscfg = join_paths(dir_prefix, get_option('sysconfdir'))

# project args
add_project_arguments('-DHAVE_CONFIG_H=1', language : 'c')
if cc.has_header('alloca.h')
	add_project_arguments('-DHAVE_ALLOCA_H=1', language : 'c')
endif
if has_ck
	add_project_arguments('-DHAVE_CONSOLEKIT=1', language : 'c')
endif
if has_pam
	add_project_arguments('-DPAM=1', language : 'c')
endif
if cc.has_header('stdlib.h')
	add_project_arguments('-DHAVE_STDLIB_H=1', language : 'c')
endif

# config.h
cfg = configuration_data()
cfg.set_quoted('PACKAGE'		, proj)
cfg.set_quoted('PACKAGE_NAME'		, proj)
cfg.set_quoted('PACKAGE_STRING'		, proj + ' ' + ver)
cfg.set_quoted('PACKAGE_URL'		, base_url + proj)
cfg.set_quoted('PACKAGE_BIN_DIR'	, dir_lib)
cfg.set_quoted('PACKAGE_DATA_DIR'	, join_paths(dir_data, proj))
cfg.set_quoted('PACKAGE_LIB_DIR'	, dir_lib)
cfg.set_quoted('PACKAGE_SBIN_DIR'	, dir_sbin)
cfg.set_quoted('SYSTEM_CONFIG_DIR'	, dir_syscfg)
cfg.set_quoted('VERSION'		, ver)
configure_file(output: 'config.h', configuration: cfg)

configure_file(input  : 'data/entrance.conf.in',
               output : 'entrance.conf',
               install_dir : join_paths(dir_syscfg, proj),
               configuration : cfg
)
configure_file(input  : 'data/entrance.service.in',
               output : 'entrance.service',
               configuration : cfg
)

# install
if has_pam
	configure_file(input  : 'data/entrance.pam.d',
                       output : 'entrance',
                       install_dir : join_paths(dir_syscfg, 'pam.d'),
                       configuration : cfg
	)
endif

##### subdirs
subdir('src/bin')
subdir('src/daemon')
subdir('data/themes/default')
